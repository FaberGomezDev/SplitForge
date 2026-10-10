using SplitForge.Core.Diagnostics;

namespace SplitForge.Tests;

public class DiagnosticsTests
{
    [Fact]
    public void DetectsInputFamiliesCaseInsensitive()
    {
        var r = InputModuleClassifier.Classify(["UnityPlayer.dll", "XINPUT1_3.dll", "hid.dll", "steam_api64.dll",
            "GameOverlayRenderer64.dll", "kernel32.dll"]);
        Assert.Equal(["xinput1_3.dll"], r.Families["xinput"]);
        Assert.Equal(["hid.dll"], r.Families["hid"]);
        Assert.True(r.Families.ContainsKey("steam_overlay_input"));
        Assert.False(r.Families.ContainsKey("directinput"));
        Assert.False(r.RequiresFocus);
        Assert.Equal(6, r.Total);
    }

    [Fact]
    public void FocusRequirementAndAntiCheatFlags()
    {
        var r = InputModuleClassifier.Classify(["Windows.Gaming.Input.dll", "EasyAntiCheat_x64.dll"]);
        Assert.True(r.RequiresFocus);
        Assert.True(r.AntiCheatLoaded);
    }

    [Fact]
    public void PathsAreReducedToNamesAndUnknownLibrariesCaptured()
    {
        var r = InputModuleClassifier.Classify([@"C:\Users\Fulanito\InControlNative.dll", "Rewired_Windows.dll", "d3d11.dll"]);
        Assert.Equal(["incontrolnative.dll", "rewired_windows.dll"], r.Relevant);
        var json = SevenDaysDiagnosticReport.Create(
            [new GameProcessDiagnostic("7DaysToDie.exe", 1, true, r, null)], [], ["PS4 Controller"]).ToJson();
        Assert.DoesNotContain("Fulanito", json);
        Assert.Contains("\"clientes_7dtd\": 1", json);
    }
}
