using SplitForge.Core.Adapters;

namespace SplitForge.Tests;

public class XInputFilterTests
{
    private static string FakeFilter(TempDir tmp)
    {
        var path = tmp.Combine(XInputFilter.DllFileName);
        File.WriteAllBytes(path, [0x4D, 0x5A, 1, 2, 3]);
        return path;
    }

    [Fact]
    public void InstallsOnlyMissingNamesAndNeverOverwritesGameFiles()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        File.WriteAllBytes(Path.Combine(game, "xinput9_1_0.dll"), [9, 9, 9]); // DLL propia del juego.
        var filter = FakeFilter(tmp);

        var ex = Assert.Throws<InvalidOperationException>(() => XInputFilter.Install(game, filter, ["xinput1_3.dll", "xinput9_1_0.dll"]));
        Assert.Contains("no lo sobrescribe", ex.Message);
        Assert.Equal([9, 9, 9], File.ReadAllBytes(Path.Combine(game, "xinput9_1_0.dll")));

        var installed = XInputFilter.Install(game, filter, ["xinput1_4.dll"]);
        Assert.Equal(["xinput1_4.dll"], installed);
        Assert.Contains("xinput1_4.dll", XInputFilter.InstalledNames(game));
        Assert.Empty(XInputFilter.Install(game, filter, ["xinput1_4.dll"])); // Idempotente.
    }

    [Fact]
    public void UninstallRemovesOnlyUnmodifiedOwnFiles()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        var filter = FakeFilter(tmp);
        XInputFilter.Install(game, filter, ["xinput1_3.dll", "xinput1_4.dll"]);
        File.WriteAllBytes(Path.Combine(game, "xinput1_4.dll"), [7]); // Cambiado por otra herramienta.

        var removed = XInputFilter.Uninstall(game);
        Assert.Equal(["xinput1_3.dll"], removed);
        Assert.False(File.Exists(Path.Combine(game, "xinput1_3.dll")));
        Assert.True(File.Exists(Path.Combine(game, "xinput1_4.dll")));
    }

    [Fact]
    public void FullUninstallLeavesNoTrace()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        XInputFilter.Install(game, FakeFilter(tmp), XInputFilter.KnownNames);
        XInputFilter.Uninstall(game);
        Assert.Empty(Directory.GetFiles(game));
    }

    [Fact]
    public void RejectsUnknownDllNames()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        Assert.Throws<ArgumentException>(() => XInputFilter.Install(game, FakeFilter(tmp), ["kernel32.dll"]));
    }

    [Fact]
    public void AssignmentsAreValidatedAndReadableByTheNativeFilter()
    {
        using var tmp = new TempDir();
        XInputFilter.WriteAssignments(tmp.Path, [(4242, 0), (5151, 1)]);
        Assert.Equal("4242 0\n5151 1\n", File.ReadAllText(tmp.Combine(XInputFilter.AssignmentFile)));
        Assert.Throws<ArgumentException>(() => XInputFilter.WriteAssignments(tmp.Path, [(1, 0), (2, 0)]));
        Assert.Throws<ArgumentException>(() => XInputFilter.WriteAssignments(tmp.Path, [(1, 4)]));
        XInputFilter.ClearAssignments(tmp.Path);
        Assert.False(File.Exists(tmp.Combine(XInputFilter.AssignmentFile)));
    }

    [Fact]
    public void ReadsStatusOfLiveProcessesOnly()
    {
        using var tmp = new TempDir();
        File.WriteAllText(tmp.Combine("estado-100.txt"), "dll=xinput1_3.dll\nreal=cargada\nranura=1\n");
        File.WriteAllText(tmp.Combine("estado-200.txt"), "dll=xinput1_4.dll\nreal=NO\nranura=-1\n");
        var all = XInputFilter.ReadStatus(tmp.Path);
        Assert.Equal(2, all.Count);
        Assert.True(all[0].RealLoaded);
        Assert.Equal(1, all[0].Slot);
        var live = XInputFilter.ReadStatus(tmp.Path, new HashSet<int> { 200 });
        Assert.Equal(200, Assert.Single(live).Pid);
    }
}
