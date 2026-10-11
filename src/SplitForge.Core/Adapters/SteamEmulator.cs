using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using SplitForge.Core.Sessions;

namespace SplitForge.Core.Adapters;

public sealed record SteamEmuStatus(bool Installed, string? Persona, string? SteamId, string? StorageDir, string? OriginalSha256);

/// <summary>Resultado de instalar: primera instalación, DLL propia sustituida por la nueva o ya al día.</summary>
public enum SteamEmuInstallResult
{
    Installed,
    Updated,
    UpToDate,
}

/// <summary>
/// Emulador de Steam (componente principal, EXPERIMENTAL, ADR-016): instala en la carpeta del juego una DLL que
/// reimplementa el contrato ABI de <c>steam_api64.dll</c> con identidad local por JSON, y la retira
/// devolviendo la original. Garantías: copia de seguridad + SHA-256 de lo instalado; nunca borra un
/// archivo que no sea el que SplitForge puso; configuración y estado solo en los archivos propios.
/// </summary>
public static class SteamEmulator
{
    public const string EmuDllName = "steam_api64.dll";
    public const string BackupName = "steam_api64_splitforge_original.dll";
    public const string ConfigName = "SplitForge.SteamEmu.json";
    public const string StateName = "SplitForge.SteamEmu.estado.txt";
    public const string ConfigEnvVar = "SPLITFORGE_STEAMEMU_CONFIG";

    private const string WarningText =
        "Experimental: el juego no hablará con Steam y usará identidades locales. Requiere el juego sin EAC. "
        + "Restaura antes de verificar archivos en Steam.";

    private sealed record ConfigModel(int app_id, string steam_id, string persona_name, string language,
        string ip_country, string storage_dir, int net_port, int net_discovery_seconds, string net_multicast_address,
        bool auto_join);

    /// <summary>
    /// JSON de configuración de una identidad. Los números de puerto compartidos permiten el P2P local y que las
    /// instancias se descubran. Con <paramref name="autoJoin"/>, esta instancia entra sola en la partida que abra otra.
    /// </summary>
    public static string BuildConfig(int appId, string steamId, string persona, string storageDir, bool autoJoin = false)
    {
        ValidateIdentity(steamId, persona, storageDir);
        var model = new ConfigModel(appId, steamId, persona, "spanish", "ES", storageDir, 27117, 2, "239.255.69.69", autoJoin);
        return JsonSerializer.Serialize(model, new JsonSerializerOptions { WriteIndented = true });
    }

    public static string DefaultStorageDir(string dataFolder, int appId, int player) =>
        Path.Combine(dataFolder, "steamemu", $"{appId}-J{player}");

    /// <summary>
    /// Instala el emulador: respalda la DLL original, copia la emulada y escribe config y estado (SHA-256).
    /// Si ya está instalado, actualiza la DLL solo cuando la del juego sigue siendo la copia que registró
    /// SplitForge; si alguien la cambió (p. ej. «Verificar archivos» de Steam), no toca nada.
    /// </summary>
    public static SteamEmuInstallResult Install(string gameFolder, string emulatorDll, string configJson)
    {
        if (!Directory.Exists(gameFolder))
            throw new InvalidOperationException("La carpeta del juego no existe.");
        if (!File.Exists(emulatorDll))
            throw new InvalidOperationException("No se encontró la DLL emulada (steam_api64.dll de native\\SplitForge.SteamEmu).");
        var target = FindDll(gameFolder, EmuDllName)
                     ?? throw new InvalidOperationException("El juego no tiene steam_api64.dll en su carpeta (ni en sus subcarpetas de plugins); no se puede respaldar.");
        if (string.IsNullOrWhiteSpace(configJson))
            throw new ArgumentException("Falta la configuración JSON del emulador.");
        try
        {
            using (JsonDocument.Parse(configJson))
            {
            }
        }
        catch (JsonException ex)
        {
            throw new ArgumentException("Configuración JSON inválida.", ex);
        }

        var folder = Path.GetDirectoryName(target)!;
        var backup = Path.Combine(folder, BackupName);
        var state = Path.Combine(folder, StateName);
        var result = SteamEmuInstallResult.Installed;
        try
        {
            if (File.Exists(backup))
            {
                var installed = Sha256(target);
                if (!File.Exists(state) || !string.Equals(installed, File.ReadAllText(state).Trim(), StringComparison.OrdinalIgnoreCase))
                    throw new InvalidOperationException("La DLL instalada no coincide con la registrada por SplitForge; no se actualiza para no pisar cambios ajenos.");
                result = string.Equals(installed, Sha256(emulatorDll), StringComparison.OrdinalIgnoreCase)
                    ? SteamEmuInstallResult.UpToDate
                    : SteamEmuInstallResult.Updated;
            }
            else
            {
                File.Copy(target, backup, overwrite: false);
            }
            if (result != SteamEmuInstallResult.UpToDate)
                File.Copy(emulatorDll, target, overwrite: true);
        }
        catch (IOException ex)
        {
            throw new InvalidOperationException("No se pudo escribir la DLL del juego: cierra el juego e inténtalo de nuevo.", ex);
        }
        File.WriteAllText(Path.Combine(folder, ConfigName), configJson, new UTF8Encoding(false));
        File.WriteAllText(state, Sha256(target), Encoding.ASCII);
        return result;
    }

    /// <summary>Restaura la DLL original solo si la instalada coincide con la registrada por SplitForge.</summary>
    public static void Restore(string gameFolder)
    {
        if (!Directory.Exists(gameFolder))
            throw new InvalidOperationException("La carpeta del juego no existe.");
        var backup = FindDll(gameFolder, BackupName);
        if (backup is null)
            throw new InvalidOperationException("No hay copia de seguridad: el emulador no está instalado (o ya se restauró).");
        var folder = Path.GetDirectoryName(backup)!;
        var target = Path.Combine(folder, EmuDllName);
        var state = Path.Combine(folder, StateName);
        if (File.Exists(state))
        {
            var registered = File.ReadAllText(state).Trim();
            if (!string.Equals(Sha256(target), registered, StringComparison.OrdinalIgnoreCase))
                throw new InvalidOperationException("La DLL instalada no coincide con la registrada por SplitForge; no se restaura para no pisar cambios ajenos.");
        }
        try
        {
            File.Copy(backup, target, overwrite: true);
            File.Delete(backup);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            throw new InvalidOperationException("No se pudo restaurar: cierra el juego e inténtalo de nuevo.", ex);
        }
        File.Delete(Path.Combine(folder, ConfigName));
        File.Delete(state);
    }

    public static bool IsInstalled(string gameFolder) =>
        Directory.Exists(gameFolder) && FindDll(gameFolder, BackupName) is not null;

    /// <summary>Estado actual: instalado o no, identidad configurada y dónde guarda el juego.</summary>
    public static SteamEmuStatus ReadStatus(string gameFolder)
    {
        var backup = Directory.Exists(gameFolder) ? FindDll(gameFolder, BackupName) : null;
        if (backup is null)
            return new SteamEmuStatus(false, null, null, null, null);
        var folder = Path.GetDirectoryName(backup)!;
        string? persona = null, steamId = null, storageDir = null;
        try
        {
            var configPath = Path.Combine(folder, ConfigName);
            if (File.Exists(configPath))
            {
                using var doc = JsonDocument.Parse(File.ReadAllText(configPath));
                var root = doc.RootElement;
                persona = root.TryGetProperty("persona_name", out var p) ? p.GetString() : null;
                steamId = root.TryGetProperty("steam_id", out var s) ? s.GetString() : null;
                storageDir = root.TryGetProperty("storage_dir", out var d) ? d.GetString() : null;
            }
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException)
        {
        }
        string? original = null;
        try
        {
            original = Sha256(backup);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
        }
        return new SteamEmuStatus(true, persona, steamId, storageDir, original);
    }

    /// <summary>Busca la DLL emulada compilada: paquete publicado, carpeta native y salida de CMake de desarrollo.</summary>
    public static string? FindEmulatorDll(string baseDirectory)
    {
        var candidates = new[]
        {
            Path.Combine(baseDirectory, "native", "SplitForge.SteamEmu", EmuDllName),
            Path.Combine(baseDirectory, "native", EmuDllName),
            Path.GetFullPath(Path.Combine(baseDirectory, "..", "..", "..", "..", "..", "native", "build-steamemu", "Release", EmuDllName)),
        };
        return candidates.FirstOrDefault(File.Exists);
    }

    /// <summary>Especificación de lanzamiento de una instancia del juego con la identidad del JSON indicado.</summary>
    public static LaunchSpec InstanceLaunch(string gameExe, string configPath, IReadOnlyList<string>? arguments = null) => new(
        gameExe,
        arguments ?? [],
        Path.GetDirectoryName(gameExe) ?? "",
        EnvironmentVariables: new Dictionary<string, string> { [ConfigEnvVar] = configPath });

    public static string Warning() => WarningText;

    /// <summary>
    /// Busca un archivo dentro de la carpeta del juego: primero en la raíz, luego en los patrones
    /// típicos de Unity (<c>*_Data\Plugins\x86_64\</c> y <c>*_Data\Plugins\</c>) y al final por
    /// niveles (hasta 5) para juegos con otra estructura. Devuelve la primera coincidencia.
    /// </summary>
    public static string? FindDll(string gameFolder, string name)
    {
        var root = Path.Combine(gameFolder, name);
        if (File.Exists(root))
            return root;
        try
        {
            foreach (var data in Directory.GetDirectories(gameFolder, "*_Data"))
            {
                foreach (var plugins in new[]
                         {
                             Path.Combine(data, "Plugins", "x86_64"),
                             Path.Combine(data, "Plugins"),
                         })
                {
                    var candidate = Path.Combine(plugins, name);
                    if (File.Exists(candidate))
                        return candidate;
                }
            }
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
        }
        var queue = new Queue<string>();
        queue.Enqueue(gameFolder);
        for (var depth = 0; depth < 5 && queue.Count > 0; depth++)
        {
            var count = queue.Count;
            for (var i = 0; i < count; i++)
            {
                var dir = queue.Dequeue();
                try
                {
                    var direct = Directory.GetFiles(dir, name);
                    if (direct.Length > 0)
                        return direct[0];
                    foreach (var sub in Directory.GetDirectories(dir))
                        queue.Enqueue(sub);
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                {
                }
            }
        }
        return null;
    }

    private static void ValidateIdentity(string steamId, string persona, string storageDir)
    {
        if (!ulong.TryParse(steamId, System.Globalization.NumberStyles.None, System.Globalization.CultureInfo.InvariantCulture, out var id) || id == 0)
            throw new ArgumentException("Steam ID inválido: entero de 64 bits mayor que cero.");
        if (string.IsNullOrWhiteSpace(persona) || persona.Length > 128)
            throw new ArgumentException("Nombre de persona inválido.");
        if (string.IsNullOrWhiteSpace(storageDir))
            throw new ArgumentException("Falta la carpeta de guardados.");
    }

    private static string Sha256(string path)
    {
        using var stream = File.OpenRead(path);
        return Convert.ToHexString(SHA256.HashData(stream));
    }
}
