namespace SplitForge.Core.Input;

/// <summary>
/// Nombre legible del tipo de mando a partir de la cadena de SDL3 (<c>SDL_GetGamepadStringForType</c>)
/// o, si SDL no lo reconoce como gamepad, de que sea un HID genérico. Función pura.
/// </summary>
public static class PadTypes
{
    public static string FriendlyLabel(string sdlGamepadType, PadKind kind)
    {
        var type = sdlGamepadType.Trim().ToLowerInvariant();
        return type switch
        {
            "xbox360" => "Xbox 360",
            "xboxone" => "Xbox One / Series",
            "ps3" => "PlayStation 3",
            "ps4" => "PlayStation 4 (DualShock 4)",
            "ps5" => "PlayStation 5 (DualSense)",
            "switchpro" => "Nintendo Switch Pro",
            "joyconleft" or "joyconright" or "joyconpair" => "Nintendo Joy-Con",
            "gamecube" => "GameCube",
            "steam" => "Steam Controller",
            "standard" => "Mando estándar (XInput)",
            _ when kind == PadKind.StandardGamepad => "Mando estándar (XInput)",
            _ => "Genérico (HID)",
        };
    }
}
