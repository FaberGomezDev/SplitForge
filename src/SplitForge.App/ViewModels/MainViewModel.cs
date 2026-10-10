using System.Collections.ObjectModel;
using System.Runtime.Versioning;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using SplitForge.Core.Adapters;
using SplitForge.Core.Diagnostics;
using SplitForge.Core.Layout;
using SplitForge.Core.Sessions;
using SplitForge.Core.Settings;
using SplitForge.Input;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

/// <summary>
/// Orquesta servicios y estado de la interfaz. La entrada (1 kHz) y la simulación del laboratorio
/// (120 Hz) viven en hilos propios: este temporizador de interfaz solo REFLEJA su estado.
/// </summary>
[SupportedOSPlatform("windows")]
public sealed partial class MainViewModel : Observable, IDisposable
{
    public const string Version = "0.7.0";
    private const int MaxLogLines = 400;

    private readonly string _dataFolder;
    private readonly SessionLog _log;
    private readonly SdlInputService _input;
    private readonly Win32WindowSystem _windows = new();
    private readonly SessionManager _sessions;
    private readonly SevenDaysServer _server;
    private readonly DispatcherTimer _timer;
    private AppSettings _settings;
    private FixedRateLoop? _labLoop;
    private long _lastSequence = -1;
    private int _tickCount;

    public MainViewModel()
    {
        _dataFolder = SettingsStore.DataFolder();
        _settings = SettingsStore.Load(_dataFolder);
        _log = new SessionLog(Path.Combine(_dataFolder, "splitforge-net.log"));
        _log.Written += line => Application.Current?.Dispatcher.BeginInvoke(() => AppendLog(line));
        _input = new SdlInputService(_settings.InputMode, _settings.Mappings, _settings.InputRateHz, _log.Write);
        var recovery = Path.Combine(_dataFolder, SessionRecovery.FileName);
        SessionRecovery.RecoverAfterCrash(recovery, _windows, _log.Write);
        _sessions = new SessionManager(_windows, new SystemProcessLauncher(), _log.Write, recoveryFile: recovery);
        _server = new SevenDaysServer(new SystemProcessLauncher(), _log.Write);

        _orientationStacked = _settings.Orientation == SplitOrientation.Stacked;
        _gutter = _settings.Gutter;
        _borderless = _settings.Borderless;
        _selectedMode = _settings.InputMode;
        _serverPort = _settings.SevenDays.Port;
        _serverMaxPlayers = _settings.SevenDays.MaxPlayers;
        _serverEac = _settings.SevenDays.EacEnabled;
        _serverPassword = _settings.SevenDays.Password;

        StopCommand = new RelayCommand(StopSession);
        RelayoutCommand = new RelayCommand(Relayout, () => _sessions.Active is not null);
        InitControllers();
        InitLab();
        InitWindows();
        InitSteam();
        InitSevenDays();

        _timer = new DispatcherTimer(DispatcherPriority.Render) { Interval = TimeSpan.FromMilliseconds(33) };
        _timer.Tick += (_, _) => OnTick();
        _timer.Start();
        _log.Write($"SplitForge {Version} (.NET {Environment.Version}) iniciado. Sesión de registro {_log.SessionId}.");
        _ = ScanSteamAsync();
    }

    public string Title => $"SplitForge {Version} · Pantalla dividida local";
    public ObservableCollection<string> LogLines { get; } = [];
    public IReadOnlyList<int> Gutters { get; } = [0, 2, 4, 8, 12, 16];

    private bool _orientationStacked;
    public bool OrientationStacked
    {
        get => _orientationStacked;
        set
        {
            if (Set(ref _orientationStacked, value))
            {
                Raise(nameof(OrientationSideBySide));
                SaveSettings();
            }
        }
    }

    public bool OrientationSideBySide
    {
        get => !_orientationStacked;
        set => OrientationStacked = !value;
    }

    private int _gutter;
    public int Gutter
    {
        get => _gutter;
        set
        {
            if (Set(ref _gutter, value))
                SaveSettings();
        }
    }

    private bool _borderless;
    public bool Borderless
    {
        get => _borderless;
        set
        {
            if (Set(ref _borderless, value))
                SaveSettings();
        }
    }

    private string _status = "Listo.";
    public string Status
    {
        get => _status;
        set => Set(ref _status, value);
    }

    private string _inputSummary = "Buscando mandos…";
    public string InputSummary
    {
        get => _inputSummary;
        set => Set(ref _inputSummary, value);
    }

    public ICommand StopCommand { get; }
    public ICommand RelayoutCommand { get; }

    private LayoutOptions Layout() =>
        new(OrientationStacked ? SplitOrientation.Stacked : SplitOrientation.SideBySide, Gutter, Borderless);

    private void Relayout() => Guard(() =>
    {
        _sessions.Relayout(Layout());
        Status = "Distribución actualizada.";
    });

    private void StopSession()
    {
        _labLoop?.Dispose();
        _labLoop = null;
        _sessions.Stop();
        Status = "Sesión detenida; ventanas restauradas.";
    }

    private void OnTick()
    {
        var snapshot = _input.Latest;
        if (snapshot.Sequence != _lastSequence)
        {
            _lastSequence = snapshot.Sequence;
            RefreshControllers(snapshot);
        }
        CaptureTick(snapshot);
        RefreshLabStatus();
        if (++_tickCount % 15 == 0) // ≈ 2 Hz
        {
            Guard(_sessions.Tick, quiet: true);
            RefreshServer();
            var active = _sessions.Active;
            if (active is { Pending: false, WindowHandles.Count: >= 2 })
                Status = $"Sesión activa · {active.WindowHandles.Count} ventanas organizadas";
        }
    }

    private void AppendLog(string line)
    {
        LogLines.Add(line);
        while (LogLines.Count > MaxLogLines)
            LogLines.RemoveAt(0);
    }

    private void SaveSettings()
    {
        _settings = _settings with
        {
            Orientation = OrientationStacked ? SplitOrientation.Stacked : SplitOrientation.SideBySide,
            Gutter = Gutter,
            Borderless = Borderless,
            InputMode = _input.Mode,
            Mappings = new(_input.Mappings),
            SevenDays = ServerOptions(),
        };
        try
        {
            SettingsStore.Save(_dataFolder, _settings);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            _log.Write("No se pudo guardar la configuración: " + ex.Message);
        }
    }

    /// <summary>Ejecuta una acción mostrando el error al usuario en vez de cerrar la aplicación.</summary>
    private void Guard(Action action, bool quiet = false)
    {
        try
        {
            action();
        }
        catch (Exception ex) when (ex is InvalidOperationException or ArgumentException or IOException
                                       or UnauthorizedAccessException or System.ComponentModel.Win32Exception)
        {
            _log.Write("ERROR: " + ex.Message);
            if (!quiet)
                MessageBox.Show(ex.Message, "SplitForge", MessageBoxButton.OK, MessageBoxImage.Warning);
        }
    }

    private static bool Confirm(string title, string text) =>
        MessageBox.Show(text, title, MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes;

    /// <summary>Antes de cerrar: restaurar ventanas y apagar de forma ordenada el servidor propio.</summary>
    public async Task<bool> PrepareCloseAsync()
    {
        StopSession();
        if (_server.Phase is ServerPhase.Starting or ServerPhase.Ready)
        {
            Status = "Apagando el servidor de 7DTD (guardando partida)…";
            await _server.StopAsync();
        }
        SaveSettings();
        return true;
    }

    public void Dispose()
    {
        _timer.Stop();
        _labLoop?.Dispose();
        _sessions.Dispose();
        _server.Dispose();
        _input.Dispose();
    }
}
