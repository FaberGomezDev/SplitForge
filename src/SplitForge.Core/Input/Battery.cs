namespace SplitForge.Core.Input;

/// <summary>Estado de energía de un mando, derivado de <c>SDL_GetJoystickPowerInfo</c>.</summary>
public enum BatteryStatus
{
    /// <summary>SDL no sabe el estado (algunos mandos por Bluetooth no lo informan).</summary>
    Unknown,
    /// <summary>Sin batería: alimentado por cable o receptor.</summary>
    Wired,
    /// <summary>Con batería, descargándose.</summary>
    Discharging,
    /// <summary>Cargando por cable.</summary>
    Charging,
    /// <summary>Batería llena y conectado.</summary>
    Charged,
    /// <summary>SDL informó un error al leer la energía.</summary>
    Error,
}

/// <summary>Carga de un mando. <see cref="Percent"/> es −1 cuando no se conoce.</summary>
public readonly record struct BatteryInfo(BatteryStatus Status, int Percent = -1)
{
    public static readonly BatteryInfo Unknown = new(BatteryStatus.Unknown, -1);

    private string PercentText => Percent >= 0
        ? Percent.ToString(System.Globalization.CultureInfo.InvariantCulture) + "%"
        : "?";

    /// <summary>Texto para la interfaz, p. ej. «Batería 80%», «Cargando 45%», «Cable».</summary>
    public string Label => Status switch
    {
        BatteryStatus.Wired => "Cable",
        BatteryStatus.Charging => $"Cargando {PercentText}",
        BatteryStatus.Charged => "Cargado",
        BatteryStatus.Discharging => $"Batería {PercentText}",
        BatteryStatus.Error => "Error",
        _ => Percent >= 0 ? $"Batería {PercentText}" : "—",
    };

    /// <summary>true si conviene avisar al jugador (batería propia por debajo del umbral).</summary>
    public bool IsLow(int threshold = 20) => Status == BatteryStatus.Discharging && Percent >= 0 && Percent <= threshold;
}
