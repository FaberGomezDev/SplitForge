using SplitForge.Core.Input;
using SplitForge.Core.Layout;
using SplitForge.Core.Sessions;

namespace SplitForge.Tests;

public class SessionTests
{
    private readonly List<string> _logs = [];
    private readonly FakeWindowSystem _windows = new();
    private readonly FakeLauncher _launcher = new();
    private long _now = 1_000;

    private SessionManager Manager() => new(_windows, _launcher, _logs.Add, () => _now);

    private static readonly LayoutOptions SideBySide = new(SplitOrientation.SideBySide, 4, Borderless: false);

    [Fact]
    public void AttachAndRestore()
    {
        var m = Manager();
        m.Attach([11, 22], SideBySide);
        Assert.Equal(2, _windows.Placed.Count);
        Assert.Equal([(nint)11, 22], m.Active!.WindowHandles);
        Assert.Equal(new Region(0, 0, 958, 1080), _windows.Placed[0].Region);
        m.Stop();
        Assert.Null(m.Active);
        Assert.Equal(2, _windows.Restored.Count);
        Assert.Empty(_windows.CloseRequests); // Nunca cierra juegos externos.
    }

    [Fact]
    public void BorderlessIsPassedAndFourWindowsSupported()
    {
        var m = Manager();
        m.Attach([1, 2, 3, 4], SideBySide with { Borderless = true });
        Assert.Equal(4, _windows.Placed.Count);
        Assert.All(_windows.Placed, p => Assert.True(p.Borderless));
    }

    [Fact]
    public void RejectsDuplicateOrTooManyWindows()
    {
        var m = Manager();
        Assert.Throws<ArgumentException>(() => m.Attach([11, 11], SideBySide));
        Assert.Throws<ArgumentException>(() => m.Attach([1, 2, 3, 4, 5], SideBySide));
        Assert.Null(m.Active);
    }

    [Fact]
    public void RollbackOnMoveError()
    {
        _windows.FailPlace = (h, _) => h == 22;
        var m = Manager();
        var ex = Assert.Throws<InvalidOperationException>(() => m.Attach([11, 22], new LayoutOptions(SplitOrientation.Stacked, 4, false)));
        Assert.Contains("no redimensionable", ex.Message);
        Assert.Null(m.Active);
        Assert.Equal(2, _windows.Restored.Count); // Se capturaron ambas antes de mover.
    }

    [Fact]
    public void RelayoutKeepsOriginalSnapshots()
    {
        var m = Manager();
        m.Attach([11, 22], SideBySide);
        m.Relayout(new LayoutOptions(SplitOrientation.Stacked, 0, true));
        Assert.Equal(4, _windows.Placed.Count);
        Assert.Equal(new Region(0, 0, 1920, 540), _windows.Placed[2].Region);
        m.Stop();
        Assert.Equal(2, _windows.Restored.Count);
    }

    [Fact]
    public void LabStartsTwoViewersPlacesWindowsAndRoutesInputs()
    {
        var m = Manager();
        m.StartLab(p => new LaunchSpec("viewer", [$"--player={p}"]), 7, 8, SideBySide);
        Assert.Equal(2, _launcher.Started.Count);
        Assert.All(_launcher.Started, s => Assert.True(s.Spec.RedirectInput));

        // Una ventana con título equivocado no cuenta.
        var p1 = _launcher.Started[0].Process.Id;
        var p2 = _launcher.Started[1].Process.Id;
        _windows.Windows.Add(new(900, p1, SessionManager.LabWindowTitle(1)));
        _windows.Windows.Add(new(901, p2, "Otra ventana"));
        m.Tick();
        Assert.Empty(_windows.Placed);
        _windows.Windows.Add(new(902, p2, SessionManager.LabWindowTitle(2)));
        m.Tick();
        Assert.Equal([(nint)900, 902], _windows.Placed.Select(x => x.Handle));

        m.RouteLab(new Dictionary<uint, PadState> { [7] = new(7, 1), [8] = new(8, -1) }, 0.05);
        Assert.Contains("ID 7", m.Active!.Lab!.StatusLines[0]);
    }

    [Fact]
    public void LabStopRequestsCloseThenKillsOnlyOwnViewersAfterGrace()
    {
        var m = Manager();
        m.StartLab(_ => new LaunchSpec("viewer", []), 7, 8, SideBySide);
        var viewers = _launcher.Started.Select(s => s.Process).ToList();
        _windows.Windows.Add(new(900, viewers[0].Id, SessionManager.LabWindowTitle(1)));
        _windows.Windows.Add(new(950, 4242, "Juego externo"));
        m.Stop();
        Assert.Equal([(nint)900], _windows.CloseRequests);
        m.Tick();
        Assert.All(viewers, v => Assert.False(v.Killed));
        _now += SessionManager.CloseGraceMs + 1;
        m.Tick();
        Assert.All(viewers, v => Assert.True(v.Killed));
    }

    [Fact]
    public void LabRequiresDistinctPadsAndIdleManager()
    {
        var m = Manager();
        Assert.Throws<ArgumentException>(() => m.StartLab(_ => new LaunchSpec("v", []), 3, 3, SideBySide));
        m.Attach([1, 2], SideBySide);
        Assert.Throws<InvalidOperationException>(() => m.StartLab(_ => new LaunchSpec("v", []), 3, 4, SideBySide));
    }

    [Fact]
    public void FailedSecondViewerLaunchCleansUpFirst()
    {
        _launcher.FailOn = new InvalidOperationException("sin memoria");
        _launcher.FailAfter = 1;
        var m = Manager();
        Assert.Throws<InvalidOperationException>(() => m.StartLab(_ => new LaunchSpec("v", []), 1, 2, SideBySide));
        Assert.Null(m.Active);
        Assert.True(_launcher.Started[0].Process.Killed);
    }

    [Fact]
    public void LabWindowTimeoutStopsWaiting()
    {
        var m = Manager();
        m.StartLab(_ => new LaunchSpec("v", []), 1, 2, SideBySide);
        _now += SessionManager.LabWindowTimeoutMs + 1;
        m.Tick();
        Assert.False(m.Active!.Pending);
        Assert.Contains(_logs, l => l.Contains("a tiempo"));
    }

    [Fact]
    public void RecoveryFileExistsOnlyWhileExternalWindowsAreModified()
    {
        using var tmp = new TempDir();
        var file = tmp.Combine(SessionRecovery.FileName);
        var m = new SessionManager(_windows, _launcher, _logs.Add, () => _now, file);
        _windows.Owners[11] = 1001;
        _windows.Owners[22] = 1002;
        m.Attach([11, 22], SideBySide with { Borderless = true });
        var saved = SessionRecovery.Load(file);
        Assert.Equal(2, saved.Count);
        Assert.Equal(1001, saved.Single(x => x.Handle == 11).Pid);
        m.Stop();
        Assert.False(File.Exists(file));
    }

    [Fact]
    public void RecoverAfterCrashRestoresOnlyWindowsOfTheSameProcess()
    {
        using var tmp = new TempDir();
        var file = tmp.Combine(SessionRecovery.FileName);
        var m = new SessionManager(_windows, _launcher, _logs.Add, () => _now, file);
        _windows.Owners[11] = 1001;
        _windows.Owners[22] = 1002;
        _windows.Owners[33] = 1003;
        m.Attach([11, 22, 33], SideBySide with { Borderless = true });
        // "Cierre inesperado": no se llama a Stop. Luego 22 se cerró y su handle es de otro proceso; 33 ya no existe.
        _windows.Owners[22] = 9999;
        _windows.Owners.Remove(33);
        var fresh = new FakeWindowSystem();
        foreach (var (h, pid) in _windows.Owners)
            fresh.Owners[h] = pid;
        Assert.Equal(1, SessionRecovery.RecoverAfterCrash(file, fresh, _logs.Add));
        Assert.Equal([(nint)11], fresh.Restored.Select(r => r.Handle));
        Assert.False(File.Exists(file));
        Assert.Equal(0, SessionRecovery.RecoverAfterCrash(file, fresh, _logs.Add)); // Sin archivo: nada que hacer.
    }

    [Fact]
    public void LabSessionsDoNotWriteRecoveryFile()
    {
        using var tmp = new TempDir();
        var file = tmp.Combine(SessionRecovery.FileName);
        var m = new SessionManager(_windows, _launcher, _logs.Add, () => _now, file);
        m.StartLab(_ => new LaunchSpec("v", []), 1, 2, SideBySide);
        var p = _launcher.Started.Select(x => x.Process.Id).ToList();
        _windows.Windows.Add(new(900, p[0], SessionManager.LabWindowTitle(1)));
        _windows.Windows.Add(new(901, p[1], SessionManager.LabWindowTitle(2)));
        m.Tick();
        Assert.Equal(2, _windows.Placed.Count);
        Assert.False(File.Exists(file));
    }

    [Fact]
    public void GameMinAspectIsAppliedAndKeptOnRelayout()
    {
        var m = Manager();
        var seven = new LayoutOptions(SplitOrientation.SideBySide, 0, true, MinAspect: AspectRatio.SixteenNine);
        m.Attach([11, 22], seven);
        Assert.Equal(new Region(0, 270, 960, 540), _windows.Placed[0].Region);
        // «Reaplicar distribución» llega sin mínimo: se conserva el del juego.
        m.Relayout(new LayoutOptions(SplitOrientation.SideBySide, 0, true));
        Assert.Equal(new Region(960, 270, 960, 540), _windows.Placed[3].Region);
        Assert.Equal(AspectRatio.SixteenNine, m.Active!.Layout.MinAspect);
    }

    [Fact]
    public void CoverTaskbarUsesTheWholeMonitorAndPlacesWindowsOnTop()
    {
        _windows.WorkArea = new Region(0, 0, 1920, 1032);
        _windows.Bounds = new Region(0, 0, 1920, 1080);
        var m = Manager();
        m.Attach([11, 22], new LayoutOptions(SplitOrientation.Stacked, 0, true, CoverTaskbar: true));
        Assert.Equal(new Region(0, 0, 1920, 540), _windows.Placed[0].Region);
        Assert.Equal(new Region(0, 540, 1920, 540), _windows.Placed[1].Region);
        Assert.All(_windows.Placed, p => Assert.True(p.Topmost));
        m.Stop();
        Assert.Equal(2, _windows.Restored.Count);

        var normal = Manager();
        normal.Attach([33, 44], new LayoutOptions(SplitOrientation.Stacked, 0, true));
        Assert.Equal(new Region(0, 516, 1920, 516), _windows.Placed[3].Region);
        Assert.False(_windows.Placed[3].Topmost);
    }
}
