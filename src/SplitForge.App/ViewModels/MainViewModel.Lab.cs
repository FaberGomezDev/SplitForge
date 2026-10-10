using System.Windows.Input;
using SplitForge.Core.Sessions;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

public sealed partial class MainViewModel
{
    public const double LabTickHz = 120;

    public ICommand StartLabCommand { get; private set; } = null!;
    public ICommand ReassignLabCommand { get; private set; } = null!;

    private PadRow? _labPlayer1;
    public PadRow? LabPlayer1 { get => _labPlayer1; set => Set(ref _labPlayer1, value); }

    private PadRow? _labPlayer2;
    public PadRow? LabPlayer2 { get => _labPlayer2; set => Set(ref _labPlayer2, value); }

    private string _labStatus = "P1: pendiente\nP2: pendiente";
    public string LabStatus { get => _labStatus; set => Set(ref _labStatus, value); }

    private void InitLab()
    {
        StartLabCommand = new RelayCommand(StartLab, () => _sessions.Active is null);
        ReassignLabCommand = new RelayCommand(ReassignLab, () => _sessions.Active?.Lab is not null);
    }

    private void StartLab() => Guard(() =>
    {
        LabPlayer1 ??= Pads.ElementAtOrDefault(0);
        LabPlayer2 ??= Pads.ElementAtOrDefault(1);
        if (LabPlayer1 is null || LabPlayer2 is null || LabPlayer1.DeviceId == LabPlayer2.DeviceId)
            throw new InvalidOperationException("Selecciona dos mandos diferentes y conectados.");
        var exe = Environment.ProcessPath ?? throw new InvalidOperationException("No se pudo localizar el ejecutable de SplitForge.");
        _sessions.StartLab(p => new LaunchSpec(exe, ["--lab-viewer", p.ToString(System.Globalization.CultureInfo.InvariantCulture)]),
            LabPlayer1.DeviceId, LabPlayer2.DeviceId, Layout());
        // La simulación corre en su propio hilo a frecuencia fija, con temporizador de alta resolución.
        _labLoop = new FixedRateLoop("SplitForge.Lab", LabTickHz,
            dt => _sessions.RouteLab(_input.Latest.States, dt),
            HighResolutionDelay.Sleep,
            ex => _log.Write("Error en la simulación: " + ex.Message));
        _input.IdentifyPlayer(LabPlayer1.DeviceId, 0);
        _input.IdentifyPlayer(LabPlayer2.DeviceId, 1);
        Status = "Arrancando el mundo compartido…";
    });

    private void ReassignLab() => Guard(() =>
    {
        var lab = _sessions.Active?.Lab ?? throw new InvalidOperationException("No hay mundo compartido en ejecución.");
        if (LabPlayer1 is null || LabPlayer2 is null)
            throw new InvalidOperationException("Selecciona dos mandos diferentes que estén conectados.");
        lab.Reassign(LabPlayer1.DeviceId, LabPlayer2.DeviceId, Pads.Select(p => p.DeviceId).ToHashSet());
        _input.IdentifyPlayer(LabPlayer1.DeviceId, 0);
        _input.IdentifyPlayer(LabPlayer2.DeviceId, 1);
        _log.Write($"Reasignación en vivo: P1 ID {LabPlayer1.DeviceId} / P2 ID {LabPlayer2.DeviceId}.");
    });

    private void RefreshLabStatus()
    {
        var lab = _sessions.Active?.Lab;
        if (lab is not null)
            LabStatus = string.Join('\n', lab.StatusLines);
    }
}
