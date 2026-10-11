using SplitForge.Core.Windows;

namespace SplitForge.Tests;

public class TaskbarTests
{
    private static readonly nint[] Session = [10, 20];
    private static readonly nint[] Taskbars = [99];

    [Fact]
    public void NothingToDoWhileTheTaskbarIsBelowEveryGame()
    {
        Assert.Empty(TaskbarPlan.ToRaise([10, 20, 5, 99, 7], Session, Taskbars));
        Assert.Empty(TaskbarPlan.ToRaise([3, 20, 10, 99], Session, Taskbars));
    }

    [Fact]
    public void GamesAreRaisedLowestFirstWhenTheTaskbarIsAboveAnyOfThem()
    {
        // Windows subió la barra al cambiar la ventana activa: queda encima de los dos juegos.
        Assert.Equal([(nint)20, (nint)10], TaskbarPlan.ToRaise([99, 10, 5, 20], Session, Taskbars));
        // O solo encima de uno (el otro se activó después): se suben los dos, conservando su orden entre sí.
        Assert.Equal([(nint)10, (nint)20], TaskbarPlan.ToRaise([20, 99, 10], Session, Taskbars));
    }

    [Fact]
    public void WithoutTaskbarOrWithoutGamesThereIsNothingToRaise()
    {
        Assert.Empty(TaskbarPlan.ToRaise([10, 20, 5], Session, Taskbars)); // barra oculta o inexistente
        Assert.Empty(TaskbarPlan.ToRaise([99, 5, 7], Session, Taskbars));  // los juegos ya se cerraron
        Assert.Empty(TaskbarPlan.ToRaise([], Session, Taskbars));
    }
}
