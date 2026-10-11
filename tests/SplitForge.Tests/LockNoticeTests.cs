using SplitForge.Core.Layout;
using SplitForge.Core.Windows;

namespace SplitForge.Tests;

public class LockNoticeTests
{
    [Fact]
    public void TextSaysTheStateAndHowToChangeIt()
    {
        var locked = LockNoticePlan.Text(true, "Ctrl+Alt+B");
        Assert.Equal("TECLADO Y RATÓN BLOQUEADOS", locked.Title);
        Assert.Contains("Ctrl+Alt+B para desbloquear", locked.Detail);
        Assert.Equal("Bloqueado · Ctrl+Alt+B", locked.Badge); // queda fija mientras dure el bloqueo

        var unlocked = LockNoticePlan.Text(false, "Ctrl+Alt+B");
        Assert.Equal("TECLADO Y RATÓN DESBLOQUEADOS", unlocked.Title);
        Assert.Contains("Ctrl+Alt+B para volver a bloquear", unlocked.Detail);
        Assert.Equal(string.Empty, unlocked.Badge); // desbloqueado: el aviso desaparece
    }

    [Fact]
    public void NoticeIsCenteredAtTheTopOfTheSessionArea()
    {
        // Monitor secundario a la derecha: se centra en SU zona, no en la pantalla principal.
        var area = new Region(1920, 0, 2560, 1440);
        Assert.Equal((1920 + (2560 - 600) / 2, LockNoticePlan.MarginPx), LockNoticePlan.TopCenter(area, 600, 90));
        // Área útil que no empieza en 0 (barra de tareas arriba).
        Assert.Equal((660, 48 + LockNoticePlan.MarginPx), LockNoticePlan.TopCenter(new Region(0, 48, 1920, 1032), 600, 90));
    }

    [Fact]
    public void NoticeNeverLeavesASmallArea()
    {
        // Más ancho o más alto que la zona: se pega a su esquina en vez de salirse por la izquierda o por abajo.
        Assert.Equal((100, 50), LockNoticePlan.TopCenter(new Region(100, 50, 300, 60), 600, 90));
        Assert.Equal((100, 50 + 10), LockNoticePlan.TopCenter(new Region(100, 50, 300, 100), 300, 90));
    }
}
