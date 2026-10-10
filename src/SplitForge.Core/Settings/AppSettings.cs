using System.Text.Json;
using System.Text.Json.Serialization;
using SplitForge.Core.Adapters;
using SplitForge.Core.Input;
using SplitForge.Core.Layout;

namespace SplitForge.Core.Settings;

/// <summary>Identidades del emulador de Steam educativo (EXPERIMENTAL, ADR-016). Sin datos personales: IDs ficticios locales.</summary>
public sealed record SteamEmuOptions
{
    public string SteamId1 { get; init; } = "76561198000000001";
    public string SteamId2 { get; init; } = "76561198000000002";
    public string Persona1 { get; init; } = "SplitForgeJ1";
    public string Persona2 { get; init; } = "SplitForgeJ2";
    public int AppId { get; init; } = 251570;
    public string EmulatorDll { get; init; } = "";
}

public sealed record AppSettings
{
    public SplitOrientation Orientation { get; init; } = SplitOrientation.SideBySide;
    public int Gutter { get; init; } = 4;
    public bool Borderless { get; init; } = true;
    public WindowFit Fit { get; init; } = WindowFit.Rellenar;
    public int AspectW { get; init; } = 16;
    public int AspectH { get; init; } = 9;
    public InputReadMode InputMode { get; init; } = InputReadMode.Auto;
    public int InputRateHz { get; init; } = 1000;
    public Dictionary<string, ControllerMapping> Mappings { get; init; } = [];
    public SevenDaysServerOptions SevenDays { get; init; } = new();
    public SteamEmuOptions SteamEmu { get; init; } = new();
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
        Mappings = s.Mappings ?? [],
        SevenDays = s.SevenDays ?? new SevenDaysServerOptions(),
        SteamEmu = s.SteamEmu ?? new SteamEmuOptions(),
    };
}
