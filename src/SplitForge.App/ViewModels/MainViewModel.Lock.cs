using System.Windows.Input;
using SplitForge.Core.Input;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

public sealed partial class MainViewModel
{
    // Atajo fijo por ahora (se muestra en la interfaz); registrado en MainWindow.
    public static readonly HotkeySpec MouseLockHotkeySpec = HotkeySpec.LockDefault;

    public string MouseLockHotkey => MouseLockHotkeySpec.Label;

    private bool _mouseLockEnabled;
    public bool MouseLockEnabled
    {
        get => _mouseLockEnabled;
        private set { if (Set(ref _mouseLockEnabled, value)) Raise(nameof(MouseLockButtonText)); }
    }

    public string MouseLockButtonText => _mouseLockEnabled ? "Soltar ratón" : "Bloquear ratón en la ventana";

    private string _mouseLockStatus = "Ratón libre.";
    public string MouseLockStatus { get => _mouseLockStatus; set => Set(ref _mouseLockStatus, value); }

    public ICommand ToggleMouseLockCommand { get; private set; } = null!;

    private bool _cursorClipped;

    private void InitLock()
    {
        ToggleMouseLockCommand = new RelayCommand(ToggleMouseLock);
        ToggleInputLockCommand = new RelayCommand(ToggleInputLock);
    }

    // --- Bloqueo de entrada «solo mandos» (Ctrl+Alt+B) ---

    public static readonly HotkeySpec InputLockHotkeySpec = HotkeySpec.InputLockDefault;

    public string InputLockHotkey => InputLockHotkeySpec.Label;

    public ICommand ToggleInputLockCommand { get; private set; } = null!;

    private bool _inputLocked;
    public bool InputLocked
    {
        get => _inputLocked;
        private set { if (Set(ref _inputLocked, value)) Raise(nameof(InputLockButtonText)); }
    }

    public string InputLockButtonText => _inputLocked ? $"Entrada bloqueada ({InputLockHotkey} para soltar)" : "Bloquear entrada (solo mandos)";

    /// <summary>
    /// Alterna el bloqueo: mientras está activo, el teclado y el ratón no llegan a nada (ni juegos ni
    /// escritorio) y los mandos siguen funcionando. Se suelta con el mismo atajo, al detener la sesión
    /// o al cerrar SplitForge; Ctrl+Alt+Supr siempre funciona.
    /// </summary>
    public void ToggleInputLock()
    {
        if (InputLocked)
        {
            ReleaseInputLock();
            return;
        }
        // El gancho avisa del atajo desde dentro: se difiere para que vuelva al instante.
        var dispatcher = System.Windows.Application.Current?.Dispatcher;
        if (!InputLock.Start(InputLockHotkeySpec, () => dispatcher?.BeginInvoke(ReleaseInputLock)))
        {
            _log.Write("No se pudo activar el bloqueo de entrada (Windows rechazó el gancho de teclado/ratón).");
            return;
        }
        InputLocked = true;
        _log.Write($"Entrada bloqueada: solo funcionan los mandos. Pulsa {InputLockHotkey} para soltarla.");
        Status = $"Entrada bloqueada (solo mandos). {InputLockHotkey} para soltar.";
    }

    private void ReleaseInputLock()
    {
        if (!InputLocked && !InputLock.IsActive)
            return;
        InputLock.Stop();
        InputLocked = false;
        _log.Write("Entrada liberada: teclado y ratón vuelven a funcionar.");
    }

    /// <summary>El atajo del bloqueo de entrada ya estaba en uso por otra app.</summary>
    public void NoteInputLockHotkeyUnavailable() =>
        _log.Write($"No se pudo registrar el atajo {InputLockHotkey} (¿lo usa otra app?). Usa el botón «Bloquear entrada».");

    /// <summary>El atajo global ya estaba en uso por otra app: el botón sigue funcionando.</summary>
    public void NoteHotkeyUnavailable()
    {
        _log.Write($"No se pudo registrar el atajo {MouseLockHotkey} (¿lo usa otra app?). Usa el botón «Bloquear ratón».");
        MouseLockStatus = $"Atajo {MouseLockHotkey} no disponible; usa el botón.";
    }

    /// <summary>Alterna el bloqueo. Lo llama el botón y el atajo global (Ctrl+Alt+L) desde MainWindow.</summary>
    public void ToggleMouseLock()
    {
        MouseLockEnabled = !MouseLockEnabled;
        if (!MouseLockEnabled)
        {
            ReleaseCursor();
            MouseLockStatus = $"Ratón libre. Pulsa {MouseLockHotkey} o el botón para volver a bloquear.";
        }
        else
        {
            MouseLockStatus = "Bloqueo activo: el ratón se queda en la ventana del jugador en primer plano.";
        }
        _log.Write(MouseLockEnabled ? "Bloqueo de ratón activado." : "Bloqueo de ratón desactivado.");
    }

    /// <summary>
    /// Reaplica el confinamiento en cada tick: Windows lo reinicia al cambiar el foco. Solo bloquea si la
    /// ventana en primer plano es una de las que SplitForge colocó (laboratorio o clientes de 7DTD).
    /// </summary>
    private void ApplyMouseLock()
    {
        if (!_mouseLockEnabled)
            return;
        var handles = _sessions.Active?.WindowHandles;
        var fg = CursorLock.ForegroundWindow();
        if (handles is { Count: > 0 } && handles.Contains(fg) && CursorLock.ClipTo(fg))
        {
            _cursorClipped = true;
            MouseLockStatus = "Ratón bloqueado en la ventana del jugador activo. Cambia de ventana con Alt+Tab.";
        }
        else
        {
            ReleaseCursor();
            MouseLockStatus = handles is { Count: > 0 }
                ? "Bloqueo en pausa: pon en primer plano una ventana de la pantalla dividida."
                : "Bloqueo activo, pero aún no hay pantalla dividida. Usa «Dividir pantalla» primero.";
        }
    }

    private void ReleaseCursor()
    {
        if (!_cursorClipped)
            return;
        CursorLock.Release();
        _cursorClipped = false;
    }
}
