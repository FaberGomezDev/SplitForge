using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace SplitForge.Core.Adapters;

public sealed record XInputFilterStatus(int Pid, string Dll, bool RealLoaded, int Slot);

/// <summary>
/// Instala, asigna y retira el filtro XInput nativo (EXPERIMENTAL) en la carpeta de un juego.
/// Garantías: nunca sobrescribe un archivo existente del juego; solo borra lo que instaló (verificado
/// por SHA-256); la asignación PID → ranura vive en la carpeta de datos de SplitForge.
/// </summary>
public static class XInputFilter
{
    public const string DllFileName = "SplitForge.XInputFilter.dll";
    public const string ManifestName = "splitforge-xinput.json";
    public const string AssignmentFile = "asignacion.txt";
    public static readonly IReadOnlyList<string> KnownNames = ["xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll"];

    private sealed record Manifest(List<Entry> Files);

    private sealed record Entry(string Name, string Sha256);

    /// <summary>Copia el filtro con cada nombre indicado que NO exista ya en la carpeta del juego.</summary>
    public static IReadOnlyList<string> Install(string gameFolder, string filterDll, IEnumerable<string> names)
    {
        if (!File.Exists(filterDll))
            throw new InvalidOperationException("No se encontró el filtro XInput compilado (native\\" + DllFileName + ").");
        if (!Directory.Exists(gameFolder))
            throw new InvalidOperationException("La carpeta del juego no existe.");
        var hash = Sha256(filterDll);
        var manifest = ReadManifest(gameFolder);
        var installed = new List<string>();
        foreach (var name in names.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            if (!KnownNames.Contains(name, StringComparer.OrdinalIgnoreCase))
                throw new ArgumentException($"Nombre de DLL no permitido: {name}");
            var target = Path.Combine(gameFolder, name);
            if (File.Exists(target))
            {
                if (manifest.Files.Any(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase)))
                    continue; // Ya es nuestro.
                throw new InvalidOperationException($"El juego ya trae su propio {name}; SplitForge no lo sobrescribe.");
            }
            File.Copy(filterDll, target, overwrite: false);
            manifest.Files.Add(new Entry(name, hash));
            installed.Add(name);
        }
        WriteManifest(gameFolder, manifest);
        return installed;
    }

    /// <summary>Retira solo los archivos instalados por SplitForge cuyo contenido no haya cambiado.</summary>
    public static IReadOnlyList<string> Uninstall(string gameFolder)
    {
        var manifest = ReadManifest(gameFolder);
        var removed = new List<string>();
        foreach (var entry in manifest.Files.ToList())
        {
            var path = Path.Combine(gameFolder, entry.Name);
            if (!File.Exists(path))
            {
                manifest.Files.Remove(entry);
                continue;
            }
            if (!string.Equals(Sha256(path), entry.Sha256, StringComparison.OrdinalIgnoreCase))
                continue; // Alguien lo cambió: no se borra a ciegas.
            try
            {
                File.Delete(path);
            }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
            {
                throw new InvalidOperationException($"No se pudo retirar {entry.Name}: cierra el juego e inténtalo de nuevo.", ex);
            }
            manifest.Files.Remove(entry);
            removed.Add(entry.Name);
        }
        if (manifest.Files.Count == 0)
            File.Delete(Path.Combine(gameFolder, ManifestName));
        else
            WriteManifest(gameFolder, manifest);
        return removed;
    }

    public static IReadOnlyList<string> InstalledNames(string gameFolder) =>
        [.. ReadManifest(gameFolder).Files.Where(f => File.Exists(Path.Combine(gameFolder, f.Name))).Select(f => f.Name)];

    /// <summary>Escribe la asignación PID → ranura física (0–3) de forma atómica.</summary>
    public static void WriteAssignments(string folder, IReadOnlyList<(int Pid, int Slot)> assignments)
    {
        if (assignments.Any(a => a.Pid <= 0 || a.Slot is < 0 or > 3))
            throw new ArgumentException("Asignación inválida: PID positivo y ranura entre 1 y 4.");
        if (assignments.Select(a => a.Slot).Distinct().Count() != assignments.Count)
            throw new ArgumentException("Cada cliente necesita una ranura de mando distinta.");
        Directory.CreateDirectory(folder);
        var path = Path.Combine(folder, AssignmentFile);
        var text = new StringBuilder();
        foreach (var (pid, slot) in assignments)
            text.Append(pid.ToString(System.Globalization.CultureInfo.InvariantCulture)).Append(' ')
                .Append(slot.ToString(System.Globalization.CultureInfo.InvariantCulture)).Append('\n');
        File.WriteAllText(path + ".tmp", text.ToString(), Encoding.ASCII);
        File.Move(path + ".tmp", path, overwrite: true);
    }

    public static void ClearAssignments(string folder)
    {
        var path = Path.Combine(folder, AssignmentFile);
        if (File.Exists(path))
            File.Delete(path);
    }

    /// <summary>Estado que escribe el filtro en cada proceso que lo carga (estado-PID.txt).</summary>
    public static IReadOnlyList<XInputFilterStatus> ReadStatus(string folder, IReadOnlySet<int>? alivePids = null)
    {
        if (!Directory.Exists(folder))
            return [];
        var result = new List<XInputFilterStatus>();
        foreach (var file in Directory.EnumerateFiles(folder, "estado-*.txt"))
        {
            var name = Path.GetFileNameWithoutExtension(file);
            if (!int.TryParse(name.AsSpan("estado-".Length), out var pid) || (alivePids is not null && !alivePids.Contains(pid)))
                continue;
            try
            {
                var values = File.ReadAllLines(file)
                    .Select(l => l.Split('=', 2))
                    .Where(p => p.Length == 2)
                    .ToDictionary(p => p[0].Trim(), p => p[1].Trim());
                result.Add(new XInputFilterStatus(pid, values.GetValueOrDefault("dll", "?"),
                    values.GetValueOrDefault("real") == "cargada",
                    int.TryParse(values.GetValueOrDefault("ranura"), out var slot) ? slot : -1));
            }
            catch (IOException)
            {
            }
        }
        return [.. result.OrderBy(s => s.Pid)];
    }

    private static Manifest ReadManifest(string gameFolder)
    {
        var path = Path.Combine(gameFolder, ManifestName);
        try
        {
            if (File.Exists(path))
                return JsonSerializer.Deserialize<Manifest>(File.ReadAllText(path)) ?? new Manifest([]);
        }
        catch (JsonException)
        {
        }
        return new Manifest([]);
    }

    private static void WriteManifest(string gameFolder, Manifest manifest)
    {
        if (manifest.Files.Count == 0)
            return;
        File.WriteAllText(Path.Combine(gameFolder, ManifestName), JsonSerializer.Serialize(manifest, new JsonSerializerOptions { WriteIndented = true }));
    }

    private static string Sha256(string path)
    {
        using var stream = File.OpenRead(path);
        return Convert.ToHexString(SHA256.HashData(stream));
    }
}
