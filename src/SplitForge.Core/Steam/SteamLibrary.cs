using System.Text.RegularExpressions;

namespace SplitForge.Core.Steam;

/// <summary>Juego instalado según su <c>appmanifest_*.acf</c>. Detectarlo NO implica compatibilidad.</summary>
public sealed record InstalledGame(
    string AppId,
    string Name,
    string InstallFolder,
    string LibraryRoot,
    string BuildId = "",
    string StateFlags = "")
{
    public const string DefaultStatus = "Sin adaptador verificado";
}

/// <summary>Lectura pasiva y sin escritura de bibliotecas Steam.</summary>
public static partial class SteamLibrary
{
    public const int MaxManifests = 3000;
    public const int MaxManifestChars = 200_000;

    [GeneratedRegex(@"^appmanifest_(\d+)\.acf$", RegexOptions.IgnoreCase | RegexOptions.CultureInvariant)]
    private static partial Regex ManifestName();

    [GeneratedRegex(@"^\d+$")]
    private static partial Regex Digits();

    /// <summary>Raíz de Steam más las bibliotecas secundarias declaradas en <c>libraryfolders.vdf</c>.</summary>
    public static IReadOnlyList<string> LibraryPaths(string steamRoot)
    {
        var libraries = new List<string> { steamRoot };
        // Steam mantiene el índice bajo steamapps; algunas versiones guardan copia en config/.
        string[] indexes =
        [
            Path.Combine(steamRoot, "steamapps", "libraryfolders.vdf"),
            Path.Combine(steamRoot, "config", "libraryfolders.vdf"),
        ];
        string? text = null;
        foreach (var file in indexes)
        {
            if (TryRead(file, KeyValuesParser.MaxLength, out text))
                break;
        }
        if (text is null)
            return libraries;

        var root = KeyValuesParser.Parse(text);
        var folders = root.Child("libraryfolders") ?? root;
        foreach (var entry in folders.Children)
        {
            // Moderno: "0" { "path" "D:\\SteamLibrary" }. Antiguo: "1" "D:\\SteamLibrary".
            var path = entry.Value is not null && Digits().IsMatch(entry.Key) ? entry.Value : entry["path"];
            if (string.IsNullOrWhiteSpace(path) || !Directory.Exists(path))
                continue;
            if (!libraries.Contains(path, StringComparer.OrdinalIgnoreCase))
                libraries.Add(path);
        }
        return libraries;
    }

    public static IReadOnlyList<InstalledGame> Scan(IEnumerable<string> steamRoots)
    {
        var games = new Dictionary<string, InstalledGame>(StringComparer.Ordinal);
        foreach (var root in steamRoots)
        {
            foreach (var library in LibraryPaths(root))
            {
                var steamapps = Path.Combine(library, "steamapps");
                IEnumerable<string> manifests;
                try
                {
                    manifests = Directory.EnumerateFiles(steamapps, "appmanifest_*.acf", new EnumerationOptions
                    {
                        MatchCasing = MatchCasing.CaseInsensitive,
                        IgnoreInaccessible = true,
                    }).Take(MaxManifests).ToList();
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                {
                    continue;
                }
                foreach (var file in manifests)
                {
                    var match = ManifestName().Match(Path.GetFileName(file));
                    if (!match.Success || !TryRead(file, MaxManifestChars, out var text))
                        continue;
                    var state = KeyValuesParser.Parse(text).Child("AppState");
                    var name = state?["name"];
                    var installDir = state?["installdir"];
                    if (string.IsNullOrWhiteSpace(name) || string.IsNullOrWhiteSpace(installDir))
                        continue;
                    var appId = match.Groups[1].Value;
                    // No inferimos compatibilidad a partir de un manifiesto instalado.
                    games[appId] = new InstalledGame(
                        appId, name, Path.Combine(steamapps, "common", installDir), library,
                        state?["buildid"] ?? string.Empty, state?["StateFlags"] ?? string.Empty);
                }
            }
        }
        return [.. games.Values.OrderBy(g => g.Name, StringComparer.OrdinalIgnoreCase)];
    }

    private static bool TryRead(string path, int maxChars, out string text)
    {
        try
        {
            using var reader = new StreamReader(path, detectEncodingFromByteOrderMarks: true);
            var buffer = new char[maxChars];
            var read = reader.ReadBlock(buffer, 0, buffer.Length);
            text = new string(buffer, 0, read);
            return true;
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            text = string.Empty;
            return false;
        }
    }
}
