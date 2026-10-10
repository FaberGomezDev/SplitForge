using SplitForge.Core.Steam;

namespace SplitForge.Core.Adapters;

/// <summary>Estados formales por juego (docs/05). Nunca se infiere COMPATIBLE de una detección.</summary>
public enum CompatibilityState
{
    Descubierto,
    Inspeccionado,
    DosClientesValidos,
    SesionComunValidada,
    InputAisladoValidado,
    RendimientoValidado,
    Compatible,
    Bloqueado,
}

public enum CheckStatus
{
    Ok,
    Warning,
    Failed,
    /// <summary>Requiere comprobación manual o un componente aún no implementado.</summary>
    Pending,
}

public sealed record PreflightCheck(string Id, string Title, CheckStatus Status, string Detail);

/// <summary>Capacidades declaradas por adaptador y build. Vacías = nada verificado todavía.</summary>
public sealed record AdapterManifest(
    string AppId,
    string Title,
    IReadOnlyList<string> SupportedBuilds,
    IReadOnlyList<string> VerifiedModes,
    int MinPlayers,
    int MaxPlayers,
    bool RequiresSeparateAccounts,
    // Refleja una opción oficial del juego (p. ej. "Launch game without EAC").
    bool NeedsAntiCheatDisabledByOfficialOption,
    string Notes);

public sealed record AdapterEnvironment(
    IReadOnlyList<InstalledGame> Games,
    int ControllersConnected,
    int RunningClients,
    bool SandboxieAvailable);

public sealed record AdapterPreflight(string AppId, CompatibilityState State, IReadOnlyList<PreflightCheck> Checks)
{
    public bool HasFailures => Checks.Any(c => c.Status == CheckStatus.Failed);
    public PreflightCheck? Find(string id) => Checks.FirstOrDefault(c => c.Id == id);
}

/// <summary>Contrato mínimo de ADR-007: preflight de solo lectura antes de cualquier acción.</summary>
public interface IGameAdapter
{
    AdapterManifest Manifest { get; }
    AdapterPreflight Preflight(AdapterEnvironment environment);
}
