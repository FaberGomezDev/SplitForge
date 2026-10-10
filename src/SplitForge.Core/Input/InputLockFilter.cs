namespace SplitForge.Core.Input;

/// <summary>Qué hacer con una tecla mientras el bloqueo de entrada está activo.</summary>
public enum InputLockDecision
{
    /// <summary>Dejarla pasar (teclas soltadas: así ningún modificador se queda «pegado»).</summary>
    Pass,
    /// <summary>Descartarla: no llega ni a los juegos ni al escritorio.</summary>
    Swallow,
    /// <summary>Es el atajo de desbloqueo: descartarla y soltar el bloqueo.</summary>
    Toggle,
}

/// <summary>
/// Decisión del bloqueo de entrada «solo mandos» (lógica pura; el gancho de Windows la aplica).
/// Mientras está activo, el teclado y el ratón no llegan a nada salvo el atajo que lo desbloquea; los
/// mandos (XInput) no pasan por aquí y siguen funcionando. Sigue el estado de los modificadores por
/// sí mismo, porque las pulsaciones descartadas no actualizan el estado de teclado de Windows.
/// </summary>
public sealed class InputLockFilter(HotkeySpec toggle)
{
    private HotkeyModifiers _held;

    public HotkeySpec Toggle { get; } = toggle;

    /// <summary>Modificadores que ya estaban pulsados al activar el bloqueo (p. ej. los del propio atajo).</summary>
    public void Reset(HotkeyModifiers heldNow) => _held = heldNow;

    public InputLockDecision OnKey(uint virtualKey, bool down)
    {
        var modifier = ModifierOf(virtualKey);
        if (modifier != HotkeyModifiers.None)
        {
            _held = down ? _held | modifier : _held & ~modifier;
            return down ? InputLockDecision.Swallow : InputLockDecision.Pass;
        }
        if (!down)
            return InputLockDecision.Pass;
        return virtualKey == Toggle.VirtualKey && _held == Toggle.Modifiers ? InputLockDecision.Toggle : InputLockDecision.Swallow;
    }

    /// <summary>Teclas virtuales de Win32 de cada modificador (genéricas e izquierda/derecha).</summary>
    public static HotkeyModifiers ModifierOf(uint virtualKey) => virtualKey switch
    {
        0x11 or 0xA2 or 0xA3 => HotkeyModifiers.Control, // VK_CONTROL, VK_LCONTROL, VK_RCONTROL
        0x12 or 0xA4 or 0xA5 => HotkeyModifiers.Alt,     // VK_MENU, VK_LMENU, VK_RMENU
        0x10 or 0xA0 or 0xA1 => HotkeyModifiers.Shift,   // VK_SHIFT, VK_LSHIFT, VK_RSHIFT
        0x5B or 0x5C => HotkeyModifiers.Win,             // VK_LWIN, VK_RWIN
        _ => HotkeyModifiers.None,
    };
}
