using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace SplitForge.Core.Adapters;

/// <summary>Estado que publica cada copia del filtro cargada en un proceso (una por nombre de DLL).</summary>
/// <param name="Reads">Llamadas a XInputGetState/Ex atendidas (−1 si el filtro es anterior y no lo informa).</param>
/// <param name="FocusForced">El filtro simula el foco de la ventana (pantalla dividida con mando asignado).</param>
public sealed record XInputFilterStatus(int Pid, string Dll, bool RealLoaded, int Slot, long Reads = -1, bool FocusForced = false);

/// <summary>Resultado de instalar: qué se copió, qué se actualizó y qué se dejó intacto.</summary>
/// <param name="Updated">Copias antiguas de SplitForge (sin modificar) sustituidas por la versión actual.</param>
/// <param name="Skipped">Archivos que no son de SplitForge o que alguien cambió: nunca se tocan.</param>
public sealed record XInputInstallResult(
    IReadOnlyList<string> Installed,
    IReadOnlyList<string> Updated,
    IReadOnlyList<string> AlreadyCurrent,
    IReadOnlyList<string> Skipped);

/// <summary>
/// Instala, asigna y retira el filtro XInput nativo (EXPERIMENTAL) en la carpeta de un juego.
/// Garantías: nunca sobrescribe un archivo que no instaló SplitForge; solo borra o actualiza lo que
/// instaló (verificado por SHA-256); la asignación PID → ranura vive en la carpeta de datos de SplitForge.
/// </summary>
public static class XInputFilter
{
    public const string DllFileName = "SplitForge.XInputFilter.dll";
    public const string ManifestName = "splitforge-xinput.json";
    public const string AssignmentFile = "asignacion.txt";
    public static readonly IReadOnlyList<string> KnownNames = ["xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll"];

    private sealed record Manifest(List<Entry> Files);

    private sealed record Entry(string Name, string Sha256);

    /// <summary>
    /// Copia el filtro con cada nombre indicado. Si el nombre ya existe y es una copia antigua de SplitForge
    /// sin modificar, la actualiza; si no es de SplitForge (o alguien la cambió), la deja intacta.
    /// El manifiesto se guarda aunque falle una copia, para poder retirar siempre lo instalado.
    /// </summary>
    public static XInputInstallResult Install(string gameFolder, string filterDll, IEnumerable<string> names)
    {
        var requested = names.Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        foreach (var name in requested.Where(n => !KnownNames.Contains(n, StringComparer.OrdinalIgnoreCase)))
            throw new ArgumentException($"Nombre de DLL no permitido: {name}");
        if (!File.Exists(filterDll))
            throw new InvalidOperationException("No se encontró el filtro XInput compilado (native\\" + DllFileName + ").");
        if (!Directory.Exists(gameFolder))
            throw new InvalidOperationException("La carpeta del juego no existe.");
        var hash = Sha256(filterDll);
        var manifest = ReadManifest(gameFolder);
        List<string> installed = [], updated = [], current = [], skipped = [];
        try
        {
            foreach (var name in requested)
            {
                var target = Path.Combine(gameFolder, name);
                var entry = manifest.Files.FirstOrDefault(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
                if (!File.Exists(target))
                {
                    if (entry is not null)
                        manifest.Files.Remove(entry); // Entrada huérfana: el archivo ya no está.
                    File.Copy(filterDll, target, overwrite: false);
                    manifest.Files.Add(new Entry(name, hash));
                    installed.Add(name);
                    continue;
                }
                var existing = Sha256(target);
                if (entry is null || !(existing.Equals(entry.Sha256, StringComparison.OrdinalIgnoreCase)
                                       || existing.Equals(hash, StringComparison.OrdinalIgnoreCase)))
                {
                    skipped.Add(name); // Del juego o modificado por otra herramienta: no se toca.
                    continue;
                }
                if (existing.Equals(hash, StringComparison.OrdinalIgnoreCase))
                {
                    current.Add(name);
                }
                else
                {
                    try
                    {
                        File.Copy(filterDll, target, overwrite: true); // Solo una copia propia verificada por SHA-256.
                    }
                    catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                    {
                        throw new InvalidOperationException($"No se pudo actualizar {name}: cierra los clientes del juego e inténtalo de nuevo.", ex);
                    }
                    updated.Add(name);
                }
                manifest.Files.Remove(entry);
                manifest.Files.Add(new Entry(name, hash));
            }
        }
        finally
        {
            WriteManifest(gameFolder, manifest);
        }
        return new XInputInstallResult(installed, updated, current, skipped);
    }

    /// <summary>¿El archivo <paramref name="name"/> de la carpeta del juego es una copia del filtro instalada por SplitForge?</summary>
    public static bool IsOwnFile(string gameFolder, string name)
    {
        var entry = ReadManifest(gameFolder).Files.FirstOrDefault(f => f.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
        var path = Path.Combine(gameFolder, Path.GetFileName(name));
        try
        {
            return entry is not null && File.Exists(path) && Sha256(path).Equals(entry.Sha256, StringComparison.OrdinalIgnoreCase);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            return false;
        }
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

    /// <summary>
    /// Estado que escribe cada copia del filtro cargada: <c>estado-PID-dll.txt</c> (o <c>estado-PID.txt</c>
    /// en versiones anteriores). Con <paramref name="alivePids"/> se descartan procesos ya cerrados.
    /// </summary>
    public static IReadOnlyList<XInputFilterStatus> ReadStatus(string folder, IReadOnlySet<int>? alivePids = null) =>
        ReadStatus([folder], alivePids);

    /// <summary>Igual que <see cref="ReadStatus(string, IReadOnlySet{int}?)"/> pero combinando varias carpetas (p. ej. la del sandbox).</summary>
    public static IReadOnlyList<XInputFilterStatus> ReadStatus(IEnumerable<string> folders, IReadOnlySet<int>? alivePids = null)
    {
        var result = new Dictionary<(int, string), XInputFilterStatus>();
        foreach (var folder in folders.Where(Directory.Exists))
        {
            foreach (var file in Directory.EnumerateFiles(folder, "estado-*.txt"))
            {
                if (!TryParseStatusName(Path.GetFileNameWithoutExtension(file), out var pid)
                    || (alivePids is not null && !alivePids.Contains(pid)))
                    continue;
                try
                {
                    var values = File.ReadAllLines(file)
                        .Select(l => l.Split('=', 2))
                        .Where(p => p.Length == 2)
                        .GroupBy(p => p[0].Trim())
                        .ToDictionary(g => g.Key, g => g.Last()[1].Trim());
                    var status = new XInputFilterStatus(pid, values.GetValueOrDefault("dll", "?"),
                        values.GetValueOrDefault("real") == "cargada",
                        int.TryParse(values.GetValueOrDefault("ranura"), out var slot) ? slot : -1,
                        long.TryParse(values.GetValueOrDefault("lecturas"), out var reads) ? reads : -1,
                        values.GetValueOrDefault("foco") == "forzado");
                    result.TryAdd((pid, status.Dll.ToLowerInvariant()), status);
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                {
                    // El filtro lo está reescribiendo: se leerá en la siguiente actualización.
                }
            }
        }
        return [.. result.Values.OrderBy(s => s.Pid).ThenBy(s => s.Dll, StringComparer.OrdinalIgnoreCase)];
    }

    /// <summary>
    /// Carpeta donde queda el estado de un cliente abierto dentro de Sandboxie-Plus con la ruta por defecto
    /// (<c>%SystemDrive%\Sandbox\%USER%\caja</c>): allí se virtualizan las escrituras del filtro.
    /// </summary>
    public static string? SandboxieStatusFolder(string systemDrive, string userName, string box)
    {
        static bool Safe(string value) => value.Length > 0 && value != "." && value != ".."
                                          && value.IndexOfAny(['\\', '/', ':', '*', '?', '"', '<', '>', '|']) < 0;
        var drive = systemDrive.TrimEnd('\\');
        if (drive.Length != 2 || drive[1] != ':' || !char.IsAsciiLetter(drive[0]) || !Safe(userName) || !Safe(box))
            return null;
        return $@"{drive}\Sandbox\{userName}\{box}\user\current\AppData\Local\SplitForge\xinput";
    }

    private static bool TryParseStatusName(string name, out int pid)
    {
        pid = 0;
        if (!name.StartsWith("estado-", StringComparison.Ordinal))
            return false;
        var rest = name.AsSpan("estado-".Length);
        var dash = rest.IndexOf('-');
        return int.TryParse(dash >= 0 ? rest[..dash] : rest, System.Globalization.NumberStyles.None,
            System.Globalization.CultureInfo.InvariantCulture, out pid) && pid > 0;
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
