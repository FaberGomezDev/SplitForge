"""Evaluación conservadora de instalaciones Steam, sin abrir ni modificar juegos.

Importante: una evaluación favorable NO demuestra compatibilidad cooperativa.
No hay bypass de DRM, redirección de mandos externos ni lanzamiento doble.
"""
from __future__ import annotations

from dataclasses import dataclass, asdict
from pathlib import Path
import json
import re

from .steam_library import InstalledGame


@dataclass(frozen=True)
class CompatibilityProfile:
    app_id: str
    title: str
    phase: str
    expected_executables: tuple[str, ...]
    gameplay_note: str
    launch_note: str


# Propuestas de investigación. NINGUNA está acreditada como adaptador funcional.
RESEARCH_PROFILES: dict[str, CompatibilityProfile] = {
    "251570": CompatibilityProfile(
        app_id="251570", title="7 Days to Die", phase="Candidato para investigar",
        expected_executables=("7DaysToDie.exe",),
        gameplay_note="Tiene opciones multijugador y servidores; falta probar dos clientes en una PC, identidad de cada jugador y LAN/offline en la versión instalada.",
        launch_note="No abrir dos clientes todavía; analizar Steam, Easy Anti-Cheat, licencias y sesión local con pruebas controladas."),
    "275850": CompatibilityProfile(
        app_id="275850", title="No Man's Sky", phase="Investigación posterior",
        expected_executables=("NMS.exe",),
        gameplay_note="El modo multijugador y la autenticación deben verificarse. No se presupone cooperativo sin Internet.",
        launch_note="No modificar la autenticación ni abrir procesos duplicados."),
    "495420": CompatibilityProfile(
        app_id="495420", title="State of Decay 2", phase="Investigación posterior",
        expected_executables=(),
        gameplay_note="El multijugador puede requerir identidades y servicios en línea distintos.",
        launch_note="No prometer modo sin conexión con dos jugadores."),
    "271590": CompatibilityProfile(
        app_id="271590", title="Grand Theft Auto V (Legacy)", phase="Investigación posterior",
        expected_executables=("GTA5.exe",),
        gameplay_note="La campaña no incluye cooperativo local nativo; opciones externas no se presuponen compatibles.",
        launch_note="No intervenir en GTA Online, protección antitrampas ni licencias."),
}


@dataclass(frozen=True)
class Preflight:
    app_id: str
    name: str
    phase: str
    folder_present: bool
    inside_steam_library: bool
    expected_executable_found: bool | None
    expected_executables: tuple[str, ...]
    executables_found: tuple[str, ...]
    build_id: str
    state_flags: str
    offline_coop_verified: bool
    input_isolation_verified: bool
    simultaneous_clients_verified: bool
    ready_for_two_players: bool
    notes: tuple[str, ...]

    def as_dict(self) -> dict:
        return asdict(self)


def _safe_install_folder(game: InstalledGame) -> tuple[Path, bool]:
    # No seguir rutas fuera del arbol common: los manifiestos no son una lista de
    # ejecutables de confianza y nunca deben autorizar ejecución arbitraria.
    folder = Path(game.folder).resolve()
    common = (Path(game.library) / 'steamapps' / 'common').resolve()
    return folder, (folder != common and common in folder.parents)


def evaluate(game: InstalledGame) -> Preflight:
    profile = RESEARCH_PROFILES.get(game.app_id)
    phase = profile.phase if profile else 'Sin perfil; aún sin verificar'
    notes: list[str] = []
    folder, is_inside = _safe_install_folder(game)
    present = folder.is_dir() if is_inside else False
    if not is_inside:
        notes.append('Ruta fuera de steamapps/common: no confiable, revisar manifiesto.')
    if not present:
        notes.append('No se encontró una carpeta de instalación válida en disco.')
    names: list[str] = []
    if present:
        try:
            # Únicamente el directorio principal; no recorrer árboles de juegos de decenas de GB.
            names = sorted((f.name for f in folder.glob('*.exe') if f.is_file()), key=str.lower)[:32]
        except OSError:
            notes.append('No fue posible inspeccionar los ejecutables del directorio.')
    expected = profile.expected_executables if profile else ()
    if expected:
        names_cf = {n.casefold() for n in names}
        detected: bool | None = any(name.casefold() in names_cf for name in expected)
        if detected is False and present:
            notes.append('No se halló el ejecutable esperado en la raíz; puede estar en una subcarpeta o haber cambiado de nombre.')
    else:
        detected = None
    if profile:
        notes.append(profile.gameplay_note)
        notes.append(profile.launch_note)
    else:
        notes.append('Sin perfil de investigación para este AppID.')
    notes.append('Sin prueba de dos cuentas/clientes, mandos aislados ni conexión cooperativa local.')
    return Preflight(
        app_id=game.app_id, name=game.name, phase=phase,
        folder_present=present, inside_steam_library=is_inside,
        expected_executable_found=detected,
        expected_executables=expected, executables_found=tuple(names),
        build_id=game.build_id, state_flags=game.state_flags,
        offline_coop_verified=False, input_isolation_verified=False,
        simultaneous_clients_verified=False, ready_for_two_players=False,
        notes=tuple(notes))


def public_report(games: list[InstalledGame]) -> dict:
    """Resumen compartible: SIN rutas de perfil de Windows, IDs de cuenta ni MAC."""
    entries = [evaluate(game).as_dict() for game in games]
    return {
        'format': 'SplitForge Steam Preflight v1',
        'game_count': len(entries),
        'games': entries,
        'declaration': 'Ninguna instalación certificada para pantalla dividida; revisión de archivos en modo solo lectura.'
    }


def report_json(games: list[InstalledGame]) -> str:
    return json.dumps(public_report(games), indent=2, ensure_ascii=False)


def steam_uri(app_id: str) -> str:
    """URL de lanzamiento de UNA sola instancia solicitada explícitamente por usuario."""
    if not re.fullmatch(r'[1-9]\d{0,14}', app_id):
        raise ValueError('App ID de Steam inválido')
    return 'steam://rungameid/' + app_id
