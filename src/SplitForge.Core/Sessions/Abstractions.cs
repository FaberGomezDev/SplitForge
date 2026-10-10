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
    /// <summary>
    /// Mueve la ventana; con <paramref name="borderless"/> quita marco y título, y con <paramref name="topmost"/>
    /// la pone por encima de la barra de tareas. Todo reversible con <see cref="Restore"/>.
    /// </summary>
    void Place(nint handle, Region region, bool borderless, bool topmost = false);
    void Restore(WindowSnapshot snapshot);
    /// <summary>Petición de cierre normal (WM_CLOSE). Nunca fuerza.</summary>
    void RequestClose(nint handle);
    /// <summary>Área útil del monitor principal (sin barra de tareas), en píxeles físicos.</summary>
    Region PrimaryWorkArea();

    /// <summary>Monitor principal completo (incluida la zona de la barra de tareas), en píxeles físicos.</summary>
    Region PrimaryBounds();

    /// <summary>PID dueño de la ventana, o null si ya no existe.</summary>
    int? ProcessId(nint handle);
}

public sealed record LaunchSpec(
    string FileName,
    IReadOnlyList<string> Arguments,
    string? WorkingDirectory = null,
    bool RedirectInput = false,
    bool NoWindow = false,
    IReadOnlyDictionary<string, string>? EnvironmentVariables = null);

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
        if (spec.EnvironmentVariables is not null)
        {
            foreach (var (key, value) in spec.EnvironmentVariables)
                info.Environment[key] = value;
        }
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

/// <param name="CoverTaskbar">
/// Usa el monitor completo y coloca las ventanas por encima de la barra de tareas (sesión de juego sin
/// barra visible). Al detener se restaura el orden original de las ventanas.
/// </param>
/// <param name="MinAspect">
/// Proporción mínima que exige el juego (p. ej. 16:9 en 7DTD, cuya interfaz se corta en ventanas más
/// estrechas). Las regiones más estrechas se reducen a esa proporción, centradas; sin valor, no se aplica.
/// </param>
public sealed record LayoutOptions(
    SplitOrientation Orientation,
    int Gutter,
    bool Borderless,
    WindowFit Fit = WindowFit.Rellenar,
    AspectRatio Aspect = default,
    AspectRatio MinAspect = default,
    bool CoverTaskbar = false)
{
    /// <summary>Proporción efectiva: la elegida o 16:9 si no se fijó una válida.</summary>
    public AspectRatio EffectiveAspect => Aspect.IsValid ? Aspect : AspectRatio.SixteenNine;

    /// <summary>Rectángulo final de cada ventana: distribución, ajuste elegido y proporción mínima del juego.</summary>
    public IReadOnlyList<Region> Regions(Region area, int players)
    {
        var regions = SplitLayout.ApplyFit(SplitLayout.Compute(area, players, Orientation, Gutter), Fit, EffectiveAspect);
        return MinAspect.IsValid ? SplitLayout.EnsureMinAspect(regions, MinAspect) : regions;
    }
}
