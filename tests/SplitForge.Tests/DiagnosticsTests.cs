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

    [Fact]
    public void ClassifiesTheRealSevenDaysReportOfTheOwner()
    {
        // Módulos del informe real (2026-10-10, 7DTD sin EAC): se reproducen familias y relevantes.
        string[] modules = ["7DaysToDie.exe", "UnityPlayer.dll", "backtracecrashpadwindows.dll", "dinput8.dll", "eossdk-win64-shipping.dll",
            "hid.dll", "InControlNative.dll", "inputhost.dll", "steam_api64.dll", "steamclient64.dll", "textinputframework.dll",
            "xinput1_3.dll", "xinput1_4.dll", "GameOverlayRenderer64.dll"];
        var r = InputModuleClassifier.Classify(modules);
        Assert.Equal(["xinput1_3.dll", "xinput1_4.dll"], r.Families["xinput"]);
        Assert.Equal(["dinput8.dll"], r.Families["directinput"]);
        Assert.Equal(["incontrolnative.dll"], r.Families["incontrol"]);
        Assert.False(r.AntiCheatLoaded); // eossdk no es el antitrampas.
        Assert.False(r.RequiresFocus);
        Assert.True(r.LoadsXInput);
        Assert.Equal("unity", r.Engine);
        Assert.Null(r.Origins);
    }

    [Theory]
    [InlineData(@"C:\Windows\System32\xinput1_4.dll", ModuleOrigin.Sistema)]
    [InlineData(@"c:/windows/SysWOW64/XINPUT1_3.dll", ModuleOrigin.Sistema)]
    [InlineData(@"D:\Juegos\steamapps\common\7 Days To Die\xinput1_4.dll", ModuleOrigin.Juego)]
    [InlineData(@"D:\Juegos\steamapps\common\7 Days To Die\7DaysToDie_Data\Plugins\x86_64\InControlNative.dll", ModuleOrigin.Juego)]
    [InlineData(@"C:\Program Files (x86)\Steam\gameoverlayrenderer64.dll", ModuleOrigin.Steam)]
    [InlineData(@"D:\Juegos\steamapps\common\7 Days To Die Server\xinput1_4.dll", ModuleOrigin.Otro)]
    [InlineData(@"C:\WindowsApps\x.dll", ModuleOrigin.Otro)]
    public void ModuleOriginIsACategoryWithoutPath(string path, ModuleOrigin expected)
    {
        var origin = ModuleOriginClassifier.Classify(path, @"C:\Windows", @"D:\Juegos\steamapps\common\7 Days To Die\",
            [@"C:\Program Files (x86)\Steam"]);
        Assert.Equal(expected, origin);
    }

    [Fact]
    public void ReportShowsOriginsAndFilterWithoutPaths()
    {
        var r = InputModuleClassifier.Classify([
            new ModuleEntry(@"C:\Users\Fulanito\Juego\xinput1_4.dll", ModuleOrigin.Juego),
            new ModuleEntry("xinput1_4.dll", ModuleOrigin.Sistema),
            new ModuleEntry("xinput1_3.dll", ModuleOrigin.Sistema),
            new ModuleEntry("steam_api64.dll", ModuleOrigin.Juego),
            new ModuleEntry("kernel32.dll", ModuleOrigin.Sistema)]);
        Assert.Equal(["juego", "sistema"], r.Origins!["xinput1_4.dll"]);
        Assert.Equal(["sistema"], r.Origins["xinput1_3.dll"]);
        Assert.False(r.Origins.ContainsKey("steam_api64.dll")); // Solo módulos de mando.
        Assert.False(r.Origins.ContainsKey("kernel32.dll"));
        Assert.Equal(4, r.Total);

        var client = new GameProcessDiagnostic("7DaysToDie.exe", 1, true, r, null) { FilterLoaded = ["xinput1_4.dll"] };
        var json = SevenDaysDiagnosticReport.Create([client], [], ["Xbox 360 Controller (xbox360)"]) with { XInputSlots = [1, 2] };
        var text = json.ToJson();
        Assert.Contains("SplitForge 7DTD Diagnostico v3", text);
        Assert.Contains("\"origenes\"", text);
        Assert.Contains("\"filtro_xinput_cargado\"", text);
        Assert.Contains("\"ranuras_xinput\"", text);
        Assert.DoesNotContain("Fulanito", text);
        Assert.DoesNotContain(":\\", text);
    }
}
