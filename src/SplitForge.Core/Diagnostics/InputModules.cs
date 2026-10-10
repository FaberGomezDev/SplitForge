using System.Text.Json.Serialization;

namespace SplitForge.Core.Diagnostics;

/// <summary>De dónde viene un módulo cargado. Se exporta solo la categoría, nunca la ruta.</summary>
public enum ModuleOrigin
{
    /// <summary>Carpeta de Windows (System32, SysWOW64, WinSxS...).</summary>
    Sistema,
    /// <summary>Carpeta del juego (junto al .exe o en sus subcarpetas).</summary>
    Juego,
    /// <summary>Instalación de Steam (por ejemplo, la superposición).</summary>
    Steam,
    Otro,
}

/// <summary>Un módulo cargado: nombre de archivo y categoría de origen.</summary>
public sealed record ModuleEntry(string Name, ModuleOrigin Origin);

/// <summary>Clasifica la ruta de un módulo en una categoría sin exponer la ruta.</summary>
public static class ModuleOriginClassifier
{
    public static ModuleOrigin Classify(string modulePath, string windowsFolder, string? gameFolder, IEnumerable<string> steamRoots)
    {
        if (IsInside(modulePath, windowsFolder))
            return ModuleOrigin.Sistema;
        if (gameFolder is not null && IsInside(modulePath, gameFolder))
            return ModuleOrigin.Juego;
        return steamRoots.Any(root => IsInside(modulePath, root)) ? ModuleOrigin.Steam : ModuleOrigin.Otro;
    }

    /// <summary>Comparación de rutas de Windows sin distinguir mayúsculas y con cualquier separador.</summary>
    public static bool IsInside(string path, string folder)
    {
        if (string.IsNullOrWhiteSpace(path) || string.IsNullOrWhiteSpace(folder))
            return false;
        static string Normalize(string p) => p.Replace('/', '\\').TrimEnd('\\');
        var root = Normalize(folder) + "\\";
        return Normalize(path).StartsWith(root, StringComparison.OrdinalIgnoreCase);
    }
}

/// <summary>
/// Clasifica los módulos cargados por un juego para saber cómo lee los mandos (XInput, DirectInput,
/// HID, Windows.Gaming.Input...). Solo recibe NOMBRES de módulo (y su categoría de origen), nunca rutas.
/// </summary>
public static class InputModuleClassifier
{
    public static readonly IReadOnlyDictionary<string, string[]> Families = new Dictionary<string, string[]>
    {
        ["xinput"] = ["xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll", "xinputuap.dll"],
        ["directinput"] = ["dinput8.dll", "dinput.dll"],
        ["hid"] = ["hid.dll"],
        ["windows_gaming_input"] = ["windows.gaming.input.dll"],
        ["gameinput"] = ["gameinput.dll"],
        ["sdl"] = ["sdl2.dll", "sdl3.dll"],
        ["incontrol"] = ["incontrolnative.dll"],
        ["steam_api"] = ["steam_api64.dll", "steam_api.dll"],
        ["steam_overlay_input"] = ["gameoverlayrenderer64.dll", "gameoverlayrenderer.dll"],
        ["easy_anti_cheat"] = ["easyanticheat_x64.dll", "easyanticheat.dll", "eosovh-win64-shipping.dll"],
    };

    private static readonly string[] Keywords = ["input", "xinput", "hid", "pad", "joy", "rewired", "incontrol", "steam", "anticheat", "eos"];

    public static InputModuleSummary Classify(IEnumerable<string> moduleNames) =>
        Classify(moduleNames.Select(n => new ModuleEntry(n, ModuleOrigin.Otro)), includeOrigins: false);

    public static InputModuleSummary Classify(IEnumerable<ModuleEntry> modules, bool includeOrigins = true)
    {
        var entries = modules
            .Select(m => m with { Name = Path.GetFileName(m.Name.Replace('\\', '/')).ToLowerInvariant() })
            .Where(m => m.Name.Length > 0)
            .ToList();
        var lowered = entries.Select(m => m.Name).Distinct().Order(StringComparer.Ordinal).ToList();
        var families = new SortedDictionary<string, IReadOnlyList<string>>(StringComparer.Ordinal);
        foreach (var (family, names) in Families)
        {
            var hits = lowered.Where(names.Contains).ToList();
            if (hits.Count > 0)
                families[family] = hits;
        }
        var relevant = lowered.Where(n => Keywords.Any(n.Contains)).ToList();
        SortedDictionary<string, IReadOnlyList<string>>? origins = null;
        if (includeOrigins)
        {
            // Solo para los módulos de mando: basta para saber si una DLL viene del sistema o de la carpeta del juego.
            var inputNames = families.Where(f => f.Key is not ("steam_api" or "easy_anti_cheat")).SelectMany(f => f.Value).ToHashSet();
            origins = new SortedDictionary<string, IReadOnlyList<string>>(StringComparer.Ordinal);
            foreach (var group in entries.Where(m => inputNames.Contains(m.Name)).GroupBy(m => m.Name))
                origins[group.Key] = [.. group.Select(m => OriginLabel(m.Origin)).Distinct().Order(StringComparer.Ordinal)];
        }
        var engine = lowered.Contains("unityplayer.dll") ? "unity" : null;
        return new InputModuleSummary(families, relevant, lowered.Count) { Origins = origins, Engine = engine };
    }

    public static string OriginLabel(ModuleOrigin origin) => origin switch
    {
        ModuleOrigin.Sistema => "sistema",
        ModuleOrigin.Juego => "juego",
        ModuleOrigin.Steam => "steam",
        _ => "otro",
    };
}

public sealed record InputModuleSummary(
    [property: JsonPropertyName("familias")] IReadOnlyDictionary<string, IReadOnlyList<string>> Families,
    [property: JsonPropertyName("modulos_relevantes")] IReadOnlyList<string> Relevant,
    [property: JsonPropertyName("total_modulos")] int Total)
{
    /// <summary>Origen de cada módulo de mando: "sistema", "juego", "steam" u "otro" (puede haber dos con el mismo nombre).</summary>
    [JsonPropertyName("origenes")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyDictionary<string, IReadOnlyList<string>>? Origins { get; init; }

    /// <summary>Motor detectado por sus módulos ("unity") o null.</summary>
    [JsonPropertyName("motor")] public string? Engine { get; init; }

    /// <summary>Windows.Gaming.Input solo entrega mandos a la ventana con foco.</summary>
    [JsonIgnore] public bool RequiresFocus => Families.ContainsKey("windows_gaming_input");
    [JsonIgnore] public bool AntiCheatLoaded => Families.ContainsKey("easy_anti_cheat");
    [JsonIgnore] public bool LoadsXInput => Families.ContainsKey("xinput");
}

public sealed record GameProcessDiagnostic(
    [property: JsonPropertyName("ejecutable")] string Executable,
    [property: JsonPropertyName("ventanas_visibles")] int VisibleWindows,
    [property: JsonPropertyName("modulos_legibles")] bool ModulesReadable,
    [property: JsonPropertyName("entrada")] InputModuleSummary? Input,
    [property: JsonPropertyName("nota")] string? Note)
{
    /// <summary>Copias del filtro XInput de SplitForge cargadas desde la carpeta del juego (verificadas por SHA-256).</summary>
    [JsonPropertyName("filtro_xinput_cargado")] public IReadOnlyList<string> FilterLoaded { get; init; } = [];
}

public sealed record SevenDaysDiagnosticReport(
    [property: JsonPropertyName("schema")] string Schema,
    [property: JsonPropertyName("clientes_7dtd")] int ClientCount,
    [property: JsonPropertyName("servidores_7dtd")] int ServerCount,
    [property: JsonPropertyName("clientes")] IReadOnlyList<GameProcessDiagnostic> Clients,
    [property: JsonPropertyName("servidores")] IReadOnlyList<GameProcessDiagnostic> Servers,
    [property: JsonPropertyName("mandos_sdl3")] IReadOnlyList<string> Controllers,
    [property: JsonPropertyName("aviso")] string Notice)
{
    public const string SchemaName = "SplitForge 7DTD Diagnostico v3";

    /// <summary>Ranuras XInput (1–4) con mando conectado según Windows, o null si no se consultó.</summary>
    [JsonPropertyName("ranuras_xinput")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public IReadOnlyList<int>? XInputSlots { get; init; }

    public static SevenDaysDiagnosticReport Create(IReadOnlyList<GameProcessDiagnostic> clients,
        IReadOnlyList<GameProcessDiagnostic> servers, IReadOnlyList<string> controllers) =>
        new(SchemaName, clients.Count, servers.Count, clients, servers, controllers,
            "Solo lectura. Sin rutas, títulos de ventana, cuentas ni identificadores únicos de mandos.");

    public string ToJson() => System.Text.Json.JsonSerializer.Serialize(this, Steam.ReportJson.Options);
}
