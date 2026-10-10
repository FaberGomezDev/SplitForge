using System.Text.Json.Serialization;

namespace SplitForge.Core.Diagnostics;

/// <summary>
/// Clasifica los módulos cargados por un juego para saber cómo lee los mandos (XInput, DirectInput,
/// HID, Windows.Gaming.Input...). Solo recibe NOMBRES de módulo, nunca rutas.
/// </summary>
public static class InputModuleClassifier
{
    public static readonly IReadOnlyDictionary<string, string[]> Families = new Dictionary<string, string[]>
    {
        ["xinput"] = ["xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll", "xinputuap.dll"],
        ["directinput"] = ["dinput8.dll", "dinput.dll"],
        ["hid"] = ["hid.dll"],
        ["windows_gaming_input"] = ["windows.gaming.input.dll"],
        ["gameinput"] = ["gameinput.dll"],
        ["sdl"] = ["sdl2.dll", "sdl3.dll"],
        ["steam_api"] = ["steam_api64.dll", "steam_api.dll"],
        ["steam_overlay_input"] = ["gameoverlayrenderer64.dll", "gameoverlayrenderer.dll"],
        ["easy_anti_cheat"] = ["easyanticheat_x64.dll", "easyanticheat.dll", "eosovh-win64-shipping.dll"],
    };

    private static readonly string[] Keywords = ["input", "xinput", "hid", "pad", "joy", "rewired", "incontrol", "steam", "anticheat", "eos"];

    public static InputModuleSummary Classify(IEnumerable<string> moduleNames)
    {
        var lowered = moduleNames.Select(n => Path.GetFileName(n.Replace('\\', '/')).ToLowerInvariant())
            .Where(n => n.Length > 0).Distinct().Order(StringComparer.Ordinal).ToList();
        var families = new SortedDictionary<string, IReadOnlyList<string>>(StringComparer.Ordinal);
        foreach (var (family, names) in Families)
        {
            var hits = lowered.Where(names.Contains).ToList();
            if (hits.Count > 0)
                families[family] = hits;
        }
        var relevant = lowered.Where(n => Keywords.Any(n.Contains)).ToList();
        return new InputModuleSummary(families, relevant, lowered.Count);
    }
}

public sealed record InputModuleSummary(
    [property: JsonPropertyName("familias")] IReadOnlyDictionary<string, IReadOnlyList<string>> Families,
    [property: JsonPropertyName("modulos_relevantes")] IReadOnlyList<string> Relevant,
    [property: JsonPropertyName("total_modulos")] int Total)
{
    /// <summary>Windows.Gaming.Input solo entrega mandos a la ventana con foco.</summary>
    [JsonIgnore] public bool RequiresFocus => Families.ContainsKey("windows_gaming_input");
    [JsonIgnore] public bool AntiCheatLoaded => Families.ContainsKey("easy_anti_cheat");
}

public sealed record GameProcessDiagnostic(
    [property: JsonPropertyName("ejecutable")] string Executable,
    [property: JsonPropertyName("ventanas_visibles")] int VisibleWindows,
    [property: JsonPropertyName("modulos_legibles")] bool ModulesReadable,
    [property: JsonPropertyName("entrada")] InputModuleSummary? Input,
    [property: JsonPropertyName("nota")] string? Note);

public sealed record SevenDaysDiagnosticReport(
    [property: JsonPropertyName("schema")] string Schema,
    [property: JsonPropertyName("clientes_7dtd")] int ClientCount,
    [property: JsonPropertyName("servidores_7dtd")] int ServerCount,
    [property: JsonPropertyName("clientes")] IReadOnlyList<GameProcessDiagnostic> Clients,
    [property: JsonPropertyName("servidores")] IReadOnlyList<GameProcessDiagnostic> Servers,
    [property: JsonPropertyName("mandos_sdl3")] IReadOnlyList<string> Controllers,
    [property: JsonPropertyName("aviso")] string Notice)
{
    public static SevenDaysDiagnosticReport Create(IReadOnlyList<GameProcessDiagnostic> clients,
        IReadOnlyList<GameProcessDiagnostic> servers, IReadOnlyList<string> controllers) =>
        new("SplitForge 7DTD Diagnostico v2", clients.Count, servers.Count, clients, servers, controllers,
            "Solo lectura. Sin rutas, títulos de ventana, cuentas ni identificadores únicos de mandos.");

    public string ToJson() => System.Text.Json.JsonSerializer.Serialize(this, Steam.ReportJson.Options);
}
