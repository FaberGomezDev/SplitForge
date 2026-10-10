using System.Windows.Input;
using Microsoft.Win32;
using SplitForge.Core.Adapters;
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
        if (!Confirm("Emulador de Steam (experimental · educativo)",
                "Se sustituirá temporalmente steam_api64.dll del juego por la versión emulada de SplitForge "
                + "(la original queda como copia de seguridad). El juego NO hablará con Steam y usará las identidades "
                + "locales de esta pestaña.\n\nSOLO con el juego sin EAC y sin cuentas reales. ¿Instalar?"))
            return;
        var config = SteamEmulator.BuildConfig(SteamEmuAppId, SteamId1, Persona1,
            SteamEmulator.DefaultStorageDir(_dataFolder, SteamEmuAppId, 1));
        SteamEmulator.Install(folder, dll, config);
        SaveSettings();
        _log.Write("Emulador de Steam instalado en 7DTD (original respaldado). Solo abrir con «Lanzar instancia» de esta pestaña.");
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

    private void LaunchSteamEmu(int player) => Guard(() =>
    {
        if (!SteamEmulator.IsInstalled(SevenDaysFolder()))
            throw new InvalidOperationException("Instala primero el emulador de Steam en 7DTD.");
        WriteSteamEmuConfig(player);
        var spec = SteamEmulator.InstanceLaunch(SevenDaysExe(), SteamEmuConfigFile(player));
        _steamEmuLauncher.Start(spec).Dispose();
        _log.Write($"Instancia J{player} de 7DTD lanzada con identidad emulada ({SpecPersona(player)}). Cierra Steam antes si estaba abierto.");
    });

    private void LaunchSteamEmuBoth() => Guard(() =>
    {
        if (!SteamEmulator.IsInstalled(SevenDaysFolder()))
            throw new InvalidOperationException("Instala primero el emulador de Steam en 7DTD.");
        WriteSteamEmuConfig(1);
        WriteSteamEmuConfig(2);
        var exe = SevenDaysExe();
        _steamEmuLauncher.Start(SteamEmulator.InstanceLaunch(exe, SteamEmuConfigFile(1))).Dispose();
        System.Threading.Thread.Sleep(1500);
        _steamEmuLauncher.Start(SteamEmulator.InstanceLaunch(exe, SteamEmuConfigFile(2))).Dispose();
        _log.Write("Dos instancias de 7DTD lanzadas con identidades emuladas J1/J2 (Steam debe estar cerrado).");
    });

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
