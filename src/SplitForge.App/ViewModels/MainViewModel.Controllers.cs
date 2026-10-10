using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;
using SplitForge.Core.Input;
using SplitForge.Input;

namespace SplitForge.App.ViewModels;

public sealed class PadRow : Observable
{
    public PadRow(PadInfo info) => Info = info;

    public PadInfo Info { get; private set; }
    public uint DeviceId => Info.DeviceId;
    public string Label => $"{Info.Name}  ·  ID {Info.DeviceId}";
    public string Kind => Info.GamepadType.Length > 0 ? $"{Info.KindLabel} ({Info.GamepadType})" : Info.KindLabel;

    private string _path = "";
    public string Path { get => _path; set => Set(ref _path, value); }

    private string _live = "SIN LECTURA";
    public string Live { get => _live; set => Set(ref _live, value); }

    private string _raw = "";
    public string Raw { get => _raw; set => Set(ref _raw, value); }

    public void Update(PadInfo info) => Info = info;
}

public sealed partial class MainViewModel
{
    public ObservableCollection<PadRow> Pads { get; } = [];
    public IReadOnlyList<InputReadMode> Modes { get; } = [InputReadMode.Auto, InputReadMode.Joystick, InputReadMode.Gamepad];
    public ObservableCollection<int> AxisChoices { get; } = [];
    public ObservableCollection<string> ButtonChoices { get; } = [];

    public ICommand ApplyModeCommand { get; private set; } = null!;
    public ICommand LearnCommand { get; private set; } = null!;
    public ICommand SaveMappingCommand { get; private set; } = null!;
    public ICommand IdentifyCommand { get; private set; } = null!;

    private InputReadMode _selectedMode;
    public InputReadMode SelectedMode { get => _selectedMode; set => Set(ref _selectedMode, value); }

    private PadRow? _selectedPad;
    public PadRow? SelectedPad
    {
        get => _selectedPad;
        set
        {
            if (Set(ref _selectedPad, value))
                LoadMapping();
        }
    }

    private int _mapAxisX;
    public int MapAxisX { get => _mapAxisX; set => Set(ref _mapAxisX, value); }
    private int _mapAxisY = 1;
    public int MapAxisY { get => _mapAxisY; set => Set(ref _mapAxisY, value); }
    private string _mapFire = "0";
    public string MapFire { get => _mapFire; set => Set(ref _mapFire, value); }
    private string _mapDash = "1";
    public string MapDash { get => _mapDash; set => Set(ref _mapDash, value); }
    private string _mapShield = "2";
    public string MapShield { get => _mapShield; set => Set(ref _mapShield, value); }
    private bool _invertX;
    public bool InvertX { get => _invertX; set => Set(ref _invertX, value); }
    private bool _invertY;
    public bool InvertY { get => _invertY; set => Set(ref _invertY, value); }
    private string _captureStatus = "Pulsa «Aprender» y después el botón de tu mando.";
    public string CaptureStatus { get => _captureStatus; set => Set(ref _captureStatus, value); }

    private int? _fireAxis;
    private float _fireRest;
    private int _fireDirection = 1;
    private string? _captureAction;
    private uint _captureDevice;
    private IReadOnlyCollection<int> _capturePrevious = [];
    private IReadOnlyList<float> _captureBaseline = [];

    private void InitControllers()
    {
        ApplyModeCommand = new RelayCommand(() =>
        {
            _input.SetMode(SelectedMode);
            SaveSettings();
            _log.Write($"Lector de mandos: {SelectedMode}.");
        });
        LearnCommand = new RelayCommand(p => StartLearn(p as string ?? "fire"), _ => SelectedPad is not null);
        SaveMappingCommand = new RelayCommand(() => SaveMapping(notify: true), () => SelectedPad is not null);
        IdentifyCommand = new RelayCommand(() =>
        {
            for (var i = 0; i < Pads.Count; i++)
                _input.IdentifyPlayer(Pads[i].DeviceId, i);
            _log.Write("Mandos identificados: cada uno muestra el color de su jugador (J1 turquesa, J2 naranja, J3 violeta, J4 amarillo).");
        });
    }

    private void RefreshControllers(InputSnapshot snap)
    {
        var ids = snap.Devices.Select(d => d.DeviceId).ToHashSet();
        for (var i = Pads.Count - 1; i >= 0; i--)
        {
            if (!ids.Contains(Pads[i].DeviceId))
                Pads.RemoveAt(i);
        }
        foreach (var info in snap.Devices)
        {
            var row = Pads.FirstOrDefault(p => p.DeviceId == info.DeviceId);
            if (row is null)
            {
                Pads.Add(row = new PadRow(info));
                SelectedPad ??= row;
            }
            else
            {
                row.Update(info);
            }
            row.Path = snap.Paths.TryGetValue(info.DeviceId, out var path) ? path.ToString() : "-";
            row.Live = snap.States.TryGetValue(info.DeviceId, out var s)
                ? $"X={s.LX:+0.00;-0.00}  Y={s.LY:+0.00;-0.00}  Disparo={(s.Fire ? "SÍ" : "no")}  Impulso={(s.Dash ? "SÍ" : "no")}  Escudo={(s.Shield ? "SÍ" : "no")}"
                : "SIN LECTURA (revisa el perfil o el modo de lectura)";
            row.Raw = snap.Raw.TryGetValue(info.DeviceId, out var raw)
                ? $"Ejes: [{string.Join("  ", raw.Axes.Select(a => a.ToString("+0.00;-0.00")))}]   Botones: [{string.Join(",", raw.PressedButtons)}]"
                : "";
        }
        InputSummary = $"{_input.Backend}{(_input.Error is null ? "" : " · " + _input.Error)}  ·  {snap.Devices.Count} mando(s)  ·  hilo de entrada {snap.MeasuredHz:0} Hz";
    }

    private void LoadMapping()
    {
        var pad = SelectedPad;
        AxisChoices.Clear();
        ButtonChoices.Clear();
        if (pad is null)
            return;
        for (var i = 0; i < pad.Info.Axes; i++)
            AxisChoices.Add(i);
        ButtonChoices.Add("");
        for (var i = 0; i < pad.Info.Buttons; i++)
            ButtonChoices.Add(i.ToString(System.Globalization.CultureInfo.InvariantCulture));
        var m = _input.Mappings.TryGetValue(pad.Info.Guid, out var saved) ? saved : ControllerMapping.DefaultFor(pad.Info.Buttons);
        MapAxisX = m.AxisX;
        MapAxisY = m.AxisY;
        MapFire = m.FireButton.ToString(System.Globalization.CultureInfo.InvariantCulture);
        MapDash = m.DashButton?.ToString(System.Globalization.CultureInfo.InvariantCulture) ?? "";
        MapShield = m.ShieldButton?.ToString(System.Globalization.CultureInfo.InvariantCulture) ?? "";
        InvertX = m.InvertX;
        InvertY = m.InvertY;
        _fireAxis = m.FireAxis;
        _fireRest = m.FireRest;
        _fireDirection = m.FireDirection;
        CaptureStatus = "Disparo con R2: " + (_fireAxis is { } a ? $"eje {a} calibrado." : "no configurado; pulsa Aprender y después Cruz o R2.");
    }

    private void StartLearn(string action)
    {
        var pad = SelectedPad;
        if (pad is null)
            return;
        if (_input.Latest.Paths.TryGetValue(pad.DeviceId, out var path) && path == ReadPath.Gamepad)
        {
            MessageBox.Show("Para calibrar botones físicos pon el lector en «Joystick» y pulsa Aplicar. Los PS4 en modo Auto ya usan joystick.",
                "Lector de mandos", MessageBoxButton.OK, MessageBoxImage.Information);
            return;
        }
        if (!_input.Latest.Raw.TryGetValue(pad.DeviceId, out var raw))
            return;
        _captureAction = action;
        _captureDevice = pad.DeviceId;
        _capturePrevious = raw.PressedButtons;
        _captureBaseline = raw.Axes;
        var label = action switch { "dash" => "IMPULSO (Círculo)", "shield" => "ESCUDO (Cuadrado)", _ => "DISPARO (Cruz o R2)" };
        CaptureStatus = $"Esperando {label} del mando ID {pad.DeviceId}. Suelta todo y púlsalo ahora.";
    }

    private void CaptureTick(InputSnapshot snap)
    {
        if (_captureAction is null)
            return;
        if (SelectedPad?.DeviceId != _captureDevice || !snap.Raw.TryGetValue(_captureDevice, out var raw))
        {
            _captureAction = null;
            CaptureStatus = "Se cambió o desconectó el mando; vuelve a pulsar Aprender.";
            return;
        }
        var fresh = AxisMath.NewPresses(raw.PressedButtons, _capturePrevious);
        _capturePrevious = raw.PressedButtons;
        var invariant = System.Globalization.CultureInfo.InvariantCulture;
        if (fresh.Count > 0)
        {
            var idx = fresh[0].ToString(invariant);
            switch (_captureAction)
            {
                case "dash": MapDash = idx; break;
                case "shield": MapShield = idx; break;
                default:
                    MapFire = idx;
                    _fireAxis = null; // Recalibrar Cruz anula el gatillo analógico previo.
                    break;
            }
            _captureAction = null;
            CaptureStatus = $"Detectado botón físico {idx}. Guardado.";
            SaveMapping(notify: false);
            return;
        }
        if (_captureAction == "fire")
        {
            // Algunos PS4 exponen R2 como eje analógico; se ignoran los ejes del stick de movimiento.
            for (var i = 0; i < Math.Min(raw.Axes.Count, _captureBaseline.Count); i++)
            {
                var delta = raw.Axes[i] - _captureBaseline[i];
                if (i != MapAxisX && i != MapAxisY && MathF.Abs(delta) >= 0.65f)
                {
                    _fireAxis = i;
                    _fireRest = _captureBaseline[i];
                    _fireDirection = delta > 0 ? 1 : -1;
                    _captureAction = null;
                    CaptureStatus = $"Detectado gatillo analógico en el eje {i}. Guardado.";
                    SaveMapping(notify: false);
                    return;
                }
            }
        }
    }

    private void SaveMapping(bool notify)
    {
        var pad = SelectedPad;
        if (pad is null)
            return;
        try
        {
            static int? Optional(string text) => int.TryParse(text, out var v) ? v : null;
            var mapping = new ControllerMapping
            {
                AxisX = MapAxisX,
                AxisY = MapAxisY,
                FireButton = Optional(MapFire) ?? 0,
                DashButton = Optional(MapDash),
                ShieldButton = Optional(MapShield),
                InvertX = InvertX,
                InvertY = InvertY,
                FireAxis = _fireAxis,
                FireRest = _fireRest,
                FireDirection = _fireDirection,
            }.Validate(pad.Info.Axes, pad.Info.Buttons);
            _input.SetMapping(pad.Info.Guid, mapping);
            SaveSettings();
            _log.Write($"Perfil guardado para {pad.Info.Name}: disparo={mapping.FireButton}, gatillo_eje={mapping.FireAxis?.ToString() ?? "-"}, "
                       + $"impulso={mapping.DashButton?.ToString() ?? "-"}, escudo={mapping.ShieldButton?.ToString() ?? "-"}.");
            if (notify)
                MessageBox.Show("Configuración guardada. Pruébala en el mundo compartido.", "Mandos", MessageBoxButton.OK, MessageBoxImage.Information);
        }
        catch (ArgumentException ex)
        {
            CaptureStatus = "No se guardó: " + ex.Message;
            if (notify)
                MessageBox.Show(ex.Message, "Mandos", MessageBoxButton.OK, MessageBoxImage.Warning);
        }
    }
}
