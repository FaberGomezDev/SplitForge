using SplitForge.Core.Settings;
using SplitForge.Core.Windows;

namespace SplitForge.Tests;

public class FakeFocusTests
{
    [Fact]
    public void MessagesMatchTheProtoInputFocusLoop()
    {
        var hwnd = (nint)0x1234;
        var messages = FakeFocusPlan.MessagesFor(hwnd);
        Assert.Equal(
            [(0x0006u, (nint)2, (nint)0), (0x001Cu, (nint)1, (nint)0), (0x0086u, (nint)1, (nint)0), (0x0007u, (nint)0, (nint)0), (0x0021u, hwnd, (nint)1)],
            messages.Select(m => (m.Msg, m.WParam, m.LParam)).ToArray());
    }

    [Fact]
    public void TargetsNeedTwoDistinctWindowsAndTheOptionEnabled()
    {
        Assert.Empty(FakeFocusPlan.Targets(true, null));
        Assert.Empty(FakeFocusPlan.Targets(true, [(nint)5]));
        Assert.Empty(FakeFocusPlan.Targets(true, [(nint)5, (nint)5, (nint)0]));
        Assert.Empty(FakeFocusPlan.Targets(false, [(nint)5, (nint)6]));
        Assert.Equal([(nint)5, (nint)6], FakeFocusPlan.Targets(true, [(nint)5, (nint)0, (nint)6, (nint)5]));
    }

    [Theory]
    [InlineData(0, FakeFocusPlan.MinIntervalMs)]
    [InlineData(100, 100)]
    [InlineData(999_999, FakeFocusPlan.MaxIntervalMs)]
    public void IntervalIsClamped(int value, int expected) => Assert.Equal(expected, FakeFocusPlan.ClampInterval(value));

    [Fact]
    public void SettingsEnableFakeFocusByDefaultAndClampTheInterval()
    {
        using var tmp = new TempDir();
        Assert.True(new AppSettings().FakeFocus);
        SettingsStore.Save(tmp.Path, new AppSettings { FakeFocus = false, FakeFocusIntervalMs = 1 });
        var loaded = SettingsStore.Load(tmp.Path);
        Assert.False(loaded.FakeFocus);
        Assert.Equal(FakeFocusPlan.MinIntervalMs, loaded.FakeFocusIntervalMs);
    }
}
