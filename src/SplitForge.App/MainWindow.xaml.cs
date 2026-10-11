using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows;
using System.Windows.Interop;
using SplitForge.Platform;

using SplitForge.App.ViewModels;

namespace SplitForge.App;

public partial class MainWindow : Window
{
    private const int MouseLockHotkeyId = 0xB001;
    private const int InputLockHotkeyId = 0xB002;
    private readonly MainViewModel _vm;
    private bool _closingConfirmed;
    private nint _hotkeyWindow;
    private LockNotice? _lockNotice;

    public MainWindow()
    {
        InitializeComponent();
        // Ajustar a pantallas pequeñas: nunca más grande que el área útil del monitor.
        var area = SystemParameters.WorkArea;
        Width = Math.Min(Width, area.Width - 24);
        Height = Math.Min(Height, area.Height - 24);
        _vm = new MainViewModel();
        DataContext = _vm;
        _vm.InputLockChanged += ShowLockNotice;
        // Mantener visible la última línea del registro.
        ((INotifyCollectionChanged)LogList.Items).CollectionChanged += (_, _) =>
        {
            if (LogList.Items.Count > 0)
                LogList.ScrollIntoView(LogList.Items[^1]);
        };
    }

    protected override void OnSourceInitialized(EventArgs e)
    {
        base.OnSourceInitialized(e);
        _hotkeyWindow = new WindowInteropHelper(this).Handle;
        // Barra de título oscura, a juego con el lanzador (Windows 10 2004 o posterior; si no, se ignora).
        var dark = 1;
        _ = DwmSetWindowAttribute(_hotkeyWindow, 20, ref dark, sizeof(int));
        HwndSource.FromHwnd(_hotkeyWindow)?.AddHook(HotkeyHook);
        var spec = MainViewModel.MouseLockHotkeySpec;
        if (!GlobalHotkey.Register(_hotkeyWindow, MouseLockHotkeyId, (uint)spec.Modifiers, spec.VirtualKey))
            _vm.NoteHotkeyUnavailable();
        // Activa el bloqueo de entrada; para soltarlo, el propio gancho detecta el atajo (no llega WM_HOTKEY).
        var input = MainViewModel.InputLockHotkeySpec;
        if (!GlobalHotkey.Register(_hotkeyWindow, InputLockHotkeyId, (uint)input.Modifiers, input.VirtualKey))
            _vm.NoteInputLockHotkeyUnavailable();
    }

    [System.Runtime.InteropServices.DllImport("dwmapi.dll")]
    private static extern int DwmSetWindowAttribute(nint hwnd, int attribute, ref int value, int size);

    private nint HotkeyHook(nint hwnd, int msg, nint wParam, nint lParam, ref bool handled)
    {
        if (msg == GlobalHotkey.WmHotkey && wParam == MouseLockHotkeyId)
        {
            _vm.ToggleMouseLock();
            handled = true;
        }
        else if (msg == GlobalHotkey.WmHotkey && wParam == InputLockHotkeyId)
        {
            _vm.ToggleInputLock();
            handled = true;
        }
        return nint.Zero;
    }

    /// <summary>Aviso en pantalla, por encima de los juegos, de que el teclado y el ratón se bloquearon o se soltaron.</summary>
    private void ShowLockNotice(bool locked)
    {
        if (_closingConfirmed)
            return;
        try
        {
            _lockNotice ??= new LockNotice();
            _lockNotice.Announce(locked, _vm.NoticeArea(), _vm.InputLockHotkey);
        }
        catch (Exception ex) when (ex is InvalidOperationException or System.ComponentModel.Win32Exception)
        {
            // El aviso es solo informativo: si Windows no deja mostrarlo, el bloqueo funciona igual.
        }
    }

    protected override async void OnClosing(CancelEventArgs e)
    {
        if (_closingConfirmed)
        {
            base.OnClosing(e);
            return;
        }
        // Restaurar ventanas y apagar el servidor propio de forma ordenada antes de salir.
        e.Cancel = true;
        IsEnabled = false;
        await _vm.PrepareCloseAsync();
        _closingConfirmed = true;
        // Si el apagado terminó al instante seguimos DENTRO de Closing, y WPF prohíbe Close() ahí:
        // se programa para justo después.
        _ = Dispatcher.BeginInvoke(Close);
    }

    protected override void OnClosed(EventArgs e)
    {
        if (_hotkeyWindow != nint.Zero)
        {
            GlobalHotkey.Unregister(_hotkeyWindow, MouseLockHotkeyId);
            GlobalHotkey.Unregister(_hotkeyWindow, InputLockHotkeyId);
        }
        _vm.InputLockChanged -= ShowLockNotice;
        _lockNotice?.Close();
        _vm.Dispose();
        base.OnClosed(e);
    }
}
