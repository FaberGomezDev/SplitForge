#:property PublishAot=false
#:property JsonSerializerIsReflectionEnabledByDefault=true

// Extrae la API plana que usa un juego con Steamworks.NET, para generar la del emulador.
// Uso: dotnet run tools/extraer_api_steamworksnet.cs -- <com.rlabrecque.steamworks.net.dll> tools/steamworksnet_api.json
//
// Solo lee metadatos: no ejecuta código del ensamblado. Guarda, por cada [DllImport] de steam_api64,
// el nombre de la entrada y la clase de ABI x64 de su retorno y parámetros; si el IntPtr devuelto se
// convierte en cadena (InteropHelp.PtrToStringUTF8); y las constantes de versión de interfaz.
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Text.Json;

if (args.Length != 2)
{
    Console.Error.WriteLine("Uso: dotnet run extraer_api_steamworksnet.cs -- <steamworks.net.dll> <salida.json>");
    return 2;
}

var asm = Assembly.LoadFrom(args[0]);
Type[] types;
try
{
    types = asm.GetTypes();
}
catch (ReflectionTypeLoadException e)
{
    // Mono admite uniones con campos de objeto que .NET rechaza; esos tipos no hacen falta aquí.
    types = [.. e.Types.Where(t => t is not null).Select(t => t!)];
}
var native = types.First(t => t.FullName == "Steamworks.NativeMethods");

// Clase de ABI x64 de Windows: int (RAX / RCX, RDX…), float, double, ptr, void o struct:N
// (estructuras de tamaño distinto de 1, 2, 4 u 8 bytes, que viajan por puntero oculto).
static string Kind(Type t)
{
    if (t == typeof(void)) return "void";
    if (t == typeof(float)) return "float";
    if (t == typeof(double)) return "double";
    if (t.IsByRef || t.IsPointer || t == typeof(IntPtr) || t == typeof(UIntPtr) || t.IsArray || !t.IsValueType) return "ptr";
    if (t.IsEnum || t.IsPrimitive) return "int";
    int size;
    try { size = Marshal.SizeOf(t); } catch (ArgumentException) { size = -1; }
    return size is 1 or 2 or 4 or 8 ? "int" : $"struct:{size}";
}

static string[] ParameterKinds(MethodInfo m)
{
    try { return [.. m.GetParameters().Select(p => Kind(p.ParameterType))]; }
    catch (TypeLoadException) { return ["?"]; }
}

var oneByte = new OpCode[0x100];
var twoByte = new OpCode[0x100];
foreach (var f in typeof(OpCodes).GetFields(BindingFlags.Public | BindingFlags.Static))
{
    var op = (OpCode)f.GetValue(null)!;
    var v = (ushort)op.Value;
    if (v < 0x100) oneByte[v] = op;
    else if ((v & 0xff00) == 0xfe00) twoByte[v & 0xff] = op;
}

IEnumerable<int> CallTokens(MethodBase m)
{
    var il = m.GetMethodBody()?.GetILAsByteArray();
    if (il is null) yield break;
    var i = 0;
    while (i < il.Length)
    {
        var op = il[i] == 0xFE ? twoByte[il[++i]] : oneByte[il[i]];
        i++;
        var size = op.OperandType switch
        {
            OperandType.InlineNone => 0,
            OperandType.ShortInlineBrTarget or OperandType.ShortInlineI or OperandType.ShortInlineVar => 1,
            OperandType.InlineVar => 2,
            OperandType.InlineI8 or OperandType.InlineR => 8,
            OperandType.InlineSwitch => 4 + 4 * BitConverter.ToInt32(il, i),
            _ => 4,
        };
        if (op == OpCodes.Call || op == OpCodes.Callvirt)
            yield return BitConverter.ToInt32(il, i);
        i += size;
    }
}

// Entradas cuyo IntPtr devuelto se convierte en cadena justo después de la llamada.
var returnsString = new HashSet<string>();
const BindingFlags All = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance | BindingFlags.DeclaredOnly;
foreach (var t in types)
{
    foreach (var m in t.GetMethods(All).Cast<MethodBase>().Concat(t.GetConstructors(All)))
    {
        string? last = null;
        foreach (var token in CallTokens(m))
        {
            MethodBase? target;
            try { target = m.Module.ResolveMethod(token); }
            catch (Exception e) when (e is ArgumentException or TypeLoadException or MissingMethodException) { last = null; continue; }
            if (target?.DeclaringType == native) last = target.Name;
            else if (target?.Name == "PtrToStringUTF8" && last is not null) { returnsString.Add(last); last = null; }
            else last = null;
        }
    }
}

var entries = native.GetMethods(BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic)
    .Select(m => (m, dll: m.GetCustomAttribute<DllImportAttribute>()))
    .Where(x => x.dll?.Value == "steam_api64")
    .Select(x => new
    {
        name = x.dll!.EntryPoint ?? x.m.Name,
        ret = Kind(x.m.ReturnType),
        @string = returnsString.Contains(x.m.Name),
        @params = ParameterKinds(x.m),
    })
    .OrderBy(e => e.name, StringComparer.Ordinal)
    .ToList();

var versions = types.Where(t => t.FullName == "Steamworks.Constants")
    .SelectMany(t => t.GetFields(BindingFlags.Public | BindingFlags.Static))
    .Where(f => f.IsLiteral && f.FieldType == typeof(string) && f.Name.Contains("INTERFACE_VERSION"))
    .OrderBy(f => f.Name, StringComparer.Ordinal)
    .ToDictionary(f => f.Name, f => (string)f.GetRawConstantValue()!);

var document = new Dictionary<string, object>
{
    ["origen"] = $"Steamworks.NET ({Path.GetFileName(args[0])}), extraído con tools/extraer_api_steamworksnet.cs",
    ["nota"] = "Solo nombres de entrada y clases de ABI x64 (int, float, double, ptr, void, struct:N). Sin código ni datos del juego.",
    ["versiones"] = versions,
    ["entradas"] = entries,
};
File.WriteAllText(args[1], JsonSerializer.Serialize(document, new JsonSerializerOptions
{
    WriteIndented = true,
    Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
}) + "\n");
Console.WriteLine($"{entries.Count} entradas de steam_api64 ({returnsString.Count} devuelven cadena), {versions.Count} versiones de interfaz.");
return 0;
