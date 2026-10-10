using System.IO.Pipes;
using SplitForge.Core.Input;
using SplitForge.Core.Ipc;
using SplitForge.Core.Lab;
using SplitForge.Core.Sessions;

namespace SplitForge.Tests;

public class SharedWorldTests
{
    [Fact]
    public void TwoUsersInSameSnapshot()
    {
        var world = new SharedWorld();
        var before = world.Snapshot();
        var after = world.Step(0.05, new PadState(10, 1, 0), new PadState(20, -1, 0));
        Assert.True(after.Player1.X > before.Player1.X);
        Assert.True(after.Player2.X < before.Player2.X);
        Assert.True(after.Player1.Connected && after.Player2.Connected);
        Assert.NotNull(WorldCodec.Decode(WorldCodec.Encode(after).AsSpan(4)));
    }

    [Fact]
    public void FireIsEdgeTriggeredAndShared()
    {
        var world = new SharedWorld();
        for (var i = 0; i < 5; i++)
            world.Step(0.016, new PadState(101, Fire: true), new PadState(102));
        Assert.Equal(1, world.ShotCount);
        Assert.Equal(1, world.Snapshot().Shots[0].Owner);
        world.Step(0.016, new PadState(101), new PadState(102));
        world.Step(0.016, new PadState(101, Fire: true), new PadState(102));
        Assert.Equal(2, world.ShotCount);
    }

    [Fact]
    public void UnplugStopsOneAndNotOther()
    {
        var world = new SharedWorld();
        var first = world.Step(0.02, new PadState(1, 0.8f), new PadState(2, -0.8f));
        var second = world.Step(0.02, null, new PadState(2, -0.8f));
        Assert.Equal(first.Player1.X, second.Player1.X);
        Assert.True(second.Player2.X < first.Player2.X);
        Assert.False(second.Player1.Connected);
    }

    [Fact]
    public void StepClampsUntrustedDt()
    {
        var after = new SharedWorld().Step(10_000, new PadState(1, 1), null);
        Assert.True(after.Player1.X < 0.25);
    }

    [Fact]
    public void ShieldBlocksDamage()
    {
        var world = new SharedWorld();
        world[2].X = 0.27;
        world[2].Y = 0.50;
        world.Step(0.01, new PadState(1, Fire: true), new PadState(2, Shield: true));
        for (var i = 0; i < 10; i++)
            world.Step(0.01, new PadState(1), new PadState(2, Shield: true));
        Assert.Equal(0, world[2].Hits);
    }

    [Fact]
    public void UnshieldedHitCounts()
    {
        var world = new SharedWorld();
        world[2].X = 0.27;
        world[2].Y = 0.50;
        world.Step(0.01, new PadState(1, Fire: true), new PadState(2));
        for (var i = 0; i < 10; i++)
            world.Step(0.01, new PadState(1), new PadState(2));
        Assert.Equal(1, world[2].Hits);
    }
}

public class WorldCodecTests
{
    private static WorldSnapshot Sample() => new SharedWorld().Step(0.016, new PadState(1, 0.5f, 0, Fire: true), new PadState(2, Shield: true));

    [Fact]
    public void RoundTrip()
    {
        var s = Sample() with { Event = "Jugador 1 recogió un núcleo compartido ñ" };
        var frame = WorldCodec.Encode(s);
        Assert.Equal(frame.Length - 4, BitConverter.ToInt32(frame, 0));
        var d = WorldCodec.Decode(frame.AsSpan(4));
        Assert.NotNull(d);
        Assert.Equal(s.Frame, d.Frame);
        Assert.Equal(s.Event, d.Event);
        Assert.Equal(s.Player2.Shield, d.Player2.Shield);
        Assert.Equal(s.Shots.Count, d.Shots.Count);
        Assert.Equal(s.Player1.X, d.Player1.X, 5);
    }

    [Fact]
    public void RejectsCorruptedPayloads()
    {
        var frame = WorldCodec.Encode(Sample());
        Assert.Null(WorldCodec.Decode(frame.AsSpan(4, 10)));      // truncado
        var badMagic = frame[4..];
        badMagic[0] ^= 0xFF;
        Assert.Null(WorldCodec.Decode(badMagic));
        var extra = frame[4..].Concat(new byte[] { 1 }).ToArray();
        Assert.Null(WorldCodec.Decode(extra));                    // bytes sobrantes
        var outOfRange = frame[4..];
        BitConverter.GetBytes(5f).CopyTo(outOfRange, 14);         // X de P1 fuera de [0,1]
        Assert.Null(WorldCodec.Decode(outOfRange));
        var nan = frame[4..];
        BitConverter.GetBytes(float.NaN).CopyTo(nan, 14);
        Assert.Null(WorldCodec.Decode(nan));
        Assert.Null(WorldCodec.Decode([]));
    }

    [Fact]
    public void LongEventIsTruncatedOnCharacterBoundary()
    {
        var s = Sample() with { Event = new string('ñ', 600) };
        var d = WorldCodec.Decode(WorldCodec.Encode(s).AsSpan(4));
        Assert.NotNull(d);
        Assert.True(System.Text.Encoding.UTF8.GetByteCount(d.Event) <= WorldCodec.MaxEventBytes);
        Assert.DoesNotContain('�', d.Event);
    }

    [Fact]
    public void LatestOnlyBuffer()
    {
        using var sink = new BlockingStream();
        using var writer = new LatestFrameWriter(sink);
        sink.Gate.Reset(); // El "visor" deja de leer.
        writer.Send(new byte[] { 0 });
        Thread.Sleep(50);
        for (var i = 1; i <= 9; i++)
            writer.Send(new[] { (byte)i });
        Assert.Equal((byte)9, writer.PeekPending()![0]);
        sink.Gate.Set();
    }

    [Fact]
    public void PipeEndToEnd()
    {
        var server = new AnonymousPipeServerStream(PipeDirection.Out);
        var client = new AnonymousPipeClientStream(PipeDirection.In, server.ClientSafePipeHandle);
        var writer = new LatestFrameWriter(server);
        var reader = new FrameReader(client);
        var expected = Sample();
        writer.Send(expected);
        WorldSnapshot? got = null;
        var deadline = DateTime.UtcNow.AddSeconds(2);
        while (got is null && DateTime.UtcNow < deadline)
        {
            got = reader.Latest(TimeSpan.FromSeconds(1));
            Thread.Sleep(5);
        }
        Assert.NotNull(got);
        Assert.Equal(expected.Frame, got.Frame);
        Assert.Equal(expected.Player1.X, got.Player1.X, 5);

        // El anfitrión cierra su extremo: el visor detecta el fin del canal.
        writer.Dispose();
        server.Dispose();
        deadline = DateTime.UtcNow.AddSeconds(2);
        while (!reader.Ended && DateTime.UtcNow < deadline)
            Thread.Sleep(5);
        Assert.True(reader.Ended);
        reader.Dispose();
    }

    private sealed class BlockingStream : Stream
    {
        public ManualResetEventSlim Gate { get; } = new(true);
        public override bool CanRead => false;
        public override bool CanSeek => false;
        public override bool CanWrite => true;
        public override long Length => 0;
        public override long Position { get; set; }
        public override void Flush() { }
        public override int Read(byte[] buffer, int offset, int count) => throw new NotSupportedException();
        public override long Seek(long offset, SeekOrigin origin) => throw new NotSupportedException();
        public override void SetLength(long value) => throw new NotSupportedException();
        public override void Write(byte[] buffer, int offset, int count) => Gate.Wait(TimeSpan.FromSeconds(5));
    }
}

public class LabSessionTests
{
    private sealed class RecordingSink : IFrameSink
    {
        public List<WorldSnapshot> Frames { get; } = [];
        public void Send(WorldSnapshot snapshot) => Frames.Add(snapshot);
    }

    [Fact]
    public void NoSpoofingOfSecondPlayerAndOneWorld()
    {
        var a = new RecordingSink();
        var b = new RecordingSink();
        var lab = new LabSession(101, 202, [a, b]);
        lab.Step(new Dictionary<uint, PadState> { [101] = new(101, 0.8f), [202] = new(202, -0.8f) }, 0.05);
        Assert.Single(a.Frames);
        Assert.Same(a.Frames[0], b.Frames[0]); // un solo mundo
        Assert.NotEqual(a.Frames[0].Player1.X, a.Frames[0].Player2.X);
        Assert.Contains("ID 101", lab.StatusLines[0]);
        Assert.Contains("ID 202", lab.StatusLines[1]);

        lab.Reassign(202, 101, new HashSet<uint> { 101, 202 });
        lab.Step(new Dictionary<uint, PadState> { [101] = new(101, 1), [202] = new(202, -1) }, 0.05);
        Assert.Contains("ID 202", lab.StatusLines[0]);
        Assert.Throws<ArgumentException>(() => lab.Reassign(101, 101, new HashSet<uint> { 101, 202 }));
        Assert.Throws<ArgumentException>(() => lab.Reassign(101, 303, new HashSet<uint> { 101, 202 }));
    }

    [Fact]
    public void MissingPadIsReportedNotSwapped()
    {
        var lab = new LabSession(1, 2, []);
        lab.Step(new Dictionary<uint, PadState> { [1] = new(1) }, 0.01);
        Assert.Contains("SIN LECTURA", lab.StatusLines[1]);
        Assert.False(lab.World.Snapshot().Player2.Connected);
    }

    [Fact]
    public void SamePadForBothPlayersRejected() =>
        Assert.Throws<ArgumentException>(() => new LabSession(5, 5, []));
}

public class FixedRateLoopTests
{
    [Fact]
    public void TicksAndStops()
    {
        var count = 0;
        using (var loop = new FixedRateLoop("test", 500, _ => Interlocked.Increment(ref count)))
        {
            var deadline = DateTime.UtcNow.AddSeconds(2);
            while (Volatile.Read(ref count) < 5 && DateTime.UtcNow < deadline)
                Thread.Sleep(5);
        }
        var stopped = Volatile.Read(ref count);
        Assert.True(stopped >= 5);
        Thread.Sleep(50);
        Assert.True(Volatile.Read(ref count) <= stopped + 1);
    }

    [Fact]
    public void ErrorsAreReportedAndLoopContinues()
    {
        var errors = 0;
        var ticks = 0;
        using (new FixedRateLoop("test", 500, _ =>
               {
                   if (Interlocked.Increment(ref ticks) == 1)
                       throw new InvalidOperationException("fallo");
               }, onError: _ => Interlocked.Increment(ref errors)))
        {
            var deadline = DateTime.UtcNow.AddSeconds(2);
            while (Volatile.Read(ref ticks) < 3 && DateTime.UtcNow < deadline)
                Thread.Sleep(5);
        }
        Assert.Equal(1, errors);
        Assert.True(ticks >= 3);
    }
}
