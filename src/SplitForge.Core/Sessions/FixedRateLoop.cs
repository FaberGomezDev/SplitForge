using System.Diagnostics;

namespace SplitForge.Core.Sessions;

/// <summary>
/// Bucle de frecuencia fija en un hilo propio, independiente de la interfaz. El temporizador se
/// inyecta: en Windows se usa uno de alta resolución para no depender del tick de 15,6 ms.
/// </summary>
public sealed class FixedRateLoop : IDisposable
{
    private readonly Action<double> _tick;
    private readonly Action<TimeSpan> _sleep;
    private readonly Action<Exception>? _onError;
    private readonly TimeSpan _period;
    private readonly Thread _thread;
    private volatile bool _stop;

    public FixedRateLoop(string name, double hz, Action<double> tick, Action<TimeSpan>? sleep = null,
        Action<Exception>? onError = null, ThreadPriority priority = ThreadPriority.AboveNormal)
    {
        if (hz is <= 0 or > 10_000)
            throw new ArgumentOutOfRangeException(nameof(hz));
        _tick = tick;
        _sleep = sleep ?? Thread.Sleep;
        _onError = onError;
        _period = TimeSpan.FromSeconds(1 / hz);
        _thread = new Thread(Run) { IsBackground = true, Name = name, Priority = priority };
        _thread.Start();
    }

    public long Ticks { get; private set; }

    private void Run()
    {
        var last = Stopwatch.GetTimestamp();
        var next = last;
        while (!_stop)
        {
            var now = Stopwatch.GetTimestamp();
            var dt = Stopwatch.GetElapsedTime(last, now).TotalSeconds;
            last = now;
            try
            {
                _tick(dt);
            }
            catch (Exception ex) when (_onError is not null)
            {
                _onError(ex);
            }
            Ticks++;
            next += (long)(_period.TotalSeconds * Stopwatch.Frequency);
            var remaining = Stopwatch.GetElapsedTime(Stopwatch.GetTimestamp(), next);
            if (remaining > TimeSpan.Zero)
                _sleep(remaining);
            else if (-remaining > _period * 5)
                next = Stopwatch.GetTimestamp(); // Muy atrasado (equipo cargado): resincronizar sin ráfagas.
        }
    }

    public void Dispose()
    {
        _stop = true;
        if (Thread.CurrentThread != _thread)
            _thread.Join(TimeSpan.FromMilliseconds(250));
    }
}
