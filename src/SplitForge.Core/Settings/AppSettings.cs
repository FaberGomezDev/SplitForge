using System.Text.Json;
using System.Text.Json.Serialization;
using SplitForge.Core.Adapters;
using SplitForge.Core.Input;
using SplitForge.Core.Layout;
using SplitForge.Core.Windows;

namespace SplitForge.Core.Settings;

/// <summary>Identidades del emulador de Steam (ADR-016). Sin datos personales: IDs ficticios locales.</summary>
public sealed record SteamEmuOptions
{
    public string SteamId1 { get; init; } = "76561198000000001";
    public string SteamId2 { get; init; } = "76561198000000002";
    public string Persona1 { get; init; } = "SplitForgeJ1";
    public string Persona2 { get; init; } = "SplitForgeJ2";
    public int AppId { get; init; } = 251570;
    public string EmulatorDll { get; init; } = "";

    /// <summary>
    /// J2 entra sola en la partida de J1 en cuanto J1 la abre (el emulador se lo pide al juego). Desactivada
    /// por defecto: al propietario le resultó un comportamiento inesperado; queda como opción.
    /// </summary>
    public bool AutoJoin { get; init; }

    /// <summary>J1 arranca cargando su última partida (argumento <c>-quick-continue</c> de 7DTD).</summary>
    public bool QuickContinue { get; init; }

    /// <summary>
    /// J2 usa su propia carpeta de datos (<c>-UserDataFolder</c>): no ve ni puede abrir las partidas de J1,
    /// así dos instancias nunca escriben el mismo guardado.
    /// </summary>
    public bool IsolateSaves { get; init; } = true;
}

/// <summary>No Man's Sky: dónde está la copia del jugador 2 (la de J1 es la de Steam y se detecta sola).</summary>
public sealed record NoMansSkyOptions
{
    /// <summary>Carpeta de la segunda copia (por ejemplo la de GOG); vacía si aún no hay o no se ha indicado.</summary>
    public string SecondCopyFolder { get; init; } = "";
}

public sealed record AppSettings
{
    /// <summary>Juego elegido en la biblioteca del lanzador (identificador de <c>LauncherGames</c>).</summary>
    public string SelectedGame { get; init; } = LauncherGames.SevenDaysToDie;
    public NoMansSkyOptions NoMansSky { get; init; } = new();
    public SplitOrientation Orientation { get; init; } = SplitOrientation.SideBySide;
    public int Gutter { get; init; } = 4;
    public bool Borderless { get; init; } = true;
    /// <summary>Sesión de juego en el monitor completo, con las ventanas por encima de la barra de tareas.</summary>
    public bool CoverTaskbar { get; init; } = true;

    /// <summary>Fondo negro detrás de la pantalla dividida: tapa el escritorio en las uniones y franjas.</summary>
    public bool BlackBackdrop { get; init; } = true;
    public WindowFit Fit { get; init; } = WindowFit.Rellenar;
    public int AspectW { get; init; } = 16;
    public int AspectH { get; init; } = 9;
    public InputReadMode InputMode { get; init; } = InputReadMode.Auto;
    public int InputRateHz { get; init; } = 1000;
    public Dictionary<string, ControllerMapping> Mappings { get; init; } = [];
    public SevenDaysServerOptions SevenDays { get; init; } = new();
    public SteamEmuOptions SteamEmu { get; init; } = new();
    /// <summary>Foco simulado en las ventanas de la sesión (ADR-020, experimental).</summary>
    public bool FakeFocus { get; init; } = true;
    public int FakeFocusIntervalMs { get; init; } = FakeFocusPlan.DefaultIntervalMs;
}

/// <summary>
/// Configuración de la versión .NET en <c>settings.v2.json</c>. El <c>settings.json</c> de la
/// versión Python se lee solo para importar preferencias y NUNCA se modifica ni se borra.
/// </summary>
public static class SettingsStore
{
    public const string FileName = "settings.v2.json";
    public const string LegacyFileName = "settings.json";

    private static readonly JsonSerializerOptions Json = new()
    {
        WriteIndented = true,
        Converters = { new JsonStringEnumConverter() },
        Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
    };

    public static string DataFolder()
    {
        var local = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        var folder = string.IsNullOrEmpty(local)
            ? Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), ".splitforge")
            : Path.Combine(local, "SplitForge");
        Directory.CreateDirectory(folder);
        return folder;
    }

    public static AppSettings Load(string folder)
    {
        var path = Path.Combine(folder, FileName);
        try
        {
            if (File.Exists(path))
                return Sanitize(JsonSerializer.Deserialize<AppSettings>(File.ReadAllText(path), Json) ?? new AppSettings());
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException or NotSupportedException)
        {
            // Archivo dañado: se usan valores por defecto sin borrarlo.
        }
        return ImportLegacy(Path.Combine(folder, LegacyFileName));
    }

    /// <summary>Escritura atómica (temporal + reemplazo) para no dejar un archivo a medias.</summary>
    public static void Save(string folder, AppSettings settings)
    {
        Directory.CreateDirectory(folder);
        var path = Path.Combine(folder, FileName);
        var tmp = path + ".tmp";
        File.WriteAllText(tmp, JsonSerializer.Serialize(settings, Json));
        File.Move(tmp, path, overwrite: true);
    }

    /// <summary>Importa orientación, separación y modo de lectura de la versión Python (solo lectura).</summary>
    public static AppSettings ImportLegacy(string legacyPath)
    {
        try
        {
            if (!File.Exists(legacyPath))
                return new AppSettings();
            using var doc = JsonDocument.Parse(File.ReadAllText(legacyPath));
            var root = doc.RootElement;
            if (root.ValueKind != JsonValueKind.Object)
                return new AppSettings();
            var settings = new AppSettings
            {
                Orientation = SplitLayout.ParseLegacy(root.TryGetProperty("orientation", out var o) && o.ValueKind == JsonValueKind.String ? o.GetString() : null),
                Gutter = root.TryGetProperty("gutter", out var g) && g.TryGetInt32(out var gv) ? gv : 4,
                InputMode = ReadPathPolicy.ParseLegacy(root.TryGetProperty("input_mode", out var m) && m.ValueKind == JsonValueKind.String ? m.GetString() : null),
            };
            return Sanitize(settings);
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or JsonException)
        {
            return new AppSettings();
        }
    }

    private static AppSettings Sanitize(AppSettings s) => s with
    {
        Gutter = Math.Clamp(s.Gutter, 0, SplitLayout.MaxGutter),
        InputRateHz = s.InputRateHz is 250 or 500 or 1000 ? s.InputRateHz : 1000,
        Orientation = Enum.IsDefined(s.Orientation) ? s.Orientation : SplitOrientation.SideBySide,
        Fit = Enum.IsDefined(s.Fit) ? s.Fit : WindowFit.Rellenar,
        AspectW = Math.Clamp(s.AspectW, 1, 64),
        AspectH = Math.Clamp(s.AspectH, 1, 64),
        FakeFocusIntervalMs = FakeFocusPlan.ClampInterval(s.FakeFocusIntervalMs),
        Mappings = s.Mappings ?? [],
        SevenDays = s.SevenDays ?? new SevenDaysServerOptions(),
        SteamEmu = s.SteamEmu ?? new SteamEmuOptions(),
        NoMansSky = s.NoMansSky ?? new NoMansSkyOptions(),
        SelectedGame = LauncherGames.Find(s.SelectedGame).Id,
    };
}
