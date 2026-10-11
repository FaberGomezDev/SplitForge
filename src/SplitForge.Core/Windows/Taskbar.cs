namespace SplitForge.Core.Windows;

/// <summary>
/// Con «Tapar barra de tareas» las ventanas de la sesión se colocan «siempre encima». La barra también lo está,
/// y Windows la vuelve a subir cuando cambia la ventana activa y esta no ocupa toda la pantalla (las de una
/// pantalla dividida nunca la ocupan): los juegos quedan debajo hasta que alguien los activa. Esta lógica
/// decide cuándo hay que volver a subirlos.
/// </summary>
public static class TaskbarPlan
{
    /// <param name="zOrder">Ventanas de nivel superior, de arriba abajo.</param>
    /// <param name="session">Ventanas de la sesión.</param>
    /// <param name="taskbars">Barras de tareas que pisan la zona de la sesión.</param>
    /// <returns>
    /// Las ventanas de la sesión que hay que volver a subir, de la más baja a la más alta (subidas en ese
    /// orden conservan el orden que tenían entre sí); vacío si ninguna barra está por encima de un juego.
    /// </returns>
    public static IReadOnlyList<nint> ToRaise(IReadOnlyList<nint> zOrder, IReadOnlyList<nint> session, IReadOnlyList<nint> taskbars)
    {
        var highestTaskbar = -1;
        var lowestSession = -1;
        for (var i = 0; i < zOrder.Count; i++)
        {
            if (highestTaskbar < 0 && taskbars.Contains(zOrder[i]))
                highestTaskbar = i;
            if (session.Contains(zOrder[i]))
                lowestSession = i;
        }
        if (highestTaskbar < 0 || lowestSession < highestTaskbar)
            return [];
        var raise = new List<nint>(session.Count);
        for (var i = zOrder.Count - 1; i >= 0; i--)
        {
            if (session.Contains(zOrder[i]))
                raise.Add(zOrder[i]);
        }
        return raise;
    }
}
