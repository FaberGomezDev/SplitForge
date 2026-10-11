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
    public string Icon => Step.Ok ? "\uE73E" : "\uE7BA"; // Segoe MDL2: marca / aviso
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

/// <summary>Un juego de la biblioteca del lanzador, con sus imágenes (de la caché local de Steam del usuario).</summary>
public sealed record GameCard(LauncherGame Game, ImageSource? Cover, ImageSource? Hero, ImageSource? Logo)
{
    public string Name => Game.Name;
    public string Tagline => Game.Tagline;
}

/// <summary>
/// Pantalla principal del lanzador: la biblioteca, la portada del juego elegido, qué falta para jugar y los
/// jugadores. Solo refleja estado ya comprobado; JUGAR y «Cerrar juegos» hacen lo que toque en cada juego.
/// </summary>
public sealed partial class MainViewModel
{
    public ObservableCollection<PlayStepRow> PlaySteps { get; } = [];
    public ObservableCollection<GameCard> Games { get; } = [];
    public ICommand PrepareCommand { get; private set; } = null!;
    public ICommand PlayCommand { get; private set; } = null!;
    public ICommand CloseGamesCommand { get; private set; } = null!;

    private GameCard _selectedGame = null!;
    /// <summary>Juego elegido en la biblioteca: de él dependen la portada, la preparación y lo que hace JUGAR.</summary>
    public GameCard SelectedGame
    {
        get => _selectedGame;
        set
        {
            // La lista no admite quedarse sin selección: con un clic en el hueco, se conserva el juego actual.
            if (value is null || !Set(ref _selectedGame, value))
                return;
            foreach (var name in new[] { nameof(GameHero), nameof(GameLogo), nameof(GameTitle), nameof(GameBanner), nameof(HowTo1),
                         nameof(HowTo2), nameof(HowTo3), nameof(EmulatedIdentity), nameof(AccountIdentity), nameof(PlayToolTip) })
                Raise(name);
            _lastPlayState = null;
            SaveSettings();
            Guard(RefreshPlay, quiet: true);
        }
    }

    private bool IsNoMansSky => _selectedGame.Game.Id == LauncherGames.NoMansSky;

    /// <summary>Imágenes del juego en la caché local de Steam del usuario (null si no las tiene).</summary>
    public ImageSource? GameHero => _selectedGame.Hero;
    public ImageSource? GameLogo => _selectedGame.Logo;
    public string GameTitle => _selectedGame.Game.Name.ToUpperInvariant();
    public string GameBanner => _selectedGame.Game.Banner;
    public string HowTo1 => _selectedGame.Game.HowTo[0];
    public string HowTo2 => _selectedGame.Game.HowTo[1];
    public string HowTo3 => _selectedGame.Game.HowTo[2];

    /// <summary>true = cada ventana entra con una identidad emulada (nombre y opciones de partida de 7DTD).</summary>
    public bool EmulatedIdentity => _selectedGame.Game.EmulatedIdentity;

    /// <summary>true = cada jugador entra con su propia copia y su cuenta (No Man's Sky).</summary>
    public bool AccountIdentity => !_selectedGame.Game.EmulatedIdentity;

    public string PlayToolTip => IsNoMansSky
        ? "Abre la copia de cada jugador, le da su mando y divide la pantalla cuando están las dos."
        : "Abre J1 y J2, da a cada uno su mando y divide la pantalla.";

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
        PlayCommand = new RelayCommand(Play);
        CloseGamesCommand = new RelayCommand(CloseGames);
        var roots = SteamLocator.FindRoots();
        foreach (var game in LauncherGames.All)
        {
            var art = SteamArt.Find(roots, game.AppId);
            Games.Add(new GameCard(game, LoadImage(art.Cover, 300), LoadImage(art.Hero, 1600), LoadImage(art.Logo, 640)));
        }
        _selectedGame = Games.First(card => card.Game.Id == LauncherGames.Find(_settings.SelectedGame).Id);
        InitNoMansSky();
        // RefreshPlay se llama desde el temporizador de la interfaz, nunca aquí: en el constructor aún no hay
        // contexto de sincronización y el refresco del estado del filtro lo necesita (el paquete se cerraba).
    }

    /// <summary>JUGAR: lo que toque en el juego elegido.</summary>
    private void Play()
    {
        if (IsNoMansSky)
            LaunchNoMansSky();
        else
            LaunchSteamEmuBoth();
    }

    /// <summary>«Cerrar juegos»: siempre un cierre normal, nunca a la fuerza.</summary>
    private void CloseGames()
    {
        if (IsNoMansSky)
            CloseNoMansSky();
        else
            CloseEmulatedClients();
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

    /// <summary>Llamado cada ≈2 s, al cambiar de juego y tras instalar algo: recalcula qué falta para jugar.</summary>
    private void RefreshPlay()
    {
        var connected = XInputProbe.ConnectedSlots();
        var steps = IsNoMansSky ? NoMansSkySteps(connected) : SevenDaysSteps(connected);
        if (!IsNoMansSky)
            RefreshNoMansSkyState(); // AVANZADO › No Man's Sky se mantiene al día aunque el juego elegido sea otro.

        // Solo se toca la lista si algo cambió: así la interfaz no parpadea ni se redibuja sin motivo.
        if (PlaySteps.Count != steps.Count || !PlaySteps.Select(row => row.Step).SequenceEqual(steps))
        {
            PlaySteps.Clear();
            foreach (var step in steps)
                PlaySteps.Add(new PlayStepRow(step));
        }
        PlayReady = PlayReadiness.Ready(steps);
        // Solo la inicial en minúscula: «J2» y «Steam» conservan su mayúscula dentro de la frase.
        PlayHeadline = PlayReady ? "Todo listo para jugar." : "Falta preparar: " + string.Join(" · ", steps.Where(s => !s.Ok).Select(s => char.ToLowerInvariant(s.Title[0]) + s.Title[1..])) + ".";
        Player1Pad = PadText(SlotPlayer1, connected);
        Player2Pad = PadText(SlotPlayer2, connected);

        static string PadText(int slot, IReadOnlyList<int> connected) =>
            connected.Contains(slot) ? $"Mando {slot} conectado" : $"Mando {slot} sin conectar";
    }

    private IReadOnlyList<PlayStep> SevenDaysSteps(IReadOnlyList<int> connected)
    {
        var folder = FindGame(ResearchProfiles.SevenDaysToDie)?.InstallFolder;
        var installed = folder is not null && Directory.Exists(folder);
        var emulator = installed && SteamEmulator.IsInstalled(folder!);
        var filter = installed && XInputFilter.InstalledNames(folder!).Count > 0;
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
        return PlayReadiness.Evaluate(installed, emulator, filter, connected, [SlotPlayer1, SlotPlayer2]);
    }

    /// <summary>Instala lo que falte (emulador y filtro de mandos), cada cosa con su confirmación.</summary>
    private void PrepareToPlay()
    {
        if (IsNoMansSky)
        {
            InstallNoMansSkyFilter();
        }
        else
        {
            if (_emulatorMissing)
                InstallSteamEmu();
            if (_filterMissing)
                InstallFilter();
        }
        RefreshPlay();
    }
}
