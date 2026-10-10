using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Xml.Linq;
using SplitForge.Core.Adapters;
using SplitForge.Core.Steam;

namespace SplitForge.Tests;

public class SevenDaysAdapterTests
{
    private static AdapterEnvironment Env(IReadOnlyList<InstalledGame> games, int pads = 2, int clients = 0, bool sandboxie = false) =>
        new(games, pads, clients, sandboxie);

    [Fact]
    public void BlockedWhenGameMissingAndNeverBeyondInspected()
    {
        var adapter = new SevenDaysToDieAdapter();
        var missing = adapter.Preflight(Env([]));
        Assert.Equal(CompatibilityState.Bloqueado, missing.State);
        Assert.Equal(CheckStatus.Failed, missing.Find("juego")!.Status);

        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        SteamFixture.CreateGame(root);
        SteamFixture.CreateGame(root, ResearchProfiles.SevenDaysToDieDedicatedServer, "7 Days to Die Dedicated Server",
            "7 Days to Die Dedicated Server", "7DaysToDieServer.exe");
        var ready = adapter.Preflight(Env(SteamLibrary.Scan([root]), pads: 2, clients: 2, sandboxie: true));
        Assert.Equal(CompatibilityState.Inspeccionado, ready.State); // Nunca COMPATIBLE automáticamente.
        Assert.Equal(CheckStatus.Ok, ready.Find("juego")!.Status);
        Assert.Equal(CheckStatus.Ok, ready.Find("servidor")!.Status);
        Assert.Equal(CheckStatus.Pending, ready.Find("aislamiento")!.Status);
        Assert.False(ready.HasFailures);
    }

    [Fact]
    public void ServerMissingIsExplained()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        SteamFixture.CreateGame(root);
        var p = new SevenDaysToDieAdapter().Preflight(Env(SteamLibrary.Scan([root]), pads: 1));
        Assert.Equal(CheckStatus.Failed, p.Find("servidor")!.Status);
        Assert.Contains("Dedicated Server", p.Find("servidor")!.Detail);
        Assert.Equal(CheckStatus.Warning, p.Find("mandos")!.Status);
    }

    [Fact]
    public void ManifestDeclaresNothingVerified()
    {
        var m = new SevenDaysToDieAdapter().Manifest;
        Assert.Empty(m.SupportedBuilds);
        Assert.Empty(m.VerifiedModes);
    }
}

public class SevenDaysServerConfigTests
{
    private const string Template = """
        <?xml version="1.0"?>
        <ServerSettings>
          <property name="ServerName" value="My Game Host" />
          <property name="ServerVisibility" value="2" />
          <property name="GameWorld" value="RWG" />
          <property name="EACEnabled" value="true" />
          <property name="ZombieMove" value="0" />
        </ServerSettings>
        """;

    private static Dictionary<string, string> Props(string xml) =>
        XDocument.Parse(xml).Root!.Elements("property").ToDictionary(e => (string)e.Attribute("name")!, e => (string)e.Attribute("value")!);

    [Fact]
    public void OverridesSafetyKeysAndKeepsGameplaySettings()
    {
        var xml = SevenDaysServerConfig.Build(Template, new SevenDaysServerOptions { Port = 27000 }, @"C:\Users\x\AppData\Local\SplitForge\7dtd\UserData");
        var p = Props(xml);
        Assert.Equal("0", p["ServerVisibility"]);
        Assert.Equal("27000", p["ServerPort"]);
        Assert.Equal("false", p["EACEnabled"]);
        Assert.Equal("RWG", p["GameWorld"]);
        Assert.Equal("0", p["ZombieMove"]);
        Assert.Equal(string.Empty, p["TelnetPassword"]); // Solo loopback.
        Assert.Equal("true", p["TelnetEnabled"]);
        Assert.Equal(@"C:\Users\x\AppData\Local\SplitForge\7dtd\UserData", p["UserDataFolder"]);
        Assert.Single(XDocument.Parse(xml).Root!.Elements("property"), e => (string)e.Attribute("name")! == "ServerPort");
    }

    [Fact]
    public void ValuesAreXmlEscaped()
    {
        var xml = SevenDaysServerConfig.Build(Template, new SevenDaysServerOptions { Password = "a\"/><property name=\"EACEnabled\" value=\"true" }, "D:\\x");
        var p = Props(xml);
        Assert.Equal("false", p["EACEnabled"]);
        Assert.StartsWith("a\"", p["ServerPassword"]);
    }

    [Fact]
    public void InvalidTemplateFallsBack()
    {
        var p = Props(SevenDaysServerConfig.Build("<roto", new SevenDaysServerOptions(), "D:\\x"));
        Assert.Equal("Navezgane", p["GameWorld"]);
        Assert.Equal("0", p["ServerVisibility"]);
    }

    [Theory]
    [InlineData(80, 8081, 2, "SplitForge")]
    [InlineData(26900, 26900, 2, "SplitForge")]
    [InlineData(26900, 8081, 1, "SplitForge")]
    [InlineData(26900, 8081, 2, "../evil")]
    [InlineData(26900, 8081, 2, "")]
    public void InvalidOptionsRejected(int port, int telnet, int players, string name) =>
        Assert.Throws<ArgumentException>(() => new SevenDaysServerOptions { Port = port, TelnetPort = telnet, MaxPlayers = players, GameName = name }.Validate());

    [Fact]
    public void LaunchUsesArgumentListWithIsolatedConfig()
    {
        var spec = SevenDaysServerConfig.Launch(@"C:\S\7DaysToDieServer.exe", @"C:\Data\serverconfig.xml", @"C:\Data\logs\s.log");
        Assert.Contains("-dedicated", spec.Arguments);
        Assert.Contains("-nographics", spec.Arguments);
        Assert.Contains(@"-configfile=C:\Data\serverconfig.xml", spec.Arguments);
        Assert.Equal(@"C:\Data\logs\s.log", spec.Arguments[spec.Arguments.ToList().IndexOf("-logfile") + 1]);
        Assert.True(spec.NoWindow);
    }
}

public class SevenDaysServerLifecycleTests
{
    [Fact]
    public async Task StartWritesOwnConfigAndShutdownIsGraceful()
    {
        using var tmp = new TempDir();
        var serverDir = tmp.Combine("server");
        Directory.CreateDirectory(serverDir);
        var exe = Path.Combine(serverDir, "7DaysToDieServer.exe");
        File.WriteAllBytes(exe, [0]);
        var template = Path.Combine(serverDir, "serverconfig.xml");
        File.WriteAllText(template, "<?xml version=\"1.0\"?><ServerSettings><property name=\"GameWorld\" value=\"Navezgane\"/></ServerSettings>");
        var templateBefore = File.ReadAllBytes(template);

        var listener = new TcpListener(IPAddress.Loopback, 0);
        listener.Start();
        var telnetPort = ((IPEndPoint)listener.LocalEndpoint).Port;
        var received = new TaskCompletionSource<string>();
        var launcher = new FakeLauncher();
        _ = Task.Run(async () =>
        {
            // Las conexiones de sondeo ("¿listo?") se cierran sin enviar nada; la de apagado envía el comando.
            while (!received.Task.IsCompleted)
            {
                try
                {
                    using var c = await listener.AcceptTcpClientAsync();
                    var s = c.GetStream();
                    await s.WriteAsync(Encoding.ASCII.GetBytes("*** Connected with 7DTD server.\r\n"));
                    var buf = new byte[64];
                    var n = await s.ReadAsync(buf).AsTask().WaitAsync(TimeSpan.FromSeconds(2));
                    if (n == 0)
                        continue;
                    received.TrySetResult(Encoding.ASCII.GetString(buf, 0, n));
                    launcher.Started[0].Process.HasExited = true;
                    launcher.Started[0].Process.ExitCode = 0;
                }
                catch (Exception ex) when (ex is IOException or SocketException or TimeoutException or ObjectDisposedException)
                {
                }
            }
        });

        var logs = new List<string>();
        var server = new SevenDaysServer(launcher, logs.Add);
        var paths = new SevenDaysServerPaths(tmp.Combine("data", "7dtd"));
        server.Start(exe, new SevenDaysServerOptions { TelnetPort = telnetPort }, paths);

        Assert.True(File.Exists(paths.ConfigFile));
        Assert.Equal(templateBefore, File.ReadAllBytes(template)); // La carpeta del servidor no se toca.
        Assert.Contains($"-configfile={paths.ConfigFile}", launcher.Started[0].Spec.Arguments);
        Assert.Equal(ServerPhase.Ready, server.Refresh());

        await server.StopAsync();
        Assert.Equal("shutdown\r\n", await received.Task.WaitAsync(TimeSpan.FromSeconds(5)));
        Assert.False(launcher.Started[0].Process.Killed);
        Assert.Equal(ServerPhase.Stopped, server.Phase);
        listener.Stop();
    }

    [Fact]
    public void UnexpectedExitIsReported()
    {
        using var tmp = new TempDir();
        var exe = tmp.Combine("7DaysToDieServer.exe");
        File.WriteAllBytes(exe, [0]);
        var launcher = new FakeLauncher();
        var logs = new List<string>();
        var server = new SevenDaysServer(launcher, logs.Add);
        server.Start(exe, new SevenDaysServerOptions { TelnetPort = 1 + 50_000 + Random.Shared.Next(1000) }, new SevenDaysServerPaths(tmp.Combine("d")));
        Assert.Throws<InvalidOperationException>(() => server.Start(exe, new SevenDaysServerOptions(), new SevenDaysServerPaths(tmp.Combine("d"))));
        launcher.Started[0].Process.HasExited = true;
        launcher.Started[0].Process.ExitCode = 3;
        Assert.Equal(ServerPhase.Exited, server.Refresh());
        Assert.Contains(logs, l => l.Contains("código 3"));
    }
}
