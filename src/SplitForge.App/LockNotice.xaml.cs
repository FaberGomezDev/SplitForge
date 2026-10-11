using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;
using SplitForge.Core.Layout;
using SplitForge.Core.Windows;
using SplitForge.Platform;

namespace SplitForge.App;

/// <summary>
/// Aviso en pantalla del bloqueo de entrada «solo mandos» (Ctrl+Alt+B). Se dibuja por encima de los juegos,
/// no recibe clics ni toma el foco. Solo refleja el estado: no bloquea ni desbloquea nada.
/// </summary>
public partial class LockNotice : Window
{
    private const string LockedGlyph = "";
    private const string UnlockedGlyph = "";

    private readonly DispatcherTimer _toast = new() { Interval = TimeSpan.FromMilliseconds(LockNoticePlan.ToastMs) };
    private readonly DispatcherTimer _keepOnTop = new() { Interval = TimeSpan.FromMilliseconds(500) };
    private nint _hwnd;
    private Region _area;
    private bool _locked;
    private string _badge = string.Empty;

    public LockNotice()
    {
        InitializeComponent();
        _toast.Tick += (_, _) => EndToast();
        // Un juego activado después quedaría delante del aviso: se vuelve a subir mientras esté visible.
        _keepOnTop.Tick += (_, _) => Place();
        SizeChanged += (_, _) => Place();
    }

    protected override void OnSourceInitialized(EventArgs e)
    {
        base.OnSourceInitialized(e);
        _hwnd = new WindowInteropHelper(this).Handle;
        OverlayWindow.MakePassive(_hwnd);
    }

    /// <summary>Muestra el cartel del nuevo estado centrado arriba de <paramref name="area"/> (píxeles).</summary>
    public void Announce(bool locked, Region area, string hotkey)
    {
        var text = LockNoticePlan.Text(locked, hotkey);
        _locked = locked;
        _area = area;
        _badge = text.Badge;
        var color = (Brush)FindResource(locked ? "Accent" : "Ok");
        Box.BorderBrush = color;
        Box.Padding = new Thickness(28, 18, 28, 18);
        Box.Opacity = 1;
        Glyph.Foreground = color;
        Glyph.Text = locked ? LockedGlyph : UnlockedGlyph;
        Glyph.FontSize = 40;
        Headline.Text = text.Title;
        Headline.FontSize = 30;
        Detail.Text = text.Detail;
        Detail.Visibility = Visibility.Visible;
        if (!IsVisible)
            Show();
        UpdateLayout();
        Place();
        _toast.Stop();
        _toast.Start();
        _keepOnTop.Start();
    }

    /// <summary>Pasado el cartel: bloqueado, queda la insignia; desbloqueado, el aviso desaparece.</summary>
    private void EndToast()
    {
        _toast.Stop();
        if (!_locked || _badge.Length == 0)
        {
            _keepOnTop.Stop();
            Hide();
            return;
        }
        Box.Padding = new Thickness(14, 7, 16, 7);
        Box.Opacity = 0.88;
        Glyph.FontSize = 18;
        Headline.Text = _badge;
        Headline.FontSize = 16;
        Detail.Visibility = Visibility.Collapsed;
    }

    private void Place()
    {
        if (IsVisible)
            OverlayWindow.PlaceTopCenter(_hwnd, _area);
    }

    protected override void OnClosed(EventArgs e)
    {
        _toast.Stop();
        _keepOnTop.Stop();
        base.OnClosed(e);
    }
}
