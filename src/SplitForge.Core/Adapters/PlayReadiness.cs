namespace SplitForge.Core.Adapters;

/// <summary>Un requisito para jugar en pantalla dividida y si se cumple.</summary>
public sealed record PlayStep(string Title, string Detail, bool Ok);

/// <summary>
/// Qué hace falta para pulsar «Jugar» (pantalla principal del lanzador). Lógica pura: recibe hechos ya
/// comprobados (archivos instalados, ranuras XInput con mando) y devuelve la lista que ve el jugador.
/// </summary>
public static class PlayReadiness
{
    /// <param name="connectedSlots">Ranuras XInput con mando, numeradas del 1 al 4.</param>
    /// <param name="playerSlots">Ranura elegida para cada jugador (1–4), en orden J1, J2…</param>
    public static IReadOnlyList<PlayStep> Evaluate(bool gameInstalled, bool emulatorInstalled, bool filterInstalled,
        IReadOnlyCollection<int> connectedSlots, IReadOnlyList<int> playerSlots)
    {
        var missing = playerSlots.Where(slot => !connectedSlots.Contains(slot)).ToList();
        var repeated = playerSlots.Distinct().Count() != playerSlots.Count;
        return
        [
            new PlayStep("Juego", gameInstalled ? "Instalado en tu biblioteca de Steam." : "No aparece en tu biblioteca de Steam.", gameInstalled),
            new PlayStep("Identidad de cada jugador", emulatorInstalled
                ? "Cada ventana entra con su propio jugador."
                : "Falta instalar el emulador de Steam en el juego.", emulatorInstalled),
            new PlayStep("Un mando por ventana", filterInstalled
                ? "Cada ventana obedece solo a su mando."
                : "Falta instalar el filtro de mandos en el juego.", filterInstalled),
            new PlayStep("Mandos", repeated
                ? "Dos jugadores tienen el mismo mando: elige uno distinto para cada uno."
                : missing.Count == 0
                    ? $"{playerSlots.Count} mandos conectados."
                    : "Sin conectar: " + string.Join(", ", missing.Select(slot => "mando " + slot)) + ".",
                !repeated && missing.Count == 0),
        ];
    }

    public static bool Ready(IReadOnlyList<PlayStep> steps) => steps.All(step => step.Ok);
}

/// <summary>
/// Carátulas de un juego tomadas de la caché local del cliente de Steam del usuario
/// (<c>appcache\librarycache</c>). Solo se leen de su disco: SplitForge no distribuye imágenes de juegos.
/// </summary>
public sealed record SteamArt(string? Hero, string? Cover, string? Logo)
{
    public static SteamArt Find(IEnumerable<string> steamRoots, string appId)
    {
        string? hero = null, cover = null, logo = null;
        foreach (var root in steamRoots)
        {
            var cache = Path.Combine(root, "appcache", "librarycache");
            hero ??= Art(cache, appId, "library_hero.jpg");
            cover ??= Art(cache, appId, "library_600x900.jpg", "library_capsule.jpg");
            logo ??= Art(cache, appId, "logo.png");
        }
        return new SteamArt(hero, cover, logo);

        // Tres disposiciones vistas en el cliente de Steam: una carpeta por juego con los archivos dentro, la misma
        // carpeta con cada imagen en una subcarpeta con nombre de huella, y la anterior (archivos «AppID_nombre»).
        static string? Art(string cache, string appId, params string[] names)
        {
            var folder = Path.Combine(cache, appId);
            foreach (var name in names)
            {
                if (File.Exists(Path.Combine(folder, name)))
                    return Path.Combine(folder, name);
                if (File.Exists(Path.Combine(cache, appId + "_" + name)))
                    return Path.Combine(cache, appId + "_" + name);
                try
                {
                    if (Directory.Exists(folder)
                        && Directory.EnumerateDirectories(folder).Order(StringComparer.OrdinalIgnoreCase)
                            .Select(sub => Path.Combine(sub, name)).FirstOrDefault(File.Exists) is { } nested)
                        return nested;
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                {
                    // Una caché que no se deja leer es lo mismo que no tener la imagen.
                }
            }
            return null;
        }
    }
}
