using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.RegularExpressions;

namespace SplitForge.Core.Steam;

/// <summary>Propuesta de investigación. NINGUNA está acreditada como adaptador funcional.</summary>
public sealed record ResearchProfile(
    string AppId,
    string Title,
    string Phase,
    IReadOnlyList<string> ExpectedExecutables,
    string GameplayNote,
    string LaunchNote);

public static class ResearchProfiles
{
    public const string SevenDaysToDie = "251570";
    public const string SevenDaysToDieDedicatedServer = "294420";

    public static readonly IReadOnlyDictionary<string, ResearchProfile> All = new Dictionary<string, ResearchProfile>
    {
        [SevenDaysToDie] = new(SevenDaysToDie, "7 Days to Die", "Adaptador experimental (servidor local)",
            ["7DaysToDie.exe"],
            "Multijugador por servidor: cada jugador necesita su propia cuenta Steam con el juego. Aislamiento de mandos aún no validado.",
            "Usar la pestaña 7 DAYS TO DIE: servidor dedicado local aislado y unión a 127.0.0.1."),
        [SevenDaysToDieDedicatedServer] = new(SevenDaysToDieDedicatedServer, "7 Days to Die Dedicated Server", "Herramienta del adaptador 7DTD",
            ["7DaysToDieServer.exe"],
            "Servidor oficial gratuito; permite una partida común en esta PC sin hospedar desde un cliente.",
            "SplitForge lo inicia con configuración y guardados propios; no modifica su carpeta."),
        ["275850"] = new("275850", "No Man's Sky", "Investigación posterior", ["NMS.exe"],
            "El modo multijugador y la autenticación deben verificarse. No se presupone cooperativo sin Internet.",
            "No modificar la autenticación ni abrir procesos duplicados."),
        ["495420"] = new("495420", "State of Decay 2", "Investigación posterior", [],
            "El multijugador puede requerir identidades y servicios en línea distintos.",
            "No prometer modo sin conexión con dos jugadores."),
        ["271590"] = new("271590", "Grand Theft Auto V (Legacy)", "Investigación posterior", ["GTA5.exe"],
            "La campaña no incluye cooperativo local nativo; opciones externas no se presuponen compatibles.",
            "No intervenir en GTA Online, protección antitrampas ni licencias."),
    };
}

/// <summary>Evaluación conservadora de una instalación. Nunca certifica cooperativo.</summary>
public sealed record Preflight(
    [property: JsonPropertyName("app_id")] string AppId,
    [property: JsonPropertyName("name")] string Name,
    [property: JsonPropertyName("phase")] string Phase,
    [property: JsonPropertyName("folder_present")] bool FolderPresent,
    [property: JsonPropertyName("inside_steam_library")] bool InsideSteamLibrary,
    [property: JsonPropertyName("expected_executable_found")] bool? ExpectedExecutableFound,
    [property: JsonPropertyName("expected_executables")] IReadOnlyList<string> ExpectedExecutables,
    [property: JsonPropertyName("executables_found")] IReadOnlyList<string> ExecutablesFound,
    [property: JsonPropertyName("build_id")] string BuildId,
    [property: JsonPropertyName("state_flags")] string StateFlags,
    [property: JsonPropertyName("notes")] IReadOnlyList<string> Notes)
{
    // Estas garantías son constantes a propósito: solo una prueba física puede cambiarlas.
    [JsonPropertyName("offline_coop_verified")] public bool OfflineCoopVerified => false;
    [JsonPropertyName("input_isolation_verified")] public bool InputIsolationVerified => false;
    [JsonPropertyName("simultaneous_clients_verified")] public bool SimultaneousClientsVerified => false;
    [JsonPropertyName("ready_for_two_players")] public bool ReadyForTwoPlayers => false;
}

public static partial class PreflightEvaluator
{
    public const int MaxExecutablesListed = 32;

    public static Preflight Evaluate(InstalledGame game)
    {
        ResearchProfiles.All.TryGetValue(game.AppId, out var profile);
        var notes = new List<string>();
        var (folder, inside) = SafeInstallFolder(game);
        var present = inside && Directory.Exists(folder);
        if (!inside)
            notes.Add("Ruta fuera de steamapps/common: no confiable, revisar manifiesto.");
        if (!present)
            notes.Add("No se encontró una carpeta de instalación válida en disco.");

        IReadOnlyList<string> names = [];
        if (present)
        {
            try
            {
                // Solo el directorio principal: no recorrer árboles de juegos de decenas de GB.
                names = Directory.EnumerateFiles(folder, "*.exe", new EnumerationOptions
                    {
                        MatchCasing = MatchCasing.CaseInsensitive,
                        IgnoreInaccessible = true,
                    })
                    .Select(Path.GetFileName)
                    .OfType<string>()
                    .Order(StringComparer.OrdinalIgnoreCase)
                    .Take(MaxExecutablesListed)
                    .ToList();
            }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
            {
                notes.Add("No fue posible inspeccionar los ejecutables del directorio.");
            }
        }

        IReadOnlyList<string> expected = profile?.ExpectedExecutables ?? [];
        bool? detected = null;
        if (expected.Count > 0)
        {
            detected = expected.Any(e => names.Contains(e, StringComparer.OrdinalIgnoreCase));
            if (detected == false && present)
                notes.Add("No se halló el ejecutable esperado en la raíz; puede estar en una subcarpeta o haber cambiado de nombre.");
        }
        if (profile is not null)
        {
            notes.Add(profile.GameplayNote);
            notes.Add(profile.LaunchNote);
        }
        else
        {
            notes.Add("Sin perfil de investigación para este AppID.");
        }
        notes.Add("Sin prueba de dos cuentas/clientes, mandos aislados ni conexión cooperativa local.");

        return new Preflight(game.AppId, game.Name, profile?.Phase ?? "Sin perfil; aún sin verificar",
            present, inside, detected, expected, names, game.BuildId, game.StateFlags, notes);
    }

    /// <summary>
    /// No seguir rutas fuera de <c>steamapps/common</c>: los manifiestos no son una lista de
    /// ejecutables de confianza y nunca deben autorizar ejecución arbitraria.
    /// </summary>
    public static (string Folder, bool Inside) SafeInstallFolder(InstalledGame game)
    {
        var folder = ResolveDirectory(game.InstallFolder);
        var common = ResolveDirectory(Path.Combine(game.LibraryRoot, "steamapps", "common"));
        var comparison = OperatingSystem.IsWindows() ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;
        var prefix = Path.TrimEndingDirectorySeparator(common) + Path.DirectorySeparatorChar;
        var inside = !string.Equals(Path.TrimEndingDirectorySeparator(folder), Path.TrimEndingDirectorySeparator(common), comparison)
                     && folder.StartsWith(prefix, comparison);
        return (folder, inside);
    }

    private static string ResolveDirectory(string path)
    {
        var full = Path.GetFullPath(path);
        try
        {
            // Un enlace simbólico o unión dentro de common podría apuntar a cualquier sitio.
            var target = new DirectoryInfo(full).ResolveLinkTarget(returnFinalTarget: true);
            if (target is not null)
                return Path.GetFullPath(target.FullName);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
        }
        return full;
    }

    [GeneratedRegex(@"^[1-9]\d{0,14}$")]
    private static partial Regex AppIdPattern();

    /// <summary>URL de lanzamiento de UNA sola instancia solicitada explícitamente por el usuario.</summary>
    public static string SteamRunUri(string appId) =>
        AppIdPattern().IsMatch(appId) ? "steam://rungameid/" + appId : throw new ArgumentException("App ID de Steam inválido", nameof(appId));
}

/// <summary>Resumen compartible: SIN rutas de perfil, IDs de cuenta ni identificadores de hardware.</summary>
public sealed record SteamPublicReport(
    [property: JsonPropertyName("format")] string Format,
    [property: JsonPropertyName("game_count")] int GameCount,
    [property: JsonPropertyName("games")] IReadOnlyList<Preflight> Games,
    [property: JsonPropertyName("declaration")] string Declaration)
{
    public static SteamPublicReport Create(IEnumerable<InstalledGame> games)
    {
        var entries = games.Select(PreflightEvaluator.Evaluate).ToList();
        return new("SplitForge Steam Preflight v1", entries.Count, entries,
            "Ninguna instalación certificada para pantalla dividida; revisión de archivos en modo solo lectura.");
    }

    public string ToJson() => JsonSerializer.Serialize(this, ReportJson.Options);
}

public static class ReportJson
{
    public static readonly JsonSerializerOptions Options = new()
    {
        WriteIndented = true,
        // Mantener tildes legibles en los informes (equivalente a ensure_ascii=False).
        Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
    };
}
