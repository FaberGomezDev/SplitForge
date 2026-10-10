using SplitForge.Core.Layout;

namespace SplitForge.Tests;

public class LayoutTests
{
    private static readonly Region Screen = new(0, 0, 1920, 1080);

    [Fact]
    public void SideBySide1080p()
    {
        var r = SplitLayout.Compute(Screen, 2, SplitOrientation.SideBySide, 4);
        Assert.Equal(new Region(0, 0, 958, 1080), r[0]);
        Assert.Equal(new Region(962, 0, 958, 1080), r[1]);
    }

    [Fact]
    public void Stacked1080p()
    {
        var r = SplitLayout.Compute(Screen, 2, SplitOrientation.Stacked, 8);
        Assert.Equal(new Region(0, 0, 1920, 536), r[0]);
        Assert.Equal(new Region(0, 544, 1920, 536), r[1]);
    }

    [Fact]
    public void OddPixelsExhaustScreen()
    {
        var r = SplitLayout.Compute(new Region(0, 0, 1365, 767), 2, SplitOrientation.SideBySide, 3);
        Assert.Equal(1365, r[1].Right);
        Assert.Equal(1365, r[0].Width + r[1].Width + 3);
    }

    [Fact]
    public void WorkAreaOffsetIsRespected()
    {
        // Monitor secundario o barra de tareas a la izquierda.
        var r = SplitLayout.Compute(new Region(1920, 40, 1920, 1040), 2, SplitOrientation.SideBySide, 0);
        Assert.Equal(1920, r[0].X);
        Assert.Equal(40, r[0].Y);
        Assert.Equal(3840, r[1].Right);
    }

    [Theory]
    [InlineData(SplitOrientation.SideBySide)]
    [InlineData(SplitOrientation.Stacked)]
    public void ThreePlayersCoverScreenWithoutOverlap(SplitOrientation orientation)
    {
        var r = SplitLayout.Compute(Screen, 3, orientation, 4);
        Assert.Equal(3, r.Count);
        AssertNoOverlap(r);
        Assert.All(r, x => Assert.True(x.X >= 0 && x.Y >= 0 && x.Right <= Screen.Width && x.Bottom <= Screen.Height));
        // Solo se pierden las franjas de separación (dos como máximo).
        Assert.True(r.Sum(x => (long)x.Width * x.Height) >= (long)(Screen.Width - 4) * (Screen.Height - 4));
    }

    [Fact]
    public void FourPlayersGrid()
    {
        var r = SplitLayout.Compute(Screen, 4, SplitOrientation.SideBySide, 0);
        Assert.Equal(new Region(0, 0, 960, 540), r[0]);
        Assert.Equal(new Region(960, 0, 960, 540), r[1]);
        Assert.Equal(new Region(0, 540, 960, 540), r[2]);
        Assert.Equal(new Region(960, 540, 960, 540), r[3]);
    }

    [Fact]
    public void InvalidInput()
    {
        Assert.Throws<ArgumentException>(() => SplitLayout.Compute(new Region(0, 0, 0, 1080), 2, SplitOrientation.SideBySide, 4));
        Assert.Throws<ArgumentOutOfRangeException>(() => SplitLayout.Compute(Screen, 2, (SplitOrientation)7, 4));
        Assert.Throws<ArgumentOutOfRangeException>(() => SplitLayout.Compute(Screen, 2, SplitOrientation.SideBySide, 100));
        Assert.Throws<ArgumentOutOfRangeException>(() => SplitLayout.Compute(Screen, 5, SplitOrientation.SideBySide, 4));
    }

    [Fact]
    public void LegacyOrientationNames()
    {
        Assert.Equal(SplitOrientation.SideBySide, SplitLayout.ParseLegacy("vertical"));
        Assert.Equal(SplitOrientation.Stacked, SplitLayout.ParseLegacy("horizontal"));
        Assert.Equal(SplitOrientation.SideBySide, SplitLayout.ParseLegacy(null));
    }

    private static void AssertNoOverlap(IReadOnlyList<Region> regions)
    {
        for (var i = 0; i < regions.Count; i++)
        {
            for (var j = i + 1; j < regions.Count; j++)
            {
                var a = regions[i];
                var b = regions[j];
                var overlap = a.X < b.Right && b.X < a.Right && a.Y < b.Bottom && b.Y < a.Bottom;
                Assert.False(overlap, $"{a} se solapa con {b}");
            }
        }
    }
}
