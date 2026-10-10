using SplitForge.Core.Input;
using SplitForge.Core.Layout;
using SplitForge.Core.Settings;

namespace SplitForge.Tests;

public class SettingsTests
{
    [Fact]
    public void ImportsLegacyPythonSettingsWithoutTouchingThem()
    {
        using var tmp = new TempDir();
        var legacy = tmp.Combine(SettingsStore.LegacyFileName);
        File.WriteAllText(legacy, """{"orientation": "horizontal", "gutter": 8, "input_mode": "joystick", "controller_mappings": {}}""");
        var before = File.ReadAllBytes(legacy);
        var s = SettingsStore.Load(tmp.Path);
        Assert.Equal(SplitOrientation.Stacked, s.Orientation);
        Assert.Equal(8, s.Gutter);
        Assert.Equal(InputReadMode.Joystick, s.InputMode);
        SettingsStore.Save(tmp.Path, s);
        Assert.Equal(before, File.ReadAllBytes(legacy));
        Assert.True(File.Exists(tmp.Combine(SettingsStore.FileName)));
    }

    [Fact]
    public void RoundTripWithMappings()
    {
        using var tmp = new TempDir();
        var s = new AppSettings
        {
            Borderless = false,
            InputRateHz = 500,
            Mappings = new() { ["0300abcd"] = new ControllerMapping { FireButton = 5, FireAxis = 4, FireRest = -1, FireDirection = 1 } },
        };
        SettingsStore.Save(tmp.Path, s);
        var loaded = SettingsStore.Load(tmp.Path);
        Assert.False(loaded.Borderless);
        Assert.Equal(500, loaded.InputRateHz);
        Assert.Equal(5, loaded.Mappings["0300abcd"].FireButton);
        Assert.Equal(4, loaded.Mappings["0300abcd"].FireAxis);
    }

    [Fact]
    public void CorruptedFileFallsBackAndIsKept()
    {
        using var tmp = new TempDir();
        var path = tmp.Combine(SettingsStore.FileName);
        File.WriteAllText(path, "{ no es json");
        var s = SettingsStore.Load(tmp.Path);
        Assert.Equal(4, s.Gutter);
        Assert.True(File.Exists(path));
    }

    [Fact]
    public void OutOfRangeValuesAreSanitized()
    {
        using var tmp = new TempDir();
        File.WriteAllText(tmp.Combine(SettingsStore.FileName), """{"Gutter": 999, "InputRateHz": 7}""");
        var s = SettingsStore.Load(tmp.Path);
        Assert.Equal(SplitLayout.MaxGutter, s.Gutter);
        Assert.Equal(1000, s.InputRateHz);
    }
}
