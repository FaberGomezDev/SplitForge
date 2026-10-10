namespace SplitForge.Core.Input;

/// <summary>Lectura mínima de una ranura XInput (botones y gatillos) para identificar mandos.</summary>
public readonly record struct XInputSlotState(bool Connected, ushort Buttons = 0, byte LeftTrigger = 0, byte RightTrigger = 0);

/// <summary>
/// Averigua en qué ranura XInput (0–3) está el mando que el jugador acaba de pulsar. Lógica pura:
/// la lectura de XInput la hace la capa de Windows en un hilo propio, nunca en el de la interfaz.
/// </summary>
public static class XInputSlotDetector
{
    public const int Slots = 4;
    public const byte TriggerThreshold = 128;

    /// <summary>Primera ranura con un botón recién pulsado o un gatillo que cruza el umbral; null si ninguna.</summary>
    public static int? NewPress(IReadOnlyList<XInputSlotState> previous, IReadOnlyList<XInputSlotState> current)
    {
        for (var slot = 0; slot < Math.Min(previous.Count, current.Count); slot++)
        {
            var before = previous[slot];
            var now = current[slot];
            // Un mando recién conectado (o desconectado) no cuenta como pulsación.
            if (!now.Connected || !before.Connected)
                continue;
            if ((now.Buttons & ~before.Buttons) != 0
                || Crossed(before.LeftTrigger, now.LeftTrigger)
                || Crossed(before.RightTrigger, now.RightTrigger))
                return slot;
        }
        return null;
    }

    /// <summary>Ranuras conectadas en formato 1–4 para mostrar al usuario.</summary>
    public static IReadOnlyList<int> ConnectedForDisplay(IReadOnlyList<XInputSlotState> states) =>
        [.. states.Select((s, i) => (s, i)).Where(x => x.s.Connected).Select(x => x.i + 1)];

    private static bool Crossed(byte before, byte now) => before < TriggerThreshold && now >= TriggerThreshold;
}
