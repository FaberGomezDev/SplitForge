namespace SplitForge.Core.Windows;

/// <summary>
/// Fondo negro detrás de la pantalla dividida: decide su sitio en el orden Z. Debe quedar justo debajo de la
/// ventana de la sesión que esté más abajo: así tapa el escritorio y la barra de tareas en las uniones y en
/// las franjas, y nunca tapa un juego.
/// </summary>
public static class BackdropPlan
{
    /// <param name="zOrder">Ventanas de nivel superior, de arriba abajo.</param>
    /// <param name="session">Ventanas de la sesión.</param>
    /// <param name="backdrop">La ventana del fondo.</param>
    /// <returns>
    /// La ventana tras la que hay que insertar el fondo, o null si ya está en su sitio o no queda ninguna
    /// ventana de la sesión.
    /// </returns>
    public static nint? InsertAfter(IReadOnlyList<nint> zOrder, IReadOnlyList<nint> session, nint backdrop)
    {
        var lowest = -1;
        for (var i = 0; i < zOrder.Count; i++)
        {
            // Una sesión tiene de 2 a 4 ventanas: la búsqueda lineal es lo más barato.
            for (var j = 0; j < session.Count; j++)
            {
                if (session[j] == zOrder[i])
                    lowest = i;
            }
        }
        if (lowest < 0)
            return null;
        return lowest + 1 < zOrder.Count && zOrder[lowest + 1] == backdrop ? null : zOrder[lowest];
    }
}
