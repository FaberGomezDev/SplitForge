using System.Text.Json;
using SplitForge.Core.Steam;

namespace SplitForge.Tests;

public sealed class TempDir : IDisposable
{
    public TempDir() => Directory.CreateDirectory(Path);

    public string Path { get; } = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "sf-" + Guid.NewGuid().ToString("N")[..10]);

    public string Combine(params string[] parts) => System.IO.Path.Combine([Path, .. parts]);

    public void Dispose()
    {
        try
        {
            Directory.Delete(Path, recursive: true);
        }
        catch (IOException)
        {
        }
    }
}

internal static class SteamFixture
{
    public static string CreateGame(string steamRoot, string appId = "251570", string name = "7 Days to Die",
        string installDir = "7 Days To Die", string? exe = "7DaysToDie.exe", bool installed = true, string buildId = "100234")
    {
        var apps = Path.Combine(steamRoot, "steamapps");
        Directory.CreateDirectory(apps);
        var folder = Path.Combine(apps, "common", installDir);
        if (installed)
        {
            Directory.CreateDirectory(folder);
            if (exe is not null)
                File.WriteAllBytes(Path.Combine(folder, exe), [1, 2, 3]);
        }
        var manifest = Path.Combine(apps, $"appmanifest_{appId}.acf");
        File.WriteAllText(manifest,
            $"\"AppState\"\n{{\n  \"appid\" \"{appId}\"\n  \"name\" \"{name}\"\n  \"installdir\" \"{installDir}\"\n"
            + $"  \"buildid\" \"{buildId}\"\n  \"StateFlags\" \"4\"\n}}");
        return manifest;
    }
}

public class KeyValuesTests
{
    [Fact]
    public void ParsesNestedSectionsCaseInsensitive()
    {
        var root = KeyValuesParser.Parse("\"AppState\" { \"Name\" \"7 Days to Die\" \"UserConfig\" { \"language\" \"spanish\" } }");
        var state = root.Child("appstate");
        Assert.NotNull(state);
        Assert.Equal("7 Days to Die", state["name"]);
        Assert.Equal("spanish", state.Child("userconfig")?["LANGUAGE"]);
    }

    [Fact]
    public void UnescapesWindowsPaths()
    {
        var root = KeyValuesParser.Parse("\"libraryfolders\" { \"0\" { \"path\" \"D:\\\\SteamLibrary\" } }");
        Assert.Equal(@"D:\SteamLibrary", root.Child("libraryfolders")?.Child("0")?["path"]);
    }

    [Fact]
    public void ToleratesCommentsConditionalsAndTruncation()
    {
        var root = KeyValuesParser.Parse("// comentario\n\"A\" { \"x\" \"1\" [$WIN32]\n \"y\" \"2\" \n \"B\" { \"z\" ");
        var a = root.Child("A");
        Assert.Equal("1", a?["x"]);
        Assert.Equal("2", a?["y"]);
    }

    [Fact]
    public void DeepNestingIsBounded()
    {
        var text = string.Concat(Enumerable.Repeat("\"k\" { ", 500));
        var root = KeyValuesParser.Parse(text);
        var depth = 0;
        for (var n = root; n.Children.Count > 0; n = n.Children[0])
            depth++;
        Assert.True(depth <= KeyValuesParser.MaxDepth);
    }
}

public class SteamLibraryTests
{
    [Fact]
    public void ScanReadsManifestWithoutChangingIt()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        var manifest = SteamFixture.CreateGame(root);
        var before = File.ReadAllBytes(manifest);
        var games = SteamLibrary.Scan([root]);
        var game = Assert.Single(games);
        Assert.Equal("251570", game.AppId);
        Assert.Equal("100234", game.BuildId);
        Assert.Equal(before, File.ReadAllBytes(manifest));
    }

    [Fact]
    public void SecondaryLibrariesModernAndLegacy()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        var second = tmp.Combine("SteamLibrary");
        var third = tmp.Combine("Old");
        Directory.CreateDirectory(Path.Combine(root, "steamapps"));
        Directory.CreateDirectory(second);
        Directory.CreateDirectory(third);
        File.WriteAllText(Path.Combine(root, "steamapps", "libraryfolders.vdf"),
            "\"libraryfolders\"\n{\n\"contentstatsid\" \"123\"\n\"1\"\n{\n\"path\" \"" + second.Replace("\\", "\\\\") + "\"\n}\n\"2\" \"" + third.Replace("\\", "\\\\") + "\"\n}");
        var libs = SteamLibrary.LibraryPaths(root);
        Assert.Contains(second, libs);
        Assert.Contains(third, libs);
        Assert.Equal(3, libs.Count);
    }

    [Fact]
    public void ConfigIndexLocationIsAlsoRead()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        var another = tmp.Combine("SteamLibrary");
        Directory.CreateDirectory(Path.Combine(root, "config"));
        Directory.CreateDirectory(another);
        File.WriteAllText(Path.Combine(root, "config", "libraryfolders.vdf"),
            "\"libraryfolders\" { \"1\" { \"path\" \"" + another.Replace("\\", "\\\\") + "\" } }");
        Assert.Contains(another, SteamLibrary.LibraryPaths(root));
    }

    [Fact]
    public void GamesFromSeveralLibrariesAreSortedByName()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        var second = tmp.Combine("Lib2");
        SteamFixture.CreateGame(root, "275850", "No Man's Sky", "No Man's Sky", "NMS.exe");
        SteamFixture.CreateGame(second, "251570");
        File.WriteAllText(Path.Combine(root, "steamapps", "libraryfolders.vdf"),
            "\"libraryfolders\" { \"1\" { \"path\" \"" + second.Replace("\\", "\\\\") + "\" } }");
        var names = SteamLibrary.Scan([root]).Select(g => g.Name).ToList();
        Assert.Equal(["7 Days to Die", "No Man's Sky"], names);
    }
}

public class PreflightTests
{
    [Fact]
    public void DetectsExecutableWithoutDeclaringCoop()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        SteamFixture.CreateGame(root);
        var p = PreflightEvaluator.Evaluate(SteamLibrary.Scan([root])[0]);
        Assert.True(p.FolderPresent);
        Assert.True(p.InsideSteamLibrary);
        Assert.True(p.ExpectedExecutableFound);
        Assert.False(p.ReadyForTwoPlayers);
        Assert.False(p.OfflineCoopVerified);
        Assert.False(p.InputIsolationVerified);
        Assert.False(p.SimultaneousClientsVerified);
    }

    [Fact]
    public void MissingExecutableDoesNotClaimSupport()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        SteamFixture.CreateGame(root, exe: null);
        var p = PreflightEvaluator.Evaluate(SteamLibrary.Scan([root])[0]);
        Assert.False(p.ExpectedExecutableFound);
        Assert.False(p.ReadyForTwoPlayers);
    }

    [Fact]
    public void ReportExcludesSensitivePaths()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Users", "private-name", "Steam");
        SteamFixture.CreateGame(root);
        var json = SteamPublicReport.Create(SteamLibrary.Scan([root])).ToJson();
        using var doc = JsonDocument.Parse(json);
        Assert.Equal(1, doc.RootElement.GetProperty("game_count").GetInt32());
        var game = doc.RootElement.GetProperty("games")[0];
        Assert.Equal("251570", game.GetProperty("app_id").GetString());
        Assert.False(game.GetProperty("ready_for_two_players").GetBoolean());
        Assert.False(game.TryGetProperty("folder", out _));
        Assert.DoesNotContain("private-name", json);
        Assert.DoesNotContain(tmp.Path, json);
    }

    [Fact]
    public void PathOutsideLibraryRejected()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        Directory.CreateDirectory(Path.Combine(root, "steamapps", "common"));
        Directory.CreateDirectory(tmp.Combine("secret"));
        var game = new InstalledGame("251570", "7 Days to Die", Path.Combine(root, "..", "secret"), root);
        var p = PreflightEvaluator.Evaluate(game);
        Assert.False(p.InsideSteamLibrary);
        Assert.False(p.FolderPresent);
    }

    [Fact]
    public void ManifestTraversalOutOfCommonRejected()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        SteamFixture.CreateGame(root, installDir: "../../../Windows");
        SteamFixture.CreateGame(root, appId: "1", name: "Raíz", installDir: ".");
        foreach (var game in SteamLibrary.Scan([root]))
            Assert.False(PreflightEvaluator.Evaluate(game).InsideSteamLibrary);
    }

    [Fact]
    public void SymlinkEscapingCommonRejected()
    {
        if (OperatingSystem.IsWindows())
            return; // Crear enlaces requiere privilegios en Windows; la lógica es la misma.
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        var common = Path.Combine(root, "steamapps", "common");
        Directory.CreateDirectory(common);
        var outside = tmp.Combine("outside");
        Directory.CreateDirectory(outside);
        Directory.CreateSymbolicLink(Path.Combine(common, "Trap"), outside);
        var p = PreflightEvaluator.Evaluate(new InstalledGame("251570", "Trap", Path.Combine(common, "Trap"), root));
        Assert.False(p.InsideSteamLibrary);
    }

    [Fact]
    public void UnknownAppIdNotVerified()
    {
        using var tmp = new TempDir();
        var root = tmp.Combine("Steam");
        var folder = Path.Combine(root, "steamapps", "common", "Unknown");
        Directory.CreateDirectory(folder);
        File.WriteAllBytes(Path.Combine(folder, "MyGame.exe"), [0]);
        var p = PreflightEvaluator.Evaluate(new InstalledGame("99999999", "Unknown", folder, root));
        Assert.Contains("Sin perfil", p.Phase);
        Assert.Null(p.ExpectedExecutableFound);
        Assert.Equal(["MyGame.exe"], p.ExecutablesFound);
    }

    [Theory]
    [InlineData("")]
    [InlineData("abc")]
    [InlineData("-251570")]
    [InlineData("251570?x=y")]
    [InlineData("0")]
    [InlineData("1/../../")]
    public void SteamUriRejectsInvalid(string value) =>
        Assert.Throws<ArgumentException>(() => PreflightEvaluator.SteamRunUri(value));

    [Fact]
    public void SteamUriValid() => Assert.Equal("steam://rungameid/251570", PreflightEvaluator.SteamRunUri("251570"));
}
