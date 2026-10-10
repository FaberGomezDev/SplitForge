using System.Windows;

namespace SplitForge.App;

public static class Program
{
    [STAThread]
    public static int Main(string[] args)
    {
        // Modo visor del laboratorio: el mismo ejecutable se lanza como proceso hijo por jugador.
        var viewer = Array.IndexOf(args, "--lab-viewer");
        if (viewer >= 0)
        {
            var player = viewer + 1 < args.Length && args[viewer + 1] == "2" ? 2 : 1;
            return SplitForge.LabViewer.LabViewer.Run(player, Console.OpenStandardInput());
        }

        var app = new Application { ShutdownMode = ShutdownMode.OnMainWindowClose };
        app.Resources.MergedDictionaries.Add(new ResourceDictionary
        {
            Source = new Uri("pack://application:,,,/SplitForge;component/Theme.xaml", UriKind.Absolute),
        });
        app.DispatcherUnhandledException += (_, e) =>
        {
            MessageBox.Show(e.Exception.Message, "SplitForge — error inesperado", MessageBoxButton.OK, MessageBoxImage.Error);
            e.Handled = true;
        };
        return app.Run(new MainWindow());
    }
}
