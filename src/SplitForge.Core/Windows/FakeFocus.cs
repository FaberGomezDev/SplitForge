namespace SplitForge.Core.Windows;

/// <summary>Mensaje de ventana que se publica a cada cliente para que se crea activo.</summary>
public readonly record struct FocusMessage(uint Msg, nint WParam, nint LParam, string Name);

/// <summary>
/// Foco simulado desde fuera del juego (EXPERIMENTAL, ADR-020): la técnica <c>FakeFocus</c> de Nucleus Co-op
/// (<c>WindowFakeFocus.cs</c>) con los mismos mensajes que el <c>FocusLoop</c> de ProtoInput. SplitForge publica
/// periódicamente a la ventana de cada cliente los mensajes de activación, para que un juego que deja de
/// procesar su mando al perder el foco (Unity) lo siga procesando. No inyecta código ni toca archivos del juego.
/// SUPUESTO sin comprobar con 7DTD: que basten mensajes externos (Nucleus añade ganchos dentro del juego).
/// </summary>
public static class FakeFocusPlan
{
    public const uint WM_ACTIVATE = 0x0006;
    public const uint WM_SETFOCUS = 0x0007;
    public const uint WM_ACTIVATEAPP = 0x001C;
    public const uint WM_MOUSEACTIVATE = 0x0021;
    public const uint WM_NCACTIVATE = 0x0086;
    public const nint WA_CLICKACTIVE = 2;
    public const nint HTCLIENT = 1;

    /// <summary>Nucleus reenvía cada 1000 ms y ProtoInput cada 5 ms; 100 ms recupera el foco sin saturar la cola.</summary>
    public const int DefaultIntervalMs = 100;
    public const int MinIntervalMs = 5;
    public const int MaxIntervalMs = 2000;

    public static int ClampInterval(int ms) => Math.Clamp(ms, MinIntervalMs, MaxIntervalMs);

    /// <summary>Mensajes para <paramref name="hwnd"/>, en el orden del <c>FocusLoop</c> de ProtoInput.</summary>
    public static IReadOnlyList<FocusMessage> MessagesFor(nint hwnd) =>
    [
        new(WM_ACTIVATE, WA_CLICKACTIVE, 0, "WM_ACTIVATE"),
        new(WM_ACTIVATEAPP, 1, 0, "WM_ACTIVATEAPP"),
        new(WM_NCACTIVATE, 1, 0, "WM_NCACTIVATE"),
        new(WM_SETFOCUS, 0, 0, "WM_SETFOCUS"),
        new(WM_MOUSEACTIVATE, hwnd, HTCLIENT, "WM_MOUSEACTIVATE"),
    ];

    /// <summary>
    /// Ventanas a las que aplicar el foco simulado: solo con dos o más clientes colocados, sin repetidos.
    /// Se excluyen las de <paramref name="focusedInGame"/>: su juego ya simula el foco por dentro (filtro XInput)
    /// y los mensajes de fuera solo estorban. COMPROBADO en el código de Unity 2022 (7DTD): cada WM_ACTIVATE
    /// ejecuta <c>PlayerWindowActivated</c> y cada WM_NCACTIVATE vuelve a recolocar y encerrar el puntero.
    /// </summary>
    public static IReadOnlyList<nint> Targets(bool enabled, IReadOnlyCollection<nint>? sessionHandles,
        IReadOnlySet<nint>? focusedInGame = null)
    {
        if (!enabled || sessionHandles is null)
            return [];
        var distinct = sessionHandles.Where(h => h != 0).Distinct().ToArray();
        if (distinct.Length < 2)
            return [];
        return focusedInGame is { Count: > 0 } ? [.. distinct.Where(h => !focusedInGame.Contains(h))] : distinct;
    }
}
