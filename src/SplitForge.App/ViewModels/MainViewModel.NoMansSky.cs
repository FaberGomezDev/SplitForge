using System.Windows.Input;
using Microsoft.Win32;
using SplitForge.Core.Adapters;
using SplitForge.Core.Sessions;
using SplitForge.Core.Steam;
using SplitForge.Platform;

namespace SplitForge.App.ViewModels;

/// <summary>
/// No Man's Sky (EXPERIMENTAL, NO VALIDADO: <c>docs/plans/no-mans-sky.md</c>). Cada jugador usa su propia copia
/// con su cuenta: J1 la de Steam y J2 otra (por ejemplo la de GOG). SplitForge abre las copias que haya, da a
/// cada proceso su mando y divide la pantalla cuando están las dos ventanas. No hay emulador de Steam aquí ni
/// se toca ningún archivo del juego (solo se añade el filtro de mandos, que es propio y se puede retirar).
/// </summary>
public sealed partial class MainViewModel
{
    public ICommand PickNoMansSkyCopyCommand { get; private set; } = null!;
    public ICommand RemoveNoMansSkyFilterCommand { get; private set; } = null!;

    private string _nmsSecondCopy = "";
    private string _nmsCopiesText = "";
    /// <summary>Qué copia usa cada jugador, para la pantalla JUGAR.</summary>
    public string NoMansSkyCopiesText { get => _nmsCopiesText; private set => Set(ref _nmsCopiesText, value); }

    private bool _nmsFilterInstalled;
    /// <summary>Hay filtro de mandos instalado en alguna copia (se puede retirar).</summary>
    public bool NoMansSkyFilterInstalled { get => _nmsFilterInstalled; private set => Set(ref _nmsFilterInstalled, value); }

    /// <summary>Lanzamiento en curso: cuántas ventanas se esperan y con qué distribución.</summary>
    private sealed record PendingNoMansSky(int Expected, LayoutOptions Layout, DateTime ReadyAt, DateTime Deadline);

    private PendingNoMansSky? _nmsPending;
    private string _nmsAssigned = "";
    private DateTime? _nmsCloseDeadline;

    private void InitNoMansSky()
    {
        _nmsSecondCopy = _settings.NoMansSky.SecondCopyFolder;
        PickNoMansSkyCopyCommand = new RelayCommand(PickNoMansSkyCopy);
        RemoveNoMansSkyFilterCommand = new RelayCommand(RemoveNoMansSkyFilter);
    }

    /// <summary>La copia de Steam (J1), validada dentro de la biblioteca; null si no está.</summary>
    private string? NoMansSkySteamCopy()
    {
        if (FindGame(NoMansSkyAdapter.AppId) is not { } game)
            return null;
        var (folder, inside) = PreflightEvaluator.SafeInstallFolder(game);
        return inside && NoMansSkyAdapter.IsInstall(folder) ? folder : null;
    }

    /// <summary>La copia de J2: la que indicó el usuario o, si no, la de GOG; null si no hay o es la misma de Steam.</summary>
    private string? NoMansSkySecondCopy(string? steamCopy)
    {
        var candidate = NoMansSkyAdapter.IsInstall(_nmsSecondCopy) ? _nmsSecondCopy : GogLocator.FindInstall(NoMansSkyAdapter.GameExe);
        if (!NoMansSkyAdapter.IsInstall(candidate))
            return null;
        return steamCopy is not null && string.Equals(Path.GetFullPath(candidate!), Path.GetFullPath(steamCopy), StringComparison.OrdinalIgnoreCase)
            ? null
            : candidate;
    }

    private static bool FilterIn(string? installFolder) =>
        installFolder is not null && XInputFilter.InstalledNames(NoMansSkyAdapter.Binaries(installFolder)).Count > 0;

    private IReadOnlyList<PlayStep> NoMansSkySteps(IReadOnlyList<int> connected)
    {
        var steam = NoMansSkySteamCopy();
        var second = NoMansSkySecondCopy(steam);
        var filterSteam = FilterIn(steam);
        var filterSecond = FilterIn(second);
        PlayCanPrepare = (steam is not null && !filterSteam) || (second is not null && !filterSecond);
        NoMansSkyFilterInstalled = filterSteam || filterSecond;
        NoMansSkyCopiesText = "J1: " + (steam is null ? "sin copia de Steam" : "copia de Steam")
                              + "  ·  J2: " + (second is null ? "sin segunda copia" : "segunda copia lista");
        return NoMansSkyAdapter.Readiness(steam is not null, second is not null, filterSteam, filterSecond, connected, [SlotPlayer1, SlotPlayer2]);
    }

    /// <summary>El usuario indica dónde está la copia de J2 eligiendo su <c>NMS.exe</c>.</summary>
    private void PickNoMansSkyCopy() => Guard(() =>
    {
        var dialog = new OpenFileDialog
        {
            Title = "Elige el NMS.exe de la copia del jugador 2 (carpeta Binaries)",
            Filter = "No Man's Sky|" + NoMansSkyAdapter.GameExe,
            FileName = NoMansSkyAdapter.GameExe,
        };
        if (dialog.ShowDialog() != true)
            return;
        var install = NoMansSkyAdapter.InstallFromPath(dialog.FileName)
                      ?? throw new InvalidOperationException("Ese archivo no es el NMS.exe de una instalación de No Man's Sky.");
        var steam = NoMansSkySteamCopy();
        if (steam is not null && string.Equals(Path.GetFullPath(install), Path.GetFullPath(steam), StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("Esa es la copia de Steam, la de J1. J2 necesita otra copia con otra cuenta: una sola copia no puede abrir dos jugadores.");
        _nmsSecondCopy = install;
        SaveSettings();
        _log.Write("No Man's Sky: copia de J2 indicada.");
        RefreshPlay();
    });

    /// <summary>Instala el filtro de mandos en las copias que no lo tengan, con una sola confirmación.</summary>
    private void InstallNoMansSkyFilter() => Guard(() =>
    {
        var steam = NoMansSkySteamCopy();
        var targets = new[] { (Name: "copia de Steam", Folder: steam), (Name: "copia de J2", Folder: NoMansSkySecondCopy(steam)) }
            .Where(t => t.Folder is not null && !FilterIn(t.Folder)).ToList();
        if (targets.Count == 0)
            return;
        if (WindowsDiagnostics.RunningImages(NoMansSkyAdapter.GameExe).Count > 0)
            throw new InvalidOperationException("Cierra No Man's Sky antes de instalar el filtro de mandos: se carga al abrir el juego.");
        var dll = Path.Combine(AppContext.BaseDirectory, "native", XInputFilter.DllFileName);
        if (!File.Exists(dll))
            throw new InvalidOperationException("Falta el filtro de mandos en el paquete de SplitForge (native\\" + XInputFilter.DllFileName + ").");
        if (!Confirm("Filtro de mandos en No Man's Sky (experimental)",
                "Se copiará el filtro de SplitForge junto al ejecutable del juego con el nombre xinput9_1_0.dll, que es el que "
                + "el juego carga. No se sobrescribe ningún archivo del juego y se puede retirar cuando quieras.\n\n"
                + "No Man's Sky es un juego en línea. El filtro solo decide qué mando ve cada ventana, pero es un archivo "
                + "añadido a su carpeta, como un mod. El juego no lleva antitrampas; aun así, la decisión es tuya.\n\n¿Instalar?"))
            return;
        foreach (var (name, folder) in targets)
        {
            var result = XInputFilter.Install(NoMansSkyAdapter.Binaries(folder!), dll, NoMansSkyAdapter.FilterNames);
            _log.Write(result.Skipped.Count > 0
                ? $"No Man's Sky ({name}): ya había un {string.Join(", ", result.Skipped)} que no es de SplitForge; no se toca y el filtro no puede actuar ahí."
                : $"No Man's Sky ({name}): filtro de mandos instalado.");
        }
    });

    private void RemoveNoMansSkyFilter() => Guard(() =>
    {
        if (WindowsDiagnostics.RunningImages(NoMansSkyAdapter.GameExe).Count > 0)
            throw new InvalidOperationException("Cierra No Man's Sky antes de quitar el filtro de mandos.");
        var steam = NoMansSkySteamCopy();
        var removed = 0;
        foreach (var folder in new[] { steam, NoMansSkySecondCopy(steam) })
        {
            if (FilterIn(folder))
                removed += XInputFilter.Uninstall(NoMansSkyAdapter.Binaries(folder!)).Count;
        }
        _log.Write($"No Man's Sky: filtro de mandos retirado ({removed} archivo(s)).");
        RefreshPlay();
    });

    /// <summary>Si el filtro instalado es de una compilación anterior de SplitForge, se pone al día antes de abrir.</summary>
    private void UpdateNoMansSkyFilter(string? installFolder)
    {
        if (!FilterIn(installFolder))
            return;
        var dll = Path.Combine(AppContext.BaseDirectory, "native", XInputFilter.DllFileName);
        if (!File.Exists(dll))
            return;
        try
        {
            var binaries = NoMansSkyAdapter.Binaries(installFolder!);
            if (XInputFilter.Install(binaries, dll, XInputFilter.InstalledNames(binaries)).Updated.Count > 0)
                _log.Write("Filtro de mandos puesto al día en No Man's Sky.");
        }
        catch (Exception ex) when (ex is InvalidOperationException or IOException or UnauthorizedAccessException)
        {
            _log.Write($"No se pudo poner al día el filtro de mandos de No Man's Sky ({ex.Message}); se abre con el instalado.");
        }
    }

    /// <summary>
    /// JUGAR: abre la copia de cada jugador (la de Steam, a través de Steam) si no está ya abierta. Los mandos
    /// se asignan en cuanto existe cada proceso y la pantalla se divide cuando están las dos ventanas.
    /// </summary>
    private void LaunchNoMansSky() => Guard(() =>
    {
        var steam = NoMansSkySteamCopy();
        var second = NoMansSkySecondCopy(steam);
        if (steam is null && second is null)
            throw new InvalidOperationException("No Man's Sky no aparece instalado.");
        UpdateNoMansSkyFilter(steam);
        UpdateNoMansSkyFilter(second);
        var open = WindowsDiagnostics.RunningImages(NoMansSkyAdapter.GameExe)
            .Select(p => NoMansSkyAdapter.PlayerOf(p.Image, steam, second)).ToHashSet();
        var expected = 0;
        if (steam is not null)
        {
            expected++;
            if (!open.Contains(1))
            {
                // La copia de Steam no arranca sola: se le pide a Steam, como al pulsar «Jugar» en su biblioteca.
                using var _ = System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(NoMansSkyAdapter.SteamLaunchUri) { UseShellExecute = true });
                _log.Write("No Man's Sky: J1 abierto a través de Steam.");
            }
        }
        if (second is not null)
        {
            expected++;
            if (!open.Contains(2))
            {
                using var _ = System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(NoMansSkyAdapter.Exe(second))
                {
                    UseShellExecute = false,
                    WorkingDirectory = NoMansSkyAdapter.Binaries(second),
                });
                _log.Write("No Man's Sky: J2 abierto desde su copia.");
            }
        }
        var now = DateTime.UtcNow;
        _nmsAssigned = "";
        _nmsPending = new PendingNoMansSky(expected, Layout(), now.AddSeconds(10), now.AddMinutes(4));
        Status = expected == 2
            ? "No Man's Sky: J1 y J2 arrancando; la pantalla se dividirá sola cuando aparezcan sus ventanas."
            : "No Man's Sky: arrancando un solo jugador. Para dividir la pantalla falta la copia del otro.";
        if (!FilterIn(steam) && !FilterIn(second))
            _log.Write("No Man's Sky: filtro de mandos sin instalar; los dos mandos controlarán todas las ventanas. Instálalo con «Preparar ahora».");
    });

    /// <summary>Llamado a ≈2 Hz mientras hay un lanzamiento en curso: mandos por proceso y, al final, pantalla dividida.</summary>
    private void TileNoMansSkyWhenReady()
    {
        if (_nmsPending is not { } pending)
            return;
        if (DateTime.UtcNow > pending.Deadline)
        {
            _nmsPending = null;
            Status = "Las ventanas de No Man's Sky no aparecieron a tiempo. Si Steam o el juego piden algo, atiéndelo y pulsa JUGAR otra vez.";
            return;
        }
        var steam = NoMansSkySteamCopy();
        var second = NoMansSkySecondCopy(steam);
        var players = new SortedDictionary<int, int>(); // jugador → PID
        foreach (var (pid, image) in WindowsDiagnostics.RunningImages(NoMansSkyAdapter.GameExe))
        {
            if (NoMansSkyAdapter.PlayerOf(image, steam, second) is { } player)
                players.TryAdd(player, pid);
        }
        // Cada proceso recibe su mando en cuanto existe, antes de que el juego empiece a leerlos.
        var key = string.Join(",", players.Select(p => $"{p.Key}:{p.Value}"));
        if (players.Count > 0 && key != _nmsAssigned)
        {
            _nmsAssigned = key;
            XInputFilter.WriteAssignments(XInputFolder, [.. players.Select(p => (p.Value, (p.Key == 1 ? SlotPlayer1 : SlotPlayer2) - 1))]);
            _log.Write("No Man's Sky, mandos: " + string.Join(" · ", players.Select(p => $"J{p.Key} (PID {p.Value}) → mando XInput {(p.Key == 1 ? SlotPlayer1 : SlotPlayer2)}")) + ".");
        }
        if (DateTime.UtcNow < pending.ReadyAt || players.Count < pending.Expected)
            return;
        var windows = _windows.EnumerateWindows();
        var handles = new List<nint>();
        foreach (var pid in players.Values)
        {
            var window = windows.FirstOrDefault(w => w.Pid == pid && w.Title == NoMansSkyAdapter.WindowTitle);
            if (window.Handle == 0)
                return; // aún no
            handles.Add(window.Handle);
        }
        _nmsPending = null;
        if (handles.Count < 2)
        {
            Status = $"No Man's Sky: abierto J{players.Keys.First()} con su mando. Para dividir la pantalla falta la copia del otro jugador.";
            _log.Write(Status);
            return;
        }
        if (_sessions.Active is not null)
            _sessions.Stop();
        _sessions.Attach(handles, pending.Layout);
        // El foco real vuelve a SplitForge: ninguna ventana queda «más enfocada» que la otra.
        System.Windows.Application.Current?.MainWindow?.Activate();
        Status = "No Man's Sky: J1 y J2 en pantalla dividida, cada uno con su mando.";
    }

    /// <summary>
    /// «Cerrar juegos»: pide el cierre normal de cada ventana. El juego solo guarda en sus puntos de guardado,
    /// así que antes se avisa.
    /// </summary>
    private void CloseNoMansSky() => Guard(() =>
    {
        var pids = WindowsDiagnostics.RunningImages(NoMansSkyAdapter.GameExe).Select(p => p.Pid).ToHashSet();
        var windows = _windows.EnumerateWindows().Where(w => pids.Contains(w.Pid) && w.Title == NoMansSkyAdapter.WindowTitle).ToList();
        if (windows.Count == 0)
        {
            Status = "No hay ninguna ventana de No Man's Sky abierta.";
            return;
        }
        if (!Confirm("Cerrar No Man's Sky",
                "Se pedirá a cada ventana que se cierre, como al pulsar su X. No Man's Sky solo guarda en sus puntos de guardado: "
                + "lo que hayas hecho desde el último se pierde. Si quieres conservarlo, guarda dentro del juego antes.\n\n¿Cerrar ahora?"))
            return;
        foreach (var window in windows)
            _windows.RequestClose(window.Handle);
        _nmsPending = null;
        _nmsCloseDeadline = DateTime.UtcNow.AddSeconds(60);
        _log.Write("No Man's Sky: cierre normal pedido a sus ventanas.");
        Status = "Cerrando No Man's Sky…";
    });

    /// <summary>Llamado a ≈2 Hz mientras hay un cierre en curso.</summary>
    private void CloseNoMansSkyStep()
    {
        if (_nmsCloseDeadline is not { } deadline)
            return;
        if (WindowsDiagnostics.RunningImages(NoMansSkyAdapter.GameExe).Count == 0)
        {
            _nmsCloseDeadline = null;
            _nmsAssigned = "";
            XInputFilter.ClearAssignments(XInputFolder);
            StopSession();
            Status = "No Man's Sky cerrado; pantalla restaurada.";
            _log.Write("No Man's Sky cerrado con normalidad.");
        }
        else if (DateTime.UtcNow > deadline)
        {
            _nmsCloseDeadline = null;
            Status = "No Man's Sky no se ha cerrado: sal desde su menú. SplitForge no lo cierra a la fuerza.";
            _log.Write(Status);
        }
    }
}
