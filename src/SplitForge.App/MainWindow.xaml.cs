using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows;
using SplitForge.App.ViewModels;

namespace SplitForge.App;

public partial class MainWindow : Window
{
    private readonly MainViewModel _vm;
    private bool _closingConfirmed;

    public MainWindow()
    {
        InitializeComponent();
        // Ajustar a pantallas pequeñas: nunca más grande que el área útil del monitor.
        var area = SystemParameters.WorkArea;
        Width = Math.Min(Width, area.Width - 24);
        Height = Math.Min(Height, area.Height - 24);
        _vm = new MainViewModel();
        DataContext = _vm;
        // Mantener visible la última línea del registro.
        ((INotifyCollectionChanged)LogList.Items).CollectionChanged += (_, _) =>
        {
            if (LogList.Items.Count > 0)
                LogList.ScrollIntoView(LogList.Items[^1]);
        };
    }

    protected override async void OnClosing(CancelEventArgs e)
    {
        if (_closingConfirmed)
        {
            base.OnClosing(e);
            return;
        }
        // Restaurar ventanas y apagar el servidor propio de forma ordenada antes de salir.
        e.Cancel = true;
        IsEnabled = false;
        await _vm.PrepareCloseAsync();
        _closingConfirmed = true;
        // Si el apagado terminó al instante seguimos DENTRO de Closing, y WPF prohíbe Close() ahí:
        // se programa para justo después.
        _ = Dispatcher.BeginInvoke(Close);
    }

    protected override void OnClosed(EventArgs e)
    {
        _vm.Dispose();
        base.OnClosed(e);
    }
}
