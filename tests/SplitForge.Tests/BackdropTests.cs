using SplitForge.Core.Settings;
using SplitForge.Core.Windows;

namespace SplitForge.Tests;

public class BackdropTests
{
    private const nint Fondo = 900;
    private static readonly nint[] Session = [10, 20];

    [Fact]
    public void BackdropGoesRightBelowTheLowestSessionWindow()
    {
        // Orden Z de arriba abajo: juego, otra ventana, juego, barra de tareas, fondo.
        Assert.Equal(20, BackdropPlan.InsertAfter([10, 77, 20, 500, Fondo], Session, Fondo));
        // El fondo por encima de un juego lo taparía: también se recoloca.
        Assert.Equal(20, BackdropPlan.InsertAfter([10, Fondo, 20, 500], Session, Fondo));
        // Aún sin insertar (recién creado, arriba del todo).
        Assert.Equal(10, BackdropPlan.InsertAfter([Fondo, 20, 10, 500], Session, Fondo));
    }

    [Fact]
    public void BackdropAlreadyInPlaceIsLeftAlone()
    {
        Assert.Null(BackdropPlan.InsertAfter([10, 20, Fondo, 500], Session, Fondo));
        Assert.Null(BackdropPlan.InsertAfter([20, 77, 10, Fondo], Session, Fondo));
    }

    [Fact]
    public void WithoutSessionWindowsThereIsNothingToFollow()
    {
        Assert.Null(BackdropPlan.InsertAfter([77, 500, Fondo], Session, Fondo));
        Assert.Null(BackdropPlan.InsertAfter([], Session, Fondo));
    }

    [Fact]
    public void BackdropIsOnByDefault() => Assert.True(new AppSettings().BlackBackdrop);
}
