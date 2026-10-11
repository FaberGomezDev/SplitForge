using SplitForge.Core.Adapters;
using SplitForge.Core.Settings;

namespace SplitForge.Tests;

public class NoMansSkyTests
{
    private static string FakeInstall(TempDir tmp, string name)
    {
        var root = tmp.Combine(name);
        Directory.CreateDirectory(Path.Combine(root, NoMansSkyAdapter.BinariesFolder));
        File.WriteAllText(NoMansSkyAdapter.Exe(root), "no es un ejecutable de verdad");
        return root;
    }

    [Fact]
    public void AnInstallIsTheFolderThatHoldsBinariesAndTheExe()
    {
        using var tmp = new TempDir();
        var install = FakeInstall(tmp, "copia");
        Assert.True(NoMansSkyAdapter.IsInstall(install));
        Assert.False(NoMansSkyAdapter.IsInstall(NoMansSkyAdapter.Binaries(install))); // la carpeta del exe no es la raíz
        Assert.False(NoMansSkyAdapter.IsInstall(tmp.Combine("no-existe")));
        Assert.False(NoMansSkyAdapter.IsInstall(null));
        Assert.False(NoMansSkyAdapter.IsInstall(""));
    }

    [Fact]
    public void TheUserPicksTheExeAndGetsTheInstallRoot()
    {
        using var tmp = new TempDir();
        var install = FakeInstall(tmp, "copia de J2");
        Assert.Equal(Path.GetFullPath(install), NoMansSkyAdapter.InstallFromPath(NoMansSkyAdapter.Exe(install)));
        Assert.Equal(Path.GetFullPath(install), NoMansSkyAdapter.InstallFromPath(install));
        // Otro ejecutable, o un NMS.exe suelto fuera de «Binaries» de una instalación, no valen.
        var other = Path.Combine(NoMansSkyAdapter.Binaries(install), "otro.exe");
        File.WriteAllText(other, "x");
        Assert.Null(NoMansSkyAdapter.InstallFromPath(other));
        var loose = tmp.Combine(NoMansSkyAdapter.GameExe);
        File.WriteAllText(loose, "x");
        Assert.Null(NoMansSkyAdapter.InstallFromPath(loose));
        Assert.Null(NoMansSkyAdapter.InstallFromPath(null));
    }

    [Fact]
    public void EachOpenProcessBelongsToThePlayerWhoseCopyItRunsFrom()
    {
        var steam = @"C:\Juegos\Steam\steamapps\common\No Man's Sky";
        var second = @"D:\GOG Games\No Man's Sky";
        Assert.Equal(1, NoMansSkyAdapter.PlayerOf(steam + @"\Binaries\NMS.exe", steam, second));
        Assert.Equal(2, NoMansSkyAdapter.PlayerOf(@"d:\gog games\no man's sky\binaries\nms.exe", steam, second));
        Assert.Null(NoMansSkyAdapter.PlayerOf(@"E:\Otro\Binaries\NMS.exe", steam, second));
        // Una carpeta cuyo nombre solo empieza igual no es la misma instalación.
        Assert.Null(NoMansSkyAdapter.PlayerOf(steam + @" copia\Binaries\NMS.exe", steam, second));
        Assert.Null(NoMansSkyAdapter.PlayerOf(steam + @"\Binaries\NMS.exe", null, null));
        Assert.Null(NoMansSkyAdapter.PlayerOf(null, steam, second));
    }

    [Fact]
    public void WithOnlyTheSteamCopyTheSecondCopyIsWhatIsMissing()
    {
        // Estado real del PC del propietario el 2026-10-11: una copia (Steam), sin filtro, dos mandos.
        var steps = NoMansSkyAdapter.Readiness(steamCopy: true, secondCopy: false, filterInSteamCopy: false, filterInSecondCopy: false, [1, 2], [1, 2]);
        Assert.Equal(["Juego de J1 (Steam)", "Juego de J2 (otra copia)", "Un mando por ventana", "Mandos"], steps.Select(s => s.Title));
        Assert.Equal([true, false, false, true], steps.Select(s => s.Ok));
        Assert.Contains("su propia copia con su cuenta", steps[1].Detail);
        Assert.Equal("Falta instalar el filtro de mandos en la copia de Steam.", steps[2].Detail);
        Assert.False(PlayReadiness.Ready(steps));
    }

    [Fact]
    public void ItIsReadyOnlyWithTwoCopiesTheFilterInBothAndTwoDifferentPads()
    {
        Assert.True(PlayReadiness.Ready(NoMansSkyAdapter.Readiness(true, true, true, true, [1, 2], [1, 2])));

        var oneFilter = NoMansSkyAdapter.Readiness(true, true, true, false, [1, 2], [1, 2]);
        Assert.Equal("Falta instalar el filtro de mandos en la copia de J2.", oneFilter[2].Detail);
        Assert.False(PlayReadiness.Ready(oneFilter));

        var samePad = NoMansSkyAdapter.Readiness(true, true, true, true, [1, 2], [1, 1]);
        Assert.Contains("mismo mando", samePad[3].Detail);
        Assert.False(samePad[3].Ok);

        var noGame = NoMansSkyAdapter.Readiness(false, false, false, false, [], [1, 2]);
        Assert.Equal([false, false, false, false], noGame.Select(s => s.Ok));
        Assert.Equal("Se instala cuando el juego esté en el equipo.", noGame[2].Detail);
    }

    [Fact]
    public void TheLibraryListsBothGamesAndFallsBackToTheFirst()
    {
        Assert.Equal([LauncherGames.SevenDaysToDie, LauncherGames.NoMansSky], LauncherGames.All.Select(g => g.Id));
        Assert.All(LauncherGames.All, g => Assert.Equal(3, g.HowTo.Count));
        Assert.Equal(NoMansSkyAdapter.AppId, LauncherGames.Find(LauncherGames.NoMansSky).AppId);
        Assert.Equal(LauncherGames.SevenDaysToDie, LauncherGames.Find("desconocido").Id);
        Assert.Equal(LauncherGames.SevenDaysToDie, LauncherGames.Find(null).Id);
        // No Man's Sky no usa identidad emulada y se presenta como «en preparación», no como compatible.
        Assert.False(LauncherGames.Find(LauncherGames.NoMansSky).EmulatedIdentity);
        Assert.Contains("SIN VALIDAR", LauncherGames.Find(LauncherGames.NoMansSky).Banner);
    }

    [Fact]
    public void SettingsRememberTheChosenGameAndTheSecondCopy()
    {
        using var tmp = new TempDir();
        Assert.Equal(LauncherGames.SevenDaysToDie, new AppSettings().SelectedGame);
        SettingsStore.Save(tmp.Path, new AppSettings { SelectedGame = LauncherGames.NoMansSky, NoMansSky = new NoMansSkyOptions { SecondCopyFolder = @"D:\Juegos\NMS" } });
        var loaded = SettingsStore.Load(tmp.Path);
        Assert.Equal(LauncherGames.NoMansSky, loaded.SelectedGame);
        Assert.Equal(@"D:\Juegos\NMS", loaded.NoMansSky.SecondCopyFolder);

        // Un identificador que ya no existe (ajustes editados) vuelve al primer juego.
        SettingsStore.Save(tmp.Path, new AppSettings { SelectedGame = "juego-borrado" });
        Assert.Equal(LauncherGames.SevenDaysToDie, SettingsStore.Load(tmp.Path).SelectedGame);
    }
}
