using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using SplitForge.Core.Layout;
using SplitForge.Core.Windows;

namespace SplitForge.Platform;

/// <summary>
/// Fondo negro detrás de la pantalla dividida: una ventana propia sin marco que tapa el escritorio (y la barra
/// de tareas, si la sesión la cubre) en las uniones entre juegos y en las franjas que deje la proporción.
/// Es lo más barato posible: una ventana Win32 con pincel negro de sistema, sin contenido ni temporizadores
/// ni dibujo propio; Windows la pinta una vez. No toma el foco ni aparece en la barra de tareas o en Alt+Tab.
/// Se crea y se usa siempre desde el hilo de la interfaz (que es quien despacha sus mensajes).
/// </summary>
[SupportedOSPlatform("windows")]
public sealed unsafe class SessionBackdrop : IDisposable
{
    private const string ClassName = "SplitForge.Fondo";
    private static bool s_registered;

    private readonly List<nint> _zOrder = new(512); // Reutilizada: comprobar el orden no genera basura.
    private nint _hwnd;
    private bool _visible;
    private Region _area;
    private bool _topmost;

    public bool Visible => _visible;

    /// <summary>
    /// Muestra el fondo sobre <paramref name="area"/> y lo deja justo debajo de las ventanas de la sesión.
    /// Se puede llamar a menudo: solo actúa si algo cambió (tamaño, modo o el orden de las ventanas).
    /// </summary>
    public void Show(Region area, IReadOnlyList<nint> sessionWindows, bool topmost)
    {
        EnsureWindow();
        if (!_visible || area != _area || topmost != _topmost)
        {
            if (Native.SetWindowPos(_hwnd, topmost ? Native.HWND_TOPMOST : Native.HWND_NOTOPMOST, area.X, area.Y,
                    area.Width, area.Height, Native.SWP_NOACTIVATE | Native.SWP_SHOWWINDOW) == 0)
                throw new Win32Exception(Marshal.GetLastPInvokeError(), "No se pudo mostrar el fondo de la sesión.");
            _visible = true;
            _area = area;
            _topmost = topmost;
        }
        _zOrder.Clear();
        for (var window = Native.GetTopWindow(0); window != 0 && _zOrder.Count < 4096; window = Native.GetWindow(window, Native.GW_HWNDNEXT))
            _zOrder.Add(window);
        if (BackdropPlan.InsertAfter(_zOrder, sessionWindows, _hwnd) is { } above)
            Native.SetWindowPos(_hwnd, above, 0, 0, 0, 0, Native.SWP_NOMOVE | Native.SWP_NOSIZE | Native.SWP_NOACTIVATE);
    }

    public void Hide()
    {
        if (!_visible)
            return;
        Native.ShowWindow(_hwnd, Native.SW_HIDE);
        _visible = false;
    }

    public void Dispose()
    {
        if (_hwnd == 0)
            return;
        Native.DestroyWindow(_hwnd);
        _hwnd = 0;
        _visible = false;
    }

    private void EnsureWindow()
    {
        if (_hwnd != 0)
            return;
        var instance = Native.GetModuleHandle(null);
        fixed (char* name = ClassName)
        {
            if (!s_registered)
            {
                var windowClass = new Native.WNDCLASSEXW
                {
                    cbSize = (uint)sizeof(Native.WNDCLASSEXW),
                    lpfnWndProc = &WindowProc,
                    hInstance = instance,
                    hbrBackground = Native.GetStockObject(Native.BLACK_BRUSH),
                    lpszClassName = name,
                };
                if (Native.RegisterClassEx(&windowClass) == 0)
                    throw new Win32Exception(Marshal.GetLastPInvokeError(), "No se pudo registrar la ventana de fondo.");
                s_registered = true;
            }
            _hwnd = Native.CreateWindowEx(Native.WS_EX_TOOLWINDOW | Native.WS_EX_NOACTIVATE, name, null, Native.WS_POPUP,
                0, 0, 0, 0, 0, 0, instance, 0);
        }
        if (_hwnd == 0)
            throw new Win32Exception(Marshal.GetLastPInvokeError(), "No se pudo crear la ventana de fondo.");
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    private static nint WindowProc(nint hwnd, uint message, nint wParam, nint lParam) =>
        // Un clic en el fondo no debe quitarle el foco a nadie.
        message == Native.WM_MOUSEACTIVATE ? Native.MA_NOACTIVATE : Native.DefWindowProc(hwnd, message, wParam, lParam);
}
