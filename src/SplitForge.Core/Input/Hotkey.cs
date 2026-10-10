namespace SplitForge.Core.Input;

/// <summary>Modificadores de un atajo global. Los valores coinciden con los <c>MOD_*</c> de Win32.</summary>
[Flags]
public enum HotkeyModifiers : uint
{
    None = 0,
    Alt = 0x0001,
    Control = 0x0002,
    Shift = 0x0004,
    Win = 0x0008,
}

/// <summary>
/// Atajo de teclado global (combinación + tecla virtual). Tipo puro: la interfaz lo registra con Win32.
/// </summary>
public sealed record HotkeySpec(HotkeyModifiers Modifiers, uint VirtualKey, string KeyLabel)
{
    /// <summary>Atajo por defecto del bloqueo de ratón: Ctrl+Alt+L (VK_L = 0x4C).</summary>
    public static readonly HotkeySpec LockDefault = new(HotkeyModifiers.Control | HotkeyModifiers.Alt, 0x4C, "L");

    /// <summary>Texto para la interfaz, p. ej. «Ctrl+Alt+L».</summary>
    public string Label
    {
        get
        {
            var parts = new List<string>(4);
            if (Modifiers.HasFlag(HotkeyModifiers.Control)) parts.Add("Ctrl");
            if (Modifiers.HasFlag(HotkeyModifiers.Alt)) parts.Add("Alt");
            if (Modifiers.HasFlag(HotkeyModifiers.Shift)) parts.Add("Shift");
            if (Modifiers.HasFlag(HotkeyModifiers.Win)) parts.Add("Win");
            parts.Add(KeyLabel);
            return string.Join("+", parts);
        }
    }
}
