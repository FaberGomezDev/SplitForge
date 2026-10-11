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
    private int _fakeFocusInGame;

    private void InitFakeFocus() => _fakeFocusEnabled = _settings.FakeFocus;

    /// <summary>
    /// Llamado a ≈2 Hz: aplica el foco simulado a las ventanas que colocó la sesión activa. Las de un juego
    /// cuyo filtro ya simula el foco por dentro se dejan en paz: los mensajes de activación repetidos hacían
    /// que cada juego recolocara y encerrara el puntero de Windows diez veces por segundo.
    /// </summary>
    private void SyncFakeFocus()
    {
        var handles = _sessions.Active is { Pending: false } active ? [.. active.WindowHandles] : Array.Empty<nint>();
        var inGame = new HashSet<nint>();
        if (_fakeFocusEnabled && _focusedInGame.Count > 0)
        {
            foreach (var handle in handles)
            {
                if (_windows.ProcessId(handle) is { } pid && _focusedInGame.Contains(pid))
                    inGame.Add(handle);
            }
        }
        var targets = FakeFocusPlan.Targets(_fakeFocusEnabled, handles, inGame);
        _fakeFocus.SetTargets(targets, _settings.FakeFocusIntervalMs);
        if (targets.Count != _fakeFocusCount || inGame.Count != _fakeFocusInGame)
        {
            _fakeFocusCount = targets.Count;
            _fakeFocusInGame = inGame.Count;
            if (targets.Count > 0)
                _log.Write($"Foco simulado en {targets.Count} ventanas (cada {FakeFocusPlan.ClampInterval(_settings.FakeFocusIntervalMs)} ms).");
            if (inGame.Count > 0)
                _log.Write($"Foco simulado: {inGame.Count} ventanas lo hacen dentro del juego (filtro XInput); no se les envían mensajes.");
        }
        FakeFocusStatus = !_fakeFocusEnabled
            ? "Foco simulado desactivado."
            : targets.Count == 0 && inGame.Count == 0
                ? "Foco simulado: esperando la pantalla dividida."
                : targets.Count == 0
                    ? $"Foco simulado dentro del juego en {inGame.Count} ventanas (filtro XInput); sin mensajes desde fuera."
                    : $"Foco simulado en {targets.Count} ventanas · {_fakeFocus.Posted} mensajes · {_fakeFocus.Failed} rechazados"
                      + (inGame.Count > 0 ? $" · {inGame.Count} dentro del juego" : "");
    }
}
