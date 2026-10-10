using System.Text.Json.Serialization;
using SplitForge.Core.Steam;

namespace SplitForge.Core.Windows;

/// <summary>Ventana visible de nivel superior.</summary>
public readonly record struct WindowInfo(nint Handle, int Pid, string Title);

public sealed record GameWindow(nint Handle, int Pid, string Title, string ExeName, string ExePath);

public static class GameWindowStates
{
    public const string TwoClients = "DOS_CLIENTES_DETECTADOS";
    public const string Unpaired = "UNO_O_MAS_SIN_PAREJA";
    public const string None = "SIN_VENTANAS";
}

public sealed record GameWindowReport(
    string AppId,
    string Name,
    string BuildId,
    IReadOnlyList<GameWindow> Windows,
    IReadOnlyList<nint> ChosenHandles,
    int DistinctPids,
    string State,
    string Detail,
    IReadOnlyList<nint> ClientHandles)
{
    public const int MaxClients = 4;

    /// <summary>Seguro para compartir: SIN títulos de ventana, rutas ni handles.</summary>
    public GameWindowPublic ToPublic() => new(
        AppId, Name, BuildId, Windows.Count, DistinctPids,
        [.. Windows.Select(w => w.ExeName).Distinct(StringComparer.OrdinalIgnoreCase).Order(StringComparer.OrdinalIgnoreCase)],
        ChosenHandles.Count == 2, State, Detail);
}

public sealed record GameWindowPublic(
    [property: JsonPropertyName("appid")] string AppId,
    [property: JsonPropertyName("juego")] string Name,
    [property: JsonPropertyName("buildid")] string BuildId,
    [property: JsonPropertyName("ventanas")] int Windows,
    [property: JsonPropertyName("procesos_distintos")] int DistinctProcesses,
    [property: JsonPropertyName("ejecutables")] IReadOnlyList<string> Executables,
    [property: JsonPropertyName("pares_aptos_para_distribucion")] bool PairReady,
    [property: JsonPropertyName("estado")] string State,
    [property: JsonPropertyName("nota")] string Note)
{
    [JsonPropertyName("cooperativo_verificado")] public bool CoopVerified => false;
    [JsonPropertyName("mandos_aislados_verificados")] public bool InputIsolationVerified => false;
    [JsonPropertyName("offline_verificado")] public bool OfflineVerified => false;
}

/// <summary>Asociación conservadora de ventanas a una instalación. Solo inspección.</summary>
public static class GameWindowInspector
{
    /// <summary>Elige dos ventanas SOLO de procesos distintos que ejecutan EL MISMO .exe.</summary>
    public static GameWindowReport Inspect(InstalledGame game, IEnumerable<WindowInfo> visible, Func<int, string?> imageLookup)
    {
        var pathByPid = new Dictionary<int, string?>();
        var found = new List<GameWindow>();
        var seen = new HashSet<nint>();
        foreach (var w in visible)
        {
            if (w.Pid <= 0 || !seen.Add(w.Handle))
                continue;
            if (!pathByPid.TryGetValue(w.Pid, out var path))
            {
                path = SafeLookup(imageLookup, w.Pid);
                pathByPid[w.Pid] = path;
            }
            if (!WinPath.IsGameExecutable(path, game.InstallFolder))
                continue;
            found.Add(new GameWindow(w.Handle, w.Pid, w.Title, WinPath.FileName(path!), path!));
        }

        // Dos ventanas en un mismo proceso NO son dos clientes.
        var groups = new Dictionary<string, SortedDictionary<int, GameWindow>>(StringComparer.Ordinal);
        foreach (var win in found)
        {
            var key = WinPath.Normalize(win.ExePath);
            if (!groups.TryGetValue(key, out var byPid))
                groups[key] = byPid = [];
            byPid.TryAdd(win.Pid, win);
        }
        var winner = groups.Values
            .Where(g => g.Count >= 2)
            .OrderByDescending(g => g.Count)
            .ThenBy(g => g.Keys.First())
            .FirstOrDefault();

        IReadOnlyList<nint> chosen = [];
        IReadOnlyList<nint> clients = [];
        string state, detail;
        if (winner is not null)
        {
            // Una ventana por proceso, en orden de PID (normalmente el orden de apertura).
            clients = [.. winner.Values.Take(GameWindowReport.MaxClients).Select(w => w.Handle)];
            chosen = [.. clients.Take(2)];
            state = GameWindowStates.TwoClients;
            detail = "Dos procesos del mismo ejecutable detectados. Solo se puede distribuir sus ventanas; "
                   + "partida común y mandos siguen sin verificar.";
        }
        else if (found.Count > 0)
        {
            state = GameWindowStates.Unpaired;
            detail = "Hay ventanas de la instalación, pero no dos clientes diferentes del mismo ejecutable.";
        }
        else
        {
            state = GameWindowStates.None;
            detail = "No hay ventanas visibles cuyos procesos pertenezcan a esta instalación. "
                   + "Abre el juego normalmente desde Steam y vuelve a analizar.";
        }
        return new GameWindowReport(game.AppId, game.Name, game.BuildId, found, chosen,
            found.Select(w => w.Pid).Distinct().Count(), state, detail, clients);
    }

    /// <summary>Revalida PID + exe + HWND justo antes de mover ventanas para evitar una selección obsoleta.</summary>
    public static (nint First, nint Second) VerifyPair(InstalledGame game, GameWindowReport report,
        IEnumerable<WindowInfo> visible, Func<int, string?> imageLookup)
    {
        if (report.AppId != game.AppId || report.ChosenHandles.Count != 2)
            throw new InvalidOperationException("No hay dos ventanas elegibles para esta instalación.");
        var current = Inspect(game, visible, imageLookup);
        if (current.ChosenHandles.Count != 2 || !current.ChosenHandles.ToHashSet().SetEquals(report.ChosenHandles))
            throw new InvalidOperationException("Las ventanas cambiaron; pulsa Analizar ventanas otra vez.");
        return (report.ChosenHandles[0], report.ChosenHandles[1]);
    }

    private static string? SafeLookup(Func<int, string?> lookup, int pid)
    {
        try
        {
            return lookup(pid);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException
                                       or ArgumentException or System.ComponentModel.Win32Exception)
        {
            return null; // Procesos protegidos o ya cerrados.
        }
    }
}
