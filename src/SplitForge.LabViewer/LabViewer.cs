using SDL;
using SplitForge.Core.Ipc;
using SplitForge.Core.Lab;
using SplitForge.Core.Sessions;
using static SDL.SDL3;

namespace SplitForge.LabViewer;

/// <summary>
/// Visor de un jugador: recibe instantáneas binarias por stdin y dibuja con el renderizador de
/// SDL3 (Direct3D en Windows) sincronizado con el monitor. No abre mandos ni simula nada.
/// </summary>
public static unsafe class LabViewer
{
    private static readonly (byte R, byte G, byte B)[] PlayerColors = [(0x5C, 0xE0, 0xCA), (0xFF, 0xAC, 0x83)];
    private static readonly TimeSpan MaxAge = TimeSpan.FromSeconds(1);

    public static int Run(int player, Stream input, int maxFrames = 0)
    {
        if (player is not (1 or 2))
            throw new ArgumentOutOfRangeException(nameof(player));
        var reader = new FrameReader(input);
        if (!SDL_Init(SDL_InitFlags.SDL_INIT_VIDEO))
        {
            Console.Error.WriteLine("SDL3 no pudo iniciar el vídeo: " + SDL_GetError());
            return 3;
        }
        SDL_Window* window = null;
        SDL_Renderer* renderer = null;
        try
        {
            window = SDL_CreateWindow(SessionManager.LabWindowTitle(player), 660, 450,
                SDL_WindowFlags.SDL_WINDOW_RESIZABLE | SDL_WindowFlags.SDL_WINDOW_HIGH_PIXEL_DENSITY);
            if (window is null)
                return Fail("No se pudo crear la ventana");
            renderer = SDL_CreateRenderer(window, (byte*)null);
            if (renderer is null)
                return Fail("No se pudo crear el renderizador");
            SDL_SetRenderVSync(renderer, 1);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BlendMode.SDL_BLENDMODE_BLEND);

            var frames = 0;
            var running = true;
            SDL_Event e;
            while (running)
            {
                while (SDL_PollEvent(&e))
                {
                    if (e.Type is SDL_EventType.SDL_EVENT_QUIT or SDL_EventType.SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                        running = false;
                }
                Draw(renderer, player, reader.Latest(MaxAge));
                SDL_RenderPresent(renderer);
                if (maxFrames > 0 && ++frames >= maxFrames)
                    break;
            }
            return 0;
        }
        finally
        {
            if (renderer is not null)
                SDL_DestroyRenderer(renderer);
            if (window is not null)
                SDL_DestroyWindow(window);
            SDL_Quit();
        }
    }

    private static int Fail(string what)
    {
        Console.Error.WriteLine($"{what}: {SDL_GetError()}");
        return 3;
    }

    private static void Draw(SDL_Renderer* r, int player, WorldSnapshot? s)
    {
        int w, h;
        SDL_GetCurrentRenderOutputSize(r, &w, &h);
        w = Math.Max(250, w);
        h = Math.Max(180, h);
        Color(r, 0x10, 0x21, 0x38);
        SDL_RenderClear(r);
        Color(r, 0x1D, 0x35, 0x50);
        for (var x = 0; x < w; x += Math.Max(25, w / 14))
            SDL_RenderLine(r, x, 0, x, h);
        for (var y = 0; y < h; y += Math.Max(25, h / 8))
            SDL_RenderLine(r, 0, y, w, y);

        if (s is null)
        {
            Text(r, w / 2f - 160, h / 2f, "Esperando sincronizacion de SplitForge...", 2, 0xDC, 0xEA, 0xF5);
            return;
        }

        var pulse = 10 + 3 * MathF.Sin(Environment.TickCount64 / 200f);
        FillCircle(r, (float)s.OrbX * w, (float)s.OrbY * h, pulse, 0xEA, 0xC1, 0x5E);
        foreach (var shot in s.Shots)
        {
            var (cr, cg, cb) = PlayerColors[shot.Owner - 1];
            FillCircle(r, (float)shot.X * w, (float)shot.Y * h, 6, cr, cg, cb);
        }
        for (var number = 1; number <= 2; number++)
        {
            var a = s.Actor(number);
            float x = (float)a.X * w, y = (float)a.Y * h;
            var (cr, cg, cb) = PlayerColors[number - 1];
            if (number == player)
            {
                RingCircle(r, x, y, 26, 2, 0xFF, 0xFF, 0xFF);
                Text(r, x - 8, y + 32, "TU", 1, 0xFF, 0xFF, 0xFF);
            }
            if (a.Shield)
                RingCircle(r, x, y, 39, 4, 0x83, 0xBD, 0xFF);
            FillCircle(r, x, y, 20, cr, cg, cb);
            Text(r, x - 4, y - 4, number.ToString(System.Globalization.CultureInfo.InvariantCulture), 1, 0x0B, 0x19, 0x30);
            if (!a.Connected)
                Text(r, x - 36, y - 36, "SIN MANDO", 1, 0xFF, 0xD2, 0xC9);
        }

        Color(r, 0x09, 0x19, 0x2E);
        var top = new SDL_FRect { x = 0, y = 0, w = w, h = 64 };
        SDL_RenderFillRect(r, &top);
        var title = $"MUNDO COMPARTIDO  -  TU PANTALLA: JUGADOR {player}";
        Text(r, 12, 10, title, FitScale(title, w - 24, 2), 0xFF, 0xFF, 0xFF);
        Text(r, 12, 40, $"P1 nucleos: {s.Player1.Score}  impactos: {s.Player1.Hits}   |   P2 nucleos: {s.Player2.Score}  impactos: {s.Player2.Hits}",
            1, 0xBF, 0xCB, 0xDB);
        Color(r, 0x09, 0x19, 0x2E);
        var bottom = new SDL_FRect { x = 0, y = h - 36, w = w, h = 36 };
        SDL_RenderFillRect(r, &bottom);
        Text(r, 12, h - 22, Ascii(s.Event), 1, 0xB5, 0xD7, 0xE4);
    }

    /// <summary>La fuente de SDL mide 8 px por carácter: reduce la escala para que el texto quepa.</summary>
    private static float FitScale(string text, float width, float preferred) =>
        Math.Clamp(width / (8f * Math.Max(1, text.Length)), 1f, preferred);

    private static void Color(SDL_Renderer* r, byte red, byte green, byte blue, byte alpha = 255) =>
        SDL_SetRenderDrawColor(r, red, green, blue, alpha);

    /// <summary>La fuente de depuración de SDL es ASCII: se quitan las tildes del texto del mundo.</summary>
    private static string Ascii(string text)
    {
        var normalized = text.Normalize(System.Text.NormalizationForm.FormD);
        return new string([.. normalized.Where(c => c < 128)]);
    }

    private static void Text(SDL_Renderer* r, float x, float y, string text, float scale, byte red, byte green, byte blue)
    {
        Color(r, red, green, blue);
        SDL_SetRenderScale(r, scale, scale);
        SDL_RenderDebugText(r, x / scale, y / scale, Ascii(text));
        SDL_SetRenderScale(r, 1, 1);
    }

    private const int Segments = 28;

    private static void FillCircle(SDL_Renderer* r, float cx, float cy, float radius, byte red, byte green, byte blue)
    {
        var color = new SDL_FColor { r = red / 255f, g = green / 255f, b = blue / 255f, a = 1 };
        var vertices = stackalloc SDL_Vertex[Segments + 1];
        var indices = stackalloc int[Segments * 3];
        vertices[0] = new SDL_Vertex { position = new SDL_FPoint { x = cx, y = cy }, color = color };
        for (var i = 0; i < Segments; i++)
        {
            var angle = i * MathF.Tau / Segments;
            vertices[i + 1] = new SDL_Vertex
            {
                position = new SDL_FPoint { x = cx + radius * MathF.Cos(angle), y = cy + radius * MathF.Sin(angle) },
                color = color,
            };
            indices[i * 3] = 0;
            indices[i * 3 + 1] = i + 1;
            indices[i * 3 + 2] = i + 1 == Segments ? 1 : i + 2;
        }
        SDL_RenderGeometry(r, null, vertices, Segments + 1, indices, Segments * 3);
    }

    private static void RingCircle(SDL_Renderer* r, float cx, float cy, float radius, int thickness, byte red, byte green, byte blue)
    {
        Color(r, red, green, blue);
        var points = stackalloc SDL_FPoint[Segments + 1];
        for (var t = 0; t < thickness; t++)
        {
            var rr = radius - t;
            for (var i = 0; i <= Segments; i++)
            {
                var angle = i * MathF.Tau / Segments;
                points[i] = new SDL_FPoint { x = cx + rr * MathF.Cos(angle), y = cy + rr * MathF.Sin(angle) };
            }
            SDL_RenderLines(r, points, Segments + 1);
        }
    }
}
