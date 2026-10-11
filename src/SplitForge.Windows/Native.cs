using System.Runtime.InteropServices;
using System.Runtime.Versioning;

namespace SplitForge.Platform;

/// <summary>
/// Declaraciones Win32 generadas en compilación (LibraryImport, sin marshalling en tiempo de ejecución).
/// Solo consultas y operaciones de ventana normales: sin inyección, sin escritura en memoria ajena.
/// </summary>
[SupportedOSPlatform("windows")]
internal static unsafe partial class Native
{
    public const int GWL_STYLE = -16;
    public const int GWL_EXSTYLE = -20;
    public const long WS_CAPTION = 0x00C00000;
    public const long WS_THICKFRAME = 0x00040000;
    public const long WS_SYSMENU = 0x00080000;
    public const long WS_MINIMIZEBOX = 0x00020000;
    public const long WS_MAXIMIZEBOX = 0x00010000;
    public const long WS_EX_DLGMODALFRAME = 0x00000001;
    public const long WS_EX_WINDOWEDGE = 0x00000100;
    public const long WS_EX_CLIENTEDGE = 0x00000200;
    public const long WS_EX_STATICEDGE = 0x00020000;
    public const uint SWP_NOZORDER = 0x0004;
    public const uint SWP_NOACTIVATE = 0x0010;
    public const uint SWP_FRAMECHANGED = 0x0020;
    public const uint SWP_SHOWWINDOW = 0x0040;
    public const uint SWP_NOSIZE = 0x0001;
    public const uint SWP_NOMOVE = 0x0002;
    public const nint HWND_TOPMOST = -1;
    public const nint HWND_NOTOPMOST = -2;
    public const long WS_EX_TOPMOST = 0x00000008;
    public const int SW_MAXIMIZE = 3;
    public const int SW_MINIMIZE = 6;
    public const int SW_RESTORE = 9;
    public const uint WM_CLOSE = 0x0010;
    public const uint MONITOR_DEFAULTTOPRIMARY = 1;
    public const uint PROCESS_QUERY_INFORMATION = 0x0400;
    public const uint PROCESS_VM_READ = 0x0010;
    public const uint PROCESS_QUERY_LIMITED_INFORMATION = 0x1000;
    public const uint LIST_MODULES_ALL = 0x03;
    public const uint CREATE_WAITABLE_TIMER_HIGH_RESOLUTION = 0x00000002;
    public const uint TIMER_ALL_ACCESS = 0x1F0003;
    public const uint INFINITE = 0xFFFFFFFF;

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int Left, Top, Right, Bottom;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct POINT
    {
        public int X, Y;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MONITORINFO
    {
        public int cbSize;
        public RECT rcMonitor;
        public RECT rcWork;
        public uint dwFlags;
    }

    // user32
    [LibraryImport("user32.dll")]
    public static partial int EnumWindows(delegate* unmanaged<nint, nint, int> callback, nint lParam);

    [LibraryImport("user32.dll", EntryPoint = "GetWindowTextLengthW")]
    public static partial int GetWindowTextLength(nint hwnd);

    [LibraryImport("user32.dll", EntryPoint = "GetWindowTextW")]
    public static partial int GetWindowText(nint hwnd, char* buffer, int maxCount);

    [LibraryImport("user32.dll")]
    public static partial uint GetWindowThreadProcessId(nint hwnd, out uint processId);

    [LibraryImport("user32.dll")]
    public static partial int IsWindowVisible(nint hwnd);

    [LibraryImport("user32.dll")]
    public static partial int IsWindow(nint hwnd);

    [LibraryImport("user32.dll")]
    public static partial int IsZoomed(nint hwnd);

    [LibraryImport("user32.dll")]
    public static partial int IsIconic(nint hwnd);

    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial int GetWindowRect(nint hwnd, out RECT rect);

    [LibraryImport("user32.dll")]
    public static partial int ShowWindow(nint hwnd, int command);

    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial int SetWindowPos(nint hwnd, nint insertAfter, int x, int y, int cx, int cy, uint flags);

    [LibraryImport("user32.dll", EntryPoint = "PostMessageW")]
    public static partial int PostMessage(nint hwnd, uint msg, nint wParam, nint lParam);

    // Ventana propia mínima (fondo negro de la sesión).
    public const uint WS_POPUP = 0x80000000;
    public const uint WS_EX_TOOLWINDOW = 0x00000080;
    public const uint WS_EX_NOACTIVATE = 0x08000000;
    public const uint WM_MOUSEACTIVATE = 0x0021;
    public const nint MA_NOACTIVATE = 3;
    public const int BLACK_BRUSH = 4;
    public const uint GW_HWNDNEXT = 2;
    public const int SW_HIDE = 0;

    [StructLayout(LayoutKind.Sequential)]
    public struct WNDCLASSEXW
    {
        public uint cbSize;
        public uint style;
        public delegate* unmanaged[Stdcall]<nint, uint, nint, nint, nint> lpfnWndProc;
        public int cbClsExtra;
        public int cbWndExtra;
        public nint hInstance;
        public nint hIcon;
        public nint hCursor;
        public nint hbrBackground;
        public char* lpszMenuName;
        public char* lpszClassName;
        public nint hIconSm;
    }

    [LibraryImport("user32.dll", EntryPoint = "RegisterClassExW", SetLastError = true)]
    public static partial ushort RegisterClassEx(WNDCLASSEXW* windowClass);

    [LibraryImport("user32.dll", EntryPoint = "CreateWindowExW", SetLastError = true)]
    public static partial nint CreateWindowEx(uint exStyle, char* className, char* windowName, uint style,
        int x, int y, int width, int height, nint parent, nint menu, nint instance, nint param);

    [LibraryImport("user32.dll", EntryPoint = "DefWindowProcW")]
    public static partial nint DefWindowProc(nint hwnd, uint msg, nint wParam, nint lParam);

    [LibraryImport("user32.dll")]
    public static partial int DestroyWindow(nint hwnd);

    [LibraryImport("user32.dll")]
    public static partial nint GetTopWindow(nint hwnd);

    [LibraryImport("user32.dll")]
    public static partial nint GetWindow(nint hwnd, uint command);

    [LibraryImport("gdi32.dll")]
    public static partial nint GetStockObject(int index);

    [LibraryImport("kernel32.dll", EntryPoint = "GetModuleHandleW")]
    public static partial nint GetModuleHandle(char* moduleName);

    [LibraryImport("user32.dll", EntryPoint = "GetWindowLongPtrW", SetLastError = true)]
    public static partial nint GetWindowLongPtr(nint hwnd, int index);

    [LibraryImport("user32.dll", EntryPoint = "SetWindowLongPtrW", SetLastError = true)]
    public static partial nint SetWindowLongPtr(nint hwnd, int index, nint value);

    [LibraryImport("user32.dll")]
    public static partial nint MonitorFromPoint(POINT point, uint flags);

    [LibraryImport("user32.dll", EntryPoint = "GetMonitorInfoW")]
    public static partial int GetMonitorInfo(nint monitor, ref MONITORINFO info);

    [LibraryImport("user32.dll")]
    public static partial int SetProcessDpiAwarenessContext(nint value);

    [LibraryImport("user32.dll")]
    public static partial nint GetForegroundWindow();

    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial int GetClientRect(nint hwnd, out RECT rect);

    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial int ClientToScreen(nint hwnd, ref POINT point);

    // ClipCursor(NULL) libera el confinamiento; con un RECT lo fija.
    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial int ClipCursor(RECT* rect);

    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial int RegisterHotKey(nint hwnd, int id, uint modifiers, uint virtualKey);

    [LibraryImport("user32.dll", SetLastError = true)]
    public static partial int UnregisterHotKey(nint hwnd, int id);

    // kernel32
    [LibraryImport("kernel32.dll", SetLastError = true)]
    public static partial nint OpenProcess(uint access, int inherit, uint processId);

    [LibraryImport("kernel32.dll")]
    public static partial int CloseHandle(nint handle);

    [LibraryImport("kernel32.dll", EntryPoint = "QueryFullProcessImageNameW", SetLastError = true)]
    public static partial int QueryFullProcessImageName(nint process, uint flags, char* buffer, ref int size);

    [LibraryImport("kernel32.dll", EntryPoint = "K32EnumProcesses", SetLastError = true)]
    public static partial int EnumProcesses(uint* processIds, uint bytes, out uint bytesNeeded);

    [LibraryImport("kernel32.dll", EntryPoint = "K32EnumProcessModulesEx", SetLastError = true)]
    public static partial int EnumProcessModulesEx(nint process, nint* modules, uint bytes, out uint bytesNeeded, uint filter);

    [LibraryImport("kernel32.dll", EntryPoint = "K32GetModuleBaseNameW", SetLastError = true)]
    public static partial uint GetModuleBaseName(nint process, nint module, char* name, uint size);

    [LibraryImport("kernel32.dll", EntryPoint = "K32GetModuleFileNameExW", SetLastError = true)]
    public static partial uint GetModuleFileNameEx(nint process, nint module, char* name, uint size);

    [LibraryImport("kernel32.dll", EntryPoint = "CreateWaitableTimerExW", SetLastError = true)]
    public static partial nint CreateWaitableTimerEx(nint attributes, nint name, uint flags, uint access);

    [LibraryImport("kernel32.dll", SetLastError = true)]
    public static partial int SetWaitableTimer(nint timer, in long dueTime, int period, nint completion, nint arg, int resume);

    [LibraryImport("kernel32.dll")]
    public static partial uint WaitForSingleObject(nint handle, uint milliseconds);

    // xinput1_4 (System32, Windows 8+): solo lectura de estado para identificar ranuras.
    [StructLayout(LayoutKind.Sequential)]
    public struct XInputState
    {
        public uint PacketNumber;
        public ushort Buttons;
        public byte LeftTrigger;
        public byte RightTrigger;
        public short ThumbLX;
        public short ThumbLY;
        public short ThumbRX;
        public short ThumbRY;
    }

    [LibraryImport("xinput1_4.dll", EntryPoint = "XInputGetState")]
    public static partial uint XInputGetState(uint userIndex, out XInputState state);
}
