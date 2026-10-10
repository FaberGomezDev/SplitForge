namespace SplitForge.Core.Layout;

/// <summary>Rectángulo en píxeles físicos de pantalla (el proceso es DPI-aware por monitor).</summary>
public readonly record struct Region(int X, int Y, int Width, int Height)
{
    public int Right => X + Width;
    public int Bottom => Y + Height;
}

public enum SplitOrientation
{
    /// <summary>Lado a lado (antes "vertical" en la versión Python).</summary>
    SideBySide,
    /// <summary>Arriba / abajo (antes "horizontal").</summary>
    Stacked,
}

/// <summary>Cálculos puros y verificables para pantalla dividida de 2 a 4 jugadores.</summary>
public static class SplitLayout
{
    public const int MinSize = 200;
    public const int MaxGutter = 64;

    public static IReadOnlyList<Region> Compute(Region area, int players, SplitOrientation orientation, int gutter)
    {
        if (area.Width < MinSize || area.Height < MinSize)
            throw new ArgumentException($"La pantalla debe tener al menos {MinSize} × {MinSize} píxeles.", nameof(area));
        if (players is < 2 or > 4)
            throw new ArgumentOutOfRangeException(nameof(players), "SplitForge distribuye entre 2 y 4 jugadores.");
        if (gutter is < 0 or > MaxGutter)
            throw new ArgumentOutOfRangeException(nameof(gutter), $"Separación fuera del rango 0–{MaxGutter}.");
        if (!Enum.IsDefined(orientation))
            throw new ArgumentOutOfRangeException(nameof(orientation), "Orientación no reconocida.");

        return players switch
        {
            2 => orientation == SplitOrientation.SideBySide ? Columns(area, gutter) : Rows(area, gutter),
            3 => Three(area, orientation, gutter),
            _ => Grid(area, gutter),
        };
    }

    /// <summary>Compatibilidad con la configuración de la versión Python ("vertical" / "horizontal").</summary>
    public static SplitOrientation ParseLegacy(string? value) => value switch
    {
        "horizontal" => SplitOrientation.Stacked,
        _ => SplitOrientation.SideBySide,
    };

    private static (int First, int Second) Split(int length, int gutter)
    {
        var first = (length - gutter) / 2;
        return (first, length - first - gutter);
    }

    private static Region[] Columns(Region a, int gutter)
    {
        var (left, right) = Split(a.Width, gutter);
        return [new(a.X, a.Y, left, a.Height), new(a.X + left + gutter, a.Y, right, a.Height)];
    }

    private static Region[] Rows(Region a, int gutter)
    {
        var (top, bottom) = Split(a.Height, gutter);
        return [new(a.X, a.Y, a.Width, top), new(a.X, a.Y + top + gutter, a.Width, bottom)];
    }

    private static Region[] Three(Region a, SplitOrientation orientation, int gutter)
    {
        if (orientation == SplitOrientation.SideBySide)
        {
            // J1 ocupa la columna izquierda; J2 y J3 se reparten la derecha.
            var columns = Columns(a, gutter);
            var right = Rows(columns[1], gutter);
            return [columns[0], right[0], right[1]];
        }
        // J1 ocupa la fila superior; J2 y J3 se reparten la inferior.
        var rows = Rows(a, gutter);
        var bottom = Columns(rows[1], gutter);
        return [rows[0], bottom[0], bottom[1]];
    }

    private static Region[] Grid(Region a, int gutter)
    {
        var rows = Rows(a, gutter);
        var top = Columns(rows[0], gutter);
        var bottom = Columns(rows[1], gutter);
        return [top[0], top[1], bottom[0], bottom[1]];
    }
}
