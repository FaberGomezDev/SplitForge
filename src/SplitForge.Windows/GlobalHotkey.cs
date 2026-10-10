using System.Runtime.Versioning;

namespace SplitForge.Platform;

/// <summary>Registro de atajos de teclado globales (Win32 RegisterHotKey). La interfaz los enruta por su HWND.</summary>
[SupportedOSPlatform("windows")]
public static class GlobalHotkey
{
    /// <summary>Evita repeticiones mientras se mantiene pulsado (MOD_NOREPEAT).</summary>
    public const uint NoRepeat = 0x4000;

    /// <summary>Mensaje WM_HOTKEY que llega a la ventana.</summary>
    public const int WmHotkey = 0x0312;

    public static bool Register(nint hwnd, int id, uint modifiers, uint virtualKey) =>
        Native.RegisterHotKey(hwnd, id, modifiers | NoRepeat, virtualKey) != 0;

    public static void Unregister(nint hwnd, int id) => Native.UnregisterHotKey(hwnd, id);
}
