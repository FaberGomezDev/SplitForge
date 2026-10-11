using System.Runtime.Versioning;
using Microsoft.Win32;
using SplitForge.Core.Sessions;

namespace SplitForge.Platform;

/// <summary>Ubicaciones de Steam según el registro del usuario y las rutas estándar. Solo lectura.</summary>
[SupportedOSPlatform("windows")]
public static class SteamLocator
{
    public static IReadOnlyList<string> FindRoots()
    {
        var candidates = new List<string>();
        foreach (var key in new[] { @"Software\Valve\Steam", @"Software\WOW6432Node\Valve\Steam" })
        {
            using var k = Registry.CurrentUser.OpenSubKey(key);
            if (k?.GetValue("SteamPath") is string path && path.Length > 0)
                candidates.Add(Path.GetFullPath(path));
        }
        foreach (var variable in new[] { "ProgramFiles(x86)", "ProgramFiles" })
        {
            var root = Environment.GetEnvironmentVariable(variable);
            if (!string.IsNullOrEmpty(root))
                candidates.Add(Path.Combine(root, "Steam"));
        }
        return [.. candidates.Where(Directory.Exists).Distinct(StringComparer.OrdinalIgnoreCase)];
    }

    public static string? SteamExecutable() =>
        FindRoots().Select(r => Path.Combine(r, "steam.exe")).FirstOrDefault(File.Exists);
}

/// <summary>
/// Juegos instalados con GOG Galaxy, según el registro (<c>HKLM\SOFTWARE\WOW6432Node\GOG.com\Games</c>). Solo
/// lectura. SUPUESTO sin comprobar en este equipo (no tiene GOG): cada juego guarda ahí su carpeta («path») y
/// su ejecutable («exe»).
/// </summary>
[SupportedOSPlatform("windows")]
public static class GogLocator
{
    /// <summary>Carpeta de instalación del juego de GOG cuyo ejecutable se llama <paramref name="exeName"/>, o null.</summary>
    public static string? FindInstall(string exeName)
    {
        foreach (var root in new[] { @"SOFTWARE\WOW6432Node\GOG.com\Games", @"SOFTWARE\GOG.com\Games" })
        {
            try
            {
                using var games = Registry.LocalMachine.OpenSubKey(root);
                if (games is null)
                    continue;
                foreach (var id in games.GetSubKeyNames())
                {
                    using var game = games.OpenSubKey(id);
                    if (game?.GetValue("exe") is string exe && game.GetValue("path") is string path
                        && string.Equals(Path.GetFileName(exe), exeName, StringComparison.OrdinalIgnoreCase)
                        && Directory.Exists(path))
                        return Path.GetFullPath(path);
                }
            }
            catch (Exception ex) when (ex is System.Security.SecurityException or UnauthorizedAccessException or IOException)
            {
                // Sin permiso de lectura: como si no hubiera GOG.
            }
        }
        return null;
    }
}

/// <summary>
/// Sandboxie-Plus permite abrir un segundo Steam en la misma sesión de
/// Windows. SplitForge solo lo detecta y lanza Steam dentro del sandbox; el inicio de sesión lo hace el jugador.
/// </summary>
[SupportedOSPlatform("windows")]
public static class SandboxieLocator
{
    public const string DefaultBox = "SplitForge_J2";

    public static string? StartExecutable()
    {
        foreach (var variable in new[] { "ProgramFiles", "ProgramW6432" })
        {
            var root = Environment.GetEnvironmentVariable(variable);
            if (string.IsNullOrEmpty(root))
                continue;
            foreach (var folder in new[] { "Sandboxie-Plus", "Sandboxie" })
            {
                var start = Path.Combine(root, folder, "Start.exe");
                if (File.Exists(start))
                    return start;
            }
        }
        return null;
    }

    public static LaunchSpec SteamInBox(string startExe, string steamExe, string box = DefaultBox) =>
        new(startExe, [$"/box:{box}", steamExe]);
}

/// <summary>Espera precisa con temporizador de alta resolución (Windows 10 1803+), sin girar la CPU.</summary>
[SupportedOSPlatform("windows")]
public static class HighResolutionDelay
{
    [ThreadStatic]
    private static nint _timer;

    public static void Sleep(TimeSpan duration)
    {
        if (duration <= TimeSpan.Zero)
            return;
        if (_timer == 0)
        {
            _timer = Native.CreateWaitableTimerEx(0, 0, Native.CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, Native.TIMER_ALL_ACCESS);
            if (_timer == 0)
            {
                Thread.Sleep(duration); // Windows antiguo: resolución estándar.
                return;
            }
        }
        var due = -duration.Ticks; // Negativo = relativo, en unidades de 100 ns.
        if (Native.SetWaitableTimer(_timer, in due, 0, 0, 0, 0) == 0)
        {
            Thread.Sleep(duration);
            return;
        }
        Native.WaitForSingleObject(_timer, Native.INFINITE);
    }
}

[SupportedOSPlatform("windows")]
public static class Dpi
{
    /// <summary>DPI por monitor (v2) para que las coordenadas sean píxeles físicos reales.</summary>
    public static void EnablePerMonitorV2() => Native.SetProcessDpiAwarenessContext(-4);
}
