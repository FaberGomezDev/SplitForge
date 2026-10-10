using System.Buffers.Binary;
using System.Diagnostics;
using SplitForge.Core.Lab;
using SplitForge.Core.Sessions;

namespace SplitForge.Core.Ipc;

/// <summary>
/// Escritor de "último valor gana": nunca bloquea al productor. Si el visor se retrasa,
/// los fotogramas viejos se descartan en lugar de acumular latencia (capacidad 1).
/// </summary>
public sealed class LatestFrameWriter : IFrameSink, IDisposable
{
    private readonly Stream _stream;
    private readonly Thread _thread;
    private readonly AutoResetEvent _signal = new(false);
    private byte[]? _pending;
    private volatile bool _closed;

    public LatestFrameWriter(Stream stream, string name = "SplitForge.FrameWriter")
    {
        _stream = stream;
        _thread = new Thread(Run) { IsBackground = true, Name = name };
        _thread.Start();
    }

    public bool IsClosed => _closed;

    public void Send(byte[] frame)
    {
        if (_closed)
            return;
        Interlocked.Exchange(ref _pending, frame);
        _signal.Set();
    }

    public void Send(WorldSnapshot snapshot) => Send(WorldCodec.Encode(snapshot));

    /// <summary>Solo para pruebas: fotograma aún no escrito.</summary>
    internal byte[]? PeekPending() => Volatile.Read(ref _pending);

    private void Run()
    {
        while (!_closed)
        {
            _signal.WaitOne();
            var frame = Interlocked.Exchange(ref _pending, null);
            if (frame is null)
                continue;
            try
            {
                _stream.Write(frame);
                _stream.Flush();
            }
            catch (Exception ex) when (ex is IOException or ObjectDisposedException or NotSupportedException)
            {
                _closed = true; // El visor se cerró: dejar de escribir sin afectar al anfitrión.
            }
        }
    }

    /// <summary>No espera al hilo: un visor bloqueado no debe congelar la interfaz.</summary>
    public void Dispose()
    {
        _closed = true;
        _signal.Set();
    }
}

/// <summary>Lector del visor: conserva el último fotograma válido y cuándo llegó.</summary>
public sealed class FrameReader : IDisposable
{
    private readonly Stream _stream;
    private readonly Thread _thread;
    private WorldSnapshot? _latest;
    private long _receivedTicks;
    private volatile bool _ended;

    public FrameReader(Stream stream)
    {
        _stream = stream;
        _thread = new Thread(Run) { IsBackground = true, Name = "SplitForge.FrameReader" };
        _thread.Start();
    }

    /// <summary>El anfitrión cerró el canal o envió datos corruptos.</summary>
    public bool Ended => _ended;

    /// <summary>Devuelve el último fotograma si no es más antiguo que <paramref name="maxAge"/>.</summary>
    public WorldSnapshot? Latest(TimeSpan maxAge)
    {
        var snapshot = Volatile.Read(ref _latest);
        if (snapshot is null)
            return null;
        var age = Stopwatch.GetElapsedTime(Interlocked.Read(ref _receivedTicks));
        return age <= maxAge ? snapshot : null;
    }

    private void Run()
    {
        Span<byte> header = stackalloc byte[4];
        var buffer = new byte[WorldCodec.MaxFrameBytes];
        try
        {
            while (ReadExactly(header))
            {
                var length = BinaryPrimitives.ReadInt32LittleEndian(header);
                if (length <= 0 || length > WorldCodec.MaxFrameBytes || !ReadExactly(buffer.AsSpan(0, length)))
                    break; // Flujo desincronizado: más seguro detenerse.
                var snapshot = WorldCodec.Decode(buffer.AsSpan(0, length));
                if (snapshot is null)
                    continue;
                Volatile.Write(ref _latest, snapshot);
                Interlocked.Exchange(ref _receivedTicks, Stopwatch.GetTimestamp());
            }
        }
        catch (Exception ex) when (ex is IOException or ObjectDisposedException)
        {
        }
        _ended = true;
    }

    private bool ReadExactly(Span<byte> target)
    {
        var read = 0;
        while (read < target.Length)
        {
            var n = _stream.Read(target[read..]);
            if (n == 0)
                return false;
            read += n;
        }
        return true;
    }

    /// <summary>
    /// Cerrar un pipe con una lectura bloqueada puede colgarse en algunos sistemas: solo se libera
    /// cuando el anfitrión ya cerró el canal. Si no, el proceso del visor lo libera al salir.
    /// </summary>
    public void Dispose()
    {
        if (_ended)
            _stream.Dispose();
    }
}
