namespace SplitForge.Core.Diagnostics;

/// <summary>
/// Registro local con identificador de sesión. No se escriben rutas de perfil, cuentas ni
/// identificadores de dispositivos: quien llama solo pasa mensajes ya saneados.
/// </summary>
public sealed class SessionLog
{
    public const long MaxBytes = 2 * 1024 * 1024;

    private readonly object _gate = new();
    private readonly string? _path;

    public SessionLog(string? path)
    {
        _path = path;
        SessionId = Guid.NewGuid().ToString("N")[..8];
        if (_path is not null)
            Rotate(_path);
    }

    public string SessionId { get; }

    public event Action<string>? Written;

    public void Write(string message)
    {
        var line = $"{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff} [{SessionId}] {message}";
        if (_path is not null)
        {
            lock (_gate)
            {
                try
                {
                    File.AppendAllText(_path, line + Environment.NewLine);
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                {
                    // El registro nunca debe interrumpir una sesión.
                }
            }
        }
        Written?.Invoke(line);
    }

    private static void Rotate(string path)
    {
        try
        {
            var info = new FileInfo(path);
            if (info.Exists && info.Length > MaxBytes)
                File.Move(path, path + ".old", overwrite: true);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
        }
    }
}
