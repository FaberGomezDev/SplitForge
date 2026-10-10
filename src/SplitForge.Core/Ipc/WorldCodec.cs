using System.Buffers.Binary;
using System.Text;
using SplitForge.Core.Lab;

namespace SplitForge.Core.Ipc;

/// <summary>
/// Protocolo binario versionado padre → visor: <c>[u32 longitud][carga]</c>, little-endian.
/// Sustituye a las líneas JSON del prototipo: sin análisis de texto y con límites estrictos.
/// </summary>
public static class WorldCodec
{
    public const uint Magic = 0x31574653; // "SFW1"
    public const ushort Version = 1;
    public const int MaxFrameBytes = 8 * 1024;
    public const int MaxShots = 64;
    public const int MaxEventBytes = 512;

    private const int ActorBytes = 4 + 4 + 4 + 4 + 1;

    public static byte[] Encode(WorldSnapshot s)
    {
        var shots = s.Shots.Count > MaxShots ? s.Shots.Take(MaxShots).ToList() : s.Shots;
        var eventBytes = Utf8Truncate(s.Event, MaxEventBytes);
        var payload = 4 + 2 + 8 + 2 * ActorBytes + 8 + 2 + shots.Count * 9 + 2 + eventBytes.Length;
        var buffer = new byte[4 + payload];
        var span = buffer.AsSpan();
        BinaryPrimitives.WriteInt32LittleEndian(span, payload);
        var w = new SpanWriter(span[4..]);
        w.U32(Magic);
        w.U16(Version);
        w.I64(s.Frame);
        w.Actor(s.Player1);
        w.Actor(s.Player2);
        w.F32(s.OrbX);
        w.F32(s.OrbY);
        w.U16((ushort)shots.Count);
        foreach (var shot in shots)
        {
            w.F32(shot.X);
            w.F32(shot.Y);
            w.U8((byte)shot.Owner);
        }
        w.U16((ushort)eventBytes.Length);
        w.Bytes(eventBytes);
        return buffer;
    }

    /// <summary>Decodifica una carga (sin el prefijo de longitud). Devuelve null si es inválida.</summary>
    public static WorldSnapshot? Decode(ReadOnlySpan<byte> payload)
    {
        try
        {
            var r = new SpanReader(payload);
            if (r.U32() != Magic || r.U16() != Version)
                return null;
            var frame = r.I64();
            var p1 = r.Actor();
            var p2 = r.Actor();
            var orbX = r.Unit();
            var orbY = r.Unit();
            int count = r.U16();
            if (frame < 0 || count > MaxShots || p1 is null || p2 is null || orbX is null || orbY is null)
                return null;
            var shots = new ShotSnapshot[count];
            for (var i = 0; i < count; i++)
            {
                var x = r.Unit();
                var y = r.Unit();
                var owner = r.U8();
                if (x is null || y is null || owner is not (1 or 2))
                    return null;
                shots[i] = new ShotSnapshot(x.Value, y.Value, owner);
            }
            int eventLength = r.U16();
            if (eventLength > MaxEventBytes)
                return null;
            var text = Encoding.UTF8.GetString(r.Bytes(eventLength));
            return r.AtEnd ? new WorldSnapshot(frame, p1, p2, shots, orbX.Value, orbY.Value, text) : null;
        }
        catch (ArgumentOutOfRangeException)
        {
            return null; // Carga truncada.
        }
    }

    private static byte[] Utf8Truncate(string text, int maxBytes)
    {
        var bytes = Encoding.UTF8.GetBytes(text);
        if (bytes.Length <= maxBytes)
            return bytes;
        var cut = maxBytes;
        while (cut > 0 && (bytes[cut] & 0xC0) == 0x80)
            cut--; // No partir un carácter multibyte.
        return bytes[..cut];
    }

    private ref struct SpanWriter(Span<byte> span)
    {
        private readonly Span<byte> _span = span;
        private int _pos;

        public void U8(byte v) => _span[_pos++] = v;
        public void U16(ushort v) { BinaryPrimitives.WriteUInt16LittleEndian(_span[_pos..], v); _pos += 2; }
        public void U32(uint v) { BinaryPrimitives.WriteUInt32LittleEndian(_span[_pos..], v); _pos += 4; }
        public void I32(int v) { BinaryPrimitives.WriteInt32LittleEndian(_span[_pos..], v); _pos += 4; }
        public void I64(long v) { BinaryPrimitives.WriteInt64LittleEndian(_span[_pos..], v); _pos += 8; }
        public void F32(double v) { BinaryPrimitives.WriteSingleLittleEndian(_span[_pos..], (float)v); _pos += 4; }
        public void Bytes(ReadOnlySpan<byte> b) { b.CopyTo(_span[_pos..]); _pos += b.Length; }

        public void Actor(ActorSnapshot a)
        {
            F32(a.X);
            F32(a.Y);
            I32(a.Score);
            I32(a.Hits);
            U8((byte)((a.Shield ? 1 : 0) | (a.Connected ? 2 : 0)));
        }
    }

    private ref struct SpanReader(ReadOnlySpan<byte> span)
    {
        private readonly ReadOnlySpan<byte> _span = span;
        private int _pos;

        public readonly bool AtEnd => _pos == _span.Length;

        public byte U8() => Take(1)[0];
        public ushort U16() => BinaryPrimitives.ReadUInt16LittleEndian(Take(2));
        public uint U32() => BinaryPrimitives.ReadUInt32LittleEndian(Take(4));
        public int I32() => BinaryPrimitives.ReadInt32LittleEndian(Take(4));
        public long I64() => BinaryPrimitives.ReadInt64LittleEndian(Take(8));
        public ReadOnlySpan<byte> Bytes(int n) => Take(n);

        /// <summary>Coordenada normalizada: finita y dentro de [0, 1].</summary>
        public double? Unit()
        {
            var v = BinaryPrimitives.ReadSingleLittleEndian(Take(4));
            return float.IsFinite(v) && v is >= 0f and <= 1f ? v : null;
        }

        public ActorSnapshot? Actor()
        {
            var x = Unit();
            var y = Unit();
            var score = I32();
            var hits = I32();
            var flags = U8();
            if (x is null || y is null || score < 0 || hits < 0 || flags > 3)
                return null;
            return new ActorSnapshot(x.Value, y.Value, score, hits, (flags & 1) != 0, (flags & 2) != 0);
        }

        private ReadOnlySpan<byte> Take(int n)
        {
            if (n < 0 || _pos + n > _span.Length)
                throw new ArgumentOutOfRangeException(nameof(n));
            var slice = _span.Slice(_pos, n);
            _pos += n;
            return slice;
        }
    }
}
