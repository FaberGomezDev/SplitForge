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

    /// <summary>Clientes y servidores de 7DTD abiertos, con las APIs de mando que cargan y su origen.</summary>
    public static SevenDaysDiagnosticReport SevenDays(IReadOnlyList<string> controllers)
    {
        var windowsByPid = new Win32WindowSystem().EnumerateWindows()
            .GroupBy(w => w.Pid).ToDictionary(g => g.Key, g => g.Count());
        var steamRoots = SteamLocator.FindRoots();
        var clients = ProcessInspector.FindByExecutable(SevenDaysToDieAdapter.GameExe)
            .Select(pid => Inspect(pid, SevenDaysToDieAdapter.GameExe, windowsByPid, steamRoots)).ToList();
        var servers = ProcessInspector.FindByExecutable(SevenDaysToDieAdapter.ServerExe)
            .Select(pid => Inspect(pid, SevenDaysToDieAdapter.ServerExe, windowsByPid, steamRoots)).ToList();
        return SevenDaysDiagnosticReport.Create(clients, servers, controllers) with { XInputSlots = XInputProbe.ConnectedSlots() };
    }

    /// <summary>Número de clientes de 7DTD con ventana visible (para el preflight).</summary>
    public static int RunningSevenDaysClients()
    {
        var pids = ProcessInspector.FindByExecutable(SevenDaysToDieAdapter.GameExe).ToHashSet();
        return new Win32WindowSystem().EnumerateWindows().Select(w => w.Pid).Where(pids.Contains).Distinct().Count();
    }

    /// <summary>
    /// Carpetas donde el filtro XInput deja su estado: la de SplitForge y, si existe, la del sandbox del
    /// jugador 2 en Sandboxie-Plus (allí se virtualizan las escrituras de los procesos del sandbox).
    /// </summary>
    public static IReadOnlyList<string> XInputStatusFolders(string dataFolder)
    {
        var folders = new List<string> { Path.Combine(dataFolder, "xinput") };
        var drive = Environment.GetEnvironmentVariable("SystemDrive") ?? "C:";
        if (XInputFilter.SandboxieStatusFolder(drive, Environment.UserName, SandboxieLocator.DefaultBox) is { } boxed && Directory.Exists(boxed))
            folders.Add(boxed);
        return folders;
    }

    /// <summary>Estado del filtro en los clientes de 7DTD abiertos (se descartan procesos cerrados).</summary>
    public static IReadOnlyList<XInputFilterStatus> XInputStatus(string dataFolder) =>
        XInputStatus(dataFolder, SevenDaysToDieAdapter.GameExe);

    /// <summary>Estado del filtro en los procesos abiertos de esos ejecutables (se descartan los ya cerrados).</summary>
    public static IReadOnlyList<XInputFilterStatus> XInputStatus(string dataFolder, params string[] gameExes) =>
        XInputFilter.ReadStatus(XInputStatusFolders(dataFolder), gameExes.SelectMany(ProcessInspector.FindByExecutable).ToHashSet());

    /// <summary>Procesos abiertos de un ejecutable, con la ruta de su imagen (para saber de qué instalación son).</summary>
    public static IReadOnlyList<(int Pid, string Image)> RunningImages(string exeName) =>
        [.. ProcessInspector.FindByExecutable(exeName).Select(pid => (pid, ProcessInspector.ImagePath(pid))).Where(p => p.Item2 is not null).Select(p => (p.pid, p.Item2!))];

    private static GameProcessDiagnostic Inspect(int pid, string exe, IReadOnlyDictionary<int, int> windowsByPid, IReadOnlyList<string> steamRoots)
    {
        var paths = ProcessInspector.ModulePaths(pid);
        var windows = windowsByPid.GetValueOrDefault(pid);
        if (paths is null)
            return new(exe, windows, false, null, "Windows denegó la lectura de módulos (¿EAC activo o proceso elevado?).");
        var windowsFolder = Environment.GetFolderPath(Environment.SpecialFolder.Windows);
        var gameFolder = ProcessInspector.ImagePath(pid) is { } image ? Path.GetDirectoryName(image) : null;
        var entries = paths.Select(p => new ModuleEntry(Path.GetFileName(p), ModuleOriginClassifier.Classify(p, windowsFolder, gameFolder, steamRoots))).ToList();
        // Copias del filtro: DLL de XInput cargadas desde la carpeta del juego y registradas por SplitForge con el mismo SHA-256.
        var filter = gameFolder is null
            ? []
            : entries.Where(e => e.Origin == ModuleOrigin.Juego
                                 && XInputFilter.KnownNames.Contains(e.Name, StringComparer.OrdinalIgnoreCase)
                                 && XInputFilter.IsOwnFile(gameFolder, e.Name))
                .Select(e => e.Name.ToLowerInvariant()).Distinct().Order(StringComparer.Ordinal).ToList();
        return new GameProcessDiagnostic(exe, windows, true, InputModuleClassifier.Classify(entries), null) { FilterLoaded = filter };
    }
}
