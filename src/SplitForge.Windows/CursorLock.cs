using System.Runtime.Versioning;

namespace SplitForge.Platform;

/// <summary>
/// Confina el ratón al área de cliente de una ventana de juego (EXPERIMENTAL), para que en pantalla
/// dividida el cursor no cruce a la otra mitad. No inyecta en otros procesos: solo usa ClipCursor, que
/// Windows reinicia al cambiar el foco, así que la interfaz lo vuelve a aplicar periódicamente.
/// </summary>
[SupportedOSPlatform("windows")]
public static unsafe class CursorLock
{
    public static nint ForegroundWindow() => Native.GetForegroundWindow();

    /// <summary>Confina el cursor al área de cliente de <paramref name="hwnd"/>. Devuelve false si no se pudo.</summary>
    public static bool ClipTo(nint hwnd)
    {
        if (hwnd == 0 || Native.IsWindow(hwnd) == 0 || Native.GetClientRect(hwnd, out var client) == 0)
            return false;
        var topLeft = new Native.POINT { X = client.Left, Y = client.Top };
        var bottomRight = new Native.POINT { X = client.Right, Y = client.Bottom };
        if (Native.ClientToScreen(hwnd, ref topLeft) == 0 || Native.ClientToScreen(hwnd, ref bottomRight) == 0)
            return false;
        if (bottomRight.X - topLeft.X < 2 || bottomRight.Y - topLeft.Y < 2)
            return false; // Ventana minimizada o sin área útil.
        var rect = new Native.RECT { Left = topLeft.X, Top = topLeft.Y, Right = bottomRight.X, Bottom = bottomRight.Y };
        return Native.ClipCursor(&rect) != 0;
    }

    /// <summary>Libera el confinamiento del cursor.</summary>
    public static void Release() => Native.ClipCursor(null);
}
