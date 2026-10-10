using System.Runtime.Versioning;
using SplitForge.Core.Windows;

namespace SplitForge.Platform;

/// <summary>
/// Foco simulado (EXPERIMENTAL, ADR-020): un hilo propio publica con <c>PostMessageW</c> los mensajes de
/// <see cref="FakeFocusPlan"/> a cada ventana de la sesión. No inyecta en otros procesos ni bloquea si el juego
/// no responde (mensajes publicados, no enviados). Nunca se ejecuta en el hilo de la interfaz.
/// </summary>
[SupportedOSPlatform("windows")]
public sealed class WindowFakeFocus : IDisposable
{
    private readonly AutoResetEvent _wake = new(false);
    private readonly object _gate = new();
    private nint[] _targets = [];
    private int _intervalMs = FakeFocusPlan.DefaultIntervalMs;
    private Thread? _thread;
    private volatile bool _stopping;
    private long _posted;
    private long _failed;

    /// <summary>Ventanas que reciben ahora el foco simulado.</summary>
    public IReadOnlyList<nint> Targets => Volatile.Read(ref _targets);

    /// <summary>Mensajes publicados con éxito desde el inicio.</summary>
    public long Posted => Interlocked.Read(ref _posted);

    /// <summary>Mensajes que Windows rechazó (ventana cerrada, cola llena o privilegios distintos).</summary>
    public long Failed => Interlocked.Read(ref _failed);

    /// <summary>Cambia las ventanas objetivo; con una lista vacía el hilo queda en espera sin publicar nada.</summary>
    public void SetTargets(IReadOnlyList<nint> handles, int intervalMs)
    {
        lock (_gate)
        {
            if (_stopping)
                return;
            Volatile.Write(ref _intervalMs, FakeFocusPlan.ClampInterval(intervalMs));
            var next = handles.Where(h => h != 0).Distinct().ToArray();
            if (next.SequenceEqual(Volatile.Read(ref _targets)))
                return;
            Volatile.Write(ref _targets, next);
            if (next.Length > 0 && _thread is null)
            {
                _thread = new Thread(Loop) { IsBackground = true, Name = "SplitForge.FakeFocus" };
                _thread.Start();
            }
            _wake.Set();
        }
    }

    public void Clear() => SetTargets([], Volatile.Read(ref _intervalMs));

    private void Loop()
    {
        while (!_stopping)
        {
            var targets = Volatile.Read(ref _targets);
            foreach (var hwnd in targets)
            {
                if (Native.IsWindow(hwnd) == 0)
                    continue;
                foreach (var m in FakeFocusPlan.MessagesFor(hwnd))
                {
                    if (Native.PostMessage(hwnd, m.Msg, m.WParam, m.LParam) != 0)
                        Interlocked.Increment(ref _posted);
                    else
                        Interlocked.Increment(ref _failed);
                }
            }
            _wake.WaitOne(targets.Length == 0 ? Timeout.Infinite : Volatile.Read(ref _intervalMs));
        }
    }

    public void Dispose()
    {
        Thread? thread;
        lock (_gate)
        {
            if (_stopping)
                return;
            _stopping = true;
            thread = _thread;
            _wake.Set();
        }
        thread?.Join(TimeSpan.FromSeconds(1));
        if (thread is null || !thread.IsAlive)
            _wake.Dispose();
    }
}
