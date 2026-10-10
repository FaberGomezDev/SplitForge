using SplitForge.Core.Windows;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

public sealed partial class MainViewModel
{
    // Hilo propio: la interfaz solo le pasa la lista de ventanas de la sesión (ADR-020).
    private readonly WindowFakeFocus _fakeFocus = new();

    private bool _fakeFocusEnabled = true;
    public bool FakeFocusEnabled
    {
        get => _fakeFocusEnabled;
        set
        {
            if (!Set(ref _fakeFocusEnabled, value))
                return;
            SaveSettings();
            _log.Write(value
                ? "Foco simulado activado: cada ventana de la pantalla dividida se cree activa."
                : "Foco simulado desactivado.");
            SyncFakeFocus();
        }
    }

    private string _fakeFocusStatus = "Foco simulado: esperando la pantalla dividida.";
    public string FakeFocusStatus { get => _fakeFocusStatus; set => Set(ref _fakeFocusStatus, value); }

    private int _fakeFocusCount;

    private void InitFakeFocus() => _fakeFocusEnabled = _settings.FakeFocus;

    /// <summary>Llamado a ≈2 Hz: aplica el foco simulado a las ventanas que colocó la sesión activa.</summary>
    private void SyncFakeFocus()
    {
        var handles = _sessions.Active is { Pending: false } active ? [.. active.WindowHandles] : Array.Empty<nint>();
        var targets = FakeFocusPlan.Targets(_fakeFocusEnabled, handles);
        _fakeFocus.SetTargets(targets, _settings.FakeFocusIntervalMs);
        if (targets.Count != _fakeFocusCount)
        {
            _fakeFocusCount = targets.Count;
            if (targets.Count > 0)
                _log.Write($"Foco simulado en {targets.Count} ventanas (cada {FakeFocusPlan.ClampInterval(_settings.FakeFocusIntervalMs)} ms).");
        }
        FakeFocusStatus = !_fakeFocusEnabled
            ? "Foco simulado desactivado."
            : targets.Count == 0
                ? "Foco simulado: esperando la pantalla dividida."
                : $"Foco simulado en {targets.Count} ventanas · {_fakeFocus.Posted} mensajes · {_fakeFocus.Failed} rechazados";
    }
}
