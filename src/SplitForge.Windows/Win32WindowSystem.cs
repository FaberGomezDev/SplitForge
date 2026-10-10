using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using SplitForge.Core.Layout;
using SplitForge.Core.Sessions;
using SplitForge.Core.Windows;

namespace SplitForge.Platform;

/// <summary>Enumera, mueve, quita bordes y restaura ventanas normales. No inyecta ni lee memoria ajena.</summary>
[SupportedOSPlatform("windows")]
public sealed unsafe class Win32WindowSystem : IWindowSystem
{
    private const long FrameStyles = Native.WS_CAPTION | Native.WS_THICKFRAME | Native.WS_SYSMENU
                                     | Native.WS_MINIMIZEBOX | Native.WS_MAXIMIZEBOX;
    private const long FrameExStyles = Native.WS_EX_DLGMODALFRAME | Native.WS_EX_WINDOWEDGE
                                       | Native.WS_EX_CLIENTEDGE | Native.WS_EX_STATICEDGE;

    // Estilos originales de las ventanas a las que SplitForge quitó el marco.
    private readonly Dictionary<nint, (nint Style, nint ExStyle)> _framed = [];

    public IReadOnlyList<WindowInfo> EnumerateWindows()
    {
        var found = new List<WindowInfo>();
        var handle = GCHandle.Alloc(found);
        try
        {
            Native.EnumWindows(&Collect, GCHandle.ToIntPtr(handle));
        }
        finally
        {
            handle.Free();
        }
        return found;
    }

    [UnmanagedCallersOnly]
    private static int Collect(nint hwnd, nint state)
    {
        if (Native.IsWindowVisible(hwnd) == 0)
            return 1;
        var length = Native.GetWindowTextLength(hwnd);
        if (length < 1)
            return 1;
        var buffer = stackalloc char[Math.Min(length + 1, 512)];
        var copied = Native.GetWindowText(hwnd, buffer, Math.Min(length + 1, 512));
        var title = new string(buffer, 0, Math.Max(0, copied)).Trim();
        if (title.Length == 0)
            return 1;
        Native.GetWindowThreadProcessId(hwnd, out var pid);
        var list = (List<WindowInfo>)GCHandle.FromIntPtr(state).Target!;
        list.Add(new WindowInfo(hwnd, (int)pid, title));
        return 1;
    }

    public WindowSnapshot Capture(nint handle)
    {
        EnsureWindow(handle);
        if (Native.GetWindowRect(handle, out var r) == 0)
            throw new InvalidOperationException("No se puede obtener la posición de la ventana.", new Win32Exception(Marshal.GetLastPInvokeError()));
        var (style, ex) = _framed.TryGetValue(handle, out var original)
            ? original
            : (Native.GetWindowLongPtr(handle, Native.GWL_STYLE), Native.GetWindowLongPtr(handle, Native.GWL_EXSTYLE));
        Native.GetWindowThreadProcessId(handle, out var pid);
        return new WindowSnapshot(handle, new Region(r.Left, r.Top, r.Right - r.Left, r.Bottom - r.Top),
            Native.IsZoomed(handle) != 0, Native.IsIconic(handle) != 0, style, ex, (int)pid);
    }

    public void Place(nint handle, Region region, bool borderless, bool topmost = false)
    {
        EnsureWindow(handle);
        Native.ShowWindow(handle, Native.SW_RESTORE); // Si estaba maximizada o minimizada.
        if (borderless)
        {
            if (!_framed.ContainsKey(handle))
            {
                var style = Native.GetWindowLongPtr(handle, Native.GWL_STYLE);
                var ex = Native.GetWindowLongPtr(handle, Native.GWL_EXSTYLE);
                _framed[handle] = (style, ex);
                Native.SetWindowLongPtr(handle, Native.GWL_STYLE, (nint)((long)style & ~FrameStyles));
                Native.SetWindowLongPtr(handle, Native.GWL_EXSTYLE, (nint)((long)ex & ~FrameExStyles));
            }
        }
        else if (_framed.Remove(handle, out var original))
        {
            Native.SetWindowLongPtr(handle, Native.GWL_STYLE, original.Style);
            Native.SetWindowLongPtr(handle, Native.GWL_EXSTYLE, original.ExStyle);
        }
        Move(handle, region, topmost ? Native.HWND_TOPMOST : null);
    }

    public void Restore(WindowSnapshot snapshot)
    {
        if (Native.IsWindow(snapshot.Handle) == 0)
        {
            _framed.Remove(snapshot.Handle);
            return; // El juego ya se cerró: nada que restaurar.
        }
        _framed.Remove(snapshot.Handle);
        Native.SetWindowLongPtr(snapshot.Handle, Native.GWL_STYLE, (nint)snapshot.Style);
        Native.SetWindowLongPtr(snapshot.Handle, Native.GWL_EXSTYLE, (nint)snapshot.ExStyle);
        // El orden «siempre encima» no se restaura con el estilo: hace falta SetWindowPos.
        Move(snapshot.Handle, snapshot.Bounds, (snapshot.ExStyle & Native.WS_EX_TOPMOST) != 0 ? Native.HWND_TOPMOST : Native.HWND_NOTOPMOST);
        if (snapshot.Maximized)
            Native.ShowWindow(snapshot.Handle, Native.SW_MAXIMIZE);
        else if (snapshot.Minimized)
            Native.ShowWindow(snapshot.Handle, Native.SW_MINIMIZE);
    }

    public void RequestClose(nint handle)
    {
        if (Native.IsWindow(handle) != 0)
            Native.PostMessage(handle, Native.WM_CLOSE, 0, 0); // Petición normal, nunca forzada.
    }

    public Region PrimaryWorkArea()
    {
        var monitor = Native.MonitorFromPoint(default, Native.MONITOR_DEFAULTTOPRIMARY);
        var info = new Native.MONITORINFO { cbSize = sizeof(Native.MONITORINFO) };
        if (monitor == 0 || Native.GetMonitorInfo(monitor, ref info) == 0)
            throw new InvalidOperationException("No se pudo leer el monitor principal.");
        var w = info.rcWork;
        return new Region(w.Left, w.Top, w.Right - w.Left, w.Bottom - w.Top);
    }

    public Region PrimaryBounds()
    {
        var monitor = Native.MonitorFromPoint(default, Native.MONITOR_DEFAULTTOPRIMARY);
        var info = new Native.MONITORINFO { cbSize = sizeof(Native.MONITORINFO) };
        if (monitor == 0 || Native.GetMonitorInfo(monitor, ref info) == 0)
            throw new InvalidOperationException("No se pudo leer el monitor principal.");
        var m = info.rcMonitor;
        return new Region(m.Left, m.Top, m.Right - m.Left, m.Bottom - m.Top);
    }

    public int? ProcessId(nint handle)
    {
        if (Native.IsWindow(handle) == 0)
            return null;
        Native.GetWindowThreadProcessId(handle, out var pid);
        return (int)pid;
    }

    /// <summary>Mueve y redimensiona; con <paramref name="zOrder"/> además cambia el orden (p. ej. siempre encima).</summary>
    private static void Move(nint handle, Region region, nint? zOrder = null)
    {
        var flags = Native.SWP_NOACTIVATE | Native.SWP_SHOWWINDOW | Native.SWP_FRAMECHANGED | (zOrder is null ? Native.SWP_NOZORDER : 0);
        if (Native.SetWindowPos(handle, zOrder ?? 0, region.X, region.Y, region.Width, region.Height, flags) == 0)
            throw new InvalidOperationException("Windows impidió mover esta ventana (¿pantalla completa exclusiva o proceso elevado?).",
                new Win32Exception(Marshal.GetLastPInvokeError()));
    }

    private static void EnsureWindow(nint handle)
    {
        if (Native.IsWindow(handle) == 0)
            throw new InvalidOperationException("La ventana seleccionada ya no existe.");
    }
}
