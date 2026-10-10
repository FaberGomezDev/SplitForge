using System.Text.Json;
using SplitForge.Core.Layout;

namespace SplitForge.Core.Sessions;

/// <summary>
/// Copia en disco de las posiciones y estilos originales de las ventanas externas mientras una
/// sesión está activa. Si SplitForge se cierra de golpe, el siguiente arranque las restaura
/// (solo si el handle sigue perteneciendo al MISMO proceso: los handles se reutilizan).
/// </summary>
public static class SessionRecovery
{
    public const string FileName = "sesion-ventanas.json";

    private sealed record Entry(long Handle, int X, int Y, int Width, int Height, bool Maximized, bool Minimized,
        long Style, long ExStyle, int Pid);

    public static void Save(string path, IEnumerable<WindowSnapshot> snapshots)
    {
        var entries = snapshots.Select(s => new Entry(s.Handle, s.Bounds.X, s.Bounds.Y, s.Bounds.Width, s.Bounds.Height,
            s.Maximized, s.Minimized, s.Style, s.ExStyle, s.Pid)).ToList();
        var tmp = path + ".tmp";
        File.WriteAllText(tmp, JsonSerializer.Serialize(entries));
        File.Move(tmp, path, overwrite: true);
    }

    public static IReadOnlyList<WindowSnapshot> Load(string path)
    {
        try
        {
            if (!File.Exists(path))
                return [];
            var entries = JsonSerializer.Deserialize<List<Entry>>(File.ReadAllText(path)) ?? [];
            return [.. entries.Take(8).Select(e => new WindowSnapshot((nint)e.Handle, new Region(e.X, e.Y, e.Width, e.Height),
                e.Maximized, e.Minimized, e.Style, e.ExStyle, e.Pid))];
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException)
        {
            return [];
        }
    }

    public static void Clear(string path)
    {
        try
        {
            File.Delete(path);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
        }
    }

    /// <summary>Restaura lo que quedó pendiente de una sesión anterior. Devuelve cuántas ventanas restauró.</summary>
    public static int RecoverAfterCrash(string path, IWindowSystem windows, Action<string> log)
    {
        var pending = Load(path);
        var restored = 0;
        foreach (var snapshot in pending)
        {
            if (snapshot.Pid == 0 || windows.ProcessId(snapshot.Handle) != snapshot.Pid)
                continue; // La ventana ya no existe o el handle es ahora de otro proceso.
            try
            {
                windows.Restore(snapshot);
                restored++;
            }
            catch (Exception ex) when (ex is InvalidOperationException or IOException or ArgumentException)
            {
                log($"No se pudo restaurar una ventana de la sesión anterior: {ex.Message}");
            }
        }
        if (pending.Count > 0)
            log($"Recuperación tras cierre inesperado: {restored} de {pending.Count} ventanas restauradas.");
        Clear(path);
        return restored;
    }
}
