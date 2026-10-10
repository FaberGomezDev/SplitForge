using System.Collections;
using System.Collections.ObjectModel;
using System.Windows.Input;
using SplitForge.Core.Windows;

namespace SplitForge.App.ViewModels;

public sealed record WindowRow(WindowInfo Info)
{
    public string Label => $"{Info.Title}   [PID {Info.Pid}]";
}

public sealed partial class MainViewModel
{
    public ObservableCollection<WindowRow> VisibleWindows { get; } = [];
    public ICommand RefreshWindowsCommand { get; private set; } = null!;
    public ICommand AttachWindowsCommand { get; private set; } = null!;

    private void InitWindows()
    {
        RefreshWindowsCommand = new RelayCommand(RefreshWindows);
        AttachWindowsCommand = new RelayCommand(p => AttachSelected(p as IList), _ => _sessions.Active is null);
        RefreshWindows();
    }

    private void RefreshWindows()
    {
        VisibleWindows.Clear();
        var own = Environment.ProcessId;
        foreach (var w in _windows.EnumerateWindows().Where(w => w.Pid != own).OrderBy(w => w.Title, StringComparer.CurrentCultureIgnoreCase))
            VisibleWindows.Add(new WindowRow(w));
    }

    private void AttachSelected(IList? selected) => Guard(() =>
    {
        var handles = selected?.OfType<WindowRow>().Select(r => r.Info.Handle).ToList() ?? [];
        if (handles.Count is < 2 or > 4)
            throw new InvalidOperationException("Selecciona entre dos y cuatro ventanas (Ctrl + clic), en el orden J1, J2, J3, J4.");
        _sessions.Attach(handles, Layout());
        Status = $"{handles.Count} ventanas distribuidas. Sus procesos no se cerrarán.";
    });
}
