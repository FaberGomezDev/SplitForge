using System.Collections.ObjectModel;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using SplitForge.Core.Adapters;
using SplitForge.Core.Steam;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

/// <summary>Fila de la lista «Preparación» de la pantalla principal.</summary>
public sealed record PlayStepRow(PlayStep Step)
{
    public string Title => Step.Title;
    public string Detail => Step.Detail;
    public string Icon => Step.Ok ? "" : ""; // Segoe MDL2: marca / aviso
    public Brush Color => Step.Ok ? OkBrush : WarnBrush;

    private static readonly Brush OkBrush = Frozen("#3DDC84");
    private static readonly Brush WarnBrush = Frozen("#FFC857");

    private static Brush Frozen(string hex)
    {
        var brush = new SolidColorBrush((Color)ColorConverter.ConvertFromString(hex));
        brush.Freeze();
        return brush;
    }
}

/// <summary>Un juego de la biblioteca del lanzador.</summary>
public sealed record GameCard(string Name, string Tagline, ImageSource? Cover);

/// <summary>
/// Pantalla principal del lanzador: la portada del juego, qué falta para jugar y los jugadores.
/// Solo refleja estado ya comprobado; las acciones son las mismas órdenes de siempre.
/// </summary>
public sealed partial class MainViewModel
{
    public ObservableCollection<PlayStepRow> PlaySteps { get; } = [];
    public ObservableCollection<GameCard> Games { get; } = [];
    public ICommand PrepareCommand { get; private set; } = null!;

    /// <summary>Imágenes del juego en la caché local de Steam del usuario (null si no las tiene).</summary>
    public ImageSource? GameHero { get; private set; }
    public ImageSource? GameLogo { get; private set; }

    private bool _playReady;
    public bool PlayReady { get => _playReady; private set { if (Set(ref _playReady, value)) Raise(nameof(PlayNeedsSetup)); } }
    public bool PlayNeedsSetup => !_playReady;

    private string _playHeadline = "Comprobando…";
    public string PlayHeadline { get => _playHeadline; private set => Set(ref _playHeadline, value); }

    private string _player1Pad = "";
    public string Player1Pad { get => _player1Pad; private set => Set(ref _player1Pad, value); }

    private string _player2Pad = "";
    public string Player2Pad { get => _player2Pad; private set => Set(ref _player2Pad, value); }

    private bool _emulatorMissing;
    private bool _filterMissing;
    private (bool Installed, bool Emulator, bool Filter)? _lastPlayState;

    private bool _playCanPrepare;
    /// <summary>Falta algo que SplitForge puede instalar por el jugador (emulador o filtro de mandos).</summary>
    public bool PlayCanPrepare { get => _playCanPrepare; private set => Set(ref _playCanPrepare, value); }

    private void InitPlay()
    {
        PrepareCommand = new RelayCommand(PrepareToPlay);
        var art = SteamArt.Find(SteamLocator.FindRoots(), ResearchProfiles.SevenDaysToDie);
        GameHero = LoadImage(art.Hero, 1600);
        GameLogo = LoadImage(art.Logo, 640);
        Games.Add(new GameCard("7 Days to Die", "Pantalla dividida · 2 jugadores", LoadImage(art.Cover, 300)));
        // RefreshPlay se llama desde el temporizador de la interfaz, nunca aquí: en el constructor aún no hay
        // contexto de sincronización y el refresco del estado del filtro lo necesita (el paquete se cerraba).
    }

    /// <summary>Carga la imagen completa en memoria y suelta el archivo: no bloquea la caché de Steam.</summary>
    private static ImageSource? LoadImage(string? path, int decodeWidth)
    {
        if (path is null)
            return null;
        try
        {
            var image = new BitmapImage();
            image.BeginInit();
            image.CacheOption = BitmapCacheOption.OnLoad;
            image.DecodePixelWidth = decodeWidth;
            image.UriSource = new Uri(path);
            image.EndInit();
            image.Freeze();
            return image;
        }
        catch (Exception ex) when (ex is IOException or NotSupportedException or UnauthorizedAccessException
                                       or InvalidOperationException or ArgumentException)
        {
            return null;
        }
    }

    /// <summary>Llamado cada ≈2 s y tras instalar algo: recalcula qué falta para jugar.</summary>
    private void RefreshPlay()
    {
        var folder = FindGame(ResearchProfiles.SevenDaysToDie)?.InstallFolder;
        var installed = folder is not null && Directory.Exists(folder);
        var emulator = installed && SteamEmulator.IsInstalled(folder!);
        var filter = installed && XInputFilter.InstalledNames(folder!).Count > 0;
        var connected = XInputProbe.ConnectedSlots();
        var steps = PlayReadiness.Evaluate(installed, emulator, filter, connected, [SlotPlayer1, SlotPlayer2]);
        _emulatorMissing = installed && !emulator;
        _filterMissing = installed && !filter;
        PlayCanPrepare = _emulatorMissing || _filterMissing;
        // Los textos de estado de AVANZADO se ponen al día cuando cambia algo (p. ej. al terminar de leer Steam).
        if (_lastPlayState != (installed, emulator, filter))
        {
            _lastPlayState = (installed, emulator, filter);
            RefreshSteamEmuStatus();
            RefreshFilterStatus();
        }

        // Solo se toca la lista si algo cambió: así la interfaz no parpadea ni se redibuja sin motivo.
        if (PlaySteps.Count != steps.Count || !PlaySteps.Select(row => row.Step).SequenceEqual(steps))
        {
            PlaySteps.Clear();
            foreach (var step in steps)
                PlaySteps.Add(new PlayStepRow(step));
        }
        PlayReady = PlayReadiness.Ready(steps);
        PlayHeadline = PlayReady ? "Todo listo para jugar." : "Falta preparar: " + string.Join(" · ", steps.Where(s => !s.Ok).Select(s => s.Title.ToLowerInvariant())) + ".";
        Player1Pad = PadText(SlotPlayer1, connected);
        Player2Pad = PadText(SlotPlayer2, connected);

        static string PadText(int slot, IReadOnlyList<int> connected) =>
            connected.Contains(slot) ? $"Mando {slot} conectado" : $"Mando {slot} sin conectar";
    }

    /// <summary>Instala lo que falte (emulador y filtro de mandos), cada cosa con su confirmación.</summary>
    private void PrepareToPlay()
    {
        if (_emulatorMissing)
            InstallSteamEmu();
        if (_filterMissing)
            InstallFilter();
        RefreshPlay();
    }
}
