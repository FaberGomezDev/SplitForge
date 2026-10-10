using System.Text.Json;
using SplitForge.Core.Adapters;

namespace SplitForge.Tests;

public class SteamEmulatorTests
{
    private static string GameFolder(TempDir tmp, bool unityLayout = false)
    {
        var game = tmp.Combine("game");
        var dllFolder = game;
        if (unityLayout)
        {
            dllFolder = Path.Combine(game, "7DaysToDie_Data", "Plugins", "x86_64");
            Directory.CreateDirectory(dllFolder);
        }
        else
        {
            Directory.CreateDirectory(game);
        }
        File.WriteAllBytes(Path.Combine(dllFolder, SteamEmulator.EmuDllName), [0x4D, 0x5A, 10, 20, 30]);
        return game;
    }

    private static string EmuDll(TempDir tmp)
    {
        var path = tmp.Combine("emu", SteamEmulator.EmuDllName);
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        File.WriteAllBytes(path, [0x4D, 0x5A, 9, 9, 9]);
        return path;
    }

    private static string Config() =>
        SteamEmulator.BuildConfig(251570, "76561198000000001", "SplitForgeJ1", @"C:\temp\j1");

    [Fact]
    public void InstallBacksUpOriginalAndWritesConfigAndState()
    {
        using var tmp = new TempDir();
        var game = GameFolder(tmp);
        var original = File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName));

        SteamEmulator.Install(game, EmuDll(tmp), Config());

        Assert.True(SteamEmulator.IsInstalled(game));
        Assert.Equal(original, File.ReadAllBytes(Path.Combine(game, SteamEmulator.BackupName)));
        Assert.Equal([0x4D, 0x5A, 9, 9, 9], File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName)));
        Assert.True(File.Exists(Path.Combine(game, SteamEmulator.ConfigName)));
        Assert.True(File.Exists(Path.Combine(game, SteamEmulator.StateName)));
        var status = SteamEmulator.ReadStatus(game);
        Assert.True(status.Installed);
        Assert.Equal("SplitForgeJ1", status.Persona);
        Assert.Equal("76561198000000001", status.SteamId);
    }

    [Fact]
    public void InstallWorksWithUnityPluginLayout()
    {
        using var tmp = new TempDir();
        var game = GameFolder(tmp, unityLayout: true);
        var plugins = Path.Combine(game, "7DaysToDie_Data", "Plugins", "x86_64");
        var original = File.ReadAllBytes(Path.Combine(plugins, SteamEmulator.EmuDllName));

        SteamEmulator.Install(game, EmuDll(tmp), Config());

        Assert.True(SteamEmulator.IsInstalled(game));
        Assert.Equal(original, File.ReadAllBytes(Path.Combine(plugins, SteamEmulator.BackupName)));
        Assert.Equal([0x4D, 0x5A, 9, 9, 9], File.ReadAllBytes(Path.Combine(plugins, SteamEmulator.EmuDllName)));
        Assert.True(File.Exists(Path.Combine(plugins, SteamEmulator.ConfigName)));
        Assert.Equal("SplitForgeJ1", SteamEmulator.ReadStatus(game).Persona);

        SteamEmulator.Restore(game);
        Assert.Equal(original, File.ReadAllBytes(Path.Combine(plugins, SteamEmulator.EmuDllName)));
        Assert.False(SteamEmulator.IsInstalled(game));
        Assert.False(File.Exists(Path.Combine(plugins, SteamEmulator.BackupName)));
    }

    [Fact]
    public void InstallAgainUpdatesOwnCopyAndKeepsTheOriginal()
    {
        using var tmp = new TempDir();
        var game = GameFolder(tmp);
        var original = File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName));
        Assert.Equal(SteamEmuInstallResult.Installed, SteamEmulator.Install(game, EmuDll(tmp), Config()));
        Assert.Equal(SteamEmuInstallResult.UpToDate, SteamEmulator.Install(game, EmuDll(tmp), Config()));

        var newer = tmp.Combine("emu2", SteamEmulator.EmuDllName);
        Directory.CreateDirectory(Path.GetDirectoryName(newer)!);
        File.WriteAllBytes(newer, [0x4D, 0x5A, 7, 7, 7, 7]);
        Assert.Equal(SteamEmuInstallResult.Updated, SteamEmulator.Install(game, newer, Config()));

        Assert.Equal([0x4D, 0x5A, 7, 7, 7, 7], File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName)));
        Assert.Equal(original, File.ReadAllBytes(Path.Combine(game, SteamEmulator.BackupName)));
        SteamEmulator.Restore(game); // el estado registra la DLL nueva: la restauración se permite
        Assert.Equal(original, File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName)));
    }

    [Fact]
    public void InstallRefusesToUpdateDllChangedByOthersOrWithoutOriginal()
    {
        using var tmp = new TempDir();
        var game = GameFolder(tmp);
        SteamEmulator.Install(game, EmuDll(tmp), Config());
        File.WriteAllBytes(Path.Combine(game, SteamEmulator.EmuDllName), [1, 2, 3]);

        var ex = Assert.Throws<InvalidOperationException>(() => SteamEmulator.Install(game, EmuDll(tmp), Config()));
        Assert.Contains("no coincide", ex.Message);
        Assert.Equal([1, 2, 3], File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName)));

        using var tmp2 = new TempDir();
        var empty = tmp2.Combine("empty");
        Directory.CreateDirectory(empty);
        Assert.Throws<InvalidOperationException>(() => SteamEmulator.Install(empty, EmuDll(tmp2), Config()));
    }

    [Fact]
    public void RestoreReturnsOriginalAndLeavesNoTrace()
    {
        using var tmp = new TempDir();
        var game = GameFolder(tmp);
        var original = File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName));
        SteamEmulator.Install(game, EmuDll(tmp), Config());

        SteamEmulator.Restore(game);

        Assert.Equal(original, File.ReadAllBytes(Path.Combine(game, SteamEmulator.EmuDllName)));
        Assert.False(File.Exists(Path.Combine(game, SteamEmulator.BackupName)));
        Assert.False(File.Exists(Path.Combine(game, SteamEmulator.ConfigName)));
        Assert.False(File.Exists(Path.Combine(game, SteamEmulator.StateName)));
        Assert.False(SteamEmulator.IsInstalled(game));
    }

    [Fact]
    public void RestoreRefusesToOverwriteChangesMadeByOthers()
    {
        using var tmp = new TempDir();
        var game = GameFolder(tmp);
        SteamEmulator.Install(game, EmuDll(tmp), Config());
        File.WriteAllBytes(Path.Combine(game, SteamEmulator.EmuDllName), [1, 2, 3]);

        var ex = Assert.Throws<InvalidOperationException>(() => SteamEmulator.Restore(game));
        Assert.Contains("no coincide", ex.Message);
        Assert.True(File.Exists(Path.Combine(game, SteamEmulator.BackupName)));
    }

    [Fact]
    public void ConfigIsValidJsonWithExpectedFields()
    {
        using var doc = JsonDocument.Parse(Config());
        Assert.Equal(251570, doc.RootElement.GetProperty("app_id").GetInt32());
        Assert.Equal("76561198000000001", doc.RootElement.GetProperty("steam_id").GetString());
        Assert.Equal("SplitForgeJ1", doc.RootElement.GetProperty("persona_name").GetString());
        Assert.True(doc.RootElement.GetProperty("net_port").GetInt32() > 0);
    }

    [Fact]
    public void IdentityIsValidated()
    {
        Assert.Throws<ArgumentException>(() => SteamEmulator.BuildConfig(480, "abc", "J", @"C:\x"));
        Assert.Throws<ArgumentException>(() => SteamEmulator.BuildConfig(480, "0", "J", @"C:\x"));
        Assert.Throws<ArgumentException>(() => SteamEmulator.BuildConfig(480, "123", "", @"C:\x"));
        Assert.Throws<ArgumentException>(() => SteamEmulator.BuildConfig(480, "123", "J", ""));
    }

    [Fact]
    public void InstanceLaunchCarriesTheConfigEnvironmentVariable()
    {
        var spec = SteamEmulator.InstanceLaunch(@"C:\game\7DaysToDie.exe", @"C:\temp\j2.json");
        Assert.Equal(SteamEmulator.ConfigEnvVar, Assert.Single(spec.EnvironmentVariables!).Key);
        Assert.Equal(@"C:\temp\j2.json", spec.EnvironmentVariables![SteamEmulator.ConfigEnvVar]);
        Assert.Equal(@"C:\game", spec.WorkingDirectory);
        Assert.Empty(spec.Arguments);

        var withArgs = SteamEmulator.InstanceLaunch(@"C:\game\7DaysToDie.exe", @"C:\temp\j1.json", ["-crossplatform=None"]);
        Assert.Equal(["-crossplatform=None"], withArgs.Arguments);
    }
}
