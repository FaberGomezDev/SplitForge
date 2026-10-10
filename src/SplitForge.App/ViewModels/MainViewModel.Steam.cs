using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Text;
using System.Windows;
using System.Windows.Input;
using Microsoft.Win32;
using SplitForge.Core.Steam;
using SplitForge.Core.Windows;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

public sealed record SteamRow(InstalledGame Game, Preflight Preflight)
{
    public string AppId => Game.AppId;
    public string Name => Game.Name;
    public string Phase => Preflight.Phase;
    public string Label => $"{Game.Name}  (AppID {Game.AppId})";
}

public sealed partial class MainViewModel
{
    public ObservableCollection<SteamRow> SteamGames { get; } = [];
    public ICommand ScanSteamCommand { get; private set; } = null!;
    public ICommand ExportSteamCommand { get; private set; } = null!;
    public ICommand OpenGameFolderCommand { get; private set; } = null!;
    public ICommand LaunchSteamCommand { get; private set; } = null!;
    public ICommand AnalyzeGameWindowsCommand { get; private set; } = null!;
    public ICommand TileGameWindowsCommand { get; private set; } = null!;

    private string _steamSummary = "Escaneando la biblioteca de Steam…";
    public string SteamSummary { get => _steamSummary; set => Set(ref _steamSummary, value); }

    private SteamRow? _selectedSteamGame;
    public SteamRow? SelectedSteamGame
    {
        get => _selectedSteamGame;
        set
        {
            if (Set(ref _selectedSteamGame, value))
                Raise(nameof(SteamDetail));
        }
    }

    public string SteamDetail
    {
        get
        {
            if (SelectedSteamGame is not { } row)
                return "Selecciona un juego para ver su diagnóstico.";
            var p = row.Preflight;
            var sb = new StringBuilder()
                .AppendLine($"{p.Name}  ·  AppID {p.AppId}  ·  build {(p.BuildId.Length > 0 ? p.BuildId : "desconocida")}")
                .AppendLine($"Instalación: {(p.FolderPresent ? "ENCONTRADA" : "NO ENCONTRADA")}  ·  Perfil: {p.Phase}")
                .AppendLine("Dos clientes / mandos independientes / offline: NO VERIFICADOS")
                .AppendLine("Ejecutables en la raíz: " + (p.ExecutablesFound.Count > 0 ? string.Join(", ", p.ExecutablesFound.Take(5)) : "ninguno"))
                .AppendLine("OBSERVACIONES:");
            foreach (var note in p.Notes.Take(4))
                sb.AppendLine(" · " + note);
            return sb.ToString();
        }
    }

    private SteamRow? _windowsGame;
    public SteamRow? WindowsGame { get => _windowsGame; set => Set(ref _windowsGame, value); }

    private string _gameWindowsReport = "Abre el juego desde Steam, selecciónalo y pulsa «Analizar ventanas».";
    public string GameWindowsReport { get => _gameWindowsReport; set => Set(ref _gameWindowsReport, value); }

    private GameWindowReport? _lastWindowReport;
    private InstalledGame? _lastWindowGame;

    private void InitSteam()
    {
        ScanSteamCommand = new RelayCommand(() => _ = ScanSteamAsync());
        ExportSteamCommand = new RelayCommand(ExportSteam);
        OpenGameFolderCommand = new RelayCommand(OpenGameFolder, () => SelectedSteamGame is not null);
        LaunchSteamCommand = new RelayCommand(LaunchSteamSingle, () => SelectedSteamGame is not null);
        AnalyzeGameWindowsCommand = new RelayCommand(AnalyzeGameWindows, () => WindowsGame is not null);
        TileGameWindowsCommand = new RelayCommand(TileGameWindows, () => _lastWindowReport is { ClientHandles.Count: >= 2 } && _sessions.Active is null);
    }

    private async Task ScanSteamAsync()
    {
        try
        {
            var rows = await Task.Run(() => WindowsDiagnostics.ScanSteam().Select(g => new SteamRow(g, PreflightEvaluator.Evaluate(g))).ToList());
            var previous = SelectedSteamGame?.AppId;
            var previousWindows = WindowsGame?.AppId;
            SteamGames.Clear();
            foreach (var row in rows)
                SteamGames.Add(row);
            SelectedSteamGame = SteamGames.FirstOrDefault(r => r.AppId == previous);
            WindowsGame = SteamGames.FirstOrDefault(r => r.AppId == (previousWindows ?? ResearchProfiles.SevenDaysToDie));
            SteamSummary = $"{rows.Count} instalaciones encontradas. Ningún juego certificado automáticamente para pantalla dividida.";
            _log.Write($"Steam: {rows.Count} títulos enumerados sin escribir en sus carpetas.");
            RefreshSevenDaysChecks();
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or System.Security.SecurityException)
        {
            SteamSummary = "No se pudo leer la biblioteca de Steam: " + ex.Message;
        }
    }

    private void ExportSteam() => Guard(() =>
    {
        var dialog = new SaveFileDialog
        {
            Title = "Guardar reporte Steam (sin rutas personales)",
            FileName = "SplitForge_Reporte_Steam.json",
            Filter = "JSON|*.json",
        };
        if (dialog.ShowDialog() != true)
            return;
        File.WriteAllText(dialog.FileName, SteamPublicReport.Create(SteamGames.Select(r => r.Game)).ToJson());
        _log.Write($"Reporte Steam exportado ({SteamGames.Count} entradas).");
        MessageBox.Show("El reporte NO incluye rutas absolutas, cuentas de Steam ni identificadores de mandos.", "Reporte guardado");
    });

    private void OpenGameFolder() => Guard(() =>
    {
        var row = SelectedSteamGame!;
        if (!row.Preflight.FolderPresent || !row.Preflight.InsideSteamLibrary)
            throw new InvalidOperationException("No hay una carpeta de instalación válida.");
        Process.Start(new ProcessStartInfo("explorer.exe") { ArgumentList = { row.Game.InstallFolder }, UseShellExecute = false });
    });

    private void LaunchSteamSingle() => Guard(() =>
    {
        var row = SelectedSteamGame!;
        if (!row.Preflight.FolderPresent)
            throw new InvalidOperationException("El título no tiene una instalación válida.");
        if (!Confirm("Abrir a través de Steam", $"Se pedirá a Steam abrir UNA instancia de {row.Name}.\n\n"
                     + "Esto NO activa pantalla dividida ni garantiza modo sin conexión. ¿Continuar?"))
            return;
        Process.Start(new ProcessStartInfo(PreflightEvaluator.SteamRunUri(row.AppId)) { UseShellExecute = true });
        _log.Write($"Solicitud enviada a Steam para AppID {row.AppId} (una instancia).");
    });

    private void AnalyzeGameWindows() => Guard(() =>
    {
        var game = WindowsGame!.Game;
        var report = WindowsDiagnostics.InspectGame(game);
        _lastWindowReport = report;
        _lastWindowGame = game;
        var sb = new StringBuilder()
            .AppendLine($"Juego: {game.Name} (AppID {game.AppId})")
            .AppendLine($"Estado: {report.State}")
            .AppendLine($"Ventanas asociadas: {report.Windows.Count}  ·  Clientes (procesos) distintos: {report.DistinctPids}")
            .AppendLine(report.Detail)
            .AppendLine();
        foreach (var w in report.Windows.Take(12))
            sb.AppendLine($"  PID {w.Pid} · {w.ExeName} · {Truncate(w.Title, 60)}");
        if (report.Windows.Count == 0)
            sb.AppendLine("  (Sin ventanas; abre el juego normalmente desde Steam.)");
        sb.AppendLine().AppendLine("ATENCIÓN: solo posición. Entrada aislada y partida común NO verificadas.");
        GameWindowsReport = sb.ToString();
        _log.Write($"Ventanas Steam AppID {game.AppId}: {report.Windows.Count} ventanas, {report.DistinctPids} PID(s). Estado {report.State}.");
    });

    private void TileGameWindows() => Guard(() =>
    {
        var game = _lastWindowGame ?? throw new InvalidOperationException("Pulsa «Analizar ventanas» antes de distribuir.");
        if (WindowsGame?.AppId != game.AppId)
            throw new InvalidOperationException("El juego seleccionado cambió. Analiza otra vez.");
        var handles = RevalidatedClients(game, _lastWindowReport!);
        if (!Confirm("Colocar clientes", $"Hay {handles.Count} ventanas del MISMO ejecutable en procesos distintos.\n\n"
                     + "SplitForge cambiará su posición y tamaño (sin tocar archivos del juego).\n"
                     + "NO separa los mandos ni conecta a los jugadores. ¿Distribuir?"))
            return;
        _sessions.Attach(handles, Layout());
        Status = $"{handles.Count} ventanas de {game.Name} distribuidas · partida común no verificada";
    });

    /// <summary>Revalida HWND + PID + exe justo antes de mover: evita mover una ventana equivocada.</summary>
    private static IReadOnlyList<nint> RevalidatedClients(InstalledGame game, GameWindowReport previous)
    {
        var current = WindowsDiagnostics.InspectGame(game);
        if (current.ClientHandles.Count < 2 || !current.ClientHandles.SequenceEqual(previous.ClientHandles))
            throw new InvalidOperationException("Las ventanas cambiaron; pulsa «Analizar ventanas» otra vez.");
        return current.ClientHandles;
    }

    private static string Truncate(string text, int max) => text.Length <= max ? text : text[..max] + "…";
}
