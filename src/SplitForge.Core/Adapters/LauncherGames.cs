namespace SplitForge.Core.Adapters;

/// <summary>Un juego de la biblioteca del lanzador y los textos de su pantalla principal.</summary>
/// <param name="Banner">Línea sobre el título: qué se consigue, o que aún está en preparación.</param>
/// <param name="HowTo">Los pasos de «Cómo se juega», en orden.</param>
/// <param name="EmulatedIdentity">
/// true = cada ventana entra con una identidad emulada (se elige el nombre del jugador y las opciones de
/// partida); false = cada jugador entra con su propia cuenta y esas opciones no aplican.
/// </param>
public sealed record LauncherGame(string Id, string Name, string AppId, string Tagline, string Banner,
    IReadOnlyList<string> HowTo, bool EmulatedIdentity);

/// <summary>
/// Biblioteca del lanzador. Estar aquí NO declara compatibilidad: solo indica que SplitForge sabe preparar y
/// abrir el juego. El estado real de cada uno está en <c>docs/05-INTEGRACION-STEAM.md</c>.
/// </summary>
public static class LauncherGames
{
    public const string SevenDaysToDie = "7dtd";
    public const string NoMansSky = "nms";

    public static IReadOnlyList<LauncherGame> All { get; } =
    [
        new(SevenDaysToDie, "7 Days to Die", "251570", "Pantalla dividida · 2 jugadores",
            "PANTALLA DIVIDIDA · 2 JUGADORES · MISMO MUNDO",
            [
                "Pulsa JUGAR: se abren las dos ventanas, cada una con su mando.",
                "Un jugador crea o continúa una partida con el multijugador del juego activado.",
                "El otro elige «Unirse a partida» → «Conectar a IP» y escribe 127.0.0.1.",
            ], EmulatedIdentity: true),
        new(NoMansSky, "No Man's Sky", NoMansSkyAdapter.AppId, "En preparación · 2 copias, 2 cuentas",
            "EN PREPARACIÓN · SIN VALIDAR · CADA JUGADOR CON SU COPIA",
            [
                "En Steam, desactiva Steam Input solo para este juego (Propiedades › Mando): si no, Steam le da los dos mandos a J1.",
                "Pulsa JUGAR: se abre la copia de cada jugador con su mando. Cada uno inicia sesión con su cuenta.",
                "Uno abre su partida y el otro entra desde «Únete a tus amigos», en el menú del juego.",
            ], EmulatedIdentity: false),
    ];

    /// <summary>El juego con ese identificador; si no existe (ajustes antiguos o editados), el primero.</summary>
    public static LauncherGame Find(string? id) => All.FirstOrDefault(game => game.Id == id) ?? All[0];
}
