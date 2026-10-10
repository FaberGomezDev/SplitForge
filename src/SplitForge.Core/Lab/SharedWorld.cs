using SplitForge.Core.Input;

namespace SplitForge.Core.Lab;

public sealed record ActorSnapshot(double X, double Y, int Score, int Hits, bool Shield, bool Connected);

public sealed record ShotSnapshot(double X, double Y, int Owner);

/// <summary>Instantánea completa e inmutable de un fotograma del mundo compartido.</summary>
public sealed record WorldSnapshot(
    long Frame,
    ActorSnapshot Player1,
    ActorSnapshot Player2,
    IReadOnlyList<ShotSnapshot> Shots,
    double OrbX,
    double OrbY,
    string Event)
{
    public ActorSnapshot Actor(int player) => player == 1 ? Player1 : Player2;
}

/// <summary>
/// Estado autoritativo de una pequeña arena cooperativa (ADR-003): se simula UNA vez en el
/// proceso principal y los visores solo dibujan. No es un juego comercial ni emula Steam Input.
/// </summary>
public sealed class SharedWorld
{
    public const double MaxStep = 0.05;
    public const int MaxShots = 60;
    private const double Speed = 0.46;

    private readonly Actor[] _actors = [new(0.20, 0.50), new(0.80, 0.50)];
    private readonly List<Shot> _shots = [];
    private double _eventAge;

    public long Frame { get; private set; }
    public double Time { get; private set; }
    public (double X, double Y) Orb { get; private set; } = (0.5, 0.5);
    public string Event { get; private set; } = "Conecta ambos mandos. Acerca las naves al orbe central.";
    public int ShotCount => _shots.Count;

    /// <summary>Acceso para pruebas y escenarios del laboratorio.</summary>
    public Actor this[int player] => _actors[player - 1];

    public WorldSnapshot Step(double dt, PadState? player1, PadState? player2)
    {
        dt = Math.Clamp(dt, 0, MaxStep);
        Time += dt;
        Frame++;
        _eventAge += dt;
        if (_eventAge > 4)
            Event = "Misma arena en las dos mitades: puedes ver al otro jugador.";

        for (var num = 1; num <= 2; num++)
        {
            var actor = _actors[num - 1];
            var pad = num == 1 ? player1 : player2;
            actor.Connected = pad is { Connected: true };
            var xdir = actor.Connected ? Math.Clamp(pad!.Value.LX, -1, 1) : 0.0;
            var ydir = actor.Connected ? -Math.Clamp(pad!.Value.LY, -1, 1) : 0.0;
            var length = Math.Max(1.0, Hypot(xdir, ydir));
            actor.X = Math.Clamp(actor.X + xdir / length * Speed * dt, 0.045, 0.955);
            actor.Y = Math.Clamp(actor.Y + ydir / length * Speed * dt, 0.09, 0.91);
            actor.Shield = pad is { Shield: true };

            var dash = pad is { Dash: true };
            if (dash && !actor.LastDash)
            {
                var (dx, dy) = Math.Abs(xdir) + Math.Abs(ydir) > 0.15 ? (xdir, ydir) : (num == 1 ? 1.0 : -1.0, 0.0);
                var d = Math.Max(1.0, Hypot(dx, dy));
                actor.X = Math.Clamp(actor.X + dx / d * 0.11, 0.045, 0.955);
                actor.Y = Math.Clamp(actor.Y + dy / d * 0.11, 0.09, 0.91);
            }
            actor.LastDash = dash;

            var firing = pad is { Fire: true };
            if (firing && !actor.LastFire)
                _shots.Add(new Shot(actor.X, actor.Y, num == 1 ? 0.67 : -0.67, num));
            actor.LastFire = firing;

            if (Hypot(actor.X - Orb.X, actor.Y - Orb.Y) < 0.07)
            {
                actor.Score++;
                Orb = (0.15 + ((actor.Score * 17 + num * 11) % 70) / 100.0,
                       0.20 + ((actor.Score * 13 + num * 7) % 55) / 100.0);
                SetEvent($"Jugador {num} recogió un núcleo compartido");
            }
        }

        var remaining = new List<Shot>(_shots.Count);
        foreach (var shot in _shots)
        {
            shot.X += shot.Vx * dt;
            var target = 3 - shot.Owner;
            var enemy = _actors[target - 1];
            if (Hypot(enemy.X - shot.X, enemy.Y - shot.Y) < 0.055)
            {
                if (!enemy.Shield)
                {
                    enemy.Hits++;
                    SetEvent($"Jugador {shot.Owner} alcanzó a Jugador {target}");
                }
                else
                {
                    SetEvent($"Jugador {target} bloqueó el disparo con el escudo");
                }
            }
            else if (shot.X is >= 0.01 and <= 0.99)
            {
                remaining.Add(shot);
            }
        }
        _shots.Clear();
        _shots.AddRange(remaining.Take(MaxShots));
        return Snapshot();
    }

    public WorldSnapshot Snapshot() => new(
        Frame,
        ToSnapshot(_actors[0]),
        ToSnapshot(_actors[1]),
        [.. _shots.Select(s => new ShotSnapshot(Math.Round(s.X, 5), Math.Round(s.Y, 5), s.Owner))],
        Math.Round(Orb.X, 5),
        Math.Round(Orb.Y, 5),
        Event);

    private void SetEvent(string text)
    {
        Event = text;
        _eventAge = 0;
    }

    private static ActorSnapshot ToSnapshot(Actor a) =>
        new(Math.Round(a.X, 5), Math.Round(a.Y, 5), a.Score, a.Hits, a.Shield, a.Connected);

    private static double Hypot(double x, double y) => Math.Sqrt(x * x + y * y);

    public sealed class Actor(double x, double y)
    {
        public double X { get; set; } = x;
        public double Y { get; set; } = y;
        public int Score { get; set; }
        public int Hits { get; set; }
        public bool Shield { get; set; }
        public bool Connected { get; set; }
        internal bool LastFire { get; set; }
        internal bool LastDash { get; set; }
    }

    private sealed class Shot(double x, double y, double vx, int owner)
    {
        public double X { get; set; } = x;
        public double Y { get; } = y;
        public double Vx { get; } = vx;
        public int Owner { get; } = owner;
    }
}
