using System.Net.Sockets;
using System.Text;
using SplitForge.Core.Sessions;

namespace SplitForge.Core.Adapters;

public enum ServerPhase
{
    Stopped,
    Starting,
    Ready,
    Stopping,
    Exited,
}

/// <summary>Rutas propias de SplitForge para el servidor: nunca dentro de la carpeta del juego.</summary>
public sealed record SevenDaysServerPaths(string Root)
{
    public string ConfigFile => Path.Combine(Root, "serverconfig.xml");
    public string UserData => Path.Combine(Root, "UserData");
    public string Logs => Path.Combine(Root, "logs");
}

/// <summary>
/// Ciclo de vida del servidor dedicado local. Es un proceso CREADO por SplitForge con guardados
/// aislados: se apaga primero de forma ordenada (telnet local "shutdown") y solo si no responde se termina.
/// </summary>
public sealed class SevenDaysServer(IProcessLauncher launcher, Action<string> log) : IDisposable
{
    public static readonly TimeSpan GracefulTimeout = TimeSpan.FromSeconds(45);

    private IChildProcess? _process;
    private SevenDaysServerOptions _options = new();

    public ServerPhase Phase { get; private set; } = ServerPhase.Stopped;
    public string? LogFile { get; private set; }
    public int? ProcessId => _process?.Id;
    public SevenDaysServerOptions Options => _options;

    public void Start(string serverExe, SevenDaysServerOptions options, SevenDaysServerPaths paths)
    {
        if (Phase is ServerPhase.Starting or ServerPhase.Ready or ServerPhase.Stopping)
            throw new InvalidOperationException("El servidor ya está en marcha.");
        _options = options.Validate();
        Directory.CreateDirectory(paths.Root);
        Directory.CreateDirectory(paths.UserData);
        Directory.CreateDirectory(paths.Logs);

        var template = ReadTemplate(Path.Combine(Path.GetDirectoryName(serverExe)!, "serverconfig.xml"));
        var xml = SevenDaysServerConfig.Build(template, options, paths.UserData);
        var tmp = paths.ConfigFile + ".tmp";
        File.WriteAllText(tmp, xml, new UTF8Encoding(false));
        File.Move(tmp, paths.ConfigFile, overwrite: true);

        LogFile = Path.Combine(paths.Logs, $"servidor-{DateTime.Now:yyyyMMdd-HHmmss}.log");
        _process = launcher.Start(SevenDaysServerConfig.Launch(serverExe, paths.ConfigFile, LogFile));
        Phase = ServerPhase.Starting;
        log($"Servidor 7DTD iniciado (PID {_process.Id}) en el puerto {options.Port}. Guardados aislados en la carpeta de SplitForge.");
    }

    /// <summary>Actualiza la fase. Listo = el puerto de control local acepta conexiones.</summary>
    public ServerPhase Refresh()
    {
        if (_process is null)
            return Phase;
        if (_process.HasExited)
        {
            if (Phase != ServerPhase.Exited && Phase != ServerPhase.Stopped)
                log($"El servidor 7DTD terminó (código {_process.ExitCode}). Revisa el registro del servidor.");
            Phase = Phase == ServerPhase.Stopping ? ServerPhase.Stopped : ServerPhase.Exited;
            _process.Dispose();
            _process = null;
            return Phase;
        }
        if (Phase == ServerPhase.Starting && PortOpen(_options.TelnetPort))
        {
            Phase = ServerPhase.Ready;
            log($"Servidor 7DTD listo. Cada jugador: Unirse a partida → IP 127.0.0.1, puerto {_options.Port}.");
        }
        return Phase;
    }

    /// <summary>Apagado ordenado; espera a que guarde. Termina el proceso propio solo si no responde.</summary>
    public async Task StopAsync(CancellationToken cancellationToken = default)
    {
        var process = _process;
        if (process is null)
        {
            Phase = ServerPhase.Stopped;
            return;
        }
        Phase = ServerPhase.Stopping;
        var sent = await TrySendCommandAsync(_options.TelnetPort, "shutdown", cancellationToken).ConfigureAwait(false);
        log(sent ? "Servidor 7DTD: apagado ordenado solicitado (guardando partida)…"
                 : "Servidor 7DTD: el canal de control no respondió.");
        var deadline = DateTime.UtcNow + GracefulTimeout;
        while (!process.HasExited && DateTime.UtcNow < deadline)
            await Task.Delay(250, cancellationToken).ConfigureAwait(false);
        if (!process.HasExited)
        {
            process.Kill();
            log("Servidor 7DTD terminado tras agotar el apagado ordenado (proceso creado por SplitForge).");
        }
        process.Dispose();
        _process = null;
        Phase = ServerPhase.Stopped;
        log("Servidor 7DTD detenido.");
    }

    public IReadOnlyList<string> TailLog(int lines = 12)
    {
        if (LogFile is null || !File.Exists(LogFile))
            return [];
        try
        {
            using var stream = new FileStream(LogFile, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete);
            var start = Math.Max(0, stream.Length - 16 * 1024);
            stream.Seek(start, SeekOrigin.Begin);
            using var reader = new StreamReader(stream, Encoding.UTF8);
            var all = reader.ReadToEnd().Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
            return all.Length <= lines ? all : all[^lines..];
        }
        catch (IOException)
        {
            return [];
        }
    }

    internal static bool PortOpen(int port)
    {
        try
        {
            using var client = new TcpClient();
            return client.ConnectAsync("127.0.0.1", port).Wait(TimeSpan.FromMilliseconds(150)) && client.Connected;
        }
        catch (Exception ex) when (ex is SocketException or AggregateException or ObjectDisposedException)
        {
            return false;
        }
    }

    internal static async Task<bool> TrySendCommandAsync(int port, string command, CancellationToken cancellationToken)
    {
        try
        {
            using var client = new TcpClient();
            using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            timeout.CancelAfter(TimeSpan.FromSeconds(3));
            await client.ConnectAsync("127.0.0.1", port, timeout.Token).ConfigureAwait(false);
            var stream = client.GetStream();
            // El servidor saluda primero; se descarta el texto de bienvenida.
            var buffer = new byte[4096];
            using var greet = CancellationTokenSource.CreateLinkedTokenSource(timeout.Token);
            greet.CancelAfter(TimeSpan.FromMilliseconds(500));
            try
            {
                _ = await stream.ReadAsync(buffer, greet.Token).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (!timeout.IsCancellationRequested)
            {
            }
            await stream.WriteAsync(Encoding.ASCII.GetBytes(command + "\r\n"), timeout.Token).ConfigureAwait(false);
            await stream.FlushAsync(timeout.Token).ConfigureAwait(false);
            return true;
        }
        catch (Exception ex) when (ex is SocketException or IOException or OperationCanceledException)
        {
            return false;
        }
    }

    private static string ReadTemplate(string path)
    {
        try
        {
            return File.Exists(path) ? File.ReadAllText(path) : SevenDaysServerConfig.FallbackTemplate;
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            return SevenDaysServerConfig.FallbackTemplate;
        }
    }

    public void Dispose() => _process?.Dispose();
}
