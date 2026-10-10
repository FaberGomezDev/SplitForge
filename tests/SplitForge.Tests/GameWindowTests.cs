using System.Text.Json;
using SplitForge.Core.Steam;
using SplitForge.Core.Windows;

namespace SplitForge.Tests;

public class WinPathTests
{
    private const string Folder = @"C:\SteamLibrary\steamapps\common\7 Days To Die";
    private const string Exe = @"C:\SteamLibrary\steamapps\common\7 Days To Die\7DaysToDie.exe";

    [Fact]
    public void PathBoundaries()
    {
        Assert.True(WinPath.IsGameExecutable(Exe, Folder));
        Assert.True(WinPath.IsGameExecutable(Exe.ToUpperInvariant(), Folder.ToLowerInvariant()));
        Assert.True(WinPath.IsGameExecutable(@"C:/SteamLibrary/steamapps/common/7 Days To Die/sub/../7DaysToDie.exe", Folder));
        Assert.False(WinPath.IsGameExecutable(@"C:\SteamLibrary\steamapps\common\7 Days To Die HACK\x.exe", Folder));
        Assert.False(WinPath.IsGameExecutable(@"C:\SteamLibrary\steamapps\common\Other\game.exe", Folder));
        Assert.False(WinPath.IsGameExecutable(@"C:\SteamLibrary\steamapps\common\7 Days To Die\..\Other\game.exe", Folder));
        Assert.False(WinPath.IsGameExecutable(@"relative\7DaysToDie.exe", Folder));
        Assert.False(WinPath.IsGameExecutable(@"\\host\share\7DaysToDie.exe", Folder));
        Assert.False(WinPath.IsGameExecutable(@"\\?\C:\SteamLibrary\steamapps\common\7 Days To Die\7DaysToDie.exe", Folder));
        Assert.False(WinPath.IsGameExecutable(Exe.Replace(".exe", ".dll"), Folder));
        Assert.False(WinPath.IsGameExecutable(null, Folder));
    }

    [Theory]
    [InlineData(@"C:\a\..\..\b", @"c:\b")]
    [InlineData(@"""C:\Games\X.exe"" ", @"c:\games\x.exe")]
    [InlineData(@"C:\a\.\b\\c", @"c:\a\b\c")]
    [InlineData(@"a\..\..\b", @"..\b")]
    public void NormalizeMatchesNtpath(string input, string expected) => Assert.Equal(expected, WinPath.Normalize(input));
}

public class GameWindowInspectorTests
{
    private static readonly InstalledGame Game = new("251570", "7 Days to Die",
        @"C:\SteamLibrary\steamapps\common\7 Days To Die", @"C:\SteamLibrary", "12345");

    private const string Exe = @"C:\SteamLibrary\steamapps\common\7 Days To Die\7DaysToDie.exe";

    private static string? Same(int _) => Exe;

    [Fact]
    public void NoWindows()
    {
        var r = GameWindowInspector.Inspect(Game, [], Same);
        Assert.Equal(GameWindowStates.None, r.State);
        Assert.Empty(r.ChosenHandles);
    }

    [Fact]
    public void SingleProcessTwoWindowsIsNotTwoClients()
    {
        var r = GameWindowInspector.Inspect(Game, [new(100, 9, "Main"), new(101, 9, "Settings")], Same);
        Assert.Equal(2, r.Windows.Count);
        Assert.Equal(1, r.DistinctPids);
        Assert.Empty(r.ChosenHandles);
        Assert.Equal(GameWindowStates.Unpaired, r.State);
    }

    [Fact]
    public void TwoClientsSameBinary()
    {
        WindowInfo[] visible = [new(100, 9, "7DTD P1"), new(101, 10, "7DTD P2")];
        var r = GameWindowInspector.Inspect(Game, visible, Same);
        Assert.Equal(GameWindowStates.TwoClients, r.State);
        Assert.Equal([(nint)100, 101], r.ChosenHandles);
        Assert.Equal(((nint)100, (nint)101), GameWindowInspector.VerifyPair(Game, r, visible, Same));
    }

    [Fact]
    public void UpToFourClientsOrderedByPid()
    {
        WindowInfo[] visible = [new(400, 40, "c"), new(100, 10, "a"), new(300, 30, "b"), new(200, 20, "d"), new(500, 50, "e")];
        var r = GameWindowInspector.Inspect(Game, visible, Same);
        Assert.Equal([(nint)100, 200, 300, 400], r.ClientHandles);
        Assert.Equal([(nint)100, 200], r.ChosenHandles);
    }

    [Fact]
    public void DifferentExesAreNotTwoClients()
    {
        var r = GameWindowInspector.Inspect(Game, [new(100, 9, "7DTD"), new(101, 10, "Launcher")],
            pid => pid == 9 ? Exe : Exe.Replace("7DaysToDie.exe", "7dLauncher.exe"));
        Assert.Equal(2, r.DistinctPids);
        Assert.Empty(r.ChosenHandles);
    }

    [Fact]
    public void OtherGamesIgnored()
    {
        var r = GameWindowInspector.Inspect(Game, [new(100, 9, "7DTD"), new(101, 10, "Other")],
            pid => pid == 9 ? Exe : @"C:\other\game.exe");
        Assert.Single(r.Windows);
    }

    [Fact]
    public void RevalidatesBeforeAttach()
    {
        var r = GameWindowInspector.Inspect(Game, [new(100, 9, "P1"), new(101, 10, "P2")], Same);
        Assert.Throws<InvalidOperationException>(() =>
            GameWindowInspector.VerifyPair(Game, r, [new(100, 9, "P1"), new(102, 10, "P2")], Same));
        Assert.Throws<InvalidOperationException>(() =>
            GameWindowInspector.VerifyPair(Game, r, [new(100, 9, "P1"), new(101, 10, "P2")],
                pid => pid == 9 ? Exe : @"C:\other\game.exe"));
    }

    [Fact]
    public void PublicReportNeverExposesPathsTitlesOrHandles()
    {
        var r = GameWindowInspector.Inspect(Game, [new(100, 9, "Sala de Fulanito")], Same);
        var json = JsonSerializer.Serialize(r.ToPublic());
        Assert.DoesNotContain("SteamLibrary", json);
        Assert.DoesNotContain("Fulanito", json);
        Assert.DoesNotContain("100", json);
        Assert.Contains("7DaysToDie.exe", json);
        Assert.Contains("\"cooperativo_verificado\":false", json);
    }

    [Fact]
    public void LookupErrorsDoNotCrash()
    {
        var r = GameWindowInspector.Inspect(Game, [new(100, 9, "P1")], _ => throw new UnauthorizedAccessException());
        Assert.Empty(r.Windows);
    }

    [Fact]
    public void SameHandleNoDuplicate()
    {
        var r = GameWindowInspector.Inspect(Game, [new(100, 9, "P1"), new(100, 10, "P2")], Same);
        Assert.Single(r.Windows);
    }
}
