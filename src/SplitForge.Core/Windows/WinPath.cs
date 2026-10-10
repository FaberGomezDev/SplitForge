namespace SplitForge.Core.Windows;

/// <summary>
/// Normalización de rutas Windows sin tocar el disco ni resolver enlaces (equivalente a
/// <c>ntpath.normcase(ntpath.normpath(...))</c>). Funciona igual en cualquier sistema operativo.
/// </summary>
public static class WinPath
{
    public static string Normalize(string value)
    {
        var s = value.Trim('"', ' ').Replace('/', '\\');
        if (s.StartsWith(@"\\", StringComparison.Ordinal))
            return s.ToLowerInvariant(); // UNC o dispositivo: se rechazan más adelante.

        var drive = string.Empty;
        if (s.Length >= 2 && s[1] == ':' && char.IsAsciiLetter(s[0]))
        {
            drive = s[..2];
            s = s[2..];
        }
        var rooted = s.StartsWith('\\');
        var parts = new List<string>();
        foreach (var part in s.Split('\\'))
        {
            if (part is "" or ".")
                continue;
            if (part == "..")
            {
                if (parts.Count > 0 && parts[^1] != "..")
                    parts.RemoveAt(parts.Count - 1);
                else if (!rooted)
                    parts.Add("..");
                continue;
            }
            parts.Add(part);
        }
        var result = drive + (rooted ? "\\" : string.Empty) + string.Join('\\', parts);
        return (result.Length == 0 ? "." : result).ToLowerInvariant();
    }

    /// <summary>Solo rutas con unidad y raíz (<c>C:\...</c>). UNC y relativas no cuentan.</summary>
    public static bool IsAbsoluteDrivePath(string normalized) =>
        normalized.Length >= 3 && normalized[1] == ':' && normalized[2] == '\\' && char.IsAsciiLetter(normalized[0]);

    public static string FileName(string path)
    {
        var s = path.Replace('/', '\\');
        var i = s.LastIndexOf('\\');
        return i < 0 ? s : s[(i + 1)..];
    }

    /// <summary>El ejecutable debe estar DENTRO de la carpeta del juego (no en una carpeta hermana con prefijo común).</summary>
    public static bool IsGameExecutable(string? path, string? gameFolder)
    {
        if (string.IsNullOrEmpty(path) || string.IsNullOrEmpty(gameFolder))
            return false;
        var full = Normalize(path);
        var root = Normalize(gameFolder);
        if (!IsAbsoluteDrivePath(full) || !IsAbsoluteDrivePath(root))
            return false;
        return full.StartsWith(root.TrimEnd('\\') + "\\", StringComparison.Ordinal)
               && full.EndsWith(".exe", StringComparison.Ordinal);
    }
}
