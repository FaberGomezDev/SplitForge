using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using Microsoft.Win32;
using SplitForge.Core.Adapters;
using SplitForge.Core.Sessions;
using SplitForge.Core.Steam;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

public sealed record CheckRow(PreflightCheck Check)
{
    public string Title => Check.Title;
    public string Detail => Check.Detail;

    public string Icon => Check.Status switch
    {
        CheckStatus.Ok => "✔",
        CheckStatus.Failed => "✖",
        CheckStatus.Warning => "▲",
        _ => "…",
    };

    public Brush Color => Check.Status switch
    {
        CheckStatus.Ok => Brushes.MediumSeaGreen,
        CheckStatus.Failed => Brushes.IndianRed,
        CheckStatus.Warning => Brushes.Orange,
        _ => Brushes.Goldenrod,
    };
}

public sealed partial class MainViewModel
{
    private readonly SevenDaysToDieAdapter _sevenDays = new();
    private bool _sevenSwap;

    public ObservableCollection<CheckRow> SevenChecks { get; } = [];
    public ICommand CheckSevenCommand { get; private set; } = null!;
    public ICommand StartServerCommand { get; private set; } = null!;
    public ICommand StopServerCommand { get; private set; } = null!;
    public ICommand OpenSecondSteamCommand { get; private set; } = null!;
    public ICommand TileSevenCommand { get; private set; } = null!;
    public ICommand SwapSevenCommand { get; private set; } = null!;
    public ICommand ExportSevenCommand { get; private set; } = null!;
    public ICommand InstallFilterCommand { get; private set; } = null!;
    public ICommand AssignFilterCommand { get; private set; } = null!;
    public ICommand RemoveFilterCommand { get; private set; } = null!;
    public ICommand DetectSlotCommand { get; private set; } = null!;

    public IReadOnlyList<int> XInputSlots { get; } = [1, 2, 3, 4];

    private int _slotPlayer1 = 1;
    public int SlotPlayer1 { get => _slotPlayer1; set => Set(ref _slotPlayer1, value); }

    private int _slotPlayer2 = 2;
    public int SlotPlayer2 { get => _slotPlayer2; set => Set(ref _slotPlayer2, value); }

    private string _filterStatus = "Filtro XInput no instalado.";
    public string FilterStatus { get => _filterStatus; set => Set(ref _filterStatus, value); }

    private string _xinputSlots = "Ranuras XInput: sin comprobar.";
    public string XInputSlotsStatus { get => _xinputSlots; set => Set(ref _xinputSlots, value); }

    private bool _detectingSlot;

    private string XInputFolder => Path.Combine(_dataFolder, "xinput");

    private string _sevenState = "Sin comprobar.";
    public string SevenState { get => _sevenState; set => Set(ref _sevenState, value); }

    private string _serverStatus = "Servidor detenido.";
    public string ServerStatus { get => _serverStatus; set => Set(ref _serverStatus, value); }

    private string _serverLog = "";
    public string ServerLog { get => _serverLog; set => Set(ref _serverLog, value); }

    private int _serverPort;
    public int ServerPort { get => _serverPort; set { if (Set(ref _serverPort, value)) Raise(nameof(JoinInstructions)); } }

    private int _serverMaxPlayers;
    public int ServerMaxPlayers { get => _serverMaxPlayers; set => Set(ref _serverMaxPlayers, value); }

    private bool _serverEac;
    public bool ServerEac { get => _serverEac; set { if (Set(ref _serverEac, value)) Raise(nameof(JoinInstructions)); } }

    private string _serverPassword;
    public string ServerPassword { get => _serverPassword; set => Set(ref _serverPassword, value); }

    public IReadOnlyList<int> PlayerCounts { get; } = [2, 3, 4];

    public string JoinInstructions =>
        $"Cada jugador: abre 7 Days to Die {(ServerEac ? "CON EAC" : "con «Launch game without EAC»")} → "
        + $"Unirse a una partida → Conectar a IP → 127.0.0.1, puerto {ServerPort}. Después pulsa «Dividir pantalla».";

    private void InitSevenDays()
    {
        CheckSevenCommand = new RelayCommand(() => _ = ScanSteamAsync());
        StartServerCommand = new RelayCommand(StartServer, () => _server.Phase is ServerPhase.Stopped or ServerPhase.Exited);
        StopServerCommand = new RelayCommand(() => _ = StopServerAsync(), () => _server.Phase is ServerPhase.Starting or ServerPhase.Ready);
        OpenSecondSteamCommand = new RelayCommand(OpenSecondSteam);
        TileSevenCommand = new RelayCommand(() => TileSeven(), () => _sessions.Active is null);
        SwapSevenCommand = new RelayCommand(SwapSeven);
        ExportSevenCommand = new RelayCommand(ExportSevenDiagnostic);
        InstallFilterCommand = new RelayCommand(InstallFilter);
        AssignFilterCommand = new RelayCommand(AssignFilter);
        RemoveFilterCommand = new RelayCommand(RemoveFilter);
        DetectSlotCommand = new RelayCommand(p => _ = DetectSlotAsync(p as string == "2" ? 2 : 1), _ => !_detectingSlot);
    }

    private InstalledGame? FindGame(string appId) => SteamGames.FirstOrDefault(r => r.AppId == appId)?.Game;

    private SevenDaysServerOptions ServerOptions() => new()
    {
        Port = ServerPort,
        MaxPlayers = ServerMaxPlayers,
        EacEnabled = ServerEac,
        Password = ServerPassword ?? string.Empty,
    };

    private void RefreshSevenDaysChecks()
    {
        var env = new AdapterEnvironment(
            [.. SteamGames.Select(r => r.Game)],
            _input.Latest.Devices.Count,
            WindowsDiagnostics.RunningSevenDaysClients(),
            SandboxieLocator.StartExecutable() is not null);
        var preflight = _sevenDays.Preflight(env);
        SevenChecks.Clear();
        foreach (var check in preflight.Checks)
            SevenChecks.Add(new CheckRow(check));
        RefreshFilterStatus();
        RefreshSteamEmuStatus();
        SevenState = preflight.State == CompatibilityState.Bloqueado
            ? "BLOQUEADO: falta 7 Days to Die."
            : $"Estado: {preflight.State.ToString().ToUpperInvariant()} (experimental). Nunca se marca compatible sin prueba física.";
    }

    private void StartServer() => Guard(() =>
    {
        var server = FindGame(ResearchProfiles.SevenDaysToDieDedicatedServer)
                     ?? throw new InvalidOperationException("Instala gratis «7 Days to Die Dedicated Server» desde Steam (Biblioteca → Herramientas) y pulsa Comprobar.");
        var exe = SevenDaysToDieAdapter.FindExecutable(server, SevenDaysToDieAdapter.ServerExe)
                  ?? throw new InvalidOperationException("No se encontró 7DaysToDieServer.exe en la carpeta del servidor.");
        var options = ServerOptions().Validate();
        _server.Start(exe, options, new SevenDaysServerPaths(Path.Combine(_dataFolder, "7dtd")));
        SaveSettings();
        ServerStatus = "Iniciando servidor local… (la primera vez puede tardar varios minutos en generar el mundo)";
    });

    private async Task StopServerAsync()
    {
        ServerStatus = "Apagando el servidor (guardando partida)…";
        await _server.StopAsync();
        ServerStatus = "Servidor detenido.";
    }

    private int _serverRefreshing;

    /// <summary>La comprobación del puerto puede tardar en Windows: se hace fuera del hilo de la interfaz.</summary>
    private void RefreshServer()
    {
        if (_server.Phase is ServerPhase.Stopped || Interlocked.Exchange(ref _serverRefreshing, 1) == 1)
        {
            ApplyServerState(_server.Phase, null);
            return;
        }
        Task.Run(() => (Phase: _server.Refresh(), Log: _server.TailLog()))
            .ContinueWith(t =>
            {
                Interlocked.Exchange(ref _serverRefreshing, 0);
                if (t.IsCompletedSuccessfully)
                    ApplyServerState(t.Result.Phase, t.Result.Log);
            }, TaskScheduler.FromCurrentSynchronizationContext());
    }

    private void ApplyServerState(ServerPhase phase, IReadOnlyList<string>? log)
    {
        ServerStatus = phase switch
        {
            ServerPhase.Starting => $"Iniciando (PID {_server.ProcessId})… espera a «LISTO».",
            ServerPhase.Ready => $"LISTO · 127.0.0.1:{_server.Options.Port} · hasta {_server.Options.MaxPlayers} jugadores · EAC {(_server.Options.EacEnabled ? "activo" : "desactivado")}",
            ServerPhase.Stopping => "Apagando el servidor (guardando partida)…",
            ServerPhase.Exited => "El servidor terminó inesperadamente. Revisa el registro.",
            _ => "Servidor detenido.",
        };
        if (log is not null && phase is ServerPhase.Starting or ServerPhase.Ready or ServerPhase.Exited)
            ServerLog = string.Join('\n', log);
    }

    private void OpenSecondSteam() => Guard(() =>
    {
        var start = SandboxieLocator.StartExecutable()
                    ?? throw new InvalidOperationException("No se encontró Sandboxie-Plus. Instálalo (gratuito) o usa otro usuario de Windows para el Steam del jugador 2.");
        var steam = SteamLocator.SteamExecutable() ?? throw new InvalidOperationException("No se encontró steam.exe.");
        if (!Confirm("Steam del jugador 2", "Se abrirá un segundo Steam dentro de Sandboxie-Plus para el jugador 2.\n\n"
                     + "SplitForge no ve ni guarda credenciales."))
            return;
        new SystemProcessLauncher().Start(SandboxieLocator.SteamInBox(start, steam)).Dispose();
        _log.Write($"Steam del jugador 2 abierto en el sandbox {SandboxieLocator.DefaultBox}.");
    });

    private void TileSeven() => Guard(() =>
    {
        var game = FindGame(ResearchProfiles.SevenDaysToDie) ?? throw new InvalidOperationException("7 Days to Die no aparece instalado.");
        var report = WindowsDiagnostics.InspectGame(game);
        if (report.ClientHandles.Count < 2)
            throw new InvalidOperationException($"Se necesitan al menos dos clientes de 7 Days to Die abiertos (hay {report.DistinctPids}).");
        IReadOnlyList<nint> handles = _sevenSwap ? [.. report.ClientHandles.Reverse()] : report.ClientHandles;
        _sessions.Attach(handles, Layout() with { MinAspect = SevenDaysToDieAdapter.MinAspect });
        foreach (var (pad, index) in Pads.Take(handles.Count).Select((p, i) => (p, i)))
            _input.IdentifyPlayer(pad.DeviceId, index);
        Status = $"7 Days to Die: {handles.Count} clientes en pantalla dividida. Aislamiento de mandos en el juego: pendiente.";
    });

    private void SwapSeven()
    {
        _sevenSwap = !_sevenSwap;
        if (_sessions.Active?.Kind == SessionKind.Attached)
        {
            _sessions.Stop();
            TileSeven();
        }
        _log.Write("Orden de jugadores de 7DTD invertido.");
    }

    /// <summary>Carpeta de instalación validada (dentro de steamapps/common) de 7DTD.</summary>
    private string SevenDaysFolder()
    {
        var game = FindGame(ResearchProfiles.SevenDaysToDie) ?? throw new InvalidOperationException("7 Days to Die no aparece instalado.");
        var (folder, inside) = PreflightEvaluator.SafeInstallFolder(game);
        if (!inside || !Directory.Exists(folder))
            throw new InvalidOperationException("La carpeta de 7 Days to Die no es válida.");
        return folder;
    }

    private void InstallFilter() => Guard(() =>
    {
        var folder = SevenDaysFolder();
        var report = WindowsDiagnostics.SevenDays([]);
        if (report.Clients.Any(c => c.Input?.AntiCheatLoaded == true || !c.ModulesReadable))
            throw new InvalidOperationException("Hay un cliente de 7DTD con EAC activo. Ciérralo y ábrelo con «Launch game without EAC» antes de instalar el filtro.");
        var dll = Path.Combine(AppContext.BaseDirectory, "native", XInputFilter.DllFileName);
        if (!Confirm("Filtro XInput (experimental)",
                "Se copiará el filtro de SplitForge en la carpeta de 7 Days to Die con los nombres de XInput que use "
                + "(7DTD carga xinput1_3 y xinput1_4). No se sobrescribe ningún archivo del juego: solo se actualizan copias "
                + "anteriores del propio filtro, y se puede retirar en cualquier momento.\n\n"
                + "Úsalo SOLO con los clientes sin EAC y con los clientes cerrados (se cargará al abrirlos).\n\n¿Instalar?"))
            return;
        var result = XInputFilter.Install(folder, dll, XInputFilter.KnownNames);
        var parts = new List<string>();
        if (result.Installed.Count > 0)
            parts.Add("instalado: " + string.Join(", ", result.Installed));
        if (result.Updated.Count > 0)
            parts.Add("actualizado: " + string.Join(", ", result.Updated));
        if (result.AlreadyCurrent.Count > 0)
            parts.Add("ya al día: " + string.Join(", ", result.AlreadyCurrent));
        _log.Write($"Filtro XInput en 7DTD → {string.Join(" · ", parts)}. Reinicia los clientes para que lo carguen.");
        if (result.Skipped.Count > 0)
        {
            var message = $"El juego ya trae {string.Join(", ", result.Skipped)} (o lo cambió otra herramienta): SplitForge no lo toca. "
                          + "El filtro no podrá actuar sobre esa DLL; si el juego lee los mandos por ella, el aislamiento no funcionará.";
            _log.Write(message);
            MessageBox.Show(message, "Filtro XInput", MessageBoxButton.OK, MessageBoxImage.Information);
        }
        RefreshFilterStatus();
    });

    /// <summary>Pide pulsar un botón en el mando del jugador y elige su ranura XInput (lectura en segundo plano).</summary>
    private async Task DetectSlotAsync(int player)
    {
        _detectingSlot = true;
        CommandManager.InvalidateRequerySuggested();
        XInputSlotsStatus = $"Pulsa cualquier botón en el mando del J{player} (10 s)…";
        try
        {
            var slot = await Task.Run(() => XInputProbe.WaitForPress(TimeSpan.FromSeconds(10)));
            if (slot is null)
            {
                XInputSlotsStatus = "No se detectó ninguna pulsación en un mando XInput. ¿Llegan los mandos como XInput (p. ej. «Xbox 360 Controller»)?";
                return;
            }
            var chosen = slot.Value + 1;
            if (player == 1)
            {
                if (SlotPlayer2 == chosen)
                    SlotPlayer2 = SlotPlayer1; // Intercambio explícito: nunca dos jugadores con la misma ranura.
                SlotPlayer1 = chosen;
            }
            else
            {
                if (SlotPlayer1 == chosen)
                    SlotPlayer1 = SlotPlayer2;
                SlotPlayer2 = chosen;
            }
            XInputSlotsStatus = $"J{player} → mando XInput {chosen}. Pulsa «Asignar mandos a los clientes» para aplicarlo.";
            _log.Write($"Ranura XInput detectada para J{player}: {chosen}.");
        }
        finally
        {
            _detectingSlot = false;
            CommandManager.InvalidateRequerySuggested();
        }
    }

    private void AssignFilter() => Guard(() =>
    {
        var game = FindGame(ResearchProfiles.SevenDaysToDie) ?? throw new InvalidOperationException("7 Days to Die no aparece instalado.");
        var report = WindowsDiagnostics.InspectGame(game);
        if (report.ClientHandles.Count < 2)
            throw new InvalidOperationException($"Se necesitan al menos dos clientes abiertos (hay {report.DistinctPids}).");
        var pidByHandle = report.Windows.ToDictionary(w => w.Handle, w => w.Pid);
        IReadOnlyList<nint> ordered = _sevenSwap ? [.. report.ClientHandles.Reverse()] : report.ClientHandles;
        var wanted = new List<int> { SlotPlayer1 - 1, SlotPlayer2 - 1 };
        wanted.AddRange(Enumerable.Range(0, 4).Where(s => !wanted.Contains(s)));
        var assignments = ordered.Select((h, i) => (pidByHandle[h], wanted[i])).ToList();
        XInputFilter.WriteAssignments(XInputFolder, assignments);
        _log.Write("Asignación XInput: " + string.Join(" · ", assignments.Select((a, i) => $"J{i + 1} → mando {a.Item2 + 1}")) + ".");
        RefreshFilterStatus();
    });

    private void RemoveFilter() => Guard(() =>
    {
        var removed = XInputFilter.Uninstall(SevenDaysFolder());
        XInputFilter.ClearAssignments(XInputFolder);
        _log.Write($"Filtro XInput retirado ({removed.Count} archivo(s)).");
        RefreshFilterStatus();
    });

    private int _filterRefreshing;
    private static readonly IReadOnlySet<int> NoPids = new HashSet<int>();

    /// <summary>Clientes cuyo filtro ya simula el foco dentro del juego (se lee cada ≈2 s, fuera del hilo de la interfaz).</summary>
    private IReadOnlySet<int> _focusedInGame = NoPids;

    /// <summary>
    /// Estado del filtro y de las ranuras XInput. Enumerar procesos y leer archivos se hace fuera del hilo
    /// de la interfaz; el resultado se publica de vuelta en él.
    /// </summary>
    private void RefreshFilterStatus()
    {
        if (Interlocked.Exchange(ref _filterRefreshing, 1) == 1)
            return;
        var dataFolder = _dataFolder;
        string? gameFolder;
        try
        {
            gameFolder = SevenDaysFolder();
        }
        catch (InvalidOperationException)
        {
            gameFolder = null;
        }
        // El resultado se deja en un campo y lo publica el tick de la interfaz (ApplyFilterStatus). Antes se
        // publicaba con una continuación atada al contexto de sincronización: si la primera llamada llegaba
        // sin contexto (al construir la ventana), lanzaba una excepción con la marca «en curso» puesta y el
        // estado no volvía a actualizarse en toda la sesión.
        Task.Run(() =>
        {
            try
            {
                Volatile.Write(ref _filterSnapshot, DescribeFilter(gameFolder, dataFolder));
            }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException
                                           or System.ComponentModel.Win32Exception)
            {
                // Se reintenta en la siguiente actualización (≈2 s).
            }
            finally
            {
                Interlocked.Exchange(ref _filterRefreshing, 0);
            }
        });
    }

    private sealed record FilterSnapshot(string Filter, string Slots, IReadOnlySet<int> FocusedInGame);

    private FilterSnapshot? _filterSnapshot;

    /// <summary>Llamado desde el tick de la interfaz (≈2 Hz): publica el último estado del filtro leído en segundo plano.</summary>
    private void ApplyFilterStatus()
    {
        if (Interlocked.Exchange(ref _filterSnapshot, null) is not { } snapshot)
            return;
        FilterStatus = snapshot.Filter;
        _focusedInGame = snapshot.FocusedInGame;
        if (!_detectingSlot)
            XInputSlotsStatus = snapshot.Slots;
    }

    private static FilterSnapshot DescribeFilter(string? gameFolder, string dataFolder)
    {
        var connected = XInputProbe.ConnectedSlots();
        var slots = connected.Count > 0
            ? $"Ranuras XInput con mando: {string.Join(", ", connected)}. Usa «Detectar» para saber cuál es cada mando."
            : "Ninguna ranura XInput con mando: los mandos deben llegar a Windows como XInput (p. ej. «Xbox 360 Controller»).";
        // El foco simulado dentro del juego se mira en todos los juegos del lanzador, no solo en 7DTD.
        var all = WindowsDiagnostics.XInputStatus(dataFolder, SevenDaysToDieAdapter.GameExe, NoMansSkyAdapter.GameExe);
        var focused = XInputFilterStatus.FocusedInGame(all);
        if (gameFolder is null)
            return new("7 Days to Die no detectado.", slots, focused);
        var installed = XInputFilter.InstalledNames(gameFolder);
        if (installed.Count == 0)
            return new("Filtro XInput no instalado.", slots, focused);
        var status = WindowsDiagnostics.XInputStatus(dataFolder);
        if (status.Count == 0)
            return new($"Instalado ({string.Join(", ", installed)}). Ningún cliente abierto lo ha cargado aún (abre o reinicia los clientes).", slots, focused);
        var perClient = status.GroupBy(st => st.Pid).Select(g =>
        {
            var slot = g.Select(st => st.Slot).FirstOrDefault(v => v >= 0, -1);
            var dlls = string.Join(", ", g.Select(st =>
                $"{st.Dll}{(st.RealLoaded ? "" : " (¡DLL real NO cargada!)")}{(st.Reads >= 0 ? $" {st.Reads} lecturas" : "")}"
                + (st.InterceptionText.Length > 0 ? $" [{st.InterceptionText}]" : "")));
            var focus = g.Any(st => st.FocusForced) ? " · foco simulado" : "";
            var pointer = g.Any(st => st.OwnCursor) ? " · puntero propio" : "";
            var keys = g.Any(st => st.KeysReleased >= 0) ? " · teclas sin pegarse" : "";
            return $"PID {g.Key}: mando {(slot >= 0 ? (slot + 1).ToString(System.Globalization.CultureInfo.InvariantCulture) : "sin asignar")} · {dlls}{focus}{pointer}{keys}";
        });
        return new($"Instalado ({string.Join(", ", installed)}).\n" + string.Join("\n", perClient), slots, focused);
    }

    private void ExportSevenDiagnostic() => Guard(() =>
    {
        var controllers = _input.Latest.Devices.Select(d => d.GamepadType.Length > 0 ? $"{d.Name} ({d.GamepadType})" : d.Name).ToList();
        var report = WindowsDiagnostics.SevenDays(controllers);
        var dialog = new SaveFileDialog { Title = "Guardar diagnóstico de 7 Days to Die", FileName = "SplitForge_Reporte_7DTD.json", Filter = "JSON|*.json" };
        if (dialog.ShowDialog() != true)
            return;
        File.WriteAllText(dialog.FileName, report.ToJson());
        var families = report.Clients.Where(c => c.Input is not null).SelectMany(c => c.Input!.Families.Keys).Distinct().ToList();
        var filtered = report.Clients.Count(c => c.FilterLoaded.Count > 0);
        MessageBox.Show($"Clientes analizados: {report.ClientCount}\nAPIs de entrada: {(families.Count > 0 ? string.Join(", ", families) : "ninguna detectada")}\n"
                        + $"Clientes con el filtro XInput cargado: {filtered}\n"
                        + $"Ranuras XInput con mando: {(report.XInputSlots is { Count: > 0 } slots ? string.Join(", ", slots) : "ninguna")}\n\n"
                        + "El informe no incluye rutas, cuentas ni títulos de ventana.", "Diagnóstico guardado");
    });
}
