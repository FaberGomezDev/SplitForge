using System.Runtime.Versioning;
using SplitForge.Core.Layout;
using SplitForge.Core.Windows;

namespace SplitForge.Platform;

/// <summary>
/// Operaciones Win32 para un aviso superpuesto propio (el del bloqueo de entrada): que no reciba clics ni tome
/// el foco, y colocarlo por encima de los juegos. Solo actúa sobre ventanas de SplitForge.
/// </summary>
[SupportedOSPlatform("windows")]
public static class OverlayWindow
{
    private const long WS_EX_TRANSPARENT = 0x00000020;

    /// <summary>La ventana deja pasar los clics, nunca se activa y no aparece en la barra de tareas ni en Alt+Tab.</summary>
    public static void MakePassive(nint hwnd)
    {
        var ex = (long)Native.GetWindowLongPtr(hwnd, Native.GWL_EXSTYLE);
        Native.SetWindowLongPtr(hwnd, Native.GWL_EXSTYLE, (nint)(ex | WS_EX_TRANSPARENT | Native.WS_EX_NOACTIVATE | Native.WS_EX_TOOLWINDOW));
    }

    /// <summary>
    /// Centra la ventana arriba de <paramref name="area"/> (píxeles) y la deja por encima de todo sin activarla.
    /// Se puede llamar a menudo: también sirve para volver a subirla si un juego quedó delante.
    /// </summary>
    public static void PlaceTopCenter(nint hwnd, Region area)
    {
        if (hwnd == 0 || Native.GetWindowRect(hwnd, out var rect) == 0)
            return;
        var (x, y) = LockNoticePlan.TopCenter(area, rect.Right - rect.Left, rect.Bottom - rect.Top);
        Native.SetWindowPos(hwnd, Native.HWND_TOPMOST, x, y, 0, 0, Native.SWP_NOSIZE | Native.SWP_NOACTIVATE);
    }
}
