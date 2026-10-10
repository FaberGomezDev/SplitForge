using System.Windows.Input;
using Microsoft.Win32;
using SplitForge.Core.Adapters;
using SplitForge.Core.Layout;
using SplitForge.Core.Sessions;
using SplitForge.Core.Steam;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

public sealed partial class MainViewModel
{
    private readonly SystemProcessLauncher _steamEmuLauncher = new();

    public ICommand InstallSteamEmuCommand { get; private set; } = null!;
    public ICommand RestoreSteamEmuCommand { get; private set; } = null!;
    public ICommand LaunchSteamEmu1Command { get; private set; } = null!;
    public ICommand LaunchSteamEmu2Command { get; private set; } = null!;
    public ICommand LaunchSteamEmuBothCommand { get; private set; } = null!;
    public ICommand PickSteamEmuDllCommand { get; private set; } = null!;

    private string _steamEmuStatus = "Emulador no instalado.";
    public string SteamEmuStatus { get => _steamEmuStatus; set => Set(ref _steamEmuStatus, value); }

    private string _steamId1 = "76561198000000001";
    public string SteamId1 { get => _steamId1; set => Set(ref _steamId1, value); }

    private string _steamId2 = "76561198000000002";
    public string SteamId2 { get => _steamId2; set => Set(ref _steamId2, value); }

    private string _persona1 = "SplitForgeJ1";
    public string Persona1 { get => _persona1; set => Set(ref _persona1, value); }

    private string _persona2 = "SplitForgeJ2";
    public string Persona2 { get => _persona2; set => Set(ref _persona2, value); }

    private int _steamEmuAppId = 251570;
    public int SteamEmuAppId { get => _steamEmuAppId; set => Set(ref _steamEmuAppId, value); }

    private string _steamEmuDllPath = "";
    public string SteamEmuDllPath
    {
        get => _steamEmuDllPath;
        set { if (Set(ref _steamEmuDllPath, value)) SaveSettings(); }
    }

    private void InitSteamEmu()
    {
        InstallSteamEmuCommand = new RelayCommand(InstallSteamEmu);
        RestoreSteamEmuCommand = new RelayCommand(RestoreSteamEmu);
        LaunchSteamEmu1Command = new RelayCommand(() => LaunchSteamEmu(1));
        LaunchSteamEmu2Command = new RelayCommand(() => LaunchSteamEmu(2));
        LaunchSteamEmuBothCommand = new RelayCommand(LaunchSteamEmuBoth);
        PickSteamEmuDllCommand = new RelayCommand(PickSteamEmuDll);

        _steamId1 = _settings.SteamEmu.SteamId1;
        _steamId2 = _settings.SteamEmu.SteamId2;
        _persona1 = _settings.SteamEmu.Persona1;
        _persona2 = _settings.SteamEmu.Persona2;
        _steamEmuAppId = _settings.SteamEmu.AppId;
        _steamEmuDllPath = _settings.SteamEmu.EmulatorDll;
    }

    private string SteamEmuDll()
    {
        if (File.Exists(_steamEmuDllPath))
            return _steamEmuDllPath;
        var found = SteamEmulator.FindEmulatorDll(AppContext.BaseDirectory)
                    ?? throw new InvalidOperationException(
                        "No se encontró la DLL emulada (native\\SplitForge.SteamEmu\\steam_api64.dll). "
                        + "Compila con Compilar_SplitForge.bat o usa «Seleccionar DLL…».");
        _steamEmuDllPath = found;
        return found;
    }

    private string SevenDaysExe() =>
        SevenDaysToDieAdapter.FindExecutable(
            FindGame(ResearchProfiles.SevenDaysToDie) ?? throw new InvalidOperationException("7 Days to Die no aparece instalado."),
            SevenDaysToDieAdapter.GameExe)
        ?? throw new InvalidOperationException("No se encontró 7DaysToDie.exe en la carpeta del juego.");

    private string SteamEmuConfigFile(int player)
    {
        var folder = Path.Combine(_dataFolder, "steamemu");
        Directory.CreateDirectory(folder);
        return Path.Combine(folder, $"j{player}.json");
    }

    private void WriteSteamEmuConfig(int player)
    {
        var (steamId, persona) = player == 1 ? (SteamId1, Persona1) : (SteamId2, Persona2);
        var config = SteamEmulator.BuildConfig(SteamEmuAppId, steamId, persona,
            SteamEmulator.DefaultStorageDir(_dataFolder, SteamEmuAppId, player));
        File.WriteAllText(SteamEmuConfigFile(player), config, new System.Text.UTF8Encoding(false));
    }

    private void InstallSteamEmu() => Guard(() =>
    {
        var folder = SevenDaysFolder();
        var report = WindowsDiagnostics.SevenDays([]);
        if (report.Clients.Any(c => c.Input?.AntiCheatLoaded == true))
            throw new InvalidOperationException("Hay un cliente de 7DTD con EAC activo. Ciérralo antes de instalar el emulador.");
        var dll = SteamEmuDll();
        if (!Confirm("Emulador de Steam (experimental)",
                "Se sustituirá steam_api64.dll del juego por la versión emulada de SplitForge "
                + "(la original queda como copia de seguridad). El juego NO hablará con Steam y usará las identidades "
                + "locales de esta pestaña.\n\nRequiere el juego sin EAC (opción oficial). ¿Instalar?"))
            return;
        var config = SteamEmulator.BuildConfig(SteamEmuAppId, SteamId1, Persona1,
            SteamEmulator.DefaultStorageDir(_dataFolder, SteamEmuAppId, 1));
        var result = SteamEmulator.Install(folder, dll, config);
        SaveSettings();
        _log.Write(result switch
        {
            SteamEmuInstallResult.Updated => "Emulador de Steam actualizado en 7DTD (la original sigue respaldada).",
            SteamEmuInstallResult.UpToDate => "El emulador de Steam de 7DTD ya estaba al día.",
            _ => "Emulador de Steam instalado en 7DTD (original respaldado). Solo abrir con «Lanzar instancia» de esta pestaña.",
        });
        RefreshSteamEmuStatus();
    });

    private void RestoreSteamEmu() => Guard(() =>
    {
        if (!Confirm("Restaurar DLL original de Steam",
                "Se devolverá la steam_api64.dll original y se borrarán la configuración y el estado del emulador. ¿Restaurar?"))
            return;
        SteamEmulator.Restore(SevenDaysFolder());
        _log.Write("Emulador de Steam retirado: DLL original restaurada.");
        RefreshSteamEmuStatus();
    });

    /// <summary>Lanzamiento planificado: ejecutable, distribución (con el mínimo 16:9 de 7DTD) y panel de cada jugador.</summary>
    private sealed record EmulatedLaunch(string Exe, LayoutOptions Layout, IReadOnlyList<Region> Panels);

    /// <summary>Clientes recién lanzados a la espera de su ventana para dividir la pantalla.</summary>
    private sealed record PendingEmulatedClients(int[] Pids, LayoutOptions Layout, DateTime ReadyAt, DateTime Deadline);

    private PendingEmulatedClients? _emuPending;

    private EmulatedLaunch PlanEmulatedLaunch()
    {
        if (!SteamEmulator.IsInstalled(SevenDaysFolder()))
            throw new InvalidOperationException("Instala primero el emulador de Steam en 7DTD.");
        var layout = Layout() with { MinAspect = SevenDaysToDieAdapter.MinAspect };
        var area = layout.CoverTaskbar ? _windows.PrimaryBounds() : _windows.PrimaryWorkArea();
        return new EmulatedLaunch(SevenDaysExe(), layout, layout.Regions(area, 2));
    }

    /// <summary>
    /// Lanza un cliente con su identidad y la configuración del handler de Nucleus Co-op: sin juego
    /// cruzado (EOS), ventana sin marco y con el tamaño de su panel desde el arranque.
    /// </summary>
    private int StartEmulatedInstance(EmulatedLaunch plan, int player)
    {
        WriteSteamEmuConfig(player);
        var panel = plan.Panels[player - 1];
        var spec = SteamEmulator.InstanceLaunch(plan.Exe, SteamEmuConfigFile(player),
            SevenDaysToDieAdapter.EmulatedClientArguments(panel.Width, panel.Height));
        using var process = _steamEmuLauncher.Start(spec);
        _log.Write($"J{player} de 7DTD lanzado con identidad emulada ({SpecPersona(player)}), ventana {panel.Width}×{panel.Height}, sin juego cruzado (EOS).");
        return process.Id;
    }

    private void LaunchSteamEmu(int player) => Guard(() => StartEmulatedInstance(PlanEmulatedLaunch(), player));

    /// <summary>
    /// Todo en un paso: lanza J1 y J2, asigna a cada uno su mando (si el filtro XInput está instalado)
    /// y divide la pantalla en cuanto aparecen sus ventanas.
    /// </summary>
    private async void LaunchSteamEmuBoth()
    {
        EmulatedLaunch? plan = null;
        var first = 0;
        Guard(() =>
        {
            plan = PlanEmulatedLaunch();
            first = StartEmulatedInstance(plan, 1);
        });
        if (plan is null || first == 0)
            return;
        // Como el handler de Nucleus (PauseBetweenStarts): el segundo arranque no compite con el primero.
        await Task.Delay(TimeSpan.FromSeconds(1.5));
        var second = 0;
        Guard(() => second = StartEmulatedInstance(plan, 2));
        if (second == 0)
            return;
        Guard(() => AssignEmulatedPads(first, second));
        var now = DateTime.UtcNow;
        // Se espera unos segundos a que el juego termine de crear su ventana (PauseBetweenProcessGrab de Nucleus).
        _emuPending = new PendingEmulatedClients([first, second], plan.Layout, now.AddSeconds(12), now.AddMinutes(2));
        Status = "7DTD: J1 y J2 arrancando; la pantalla se dividirá sola cuando aparezcan sus ventanas.";
    }

    /// <summary>Cada cliente ve solo el mando de su jugador (ranuras de la pestaña 06), por PID.</summary>
    private void AssignEmulatedPads(int pid1, int pid2)
    {
        if (XInputFilter.InstalledNames(SevenDaysFolder()).Count == 0)
        {
            _log.Write("Filtro XInput no instalado: los dos mandos controlarán ambas ventanas. Instálalo en la pestaña 06 (con los clientes cerrados).");
            return;
        }
        XInputFilter.WriteAssignments(XInputFolder, [(pid1, SlotPlayer1 - 1), (pid2, SlotPlayer2 - 1)]);
        _log.Write($"Mandos asignados: J1 (PID {pid1}) → mando XInput {SlotPlayer1} · J2 (PID {pid2}) → mando XInput {SlotPlayer2}.");
        RefreshFilterStatus();
    }

    /// <summary>Llamado a ≈2 Hz: divide la pantalla cuando las ventanas de los clientes lanzados ya existen.</summary>
    private void TileEmulatedClientsWhenReady()
    {
        if (_emuPending is not { } pending || DateTime.UtcNow < pending.ReadyAt)
            return;
        if (DateTime.UtcNow > pending.Deadline)
        {
            _emuPending = null;
            Status = "Las ventanas de 7DTD no aparecieron a tiempo: usa «Dividir pantalla» en la pestaña 06.";
            return;
        }
        var windows = _windows.EnumerateWindows();
        var handles = new List<nint>();
        foreach (var pid in pending.Pids)
        {
            var window = windows.FirstOrDefault(w => w.Pid == pid && w.Title == SevenDaysToDieAdapter.WindowTitle);
            if (window.Handle == 0)
                return; // aún no
            handles.Add(window.Handle);
        }
        _emuPending = null;
        if (_sessions.Active is not null)
            _sessions.Stop();
        _sessions.Attach(handles, pending.Layout);
        Status = "7DTD: J1 y J2 en pantalla dividida, cada uno con su mando.";
    }

    private string SpecPersona(int player) => player == 1 ? Persona1 : Persona2;

    private void PickSteamEmuDll() => Guard(() =>
    {
        var dialog = new OpenFileDialog
        {
            Title = "Selecciona la DLL emulada (steam_api64.dll)",
            Filter = "DLL de Steam emulada|steam_api64.dll",
            FileName = "steam_api64.dll",
        };
        if (dialog.ShowDialog() != true)
            return;
        SteamEmuDllPath = dialog.FileName;
        _log.Write("DLL del emulador seleccionada: " + dialog.FileName);
        RefreshSteamEmuStatus();
    });

    private void RefreshSteamEmuStatus()
    {
        try
        {
            var folder = SevenDaysFolder();
            var status = SteamEmulator.ReadStatus(folder);
            if (!status.Installed)
            {
                SteamEmuStatus = SteamEmulator.FindEmulatorDll(AppContext.BaseDirectory) is null && !File.Exists(_steamEmuDllPath)
                    ? "Emulador no instalado. Falta la DLL emulada: usa «Seleccionar DLL…» o compila con Compilar_SplitForge.bat."
                    : "Emulador no instalado.";
                return;
            }
            SteamEmuStatus = $"Instalado · identidad local: {status.Persona ?? "?"} ({status.SteamId ?? "?"}) · guardados: {status.StorageDir ?? "?"} · original respaldado: {(status.OriginalSha256 is null ? "no" : "sí")}.";
        }
        catch (InvalidOperationException)
        {
            SteamEmuStatus = "7 Days to Die no detectado.";
        }
    }
}
