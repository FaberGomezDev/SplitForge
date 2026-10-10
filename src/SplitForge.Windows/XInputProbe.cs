using System.Runtime.Versioning;
using SplitForge.Core.Input;

namespace SplitForge.Platform;

/// <summary>
/// Consulta de solo lectura de las 4 ranuras XInput de Windows, para saber qué ranura usa cada mando
/// antes de asignarlo a un cliente. Se llama desde un hilo de fondo, nunca desde el de la interfaz.
/// </summary>
[SupportedOSPlatform("windows")]
public static class XInputProbe
{
    private const uint ErrorSuccess = 0;

    public static IReadOnlyList<XInputSlotState> ReadAll()
    {
        var states = new XInputSlotState[XInputSlotDetector.Slots];
        try
        {
            for (var slot = 0; slot < states.Length; slot++)
            {
                states[slot] = Native.XInputGetState((uint)slot, out var s) == ErrorSuccess
                    ? new XInputSlotState(true, s.Buttons, s.LeftTrigger, s.RightTrigger)
                    : default;
            }
        }
        catch (Exception ex) when (ex is DllNotFoundException or EntryPointNotFoundException)
        {
            // Sin xinput1_4 (Windows anterior a 8): ninguna ranura disponible.
        }
        return states;
    }

    /// <summary>Ranuras conectadas, numeradas 1–4.</summary>
    public static IReadOnlyList<int> ConnectedSlots() => XInputSlotDetector.ConnectedForDisplay(ReadAll());

    /// <summary>Espera a que se pulse un botón o un gatillo en algún mando y devuelve su ranura (0–3), o null.</summary>
    public static int? WaitForPress(TimeSpan timeout, CancellationToken cancel = default)
    {
        var previous = ReadAll();
        var until = DateTime.UtcNow + timeout;
        while (DateTime.UtcNow < until && !cancel.IsCancellationRequested)
        {
            Thread.Sleep(8);
            var current = ReadAll();
            if (XInputSlotDetector.NewPress(previous, current) is { } slot)
                return slot;
            previous = current;
        }
        return null;
    }
}
