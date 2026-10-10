using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using SplitForge.Core.Input;

namespace SplitForge.Platform;

/// <summary>
/// Bloqueo de entrada «solo mandos» (técnica del bloqueo de entrada de Nucleus Co-op, sin inyección):
/// ganchos de bajo nivel de teclado y ratón (WH_KEYBOARD_LL / WH_MOUSE_LL) que descartan todo salvo el
/// atajo de desbloqueo, decidido por <see cref="InputLockFilter"/>. Los mandos XInput no pasan por aquí.
/// Debe usarse desde un hilo con bucle de mensajes (el de la interfaz). Ctrl+Alt+Supr nunca se puede
/// bloquear, y Windows retira el gancho por sí solo si la aplicación deja de responder.
/// </summary>
[SupportedOSPlatform("windows")]
public static unsafe partial class InputLock
{
    private const int WH_KEYBOARD_LL = 13;
    private const int WH_MOUSE_LL = 14;
    private const nint WM_KEYDOWN = 0x0100;
    private const nint WM_SYSKEYDOWN = 0x0104;

    private static nint _keyboardHook;
    private static nint _mouseHook;
    private static InputLockFilter? _filter;
    private static Action? _onToggle;

    public static bool IsActive => _keyboardHook != 0;

    /// <summary>
    /// Activa el bloqueo. <paramref name="onToggle"/> se llama (dentro del gancho, que debe volver rápido:
    /// conviene diferir el trabajo) cuando se pulsa el atajo para soltarlo.
    /// </summary>
    public static bool Start(HotkeySpec toggle, Action onToggle)
    {
        if (IsActive)
            return true;
        _filter = new InputLockFilter(toggle);
        _filter.Reset(HeldModifiers());
        _onToggle = onToggle;
        var module = GetModuleHandleW(null);
        _keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, &KeyboardProc, module, 0);
        _mouseHook = SetWindowsHookExW(WH_MOUSE_LL, &MouseProc, module, 0);
        if (_keyboardHook != 0 && _mouseHook != 0)
            return true;
        Stop();
        return false;
    }

    public static void Stop()
    {
        if (_keyboardHook != 0)
            UnhookWindowsHookEx(_keyboardHook);
        if (_mouseHook != 0)
            UnhookWindowsHookEx(_mouseHook);
        _keyboardHook = 0;
        _mouseHook = 0;
        _filter = null;
        _onToggle = null;
    }

    private static HotkeyModifiers HeldModifiers()
    {
        var held = HotkeyModifiers.None;
        if (IsDown(0x11)) held |= HotkeyModifiers.Control;
        if (IsDown(0x12)) held |= HotkeyModifiers.Alt;
        if (IsDown(0x10)) held |= HotkeyModifiers.Shift;
        if (IsDown(0x5B) || IsDown(0x5C)) held |= HotkeyModifiers.Win;
        return held;
    }

    private static bool IsDown(int virtualKey) => (GetAsyncKeyState(virtualKey) & 0x8000) != 0;

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    private static nint KeyboardProc(int code, nint message, nint data)
    {
        if (code >= 0 && _filter is { } filter)
        {
            var info = (KBDLLHOOKSTRUCT*)data;
            switch (filter.OnKey(info->vkCode, message == WM_KEYDOWN || message == WM_SYSKEYDOWN))
            {
                case InputLockDecision.Toggle:
                    _onToggle?.Invoke();
                    return 1;
                case InputLockDecision.Swallow:
                    return 1;
            }
        }
        return CallNextHookEx(0, code, message, data);
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvStdcall)])]
    private static nint MouseProc(int code, nint message, nint data) =>
        code >= 0 && _filter is not null ? 1 : CallNextHookEx(0, code, message, data);

    [StructLayout(LayoutKind.Sequential)]
    private struct KBDLLHOOKSTRUCT
    {
        public uint vkCode;
        public uint scanCode;
        public uint flags;
        public uint time;
        public nuint dwExtraInfo;
    }

    [LibraryImport("user32.dll", SetLastError = true)]
    private static partial nint SetWindowsHookExW(int idHook, delegate* unmanaged[Stdcall]<int, nint, nint, nint> proc, nint module, uint threadId);

    [LibraryImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static partial bool UnhookWindowsHookEx(nint hook);

    [LibraryImport("user32.dll")]
    private static partial nint CallNextHookEx(nint hook, int code, nint wParam, nint lParam);

    [LibraryImport("user32.dll")]
    private static partial short GetAsyncKeyState(int virtualKey);

    [LibraryImport("kernel32.dll", StringMarshalling = StringMarshalling.Utf16)]
    private static partial nint GetModuleHandleW(string? moduleName);
}
