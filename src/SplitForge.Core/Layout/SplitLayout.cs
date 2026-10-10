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

/// <summary>Cómo encaja la ventana del juego dentro de su región de pantalla dividida.</summary>
public enum WindowFit
{
    /// <summary>La ventana ocupa toda la región (el juego debe estar en modo ventana a esa resolución).</summary>
    Rellenar,
    /// <summary>Se centra el mayor rectángulo con la proporción elegida dentro de la región (sin deformar).</summary>
    MantenerProporcion,
}

/// <summary>Proporción de imagen (p. ej. 16:9). <see cref="Value"/> es ancho/alto.</summary>
public readonly record struct AspectRatio(int W, int H)
{
    public static readonly AspectRatio SixteenNine = new(16, 9);
    public static readonly IReadOnlyList<AspectRatio> Common = [new(16, 9), new(16, 10), new(21, 9), new(4, 3)];

    public double Value => H > 0 ? (double)W / H : 0;
    public string Label => $"{W}:{H}";
    public bool IsValid => W > 0 && H > 0;
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

    /// <summary>
    /// Mayor rectángulo con la proporción <paramref name="aspect"/> centrado dentro de <paramref name="region"/>.
    /// Así la ventana del juego conserva su forma (sin estirarse) aunque la región no tenga esa proporción.
    /// </summary>
    public static Region Fit(Region region, AspectRatio aspect)
    {
        if (!aspect.IsValid || region.Width <= 0 || region.Height <= 0)
            return region;
        var target = aspect.Value;
        var regionAspect = (double)region.Width / region.Height;
        int width, height;
        if (regionAspect > target)
        {
            height = region.Height;
            width = (int)Math.Round(height * target);
        }
        else
        {
            width = region.Width;
            height = (int)Math.Round(width / target);
        }
        width = Math.Clamp(width, 1, region.Width);
        height = Math.Clamp(height, 1, region.Height);
        var x = region.X + (region.Width - width) / 2;
        var y = region.Y + (region.Height - height) / 2;
        return new Region(x, y, width, height);
    }

    /// <summary>Aplica el modo de ajuste a cada región ya calculada.</summary>
    public static IReadOnlyList<Region> ApplyFit(IReadOnlyList<Region> regions, WindowFit fit, AspectRatio aspect) =>
        fit == WindowFit.MantenerProporcion && aspect.IsValid ? [.. regions.Select(r => Fit(r, aspect))] : regions;

    /// <summary>
    /// Reduce a <paramref name="minimum"/> (centrada) cada región más estrecha que esa proporción; las
    /// regiones iguales o más anchas no cambian. Para juegos cuya interfaz se corta por debajo de 16:9.
    /// </summary>
    public static IReadOnlyList<Region> EnsureMinAspect(IReadOnlyList<Region> regions, AspectRatio minimum) =>
        minimum.IsValid
            ? [.. regions.Select(r => r.Height > 0 && (double)r.Width / r.Height < minimum.Value - 1e-6 ? Fit(r, minimum) : r)]
            : regions;

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
