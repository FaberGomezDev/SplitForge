using System.Collections.Concurrent;
using System.Collections.Immutable;
using System.Diagnostics;
using SDL;
using SplitForge.Core.Input;
using static SDL.SDL3;

namespace SplitForge.Input;

/// <summary>Fotografía inmutable de todos los mandos en un instante.</summary>
public sealed record InputSnapshot(
    long Sequence,
    IReadOnlyList<PadInfo> Devices,
    IReadOnlyDictionary<uint, PadState> States,
    IReadOnlyDictionary<uint, RawPadState> Raw,
    IReadOnlyDictionary<uint, ReadPath> Paths,
    double MeasuredHz,
    IReadOnlyDictionary<uint, BatteryInfo> Batteries)
{
    public static readonly InputSnapshot Empty = new(0, [], ImmutableDictionary<uint, PadState>.Empty,
        ImmutableDictionary<uint, RawPadState>.Empty, ImmutableDictionary<uint, ReadPath>.Empty, 0,
        ImmutableDictionary<uint, BatteryInfo>.Empty);
}

/// <summary>
/// Lee los mandos con SDL3 en un hilo propio a alta frecuencia (hasta 1000 Hz), totalmente
/// independiente de la interfaz: una ventana bloqueada o un diálogo no frenan la entrada.
/// Solo este hilo toca SDL; el resto del programa lee <see cref="Latest"/> sin bloquear.
/// </summary>
public sealed unsafe class SdlInputService : IDisposable
{
    // Colores de la barra de luz por jugador (mismos que el laboratorio).
    private static readonly (byte R, byte G, byte B)[] PlayerColors =
        [(0x30, 0xC7, 0xB3), (0xFF, 0x8A, 0x4C), (0x8A, 0x7C, 0xFF), (0xFF, 0xD1, 0x4A)];

    private readonly Thread _thread;
    private readonly ConcurrentQueue<Action> _commands = new();
    private readonly Action<string> _log;
    private readonly int _rateHz;
    private volatile bool _stop;
    private volatile InputReadMode _mode;
    private ImmutableDictionary<string, ControllerMapping> _mappings;
    private InputSnapshot _latest = InputSnapshot.Empty;

    public SdlInputService(InputReadMode mode, IReadOnlyDictionary<string, ControllerMapping> mappings, int rateHz = 1000,
        Action<string>? log = null)
    {
        _mode = mode;
        _mappings = mappings.ToImmutableDictionary();
        _rateHz = Math.Clamp(rateHz, 60, 1000);
        _log = log ?? (_ => { });
        _thread = new Thread(Run) { IsBackground = true, Name = "SplitForge.Input", Priority = ThreadPriority.AboveNormal };
        _thread.Start();
    }

    public InputSnapshot Latest => Volatile.Read(ref _latest);
    public string Backend { get; private set; } = "SDL3 (iniciando)";
    public string? Error { get; private set; }
    public InputReadMode Mode => _mode;
    public IReadOnlyDictionary<string, ControllerMapping> Mappings => _mappings;

    public void SetMode(InputReadMode mode) => _mode = mode;

    public void SetMapping(string guid, ControllerMapping mapping) =>
        ImmutableInterlocked.Update(ref _mappings, m => m.SetItem(guid, mapping));

    /// <summary>Enciende la barra de luz del mando con el color del jugador y vibra brevemente.</summary>
    public void IdentifyPlayer(uint deviceId, int playerIndex) => _commands.Enqueue(() =>
    {
        if (!_devices.TryGetValue(deviceId, out var d))
            return;
        var (r, g, b) = PlayerColors[Math.Clamp(playerIndex, 0, PlayerColors.Length - 1)];
        SDL_SetJoystickLED(d.Joystick, r, g, b);
        SDL_SetJoystickPlayerIndex(d.Joystick, playerIndex);
        SDL_RumbleJoystick(d.Joystick, 0x4000, 0x8000, 250);
    });

    private readonly Dictionary<uint, Device> _devices = [];

    private sealed class Device(PadInfo info, SDL_Joystick* joystick, SDL_Gamepad* gamepad)
    {
        public PadInfo Info { get; } = info;
        public SDL_Joystick* Joystick { get; } = joystick;
        public SDL_Gamepad* Gamepad { get; } = gamepad;
        public float[] Axes { get; } = new float[Math.Max(0, info.Axes)];
        public bool[] Buttons { get; } = new bool[Math.Max(0, info.Buttons)];
        public BatteryInfo Battery { get; set; } = BatteryInfo.Unknown;
    }

    private void Run()
    {
        try
        {
            // Imprescindible: los juegos tendrán el foco, no SplitForge.
            SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1"u8);
            if (!SDL_Init(SDL_InitFlags.SDL_INIT_JOYSTICK | SDL_InitFlags.SDL_INIT_GAMEPAD))
            {
                Error = "SDL3 no pudo iniciar los mandos: " + SDL_GetError();
                Backend = "SDL3 (error)";
                _log(Error);
                return;
            }
            var v = SDL_GetVersion();
            Backend = $"SDL3 {v / 1_000_000}.{v / 1000 % 1000}.{v % 1000}";
            _log($"Entrada: {Backend}, hilo dedicado a {_rateHz} Hz, eventos en segundo plano activados.");
            Loop();
        }
        catch (Exception ex) when (ex is DllNotFoundException or EntryPointNotFoundException or BadImageFormatException)
        {
            Error = "No se encontró SDL3 nativo: " + ex.Message;
            Backend = "SDL3 (no disponible)";
            _log(Error);
        }
        finally
        {
            foreach (var d in _devices.Values)
                CloseDevice(d);
            _devices.Clear();
            if (Error is null)
                SDL_Quit();
        }
    }

    private void Loop()
    {
        using (var joysticks = SDL_GetJoysticks())
        {
            if (joysticks is not null)
            {
                for (var i = 0; i < joysticks.Count; i++)
                    Open(joysticks[i]);
            }
        }
        var period = 1_000_000_000UL / (ulong)_rateHz;
        var changed = true;
        long sequence = 0, loops = 0;
        var rateWindow = Stopwatch.StartNew();
        double measured = 0;
        // Plazos absolutos: si una espera se pasa, la siguiente es más corta y la frecuencia media se mantiene.
        var next = SDL_GetTicksNS();
        SDL_Event e;
        while (!_stop)
        {
            while (_commands.TryDequeue(out var command))
                command();
            while (SDL_PollEvent(&e))
            {
                switch (e.Type)
                {
                    case SDL_EventType.SDL_EVENT_JOYSTICK_ADDED:
                        changed |= Open(e.jdevice.which);
                        break;
                    case SDL_EventType.SDL_EVENT_JOYSTICK_REMOVED:
                        changed |= Close(e.jdevice.which);
                        break;
                }
            }
            changed |= Sample();
            loops++;
            if (rateWindow.ElapsedMilliseconds >= 1000)
            {
                measured = loops * 1000.0 / rateWindow.ElapsedMilliseconds;
                loops = 0;
                rateWindow.Restart();
                PollBatteries(); // La carga cambia despacio: basta con leerla una vez por segundo.
                changed = true; // Refresca la frecuencia medida al menos una vez por segundo.
            }
            if (changed)
            {
                Publish(++sequence, measured);
                changed = false;
            }
            next += period;
            var now = SDL_GetTicksNS();
            if (next > now)
                SDL_DelayNS(next - now); // Temporizador de alta resolución de SDL3, sin girar la CPU.
            else if (now - next > 5 * period)
                next = now; // Muy atrasado (equipo cargado): resincronizar sin ráfagas.
        }
    }

    private bool Open(SDL_JoystickID id)
    {
        if (_devices.ContainsKey((uint)id))
            return false;
        var joystick = SDL_OpenJoystick(id);
        if (joystick is null)
        {
            _log($"No se pudo abrir el mando {(uint)id}: {SDL_GetError()}");
            return false;
        }
        var gamepad = SDL_IsGamepad(id) ? SDL_OpenGamepad(id) : null;
        var guidBuffer = stackalloc byte[33];
        SDL_GUIDToString(SDL_GetJoystickGUID(joystick), guidBuffer, 33);
        var guid = new string((sbyte*)guidBuffer);
        var type = gamepad is not null ? SDL_GetGamepadStringForType(SDL_GetGamepadTypeForID(id)) ?? string.Empty : string.Empty;
        var info = new PadInfo((uint)id, SDL_GetJoystickName(joystick) ?? "Mando", guid,
            gamepad is not null ? PadKind.StandardGamepad : PadKind.GenericHid,
            SDL_GetNumJoystickAxes(joystick), SDL_GetNumJoystickButtons(joystick), type);
        int percent;
        var device = new Device(info, joystick, gamepad) { Battery = new BatteryInfo(MapBattery(SDL_GetJoystickPowerInfo(joystick, &percent)), percent) };
        _devices[(uint)id] = device;
        _log($"Mando conectado: {info.Name} (ID {info.DeviceId}, {PadTypes.FriendlyLabel(type, info.Kind)}{(type.Length > 0 ? ", " + type : "")}).");
        return true;
    }

    private bool Close(SDL_JoystickID id)
    {
        if (!_devices.Remove((uint)id, out var d))
            return false;
        CloseDevice(d);
        _log($"Mando desconectado: {d.Info.Name} (ID {d.Info.DeviceId}). Reasigna si es necesario; no se intercambia en silencio.");
        return true;
    }

    private static void CloseDevice(Device d)
    {
        if (d.Gamepad is not null)
            SDL_CloseGamepad(d.Gamepad);
        SDL_CloseJoystick(d.Joystick);
    }

    /// <summary>Lee el estado crudo de todos los mandos; devuelve true si algo cambió.</summary>
    private bool Sample()
    {
        var changed = false;
        foreach (var d in _devices.Values)
        {
            for (var i = 0; i < d.Axes.Length; i++)
            {
                var value = AxisMath.FromSdl(SDL_GetJoystickAxis(d.Joystick, i));
                if (MathF.Abs(value - d.Axes[i]) > 0.0005f)
                {
                    d.Axes[i] = value;
                    changed = true;
                }
            }
            for (var i = 0; i < d.Buttons.Length; i++)
            {
                bool pressed = SDL_GetJoystickButton(d.Joystick, i);
                if (pressed != d.Buttons[i])
                {
                    d.Buttons[i] = pressed;
                    changed = true;
                }
            }
        }
        return changed;
    }

    /// <summary>Lee la energía de cada mando (solo el hilo de SDL). percent = −1 cuando SDL no lo sabe.</summary>
    private void PollBatteries()
    {
        foreach (var d in _devices.Values)
        {
            int percent;
            var state = SDL_GetJoystickPowerInfo(d.Joystick, &percent);
            d.Battery = new BatteryInfo(MapBattery(state), percent);
        }
    }

    private static BatteryStatus MapBattery(SDL_PowerState state) => state switch
    {
        SDL_PowerState.SDL_POWERSTATE_ON_BATTERY => BatteryStatus.Discharging,
        SDL_PowerState.SDL_POWERSTATE_NO_BATTERY => BatteryStatus.Wired,
        SDL_PowerState.SDL_POWERSTATE_CHARGING => BatteryStatus.Charging,
        SDL_PowerState.SDL_POWERSTATE_CHARGED => BatteryStatus.Charged,
        SDL_PowerState.SDL_POWERSTATE_ERROR => BatteryStatus.Error,
        _ => BatteryStatus.Unknown,
    };

    private void Publish(long sequence, double measured)
    {
        var mode = _mode;
        var mappings = Volatile.Read(ref _mappings);
        var devices = new List<PadInfo>(_devices.Count);
        var states = ImmutableDictionary.CreateBuilder<uint, PadState>();
        var raw = ImmutableDictionary.CreateBuilder<uint, RawPadState>();
        var paths = ImmutableDictionary.CreateBuilder<uint, ReadPath>();
        var batteries = ImmutableDictionary.CreateBuilder<uint, BatteryInfo>();
        foreach (var d in _devices.Values.OrderBy(x => x.Info.DeviceId))
        {
            devices.Add(d.Info);
            batteries[d.Info.DeviceId] = d.Battery;
            raw[d.Info.DeviceId] = new RawPadState([.. d.Axes.Select(a => MathF.Round(a, 2))],
                [.. Enumerable.Range(0, d.Buttons.Length).Where(i => d.Buttons[i])]);
            var path = ReadPathPolicy.Choose(d.Info, d.Gamepad is not null, mode);
            paths[d.Info.DeviceId] = path;
            if (path == ReadPath.Gamepad)
            {
                states[d.Info.DeviceId] = PadInterpreter.FromGamepad(d.Info.DeviceId,
                    AxisMath.FromSdl(SDL_GetGamepadAxis(d.Gamepad, SDL_GamepadAxis.SDL_GAMEPAD_AXIS_LEFTX)),
                    AxisMath.FromSdl(SDL_GetGamepadAxis(d.Gamepad, SDL_GamepadAxis.SDL_GAMEPAD_AXIS_LEFTY)),
                    SDL_GetGamepadButton(d.Gamepad, SDL_GamepadButton.SDL_GAMEPAD_BUTTON_SOUTH),
                    SDL_GetGamepadButton(d.Gamepad, SDL_GamepadButton.SDL_GAMEPAD_BUTTON_EAST),
                    SDL_GetGamepadButton(d.Gamepad, SDL_GamepadButton.SDL_GAMEPAD_BUTTON_WEST));
            }
            else if (d.Axes.Length >= 2 && d.Buttons.Length >= 1)
            {
                var mapping = mappings.TryGetValue(d.Info.Guid, out var m) ? m : ControllerMapping.DefaultFor(d.Buttons.Length);
                try
                {
                    states[d.Info.DeviceId] = PadInterpreter.FromJoystick(d.Info.DeviceId, d.Axes, d.Buttons, mapping);
                }
                catch (ArgumentException)
                {
                    // Perfil inválido para este dispositivo: sin estado ("SIN LECTURA") hasta recalibrar.
                }
            }
        }
        Volatile.Write(ref _latest, new InputSnapshot(sequence, devices, states.ToImmutable(), raw.ToImmutable(), paths.ToImmutable(), measured, batteries.ToImmutable()));
    }

    public void Dispose()
    {
        _stop = true;
        if (Thread.CurrentThread != _thread)
            _thread.Join(TimeSpan.FromSeconds(1));
    }
}
