using SplitForge.Core.Layout;

namespace SplitForge.Core.Windows;

/// <summary>Textos del aviso en pantalla del bloqueo de entrada.</summary>
/// <param name="Badge">Texto de la insignia que queda fija mientras dura el bloqueo; vacío si no queda ninguna.</param>
public readonly record struct LockNoticeText(string Title, string Detail, string Badge);

/// <summary>
/// Aviso en pantalla del bloqueo de entrada «solo mandos»: al cambiar el estado se muestra un cartel grande
/// durante <see cref="ToastMs"/> y, mientras el teclado y el ratón siguen bloqueados, queda una insignia
/// pequeña. Así se sabe en todo momento, con los juegos delante, si el bloqueo está puesto.
/// </summary>
public static class LockNoticePlan
{
    /// <summary>Tiempo que se ve el cartel grande.</summary>
    public const int ToastMs = 3000;

    /// <summary>Separación entre el aviso y el borde superior de la zona.</summary>
    public const int MarginPx = 28;

    public static LockNoticeText Text(bool locked, string hotkey) => locked
        ? new("TECLADO Y RATÓN BLOQUEADOS", $"Solo funcionan los mandos · {hotkey} para desbloquear", $"Bloqueado · {hotkey}")
        : new("TECLADO Y RATÓN DESBLOQUEADOS", $"{hotkey} para volver a bloquear", string.Empty);

    /// <summary>
    /// Esquina superior izquierda (en píxeles) para centrar arriba un aviso de <paramref name="width"/> ×
    /// <paramref name="height"/> dentro de <paramref name="area"/>, sin salirse de ella.
    /// </summary>
    public static (int X, int Y) TopCenter(Region area, int width, int height)
    {
        var x = area.X + Math.Max(0, (area.Width - width) / 2);
        var y = area.Y + Math.Clamp(MarginPx, 0, Math.Max(0, area.Height - height));
        return (x, y);
    }
}
