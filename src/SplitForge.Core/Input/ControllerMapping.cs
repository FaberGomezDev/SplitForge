namespace SplitForge.Core.Input;

/// <summary>
/// Perfil de un modelo de mando (por GUID). El índice físico de Cruz NO es universal:
/// se aprende desde la interfaz. R2 puede ser un eje analógico con reposo y dirección.
/// </summary>
public sealed record ControllerMapping
{
    public int AxisX { get; init; }
    public int AxisY { get; init; } = 1;
    public int FireButton { get; init; }
    public int? DashButton { get; init; } = 1;
    public int? ShieldButton { get; init; } = 2;
    public bool InvertX { get; init; }
    public bool InvertY { get; init; }
    public int? FireAxis { get; init; }
    public float FireRest { get; init; }
    public int FireDirection { get; init; } = 1;

    /// <summary>Perfil por defecto que nunca apunta a botones inexistentes.</summary>
    public static ControllerMapping DefaultFor(int buttons) => new()
    {
        DashButton = buttons > 1 ? 1 : null,
        ShieldButton = buttons > 2 ? 2 : null,
    };

    /// <summary>Rechaza índices fuera de rango antes de leer el dispositivo.</summary>
    public ControllerMapping Validate(int axes, int buttons)
    {
        if (AxisX < 0 || AxisX >= axes || AxisY < 0 || AxisY >= axes)
            throw new ArgumentException($"Los ejes deben estar entre 0 y {axes - 1}.");
        if (FireButton < 0 || FireButton >= buttons)
            throw new ArgumentException($"El botón debe estar entre 0 y {buttons - 1}.");
        foreach (var (name, value) in new[] { ("Impulso", DashButton), ("Escudo", ShieldButton) })
        {
            if (value is { } b && (b < 0 || b >= buttons))
                throw new ArgumentException($"{name}: botón fuera del rango 0..{buttons - 1}.");
        }
        if (FireAxis is { } axis)
        {
            if (axis < 0 || axis >= axes)
                throw new ArgumentException($"Eje de disparo fuera del rango 0..{axes - 1}.");
            if (FireRest is < -1f or > 1f || FireDirection is not (-1 or 1))
                throw new ArgumentException("Calibración del gatillo inválida.");
        }
        return this;
    }
}

/// <summary>Convierte lecturas crudas en <see cref="PadState"/>. Funciones puras, probadas sin hardware.</summary>
public static class PadInterpreter
{
    public static PadState FromJoystick(uint deviceId, IReadOnlyList<float> axes, IReadOnlyList<bool> buttons, ControllerMapping mapping)
    {
        var m = mapping.Validate(axes.Count, buttons.Count);
        var lx = AxisMath.Normalize(axes[m.AxisX]);
        var ly = -AxisMath.Normalize(axes[m.AxisY]);
        if (m.InvertX)
            lx = -lx;
        if (m.InvertY)
            ly = -ly;
        // Con R2 calibrado, el botón digital por defecto (quizá otro botón físico) no dispara.
        var fire = m.FireAxis is { } fa
            ? AxisMath.AnalogTriggerPressed(axes[fa], m.FireRest, m.FireDirection)
            : buttons[m.FireButton];
        var dash = m.DashButton is { } d && buttons[d];
        var shield = m.ShieldButton is { } s && buttons[s];
        return new PadState(deviceId, lx, ly, fire, true, dash, shield);
    }

    /// <summary>Ruta SDL Gamepad: ejes y botones estándar (Sur = Cruz, Este = Círculo, Oeste = Cuadrado).</summary>
    public static PadState FromGamepad(uint deviceId, float leftX, float leftY, bool south, bool east, bool west) =>
        new(deviceId, AxisMath.Normalize(leftX), -AxisMath.Normalize(leftY), south, true, east, west);
}
