using System.Runtime.Versioning;
using SplitForge.Core.Windows;

namespace SplitForge.Platform;

/// <summary>
/// Mantiene las ventanas de la sesión por encima de la barra de tareas del monitor principal (opción «Tapar
/// barra de tareas»). No oculta ni modifica la barra: solo vuelve a subir, sin activarlas, las ventanas que
/// SplitForge colocó. Se llama desde el hilo de la interfaz, a ≈2 Hz.
/// </summary>
[SupportedOSPlatform("windows")]
public static unsafe class TaskbarGuard
{
    private const string TaskbarClass = "Shell_TrayWnd";
    private static readonly List<nint> s_zOrder = new(512); // Reutilizadas: comprobar el orden no genera basura.
    private static readonly List<nint> s_overlapping = new(4);
    private static readonly nint[] s_taskbar = new nint[1];

    /// <returns>Cuántas ventanas hubo que volver a subir (0 si la barra ya estaba debajo de las que pisa).</returns>
    public static int KeepAbove(IReadOnlyList<nint> sessionWindows)
    {
        nint taskbar;
        fixed (char* name = TaskbarClass)
            taskbar = Native.FindWindow(name, null);
        if (taskbar == 0 || Native.IsWindowVisible(taskbar) == 0 || Native.GetWindowRect(taskbar, out var bar) == 0)
            return 0;
        // Solo importan las ventanas que la barra pisa: las demás pueden quedar debajo de ella sin que se note.
        s_overlapping.Clear();
        for (var i = 0; i < sessionWindows.Count; i++)
        {
            if (Native.GetWindowRect(sessionWindows[i], out var r) != 0
                && r.Left < bar.Right && bar.Left < r.Right && r.Top < bar.Bottom && bar.Top < r.Bottom)
                s_overlapping.Add(sessionWindows[i]);
        }
        if (s_overlapping.Count == 0)
            return 0;
        s_taskbar[0] = taskbar;
        s_zOrder.Clear();
        for (var window = Native.GetTopWindow(0); window != 0 && s_zOrder.Count < 4096; window = Native.GetWindow(window, Native.GW_HWNDNEXT))
            s_zOrder.Add(window);
        var raise = TaskbarPlan.ToRaise(s_zOrder, s_overlapping, s_taskbar);
        foreach (var hwnd in raise)
            Native.SetWindowPos(hwnd, Native.HWND_TOPMOST, 0, 0, 0, 0, Native.SWP_NOMOVE | Native.SWP_NOSIZE | Native.SWP_NOACTIVATE);
        return raise.Count;
    }
}
