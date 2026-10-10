"""Asociación conservadora de ventanas Win32 a títulos Steam instalados.

Solo inspección: no crea procesos, no modifica juegos, ni redirige entradas.
"""
from __future__ import annotations

from dataclasses import dataclass
import ctypes
from ctypes import wintypes
import ntpath
import os
from collections import defaultdict
from typing import Callable, Iterable

from .steam_library import InstalledGame
from .windows import WindowInfo, enumerate_windows


@dataclass(frozen=True)
class GameWindow:
    handle: int
    pid: int
    title: str
    exe_name: str
    exe_path: str  # Solo memoria local; nunca exportarlo al informe.


@dataclass(frozen=True)
class GameWindowReport:
    app_id: str
    name: str
    build_id: str
    windows: tuple[GameWindow, ...]
    chosen_handles: tuple[int, ...]
    distinct_pids: int
    state: str
    detail: str

    def public_dict(self) -> dict:
        """Seguro para compartir: SIN títulos de ventana o rutas de usuario."""
        return {
            "appid": self.app_id,
            "juego": self.name,
            "buildid": self.build_id,
            "ventanas": len(self.windows),
            "procesos_distintos": self.distinct_pids,
            "ejecutables": sorted(set(x.exe_name for x in self.windows)),
            "pares_aptos_para_distribucion": len(self.chosen_handles) == 2,
            "estado": self.state,
            "nota": self.detail,
            "cooperativo_verificado": False,
            "mandos_aislados_verificados": False,
            "offline_verificado": False,
        }


def _win_norm(value: str) -> str:
    """Normalización de rutas Windows, sin acceder a carpetas ni resolver symlinks."""
    return ntpath.normcase(ntpath.normpath(value.replace('/', '\\').strip('" ')))


def is_game_executable(path: str | None, game_folder: str) -> bool:
    if not path or not game_folder:
        return False
    full, root = _win_norm(path), _win_norm(game_folder)
    # Rechazar rutas relativas, UNC y dispositivos para evitar coincidencias ambiguas.
    if (not ntpath.isabs(full) or not ntpath.isabs(root)
            or full.startswith('\\\\') or root.startswith('\\\\')):
        return False
    return full.startswith(root.rstrip('\\') + '\\') and full.lower().endswith('.exe')


def process_image_path(pid: int) -> str | None:
    """Leer imagen del proceso con QUERY_LIMITED_INFORMATION. No obtiene escritura."""
    if os.name != 'nt' or pid <= 0:
        return None
    kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
    kernel32.OpenProcess.restype = wintypes.HANDLE
    kernel32.QueryFullProcessImageNameW.argtypes = [wintypes.HANDLE, wintypes.DWORD,
                                                    wintypes.LPWSTR, ctypes.POINTER(wintypes.DWORD)]
    kernel32.QueryFullProcessImageNameW.restype = wintypes.BOOL
    kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
    kernel32.CloseHandle.restype = wintypes.BOOL
    handle = kernel32.OpenProcess(0x1000, False, pid)
    if not handle:
        return None
    try:
        buf = ctypes.create_unicode_buffer(32768)
        size = wintypes.DWORD(len(buf))
        if not kernel32.QueryFullProcessImageNameW(handle, 0, buf, ctypes.byref(size)):
            return None
        return buf.value
    finally:
        kernel32.CloseHandle(handle)


def inspect_game_windows(
    game: InstalledGame,
    visible: Iterable[WindowInfo] | None = None,
    image_lookup: Callable[[int], str | None] = process_image_path,
) -> GameWindowReport:
    """Selecciona dos ventanas SOLO de procesos distintos que usan EL MISMO .exe.

    Que exista ese par NO significa que puedan usar una partida conjunta.
    """
    all_windows = list(enumerate_windows() if visible is None else visible)
    pid_cache: dict[int, str | None] = {}
    found: list[GameWindow] = []
    seen_handles: set[int] = set()
    for w in all_windows:
        if w.handle in seen_handles or w.pid <= 0:
            continue
        seen_handles.add(w.handle)
        if w.pid not in pid_cache:
            try:
                pid_cache[w.pid] = image_lookup(w.pid)
            except (OSError, PermissionError, ValueError):
                pid_cache[w.pid] = None
        path = pid_cache[w.pid]
        if not is_game_executable(path, game.folder):
            continue
        found.append(GameWindow(w.handle, w.pid, w.title,
                                ntpath.basename(path or ''), path or ''))

    # Dos ventanas en un mismo proceso NO cuentan como dos clientes.
    groups: dict[str, dict[int, GameWindow]] = defaultdict(dict)
    for win in found:
        groups[_win_norm(win.exe_path)].setdefault(win.pid, win)
    eligible = [wins for wins in groups.values() if len(wins) >= 2]
    handles: tuple[int, ...] = ()
    if eligible:
        winner = sorted(eligible, key=lambda g: (-len(g), min(g)))[0]
        first_two = sorted(winner.values(), key=lambda w: w.pid)[:2]
        handles = (first_two[0].handle, first_two[1].handle)
        state = 'DOS_CLIENTES_DETECTADOS'
        detail = ('Dos procesos del mismo ejecutable detectados. Solo se puede distribuir '
                  'sus ventanas; cooperativo, cuentas y controles siguen sin verificar.')
    elif found:
        state = 'UNO_O_MAS_SIN_PAREJA'
        detail = ('Hay ventanas de la instalación, pero no dos clientes diferentes del '
                  'mismo ejecutable. No se puede activar una sesión Steam de dos ventanas.')
    else:
        state = 'SIN_VENTANAS'
        detail = ('No hay ventanas visibles cuyos procesos pertenezcan a esta instalación. '
                  'Abre el juego normalmente desde Steam y vuelve a analizar.')

    return GameWindowReport(game.app_id, game.name, game.build_id, tuple(found), handles,
                            len({win.pid for win in found}), state, detail)


def verify_pair(game: InstalledGame, report: GameWindowReport,
                visible: Iterable[WindowInfo] | None = None,
                image_lookup: Callable[[int], str | None] = process_image_path) -> tuple[int, int]:
    """Revalidar PID+exe+HWND justo antes de mover ventanas, evita selección obsoleta."""
    if report.app_id != game.app_id or len(report.chosen_handles) != 2:
        raise ValueError('No hay dos ventanas elegibles para esta instalación.')
    current = inspect_game_windows(game, visible, image_lookup)
    selected = report.chosen_handles
    if len(current.chosen_handles) != 2 or set(current.chosen_handles) != set(selected):
        raise ValueError('Las ventanas cambiaron; pulsa Analizar ventanas otra vez.')
    return selected[0], selected[1]
