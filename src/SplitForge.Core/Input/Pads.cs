namespace SplitForge.Core.Input;

public enum PadKind
{
    /// <summary>SDL conoce su mapeo estándar (PS4, PS5, Xbox...).</summary>
    StandardGamepad,
    /// <summary>Joystick HID sin mapeo SDL: se usa un perfil editable.</summary>
    GenericHid,
}

/// <summary>Dispositivo detectado. <see cref="DeviceId"/> es estable mientras siga conectado.</summary>
public sealed record PadInfo(uint DeviceId, string Name, string Guid, PadKind Kind, int Axes, int Buttons, string GamepadType = "")
{
    public string KindLabel => Kind == PadKind.StandardGamepad ? "SDL estándar" : "HID genérico";
}

/// <summary>Estado de un mando ya interpretado. Y positiva = arriba, como XInput.</summary>
public readonly record struct PadState(
    uint DeviceId,
    float LX = 0f,
    float LY = 0f,
    bool Fire = false,
    bool Connected = true,
    bool Dash = false,
    bool Shield = false);

/// <summary>Lectura cruda para calibrar y diagnosticar (no se exporta en informes).</summary>
public sealed record RawPadState(IReadOnlyList<float> Axes, IReadOnlyList<int> PressedButtons);

public static class AxisMath
{
    public const float DefaultDeadzone = 0.14f;

    /// <summary>Zona muerta por eje y reescalado, igual que la versión validada con los PS4.</summary>
    public static float Normalize(float value, float deadzone = DefaultDeadzone)
    {
        value = Math.Clamp(value, -1f, 1f);
        if (MathF.Abs(value) <= deadzone)
            return 0f;
        var scaled = (MathF.Abs(value) - deadzone) / (1f - deadzone);
        return MathF.Round(scaled * MathF.Sign(value), 4);
    }

    /// <summary>Convierte un eje SDL (−32768..32767) a −1..1.</summary>
    public static float FromSdl(short raw) => Math.Clamp(raw / 32767f, -1f, 1f);

    /// <summary>Gatillo calibrado contra su reposo, no contra un índice fijo.</summary>
    public static bool AnalogTriggerPressed(float value, float rest, int direction, float threshold = 0.60f) =>
        (value - rest) * direction >= threshold;

    /// <summary>Solo flancos de subida: útil para aprender botones sin falsos positivos.</summary>
    public static IReadOnlyList<int> NewPresses(IReadOnlyCollection<int> current, IReadOnlyCollection<int> previous) =>
        [.. current.Except(previous).Order()];
}

public enum InputReadMode
{
    Auto,
    Joystick,
    Gamepad,
}

public enum ReadPath
{
    Joystick,
    Gamepad,
}

/// <summary>
/// Elige el lector efectivo. ADR-002: los PS4/DualShock usan el joystick directo en modo auto,
/// porque la ruta normalizada enumeraba los mandos sin entregar movimiento en el prototipo.
/// Detección ≠ lectura ≠ asignación.
/// </summary>
public static class ReadPathPolicy
{
    private static readonly string[] PlayStationTokens = ["ps4", "dualshock", "wireless controller", "playstation"];

    public static ReadPath Choose(PadInfo info, bool hasStandardGamepad, InputReadMode mode) => mode switch
    {
        InputReadMode.Joystick => ReadPath.Joystick,
        InputReadMode.Gamepad => hasStandardGamepad ? ReadPath.Gamepad : ReadPath.Joystick,
        _ when IsPlayStation(info.Name) => ReadPath.Joystick,
        _ => hasStandardGamepad ? ReadPath.Gamepad : ReadPath.Joystick,
    };

    public static bool IsPlayStation(string name)
    {
        var lower = name.ToLowerInvariant();
        return PlayStationTokens.Any(lower.Contains);
    }

    public static InputReadMode ParseLegacy(string? value) => value switch
    {
        "joystick" => InputReadMode.Joystick,
        "controller" or "gamepad" => InputReadMode.Gamepad,
        _ => InputReadMode.Auto,
    };
}
