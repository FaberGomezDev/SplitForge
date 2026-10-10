using System.Diagnostics;
using SplitForge.Core.Layout;
using SplitForge.Core.Windows;

namespace SplitForge.Core.Sessions;

/// <summary>Posición, estado y estilo originales de una ventana, para restaurarla al detener.</summary>
/// <remarks>El PID permite comprobar, tras un cierre inesperado, que el handle sigue siendo la misma ventana.</remarks>
public sealed record WindowSnapshot(nint Handle, Region Bounds, bool Maximized, bool Minimized, long Style, long ExStyle, int Pid = 0);

/// <summary>Operaciones de ventana que SplitForge necesita. Win32 en Windows; falsas en pruebas.</summary>
public interface IWindowSystem
{
    IReadOnlyList<WindowInfo> EnumerateWindows();
    WindowSnapshot Capture(nint handle);
    /// <summary>Mueve la ventana; con <paramref name="borderless"/> quita marco y título (reversible).</summary>
    void Place(nint handle, Region region, bool borderless);
    void Restore(WindowSnapshot snapshot);
    /// <summary>Petición de cierre normal (WM_CLOSE). Nunca fuerza.</summary>
    void RequestClose(nint handle);
    /// <summary>Área útil del monitor principal (sin barra de tareas), en píxeles físicos.</summary>
    Region PrimaryWorkArea();

    /// <summary>PID dueño de la ventana, o null si ya no existe.</summary>
    int? ProcessId(nint handle);
}

public sealed record LaunchSpec(
    string FileName,
    IReadOnlyList<string> Arguments,
    string? WorkingDirectory = null,
    bool RedirectInput = false,
    bool NoWindow = false);

public interface IChildProcess : IDisposable
{
    int Id { get; }
    bool HasExited { get; }
    int? ExitCode { get; }
    Stream? Input { get; }
    void Kill();
}

public interface IProcessLauncher
{
    IChildProcess Start(LaunchSpec spec);
}

/// <summary>Lanzador real: sin shell y con lista de argumentos (sin concatenar texto externo).</summary>
public sealed class SystemProcessLauncher : IProcessLauncher
{
    public IChildProcess Start(LaunchSpec spec)
    {
        var info = new ProcessStartInfo(spec.FileName)
        {
            UseShellExecute = false,
            RedirectStandardInput = spec.RedirectInput,
            CreateNoWindow = spec.NoWindow,
            WorkingDirectory = spec.WorkingDirectory ?? Path.GetDirectoryName(spec.FileName) ?? Environment.CurrentDirectory,
        };
        foreach (var arg in spec.Arguments)
            info.ArgumentList.Add(arg);
        var process = Process.Start(info) ?? throw new InvalidOperationException($"No se pudo iniciar {Path.GetFileName(spec.FileName)}.");
        return new SystemChildProcess(process, spec.RedirectInput);
    }

    private sealed class SystemChildProcess(Process process, bool redirected) : IChildProcess
    {
        public int Id { get; } = process.Id;
        public bool HasExited => process.HasExited;
        public int? ExitCode => process.HasExited ? process.ExitCode : null;
        public Stream? Input { get; } = redirected ? process.StandardInput.BaseStream : null;

        public void Kill()
        {
            if (!process.HasExited)
                process.Kill(entireProcessTree: false);
        }

        public void Dispose() => process.Dispose();
    }
}

public sealed record LayoutOptions(
    SplitOrientation Orientation,
    int Gutter,
    bool Borderless,
    WindowFit Fit = WindowFit.Rellenar,
    AspectRatio Aspect = default)
{
    /// <summary>Proporción efectiva: la elegida o 16:9 si no se fijó una válida.</summary>
    public AspectRatio EffectiveAspect => Aspect.IsValid ? Aspect : AspectRatio.SixteenNine;
}
