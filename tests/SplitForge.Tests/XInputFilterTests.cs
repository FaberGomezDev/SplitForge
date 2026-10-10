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
    public void InstallsMissingNamesAndNeverOverwritesGameFiles()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        File.WriteAllBytes(Path.Combine(game, "xinput9_1_0.dll"), [9, 9, 9]); // DLL propia del juego.
        var filter = FakeFilter(tmp);

        var result = XInputFilter.Install(game, filter, ["xinput1_3.dll", "xinput9_1_0.dll"]);
        Assert.Equal(["xinput1_3.dll"], result.Installed);
        Assert.Equal(["xinput9_1_0.dll"], result.Skipped);
        Assert.Equal([9, 9, 9], File.ReadAllBytes(Path.Combine(game, "xinput9_1_0.dll")));
        Assert.Equal(["xinput1_3.dll"], XInputFilter.InstalledNames(game));

        var again = XInputFilter.Install(game, filter, ["xinput1_3.dll", "xinput1_4.dll"]);
        Assert.Equal(["xinput1_4.dll"], again.Installed);
        Assert.Equal(["xinput1_3.dll"], again.AlreadyCurrent); // Idempotente.
        Assert.Empty(again.Updated);

        XInputFilter.Uninstall(game);
        Assert.Equal(["xinput9_1_0.dll"], Directory.GetFiles(game).Select(Path.GetFileName));
    }

    [Fact]
    public void UpdatesOnlyUnmodifiedOwnCopiesOfAnOlderFilter()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        var filter = FakeFilter(tmp);
        XInputFilter.Install(game, filter, ["xinput1_3.dll", "xinput1_4.dll"]);
        File.WriteAllBytes(Path.Combine(game, "xinput1_4.dll"), [7]); // Cambiado por otra herramienta.
        File.WriteAllBytes(filter, [0x4D, 0x5A, 4, 5, 6]); // Nueva versión del filtro.

        var result = XInputFilter.Install(game, filter, ["xinput1_3.dll", "xinput1_4.dll"]);
        Assert.Equal(["xinput1_3.dll"], result.Updated);
        Assert.Equal(["xinput1_4.dll"], result.Skipped);
        Assert.Equal(File.ReadAllBytes(filter), File.ReadAllBytes(Path.Combine(game, "xinput1_3.dll")));
        Assert.Equal([7], File.ReadAllBytes(Path.Combine(game, "xinput1_4.dll")));
        Assert.True(XInputFilter.IsOwnFile(game, "xinput1_3.dll"));
        Assert.False(XInputFilter.IsOwnFile(game, "xinput1_4.dll"));
        Assert.False(XInputFilter.IsOwnFile(game, "xinput9_1_0.dll"));

        // La copia actualizada se puede retirar; la modificada por otra herramienta, no.
        Assert.Equal(["xinput1_3.dll"], XInputFilter.Uninstall(game));
        Assert.True(File.Exists(Path.Combine(game, "xinput1_4.dll")));
    }

    [Fact]
    public void ManifestKeepsWhatWasCopiedEvenIfALaterCopyFails()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        var filter = FakeFilter(tmp);
        // Un directorio con el nombre de la DLL hace fallar la copia del segundo nombre.
        Directory.CreateDirectory(Path.Combine(game, "xinput1_4.dll"));

        Assert.ThrowsAny<Exception>(() => XInputFilter.Install(game, filter, ["xinput1_3.dll", "xinput1_4.dll"]));
        Assert.Equal(["xinput1_3.dll"], XInputFilter.InstalledNames(game));
        Assert.Equal(["xinput1_3.dll"], XInputFilter.Uninstall(game));
        Assert.False(File.Exists(Path.Combine(game, "xinput1_3.dll")));
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
    public void RejectsUnknownDllNamesBeforeCopyingAnything()
    {
        using var tmp = new TempDir();
        var game = tmp.Combine("game");
        Directory.CreateDirectory(game);
        Assert.Throws<ArgumentException>(() => XInputFilter.Install(game, FakeFilter(tmp), ["xinput1_3.dll", "kernel32.dll"]));
        Assert.Empty(Directory.GetFileSystemEntries(game));
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
        Assert.Equal(-1, all[0].Reads); // Versión anterior del filtro: no informa lecturas.
        var live = XInputFilter.ReadStatus(tmp.Path, new HashSet<int> { 200 });
        Assert.Equal(200, Assert.Single(live).Pid);
    }

    [Fact]
    public void ReadsOneStatusPerLoadedDllAndMergesFolders()
    {
        using var host = new TempDir();
        using var box = new TempDir();
        // 7DTD carga xinput1_3 y xinput1_4: cada copia del filtro escribe su propio archivo.
        File.WriteAllText(host.Combine("estado-300-xinput1_3.txt"), "dll=xinput1_3.dll\nreal=cargada\nranura=0\nlecturas=0\n");
        File.WriteAllText(host.Combine("estado-300-xinput1_4.txt"), "dll=xinput1_4.dll\nreal=cargada\nranura=0\nlecturas=5321\n");
        File.WriteAllText(box.Combine("estado-400-xinput1_4.txt"), "dll=xinput1_4.dll\nreal=cargada\nranura=1\nlecturas=77\n");
        File.WriteAllText(host.Combine("estado-abc.txt"), "dll=x\n");
        File.WriteAllText(host.Combine("estado--5-x.txt"), "dll=x\n");

        var status = XInputFilter.ReadStatus([host.Path, box.Path, host.Combine("no-existe")]);
        Assert.Equal([(300, "xinput1_3.dll", 0L), (300, "xinput1_4.dll", 5321L), (400, "xinput1_4.dll", 77L)],
            status.Select(s => (s.Pid, s.Dll, s.Reads)));
        Assert.Equal(1, status[2].Slot);
    }

    [Fact]
    public void SandboxieStatusFolderUsesTheDefaultBoxLayoutAndRejectsUnsafeNames()
    {
        Assert.Equal(@"C:\Sandbox\jugador\SplitForge_J2\user\current\AppData\Local\SplitForge\xinput",
            XInputFilter.SandboxieStatusFolder("C:", "jugador", "SplitForge_J2"));
        Assert.Equal(@"D:\Sandbox\a\b\user\current\AppData\Local\SplitForge\xinput",
            XInputFilter.SandboxieStatusFolder(@"D:\", "a", "b"));
        Assert.Null(XInputFilter.SandboxieStatusFolder("C:", "..", "SplitForge_J2"));
        Assert.Null(XInputFilter.SandboxieStatusFolder("C:", "jugador", @"x\y"));
        Assert.Null(XInputFilter.SandboxieStatusFolder("C:\\Windows", "jugador", "b"));
        Assert.Null(XInputFilter.SandboxieStatusFolder("C:", "", "b"));
    }
}
