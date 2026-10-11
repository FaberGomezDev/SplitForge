using SplitForge.Core.Adapters;

namespace SplitForge.Tests;

public class PlayReadinessTests
{
    [Fact]
    public void EverythingInPlaceMeansReady()
    {
        var steps = PlayReadiness.Evaluate(true, true, true, [1, 2], [1, 2]);
        Assert.True(PlayReadiness.Ready(steps));
        Assert.Equal(4, steps.Count);
        Assert.Equal("2 mandos conectados.", steps[3].Detail);
    }

    [Fact]
    public void EachMissingPieceIsNamedAndBlocksPlay()
    {
        var steps = PlayReadiness.Evaluate(gameInstalled: true, emulatorInstalled: false, filterInstalled: false, [1], [1, 2]);
        Assert.False(PlayReadiness.Ready(steps));
        Assert.Equal([true, false, false, false], steps.Select(s => s.Ok));
        Assert.Contains("emulador", steps[1].Detail);
        Assert.Contains("filtro", steps[2].Detail);
        Assert.Equal("Sin conectar: mando 2.", steps[3].Detail);
    }

    [Fact]
    public void TwoPlayersCannotShareOnePad()
    {
        var steps = PlayReadiness.Evaluate(true, true, true, [1, 2], [1, 1]);
        Assert.False(steps[3].Ok);
        Assert.Contains("mismo mando", steps[3].Detail);
    }

    [Fact]
    public void ArtComesFromTheLocalSteamCacheInEitherLayout()
    {
        using var tmp = new TempDir();
        var current = tmp.Combine("steam1");
        var older = tmp.Combine("steam2");
        Directory.CreateDirectory(Path.Combine(current, "appcache", "librarycache", "251570"));
        Directory.CreateDirectory(Path.Combine(older, "appcache", "librarycache"));
        var hero = Path.Combine(current, "appcache", "librarycache", "251570", "library_hero.jpg");
        var cover = Path.Combine(older, "appcache", "librarycache", "251570_library_600x900.jpg");
        File.WriteAllBytes(hero, [1]);
        File.WriteAllBytes(cover, [1]);

        var art = SteamArt.Find([current, older, tmp.Combine("no-existe")], "251570");
        Assert.Equal(hero, art.Hero);
        Assert.Equal(cover, art.Cover);
        Assert.Null(art.Logo);
        Assert.Equal(new SteamArt(null, null, null), SteamArt.Find([current], "999"));
    }
}
