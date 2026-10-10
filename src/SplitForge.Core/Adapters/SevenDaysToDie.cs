using System.Xml.Linq;
using SplitForge.Core.Layout;
using SplitForge.Core.Sessions;
using SplitForge.Core.Steam;

namespace SplitForge.Core.Adapters;

public sealed record SevenDaysServerOptions
{
    public int Port { get; init; } = 26900;
    public int MaxPlayers { get; init; } = 2;
    /// <summary>Por defecto sin EAC: el aislamiento de mandos lo necesitará y los clientes deben coincidir.</summary>
    public bool EacEnabled { get; init; }
    public string Password { get; init; } = string.Empty;
    public string GameName { get; init; } = "SplitForge";
    public int TelnetPort { get; init; } = 8081;

    public SevenDaysServerOptions Validate()
    {
        if (Port is < 1024 or > 65000)
            throw new ArgumentException("Puerto del servidor fuera del rango 1024–65000.");
        if (TelnetPort is < 1024 or > 65535 || TelnetPort == Port)
            throw new ArgumentException("Puerto de control inválido.");
        if (MaxPlayers is < 2 or > 8)
            throw new ArgumentException("Jugadores máximos fuera del rango 2–8.");
        if (string.IsNullOrWhiteSpace(GameName) || GameName.Length > 40 || GameName.IndexOfAny(Path.GetInvalidFileNameChars()) >= 0
            || GameName.Contains('\\') || GameName.Contains('/'))
            throw new ArgumentException("Nombre de partida inválido.");
        if (Password.Length > 64)
            throw new ArgumentException("Contraseña demasiado larga.");
        return this;
    }
}

/// <summary>
/// Adaptador experimental de 7 Days to Die: un servidor dedicado oficial local con guardados
/// aislados y dos clientes en pantalla dividida.
/// </summary>
public sealed class SevenDaysToDieAdapter : IGameAdapter
{
    public const string GameExe = "7DaysToDie.exe";
    public const string ServerExe = "7DaysToDieServer.exe";

    /// <summary>Título de la ventana principal del cliente (comprobado en V 3.3.0).</summary>
    public const string WindowTitle = "7 Days To Die";

    /// <summary>
    /// Proporción mínima de cada ventana. Comprobado el 2026-10-10 (V 3.3.0 b18): a 960×1032 el menú
    /// principal se corta por la izquierda; a 960×540 (16:9) se ve completo.
    /// </summary>
    public static readonly AspectRatio MinAspect = AspectRatio.SixteenNine;

    /// <summary>
    /// Argumentos de un cliente con el emulador de Steam (técnica del handler de Nucleus Co-op):
    /// sin juego cruzado (EOS rechaza los tickets locales y bloquea el menú), servidores por Steam,
    /// ventana sin marco y con el tamaño de su panel desde el arranque. El juego lee
    /// -crossplatform / -serverplatforms como «-clave=valor» (PlatformManager.GetCommandLineOverrides).
    /// </summary>
    public static IReadOnlyList<string> EmulatedClientArguments(int width, int height)
    {
        if (width <= 0 || height <= 0)
            throw new ArgumentOutOfRangeException(nameof(width), "El tamaño de la ventana debe ser positivo.");
        var inv = System.Globalization.CultureInfo.InvariantCulture;
        return
        [
            "-crossplatform=None", "-serverplatforms=Steam", "-noeac",
            "-screen-fullscreen", "0", "-popupwindow",
            "-screen-width", width.ToString(inv), "-screen-height", height.ToString(inv),
        ];
    }

    public AdapterManifest Manifest { get; } = new(
        ResearchProfiles.SevenDaysToDie, "7 Days to Die",
        SupportedBuilds: [],
        VerifiedModes: [],
        MinPlayers: 2,
        MaxPlayers: 4,
        NeedsAntiCheatDisabledByOfficialOption: true,
        "Servidor dedicado local + un cliente por jugador. Diagnóstico real (2026-10-10, sin EAC): carga xinput1_3 y xinput1_4, "
        + "DirectInput, HID e InControl (Unity). Aislamiento de mandos con el filtro XInput: pendiente de prueba con dos clientes.");

    public AdapterPreflight Preflight(AdapterEnvironment env)
    {
        var checks = new List<PreflightCheck>();
        var game = env.Games.FirstOrDefault(g => g.AppId == ResearchProfiles.SevenDaysToDie);
        var server = env.Games.FirstOrDefault(g => g.AppId == ResearchProfiles.SevenDaysToDieDedicatedServer);

        var gameExe = game is null ? null : FindExecutable(game, GameExe);
        checks.Add(game is null
            ? new("juego", "7 Days to Die instalado", CheckStatus.Failed, "No aparece en las bibliotecas Steam detectadas.")
            : gameExe is null
                ? new("juego", "7 Days to Die instalado", CheckStatus.Failed, $"Manifiesto encontrado, pero falta {GameExe} en la carpeta.")
                : new("juego", "7 Days to Die instalado", CheckStatus.Ok, $"Build {Display(game.BuildId)}."));

        var serverExe = server is null ? null : FindExecutable(server, ServerExe);
        checks.Add(serverExe is null
            ? new("servidor", "Servidor dedicado oficial", CheckStatus.Failed,
                "Instala gratis \"7 Days to Die Dedicated Server\" (Steam → Biblioteca → Herramientas).")
            : new("servidor", "Servidor dedicado oficial", CheckStatus.Ok, $"Build {Display(server!.BuildId)}."));

        if (game is not null && server is not null && game.BuildId.Length > 0 && server.BuildId.Length > 0)
        {
            // Builds distintas de Steam no implican versiones incompatibles, pero conviene actualizar ambos.
            checks.Add(new("versiones", "Juego y servidor actualizados", CheckStatus.Pending,
                "Si el cliente rechaza la conexión por versión, actualiza ambos en Steam."));
        }

        checks.Add(new("segundo_steam", "Segunda sesión de Steam", env.SandboxieAvailable ? CheckStatus.Ok : CheckStatus.Pending,
            env.SandboxieAvailable
                ? "Sandboxie-Plus detectado: puedes abrir el Steam del jugador 2 en un sandbox."
                : "Usa otro usuario de Windows o instala Sandboxie-Plus para el Steam del jugador 2."));
        checks.Add(new("eac", "Clientes sin EAC (opción oficial)", CheckStatus.Pending,
            "Steam → Propiedades → General → Preguntar al iniciar → \"Launch game without EAC\"."));
        checks.Add(new("mandos", "Dos mandos conectados", env.ControllersConnected >= 2 ? CheckStatus.Ok : CheckStatus.Warning,
            $"{env.ControllersConnected} mando(s) leídos por SDL3."));
        checks.Add(new("clientes", "Clientes abiertos", env.RunningClients >= 2 ? CheckStatus.Ok : CheckStatus.Pending,
            $"{env.RunningClients} proceso(s) de {GameExe} con ventana."));
        checks.Add(new("aislamiento", "Un mando por ventana dentro del juego", CheckStatus.Pending,
            "7DTD carga XInput (xinput1_3 y xinput1_4): instala el filtro XInput, detecta la ranura de cada mando y asígnalas. "
            + "Sin validar con dos clientes."));

        var state = gameExe is null ? CompatibilityState.Bloqueado : CompatibilityState.Inspeccionado;
        return new AdapterPreflight(Manifest.AppId, state, checks);
    }

    public static string? FindExecutable(InstalledGame game, string exe)
    {
        var (folder, inside) = PreflightEvaluator.SafeInstallFolder(game);
        if (!inside || !Directory.Exists(folder))
            return null;
        return Directory.EnumerateFiles(folder, exe, new EnumerationOptions { MatchCasing = MatchCasing.CaseInsensitive })
            .FirstOrDefault();
    }

    private static string Display(string buildId) => buildId.Length > 0 ? buildId : "desconocida";
}

/// <summary>
/// Genera el serverconfig.xml PROPIO de SplitForge a partir de la plantilla del servidor instalado,
/// sin modificar la carpeta del juego. Guardados aislados en la carpeta de datos de SplitForge.
/// </summary>
public static class SevenDaysServerConfig
{
    /// <summary>Plantilla mínima si el servidor no trae serverconfig.xml legible.</summary>
    public const string FallbackTemplate = """
        <?xml version="1.0"?>
        <ServerSettings>
          <property name="ServerName" value="SplitForge" />
          <property name="GameWorld" value="Navezgane" />
          <property name="GameDifficulty" value="1" />
        </ServerSettings>
        """;

    public static string Build(string templateXml, SevenDaysServerOptions options, string userDataFolder)
    {
        options.Validate();
        XDocument doc;
        try
        {
            doc = XDocument.Parse(templateXml);
        }
        catch (System.Xml.XmlException)
        {
            doc = XDocument.Parse(FallbackTemplate);
        }
        var root = doc.Root is { Name.LocalName: "ServerSettings" } r ? r : XDocument.Parse(FallbackTemplate).Root!;

        Set(root, "ServerName", "SplitForge local");
        Set(root, "ServerDescription", "Partida local de SplitForge (pantalla dividida)");
        Set(root, "ServerVisibility", "0"); // No se anuncia en la lista pública.
        Set(root, "ServerPort", options.Port.ToString(System.Globalization.CultureInfo.InvariantCulture));
        Set(root, "ServerPassword", options.Password);
        Set(root, "ServerMaxPlayerCount", options.MaxPlayers.ToString(System.Globalization.CultureInfo.InvariantCulture));
        Set(root, "ServerAllowCrossplay", "false");
        Set(root, "EACEnabled", options.EacEnabled ? "true" : "false");
        Set(root, "GameName", options.GameName);
        Set(root, "UserDataFolder", userDataFolder);
        // Telnet sin contraseña solo escucha en 127.0.0.1: se usa para un apagado ordenado.
        Set(root, "TelnetEnabled", "true");
        Set(root, "TelnetPort", options.TelnetPort.ToString(System.Globalization.CultureInfo.InvariantCulture));
        Set(root, "TelnetPassword", string.Empty);
        Set(root, "WebDashboardEnabled", "false");
        return new XDocument(new XDeclaration("1.0", "utf-8", null), root).ToString();
    }

    public static LaunchSpec Launch(string serverExe, string configPath, string logPath) => new(
        serverExe,
        ["-logfile", logPath, "-quit", "-batchmode", "-nographics", "-configfile=" + configPath, "-dedicated"],
        Path.GetDirectoryName(serverExe),
        RedirectInput: false,
        NoWindow: true);

    private static void Set(XElement root, string name, string value)
    {
        var existing = root.Elements("property")
            .FirstOrDefault(e => string.Equals((string?)e.Attribute("name"), name, StringComparison.OrdinalIgnoreCase));
        if (existing is null)
            root.Add(new XElement("property", new XAttribute("name", name), new XAttribute("value", value)));
        else
            existing.SetAttributeValue("value", value);
    }
}
