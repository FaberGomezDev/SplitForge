using System.Runtime.Versioning;
using SplitForge.Core.Adapters;
using SplitForge.Core.Diagnostics;
using SplitForge.Core.Steam;
using SplitForge.Core.Windows;

namespace SplitForge.Platform;

/// <summary>Diagnósticos de solo lectura compartidos por la aplicación y la herramienta de consola.</summary>
[SupportedOSPlatform("windows")]
public static class WindowsDiagnostics
{
    public static IReadOnlyList<InstalledGame> ScanSteam() => SteamLibrary.Scan(SteamLocator.FindRoots());

    public static GameWindowReport InspectGame(InstalledGame game, IReadOnlyList<WindowInfo>? visible = null) =>
        GameWindowInspector.Inspect(game, visible ?? new Win32WindowSystem().EnumerateWindows(), ProcessInspector.ImagePath);

    public static IReadOnlyList<GameWindowReport> InspectGames(IEnumerable<InstalledGame> games)
    {
        var visible = new Win32WindowSystem().EnumerateWindows();
        return [.. games.Select(g => InspectGame(g, visible))];
    }

    public static (nint, nint) VerifyPair(InstalledGame game, GameWindowReport report) =>
        GameWindowInspector.VerifyPair(game, report, new Win32WindowSystem().EnumerateWindows(), ProcessInspector.ImagePath);

    /// <summary>Clientes y servidores de 7DTD abiertos, con las APIs de mando que cargan.</summary>
    public static SevenDaysDiagnosticReport SevenDays(IReadOnlyList<string> controllers)
    {
        var windowsByPid = new Win32WindowSystem().EnumerateWindows()
            .GroupBy(w => w.Pid).ToDictionary(g => g.Key, g => g.Count());
        var clients = ProcessInspector.FindByExecutable(SevenDaysToDieAdapter.GameExe)
            .Select(pid => Inspect(pid, SevenDaysToDieAdapter.GameExe, windowsByPid)).ToList();
        var servers = ProcessInspector.FindByExecutable(SevenDaysToDieAdapter.ServerExe)
            .Select(pid => Inspect(pid, SevenDaysToDieAdapter.ServerExe, windowsByPid)).ToList();
        return SevenDaysDiagnosticReport.Create(clients, servers, controllers);
    }

    /// <summary>Número de clientes de 7DTD con ventana visible (para el preflight).</summary>
    public static int RunningSevenDaysClients()
    {
        var pids = ProcessInspector.FindByExecutable(SevenDaysToDieAdapter.GameExe).ToHashSet();
        return new Win32WindowSystem().EnumerateWindows().Select(w => w.Pid).Where(pids.Contains).Distinct().Count();
    }

    private static GameProcessDiagnostic Inspect(int pid, string exe, IReadOnlyDictionary<int, int> windowsByPid)
    {
        var modules = ProcessInspector.ModuleNames(pid);
        var windows = windowsByPid.GetValueOrDefault(pid);
        return modules is null
            ? new(exe, windows, false, null, "Windows denegó la lectura de módulos (¿EAC activo o proceso elevado?).")
            : new(exe, windows, true, InputModuleClassifier.Classify(modules), null);
    }
}
