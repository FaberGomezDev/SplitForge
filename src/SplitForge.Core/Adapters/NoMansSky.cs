namespace SplitForge.Core.Adapters;

/// <summary>
/// Adaptador de No Man's Sky (EXPERIMENTAL, NO VALIDADO: ver <c>docs/plans/no-mans-sky.md</c>).
/// Aquí NO sirve la vía de 7 Days to Die. El multijugador pasa por los servicios en línea del juego y el
/// ejecutable lleva la protección de Steam, así que no hay identidad emulable: cada jugador juega con su
/// propia copia y su propia cuenta (J1 la de Steam; J2 otra, por ejemplo la de GOG). SplitForge solo abre las
/// dos copias, da un mando a cada ventana y divide la pantalla. No se modifica ningún archivo del juego.
/// </summary>
public static class NoMansSkyAdapter
{
    public const string AppId = "275850";
    public const string GameExe = "NMS.exe";
    public const string BinariesFolder = "Binaries";
    public const string WindowTitle = "No Man's Sky";

    /// <summary>La copia de Steam hay que abrirla a través de Steam: su ejecutable no arranca solo.</summary>
    public const string SteamLaunchUri = "steam://rungameid/" + AppId;

    /// <summary>HECHO (2026-10-11): el ejecutable importa XInput con este nombre y carga el filtro desde su carpeta.</summary>
    public static readonly IReadOnlyList<string> FilterNames = ["xinput9_1_0.dll"];

    /// <summary>Carpeta del ejecutable dentro de una instalación: ahí va el filtro de mandos.</summary>
    public static string Binaries(string installFolder) => Path.Combine(installFolder, BinariesFolder);

    public static string Exe(string installFolder) => Path.Combine(installFolder, BinariesFolder, GameExe);

    /// <summary>¿Es la carpeta raíz de una instalación del juego (la que contiene <c>Binaries\NMS.exe</c>)?</summary>
    public static bool IsInstall(string? installFolder) =>
        !string.IsNullOrWhiteSpace(installFolder) && File.Exists(Exe(installFolder));

    /// <summary>
    /// A partir del ejecutable elegido por el usuario (o de la propia carpeta), la carpeta raíz de esa
    /// instalación; null si no es una instalación del juego.
    /// </summary>
    public static string? InstallFromPath(string? path)
    {
        if (string.IsNullOrWhiteSpace(path))
            return null;
        if (IsInstall(path))
            return Path.GetFullPath(path);
        var file = Path.GetFullPath(path);
        if (!string.Equals(Path.GetFileName(file), GameExe, StringComparison.OrdinalIgnoreCase))
            return null;
        var binaries = Path.GetDirectoryName(file);
        var root = binaries is null ? null : Path.GetDirectoryName(binaries);
        return IsInstall(root) ? root : null;
    }

    /// <summary>
    /// Jugador al que pertenece un proceso abierto del juego, por la carpeta de su ejecutable: 1 la copia de
    /// Steam, 2 la segunda copia; null si no es de ninguna de las dos.
    /// </summary>
    public static int? PlayerOf(string? imagePath, string? steamCopy, string? secondCopy)
    {
        if (string.IsNullOrEmpty(imagePath))
            return null;
        if (Inside(imagePath, steamCopy))
            return 1;
        return Inside(imagePath, secondCopy) ? 2 : null;

        static bool Inside(string image, string? install)
        {
            if (string.IsNullOrWhiteSpace(install))
                return false;
            var root = Path.GetFullPath(install).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar) + Path.DirectorySeparatorChar;
            return Path.GetFullPath(image).StartsWith(root, StringComparison.OrdinalIgnoreCase);
        }
    }

    /// <summary>
    /// Qué hace falta para jugar dos en la misma partida. Lógica pura: recibe hechos ya comprobados.
    /// </summary>
    /// <param name="connectedSlots">Ranuras XInput con mando, numeradas del 1 al 4.</param>
    /// <param name="playerSlots">Ranura elegida para cada jugador (1–4), en orden J1, J2.</param>
    public static IReadOnlyList<PlayStep> Readiness(bool steamCopy, bool secondCopy, bool filterInSteamCopy, bool filterInSecondCopy,
        IReadOnlyCollection<int> connectedSlots, IReadOnlyList<int> playerSlots)
    {
        var missing = playerSlots.Where(slot => !connectedSlots.Contains(slot)).ToList();
        var repeated = playerSlots.Distinct().Count() != playerSlots.Count;
        var withoutFilter = new List<string>();
        if (steamCopy && !filterInSteamCopy)
            withoutFilter.Add("la copia de Steam");
        if (secondCopy && !filterInSecondCopy)
            withoutFilter.Add("la copia de J2");
        var anyCopy = steamCopy || secondCopy;
        return
        [
            new PlayStep("Juego de J1 (Steam)", steamCopy ? "Instalado en tu biblioteca de Steam." : "No aparece en tu biblioteca de Steam.", steamCopy),
            new PlayStep("Juego de J2 (otra copia)", secondCopy
                ? "Segunda copia encontrada."
                : "Cada jugador necesita su propia copia con su cuenta: el juego une a los jugadores por sus servidores. "
                  + "Instala otra copia (por ejemplo la de GOG) e indícala con «Elegir copia de J2».", secondCopy),
            new PlayStep("Un mando por ventana", !anyCopy
                ? "Se instala cuando el juego esté en el equipo."
                : withoutFilter.Count == 0
                    ? "Cada ventana obedece solo a su mando."
                    : "Falta instalar el filtro de mandos en " + string.Join(" y en ", withoutFilter) + ".",
                anyCopy && withoutFilter.Count == 0),
            new PlayStep("Mandos", repeated
                ? "Dos jugadores tienen el mismo mando: elige uno distinto para cada uno."
                : missing.Count == 0
                    ? $"{playerSlots.Count} mandos conectados."
                    : "Sin conectar: " + string.Join(", ", missing.Select(slot => "mando " + slot)) + ".",
                !repeated && missing.Count == 0),
        ];
    }
}
