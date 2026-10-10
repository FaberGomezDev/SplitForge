using System.IO.Pipes;
using SplitForge.Core.Layout;
using SplitForge.Core.Sessions;
using SplitForge.Core.Windows;

namespace SplitForge.Tests;

internal sealed class FakeWindowSystem : IWindowSystem
{
    public List<WindowInfo> Windows { get; } = [];
    public List<(nint Handle, Region Region, bool Borderless)> Placed { get; } = [];
    public List<WindowSnapshot> Restored { get; } = [];
    public List<nint> CloseRequests { get; } = [];
    public Func<nint, Region, bool>? FailPlace { get; set; }
    public Region WorkArea { get; set; } = new(0, 0, 1920, 1080);

    public IReadOnlyList<WindowInfo> EnumerateWindows() => [.. Windows];

    public Dictionary<nint, int> Owners { get; } = [];

    public WindowSnapshot Capture(nint handle) =>
        new(handle, new Region(10, 20, 640, 480), false, false, 0x10CF0000, 0, Owners.GetValueOrDefault(handle, 77));

    public int? ProcessId(nint handle) => Owners.TryGetValue(handle, out var pid) ? pid : null;

    public void Place(nint handle, Region region, bool borderless)
    {
        if (FailPlace?.Invoke(handle, region) == true)
            throw new InvalidOperationException("Ventana no redimensionable");
        Placed.Add((handle, region, borderless));
    }

    public void Restore(WindowSnapshot snapshot) => Restored.Add(snapshot);

    public void RequestClose(nint handle) => CloseRequests.Add(handle);

    public Region PrimaryWorkArea() => WorkArea;
}

internal sealed class FakeProcess(int id, bool withInput) : IChildProcess
{
    private readonly AnonymousPipeServerStream? _server = withInput ? new(PipeDirection.Out) : null;

    public int Id { get; } = id;
    public bool HasExited { get; set; }
    public int? ExitCode { get; set; }
    public bool Killed { get; private set; }
    public Stream? Input => _server;

    public void Kill()
    {
        Killed = true;
        HasExited = true;
    }

    public void Dispose() => _server?.Dispose();
}

internal sealed class FakeLauncher : IProcessLauncher
{
    private int _nextId = 500;

    public List<(LaunchSpec Spec, FakeProcess Process)> Started { get; } = [];
    public Exception? FailOn { get; set; }
    public int FailAfter { get; set; } = int.MaxValue;

    public IChildProcess Start(LaunchSpec spec)
    {
        if (FailOn is not null && Started.Count >= FailAfter)
            throw FailOn;
        var process = new FakeProcess(_nextId++, spec.RedirectInput);
        Started.Add((spec, process));
        return process;
    }
}
