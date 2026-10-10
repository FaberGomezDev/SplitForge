using SplitForge.Core.Input;
using SplitForge.Core.Ipc;
using SplitForge.Core.Layout;

namespace SplitForge.Core.Sessions;

public enum SessionKind
{
    /// <summary>Laboratorio: dos visores propios del mismo mundo compartido.</summary>
    Lab,
    /// <summary>Ventanas externas existentes (juegos): solo se colocan y se restauran.</summary>
    Attached,
}

public sealed class ActiveSession(SessionKind kind, LayoutOptions layout)
{
    public SessionKind Kind { get; } = kind;
    public LayoutOptions Layout { get; set; } = layout;
    public List<IChildProcess> Processes { get; } = [];
    public List<LatestFrameWriter> Writers { get; } = [];
    public List<nint> WindowHandles { get; } = [];
    public Dictionary<nint, WindowSnapshot> Saved { get; } = [];
    public HashSet<int> ReportedExits { get; } = [];
    public long DeadlineMs { get; init; }
    public bool Pending { get; set; }
    public LabSession? Lab { get; init; }
}

/// <summary>
/// Ciclo de vida de sesiones con restauración de posiciones originales. Nunca fuerza el cierre de
/// juegos externos (pueden estar guardando); solo termina visores propios tras un cierre normal fallido.
/// </summary>
public sealed class SessionManager(IWindowSystem windows, IProcessLauncher launcher, Action<string> log,
    Func<long>? clockMs = null) : IDisposable
{
    public const string LabWindowTitlePrefix = "SplitForge MUNDO COMPARTIDO - Jugador ";
    public const int LabWindowTimeoutMs = 12_000;
    public const int CloseGraceMs = 2_000;

    private readonly Func<long> _clock = clockMs ?? (() => Environment.TickCount64);
    private readonly List<(long Deadline, List<IChildProcess> Processes)> _closing = [];

    // Lo lee el hilo de simulación del laboratorio; lo cambia la interfaz.
    private volatile ActiveSession? _active;

    public ActiveSession? Active
    {
        get => _active;
        private set => _active = value;
    }

    public static string LabWindowTitle(int player) => LabWindowTitlePrefix + player;

    public void StartLab(Func<int, LaunchSpec> viewerSpec, uint player1, uint player2, LayoutOptions layout)
    {
        RequireIdle();
        if (player1 == player2)
            throw new ArgumentException("Cada jugador necesita un mando distinto.");
        var writers = new List<LatestFrameWriter>();
        var processes = new List<IChildProcess>();
        try
        {
            for (var player = 1; player <= 2; player++)
            {
                var proc = launcher.Start(viewerSpec(player) with { RedirectInput = true });
                processes.Add(proc);
                writers.Add(new LatestFrameWriter(proc.Input ?? throw new InvalidOperationException("El visor no aceptó entrada."),
                    $"SplitForge.LabWriter.P{player}"));
                log($"Mundo compartido: visor P{player} PID {proc.Id}, mando ID {(player == 1 ? player1 : player2)}.");
            }
        }
        catch
        {
            // Solo procesos propios recién creados y sin partidas que guardar.
            foreach (var w in writers)
                w.Dispose();
            foreach (var p in processes)
                SafeKill(p);
            throw;
        }
        var session = new ActiveSession(SessionKind.Lab, layout)
        {
            DeadlineMs = _clock() + LabWindowTimeoutMs,
            Pending = true,
            Lab = new LabSession(player1, player2, writers),
        };
        session.Processes.AddRange(processes);
        session.Writers.AddRange(writers);
        Active = session;
        log("Simulación autoritativa local iniciada: un mundo, dos cámaras.");
    }

    /// <summary>Coloca 2–4 ventanas existentes. Sus procesos nunca serán cerrados por SplitForge.</summary>
    public void Attach(IReadOnlyList<nint> handles, LayoutOptions layout)
    {
        if (handles.Count is < 2 or > 4 || handles.Distinct().Count() != handles.Count)
            throw new ArgumentException("Selecciona entre dos y cuatro ventanas diferentes.");
        RequireIdle();
        var session = new ActiveSession(SessionKind.Attached, layout);
        Active = session;
        try
        {
            Place(session, handles);
        }
        catch
        {
            Stop();
            throw;
        }
        log($"{handles.Count} ventanas existentes organizadas. Sus procesos no serán cerrados por SplitForge.");
    }

    /// <summary>Reaplica la distribución con nuevas opciones sin perder las posiciones originales.</summary>
    public void Relayout(LayoutOptions layout)
    {
        var s = Active ?? throw new InvalidOperationException("No hay sesión activa.");
        s.Layout = layout;
        if (s.WindowHandles.Count >= 2)
            Place(s, [.. s.WindowHandles]);
    }

    /// <summary>Llamar periódicamente (≈2 Hz) desde la interfaz.</summary>
    public void Tick()
    {
        var now = _clock();
        for (var i = _closing.Count - 1; i >= 0; i--)
        {
            var (deadline, processes) = _closing[i];
            var alive = processes.Where(p => !p.HasExited).ToList();
            if (alive.Count == 0)
            {
                processes.ForEach(p => p.Dispose());
                _closing.RemoveAt(i);
            }
            else if (now >= deadline)
            {
                foreach (var proc in alive)
                {
                    // Exclusivamente visores creados por SplitForge.
                    SafeKill(proc);
                    log($"Visor {proc.Id} terminado tras agotar el cierre normal.");
                }
                processes.ForEach(p => p.Dispose());
                _closing.RemoveAt(i);
            }
        }

        var s = Active;
        if (s is null)
            return;
        foreach (var proc in s.Processes)
        {
            if (proc.HasExited && s.ReportedExits.Add(proc.Id))
                log($"Proceso {proc.Id} finalizó con código {proc.ExitCode}.");
        }
        if (!s.Pending || s.Kind != SessionKind.Lab)
            return;

        var visible = windows.EnumerateWindows();
        var handles = new List<nint>();
        for (var player = 1; player <= s.Processes.Count; player++)
        {
            var pid = s.Processes[player - 1].Id;
            var title = LabWindowTitle(player);
            var match = visible.FirstOrDefault(w => w.Pid == pid && w.Title == title);
            if (match.Handle != 0)
                handles.Add(match.Handle);
        }
        if (handles.Count == s.Processes.Count)
        {
            try
            {
                Place(s, handles);
            }
            catch (Exception ex) when (ex is InvalidOperationException or IOException or ArgumentException)
            {
                log($"No se pudieron acomodar las ventanas: {ex.Message}");
                s.Pending = false;
            }
        }
        else if (now >= s.DeadlineMs)
        {
            s.Pending = false;
            log($"No se encontraron las ventanas del laboratorio a tiempo ({handles.Count}/{s.Processes.Count}).");
        }
    }

    public void Stop()
    {
        var s = Active;
        if (s is null)
            return;
        Active = null;
        foreach (var saved in s.Saved.Values.Reverse())
        {
            try
            {
                windows.Restore(saved);
            }
            catch (Exception ex) when (ex is InvalidOperationException or IOException or ArgumentException)
            {
                log($"Aviso al restaurar una ventana: {ex.Message}");
            }
        }
        if (s.Kind == SessionKind.Lab)
        {
            foreach (var writer in s.Writers)
                writer.Dispose();
            var pids = s.Processes.Select(p => p.Id).ToHashSet();
            foreach (var w in windows.EnumerateWindows().Where(w => pids.Contains(w.Pid)))
                windows.RequestClose(w.Handle);
            _closing.Add((_clock() + CloseGraceMs, s.Processes));
            log("Laboratorio detenido; cerrando ventanas de SplitForge.");
        }
        else
        {
            log("Distribución restaurada. Los juegos externos permanecen abiertos para evitar perder partidas.");
        }
    }

    public void RouteLab(IReadOnlyDictionary<uint, PadState> states, double dt) => Active?.Lab?.Step(states, dt);

    private void Place(ActiveSession session, IReadOnlyList<nint> handles)
    {
        var regions = SplitLayout.Compute(windows.PrimaryWorkArea(), handles.Count, session.Layout.Orientation, session.Layout.Gutter);
        // Primero capturar TODAS las posiciones originales: si una colocación falla, Stop() las restaura.
        foreach (var hwnd in handles)
        {
            if (!session.Saved.ContainsKey(hwnd))
                session.Saved[hwnd] = windows.Capture(hwnd);
        }
        for (var i = 0; i < handles.Count; i++)
            windows.Place(handles[i], regions[i], session.Layout.Borderless);
        session.WindowHandles.Clear();
        session.WindowHandles.AddRange(handles);
        session.Pending = false;
        log($"Ventanas colocadas ({session.Layout.Orientation}, separación {session.Layout.Gutter} px"
            + (session.Layout.Borderless ? ", sin bordes)." : ")."));
    }

    private void RequireIdle()
    {
        if (Active is not null)
            throw new InvalidOperationException("Detén la sesión actual antes de iniciar otra.");
    }

    private static void SafeKill(IChildProcess process)
    {
        try
        {
            process.Kill();
        }
        catch (Exception ex) when (ex is InvalidOperationException or System.ComponentModel.Win32Exception or NotSupportedException)
        {
        }
    }

    public void Dispose() => Stop();
}
