using SplitForge.Core.Input;
using SplitForge.Core.Lab;

namespace SplitForge.Core.Sessions;

public interface IFrameSink
{
    void Send(WorldSnapshot snapshot);
}

/// <summary>
/// Un mundo, dos cámaras: el anfitrión simula y difunde la MISMA instantánea a cada visor.
/// <see cref="Step"/> se llama desde el hilo de simulación; el resto, desde la interfaz.
/// </summary>
public sealed class LabSession
{
    private readonly object _gate = new();
    private readonly SharedWorld _world = new();
    private readonly IReadOnlyList<IFrameSink> _sinks;
    private uint[] _assigned;
    private string[] _status = ["P1: pendiente", "P2: pendiente"];

    public LabSession(uint player1, uint player2, IReadOnlyList<IFrameSink> sinks)
    {
        if (player1 == player2)
            throw new ArgumentException("Cada jugador necesita un mando distinto.");
        _assigned = [player1, player2];
        _sinks = sinks;
    }

    public IReadOnlyList<uint> Assigned
    {
        get { lock (_gate) return [.. _assigned]; }
    }

    public IReadOnlyList<string> StatusLines
    {
        get { lock (_gate) return [.. _status]; }
    }

    public SharedWorld World => _world;

    public WorldSnapshot Step(IReadOnlyDictionary<uint, PadState> states, double dt)
    {
        WorldSnapshot snapshot;
        lock (_gate)
        {
            var pads = new PadState?[2];
            var status = new string[2];
            for (var i = 0; i < 2; i++)
            {
                var id = _assigned[i];
                if (states.TryGetValue(id, out var s))
                {
                    pads[i] = s;
                    status[i] = $"P{i + 1} (ID {id}): X={s.LX:+0.00;-0.00} Y={s.LY:+0.00;-0.00} D={(s.Fire ? "SI" : "NO")}";
                }
                else
                {
                    status[i] = $"P{i + 1} (ID {id}): SIN LECTURA";
                }
            }
            _status = status;
            snapshot = _world.Step(dt, pads[0], pads[1]);
        }
        foreach (var sink in _sinks)
            sink.Send(snapshot);
        return snapshot;
    }

    /// <summary>Reasignación en vivo tras una reconexión; nunca intercambia mandos en silencio.</summary>
    public void Reassign(uint player1, uint player2, IReadOnlySet<uint> connected)
    {
        if (player1 == player2 || !connected.Contains(player1) || !connected.Contains(player2))
            throw new ArgumentException("Selecciona dos mandos diferentes que estén conectados.");
        lock (_gate)
            _assigned = [player1, player2];
    }
}
