using System.Text.Json;
using SplitForge.Core.Diagnostics;
using SplitForge.Core.Input;
using SplitForge.Core.Steam;
using SplitForge.Input;
using SplitForge.Platform;

Console.OutputEncoding = System.Text.Encoding.UTF8;
var command = args.Length > 0 ? args[0].ToLowerInvariant() : "ayuda";
string? Option(string name)
{
    var i = Array.IndexOf(args, name);
    return i >= 0 && i + 1 < args.Length ? args[i + 1] : null;
}

try
{
    return command switch
    {
        "steam" => Steam(Option("--salida")),
        "ventanas" => OperatingSystem.IsWindows()
            ? Windows(Option("--appid") ?? ResearchProfiles.SevenDaysToDie, args.Contains("--todos"), Option("--salida"))
            : NotWindows(),
        "7dtd" => OperatingSystem.IsWindows() ? SevenDays(Option("--salida")) : NotWindows(),
        "mandos" => Pads(int.TryParse(Option("--segundos"), out var s) ? Math.Clamp(s, 1, 120) : 20),
        "demo-frames" => DemoFrames(int.TryParse(Option("--segundos"), out var d) ? Math.Clamp(d, 1, 600) : 10),
        "visor-lab" => SplitForge.LabViewer.LabViewer.Run(args.Length > 1 && args[1] == "2" ? 2 : 1, Console.OpenStandardInput()),
        _ => Help(),
    };
}
catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException)
{
    Console.Error.WriteLine("ERROR: " + ex.Message);
    return 1;
}

static int Help()
{
    Console.WriteLine("""
        SplitForge.Diagnostico — herramientas de solo lectura

          steam      [--salida archivo.json]   Juegos Steam instalados (informe sin rutas personales)
          ventanas   [--appid 251570] [--todos] [--salida archivo.json]
                                               Ventanas/procesos de un juego abierto
          7dtd       [--salida archivo.json]   7 Days to Die abierto: APIs de mando cargadas
          mandos     [--segundos 20]           Prueba en vivo de los mandos con SDL3
        """);
    return 0;
}

static int NotWindows()
{
    Console.WriteLine("Este diagnóstico requiere Windows 10/11.");
    return 2;
}

static IReadOnlyList<InstalledGame> ScanSteam() =>
    OperatingSystem.IsWindows() ? WindowsDiagnostics.ScanSteam() : [];

static string Target(string? requested, string fileName)
{
    if (requested is not null)
        return requested;
    var desktop = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
    return Path.Combine(Directory.Exists(desktop) ? desktop : Environment.CurrentDirectory, fileName);
}

static int Steam(string? output)
{
    var games = ScanSteam();
    var target = Target(output, "SplitForge_Reporte_Steam.json");
    File.WriteAllText(target, SteamPublicReport.Create(games).ToJson());
    Console.WriteLine($"Instalaciones encontradas: {games.Count}");
    foreach (var p in games.Select(PreflightEvaluator.Evaluate))
        Console.WriteLine($" - {p.Name} [AppID {p.AppId}] | {p.Phase} | doble jugador verificado: NO");
    Console.WriteLine("Reporte guardado: " + target);
    Console.WriteLine("El reporte no incluye rutas absolutas, cuentas de Steam ni datos de mandos.");
    return 0;
}

[System.Runtime.Versioning.SupportedOSPlatform("windows")]
static int Windows(string appId, bool all, string? output)
{
    var games = ScanSteam();
    var selected = all ? games : [.. games.Where(g => g.AppId == appId)];
    var reports = WindowsDiagnostics.InspectGames(selected).Select(r => r.ToPublic()).ToList();
    var target = Target(output, "SplitForge_Reporte_Ventanas_Steam.json");
    File.WriteAllText(target, JsonSerializer.Serialize(new
    {
        schema = "SplitForge Steam Windows v1",
        installed_count = games.Count,
        requested_appid = all ? null : appId,
        games = reports,
        notice = "Solo enumera procesos. Dos instancias no garantizan multijugador, offline ni aislamiento de mandos.",
    }, ReportJson.Options));
    Console.WriteLine($"Juegos encontrados: {games.Count} | analizados: {reports.Count}");
    foreach (var r in reports)
        Console.WriteLine($"{r.Name}: {r.State} ({r.Windows} ventanas; {r.DistinctProcesses} procesos)");
    if (reports.Count == 0)
        Console.WriteLine("El juego solicitado no aparece instalado en la biblioteca detectada.");
    Console.WriteLine("Reporte guardado: " + target);
    return 0;
}

[System.Runtime.Versioning.SupportedOSPlatform("windows")]
static int SevenDays(string? output)
{
    var controllers = ReadControllers(TimeSpan.FromSeconds(1.5)).Select(p => p.GamepadType.Length > 0 ? $"{p.Name} ({p.GamepadType})" : p.Name).ToList();
    var report = WindowsDiagnostics.SevenDays(controllers);
    var clients = report.Clients;
    var servers = report.Servers;
    var target = Target(output, "SplitForge_Reporte_7DTD.json");
    File.WriteAllText(target, report.ToJson());

    Console.WriteLine($"Clientes 7DTD abiertos: {clients.Count} | Servidores: {servers.Count} | Mandos SDL3: {controllers.Count}");
    for (var i = 0; i < clients.Count; i++)
    {
        var c = clients[i];
        if (c.Input is null)
        {
            Console.WriteLine($"  Cliente {i + 1}: {c.Note}");
            continue;
        }
        var families = c.Input.Families.Count > 0 ? string.Join(", ", c.Input.Families.Keys) : "ninguna detectada";
        Console.WriteLine($"  Cliente {i + 1}: ventanas={c.VisibleWindows} | APIs de entrada: {families}"
                          + (c.Input.RequiresFocus ? " | ATENCIÓN: Windows.Gaming.Input exige foco" : string.Empty));
    }
    if (clients.Count == 0)
        Console.WriteLine("No hay 7 Days to Die abierto. Ábrelo (sin EAC) y vuelve a ejecutar este diagnóstico.");
    Console.WriteLine("Reporte guardado: " + target);
    return 0;
}

static IReadOnlyList<PadInfo> ReadControllers(TimeSpan wait)
{
    using var input = new SdlInputService(InputReadMode.Auto, new Dictionary<string, ControllerMapping>(), 250);
    Thread.Sleep(wait);
    return input.Latest.Devices;
}

/// <summary>Prueba del visor sin mandos: emite fotogramas sintéticos por stdout (demo-frames | visor-lab 1).</summary>
static int DemoFrames(int seconds)
{
    var world = new SplitForge.Core.Lab.SharedWorld();
    using var stdout = Console.OpenStandardOutput();
    var until = DateTime.UtcNow.AddSeconds(seconds);
    var t = 0.0;
    while (DateTime.UtcNow < until)
    {
        t += 1 / 120.0;
        var p1 = new PadState(1, (float)Math.Cos(t), (float)Math.Sin(t), Fire: (int)(t * 2) % 2 == 0);
        var p2 = new PadState(2, (float)-Math.Sin(t), (float)Math.Cos(t), Shield: true);
        stdout.Write(SplitForge.Core.Ipc.WorldCodec.Encode(world.Step(1 / 120.0, p1, p2)));
        stdout.Flush();
        Thread.Sleep(8);
    }
    return 0;
}

static int Pads(int seconds)
{
    using var input = new SdlInputService(InputReadMode.Auto, new Dictionary<string, ControllerMapping>(), 1000, Console.WriteLine);
    Thread.Sleep(800);
    Console.WriteLine($"Motor: {input.Backend}{(input.Error is null ? string.Empty : " | " + input.Error)}");
    Console.WriteLine($"Mueve los sticks de TODOS los mandos y pulsa Cruz durante {seconds} s (funciona aunque otra ventana tenga el foco).");
    var last = new Dictionary<uint, string>();
    var until = DateTime.UtcNow.AddSeconds(seconds);
    while (DateTime.UtcNow < until)
    {
        var snap = input.Latest;
        foreach (var d in snap.Devices)
        {
            snap.States.TryGetValue(d.DeviceId, out var s);
            snap.Raw.TryGetValue(d.DeviceId, out var raw);
            var line = $"ID {d.DeviceId} {d.Name} [{snap.Paths.GetValueOrDefault(d.DeviceId)}] X={s.LX:+0.0;-0.0} Y={s.LY:+0.0;-0.0} "
                       + $"Disparo={(s.Fire ? 1 : 0)} | botones=[{string.Join(',', raw?.PressedButtons ?? [])}]";
            if (!last.TryGetValue(d.DeviceId, out var previous) || previous != line)
            {
                Console.WriteLine($"{seconds - (until - DateTime.UtcNow).TotalSeconds:00.0}s | {line}");
                last[d.DeviceId] = line;
            }
        }
        Thread.Sleep(50);
    }
    Console.WriteLine($"Frecuencia medida del hilo de entrada: {input.Latest.MeasuredHz:0} Hz. Mandos: {input.Latest.Devices.Count}.");
    return 0;
}
