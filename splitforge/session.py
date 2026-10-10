"""Orquestación de sesiones con recuperación de posiciones originales.

No se fuerza la terminación de juegos externos: sus partidas pueden estar guardándose.
"""
from __future__ import annotations
import os
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable

from .gamepads import PadState
from .shared_world import SharedWorld
from .shared_io import WorldWriter
from .input_stream import InputWriter
from .layout import two_player_regions
from .windows import (WindowSnapshot, enumerate_windows,
                      move_window, request_close, restore_window, screen_size, snapshot)


@dataclass
class ActiveSession:
    kind: str
    orientation: str
    gutter: int
    processes: list[subprocess.Popen] = field(default_factory=list)
    assigned_ids: list[int | None] = field(default_factory=list)
    input_writers: list[InputWriter] = field(default_factory=list)
    window_handles: list[int] = field(default_factory=list)
    saved: dict[int, WindowSnapshot] = field(default_factory=dict)
    deadline: float = 0.0
    pending: bool = False
    reported_exits: set[int] = field(default_factory=set)
    world: SharedWorld | None = None
    last_input_status: list[str] = field(default_factory=lambda: ["P1: pendiente", "P2: pendiente"])


class SessionManager:
    def __init__(self, log: Callable[[str], None]):
        self.active: ActiveSession | None = None
        self.log = log
        self._closing_demo: list[tuple[float, list[subprocess.Popen]]] = []

    def _require_idle(self) -> None:
        if self.active:
            raise RuntimeError("Detén la sesión actual antes de iniciar otra.")

    def start_demo(self, script: str, assigned_ids: list[int | None], orientation: str, gutter: int) -> None:
        self._require_idle()
        session = ActiveSession("demo", orientation, gutter, deadline=time.monotonic() + 12,
                                pending=True, assigned_ids=list(assigned_ids))
        self.active = session
        try:
            for player in (1, 2):
                device = assigned_ids[player - 1] if len(assigned_ids) >= player else None
                cmd = [sys.executable, script, "--demo-player", str(player)]
                proc = subprocess.Popen(cmd, cwd=str(Path(script).parent),
                                        stdin=subprocess.PIPE, text=True, encoding="utf-8")
                session.processes.append(proc)
                session.input_writers.append(InputWriter(proc.stdin))
                self.log(f"Jugador {player}: proceso de prueba PID {proc.pid}; SDL ID {device if device is not None else 'teclado'}.")
        except Exception:
            self._cleanup_failed_launch(session)
            raise

    def start_shared_demo(self, script: str, assigned_ids: list[int | None],
                          orientation: str, gutter: int) -> None:
        """Un anfitrión simula el mundo; dos hijos independientes lo presentan."""
        self._require_idle()
        if len(assigned_ids) != 2 or None in assigned_ids:
            raise ValueError("Asigna los dos mandos antes de iniciar Mundo Compartido.")
        if assigned_ids[0] == assigned_ids[1]:
            raise ValueError("Cada jugador necesita un mando distinto.")
        s = ActiveSession("shared_demo", orientation, gutter,
                          deadline=time.monotonic() + 12, pending=True,
                          assigned_ids=list(assigned_ids), world=SharedWorld())
        self.active = s
        try:
            for player in (1, 2):
                cmd = [sys.executable, script, "--shared-player", str(player)]
                proc = subprocess.Popen(cmd, cwd=str(Path(script).parent),
                                        stdin=subprocess.PIPE, text=True, encoding="utf-8")
                s.processes.append(proc)
                s.input_writers.append(WorldWriter(proc.stdin))
                self.log(f"Mundo compartido: visor P{player} PID {proc.pid}, mando ID {assigned_ids[player-1]}.")
            self.log("Simulación autoritativa local iniciada: un mundo, dos cámaras.")
        except Exception:
            self._cleanup_failed_launch(s)
            raise

    def route_shared_inputs(self, states: dict[int, PadState], dt: float) -> None:
        s = self.active
        if s is None or s.kind != "shared_demo" or s.world is None:
            return
        controls = {}
        for idx, device_id in enumerate(s.assigned_ids[:2]):
            sample = states.get(device_id) if device_id is not None else None
            controls[idx + 1] = sample
            s.last_input_status[idx] = (
                f"P{idx+1} (ID {device_id}): X={sample.lx:+.2f} Y={sample.ly:+.2f} "
                f"D={'SI' if sample.a_pressed else 'NO'}" if sample is not None
                else f"P{idx+1} (ID {device_id}): SIN LECTURA"
            )
        world = s.world.step(dt, controls)
        for writer in s.input_writers:
            writer.send(world)

    def reassign_shared(self, ids: list[int], devices: set[int]) -> None:
        s = self.active
        if s is None or s.kind != "shared_demo":
            raise RuntimeError("No hay sesión compartida en ejecución")
        if len(ids) != 2 or ids[0] == ids[1] or any(i not in devices for i in ids):
            raise ValueError("Selecciona dos mandos diferentes que estén conectados")
        s.assigned_ids = ids[:]
        self.log(f"Reasignación en vivo: P1 ID {ids[0]} / P2 ID {ids[1]}")

    def start_external(self, executable: str, args_1: str, args_2: str,
                       orientation: str, gutter: int) -> None:
        self._require_idle()
        path = Path(executable).expanduser().resolve()
        if not path.is_file() or path.suffix.lower() != ".exe":
            raise ValueError("Selecciona un ejecutable .exe existente.")
        session = ActiveSession("external", orientation, gutter,
                                deadline=time.monotonic() + 16, pending=True)
        self.active = session
        try:
            for player, arguments in ((1, args_1), (2, args_2)):
                # Argumentos opcionales introducidos por el usuario para la línea de comandos
                # Win32. No se usa shell=True, y executable fija el binario iniciado.
                command = f'"{path}" {arguments}'
                proc = subprocess.Popen(command, executable=str(path), cwd=str(path.parent))
                session.processes.append(proc)
                self.log(f"Jugador {player}: iniciado {path.name} (PID {proc.pid}).")
        except Exception:
            self.log("El lanzamiento parcial puede haber dejado un juego abierto. Compruébalo antes de reintentar.")
            self.stop()
            raise

    def attach(self, handles: list[int], orientation: str, gutter: int) -> None:
        if len(handles) != 2 or handles[0] == handles[1]:
            raise ValueError("Selecciona dos ventanas diferentes.")
        # Permite resolver manualmente los launchers que crean procesos hijos
        # durante una sesión externa en la que no se detectaron ambas ventanas.
        if self.active and self.active.kind == "external" and not self.active.window_handles:
            session = self.active
            session.orientation, session.gutter = orientation, gutter
            self._place(session, handles)
            self.log("Ventanas del juego asociadas manualmente a la sesión iniciada.")
            return
        self._require_idle()
        session = ActiveSession("attached", orientation, gutter)
        self.active = session
        try:
            self._place(session, handles)
        except Exception:
            self.stop()
            raise
        self.log("Dos ventanas existentes organizadas. Sus procesos no serán cerrados por SplitForge.")

    def _place(self, session: ActiveSession, handles: list[int]) -> None:
        regions = two_player_regions(*screen_size(), orientation=session.orientation,
                                     gutter=session.gutter)
        # Primero capturamos las posiciones originales. Si la segunda falla,
        # stop() restaura ambas.
        for hwnd in handles:
            if hwnd not in session.saved:
                session.saved[hwnd] = snapshot(hwnd)
        for hwnd, region in zip(handles, regions):
            move_window(hwnd, region)
        session.window_handles = handles[:]
        session.pending = False
        self.log("Ventanas colocadas en la pantalla principal. Distribución: " + session.orientation + ".")

    def tick(self) -> None:
        now = time.monotonic()
        still_closing = []
        for deadline, processes in self._closing_demo:
            alive = [p for p in processes if p.poll() is None]
            if alive and now >= deadline:
                for proc in alive:
                    # Exclusivamente procesos de demostración creados por nosotros.
                    try:
                        proc.terminate()
                        self.log(f"Proceso de prueba {proc.pid} terminado tras agotar el cierre normal.")
                    except OSError:
                        pass
            elif alive:
                still_closing.append((deadline, alive))
        self._closing_demo = still_closing
        s = self.active
        if not s:
            return
        for proc in s.processes:
            status = proc.poll()
            if status is not None and proc.pid not in s.reported_exits:
                self.log(f"Proceso {proc.pid} finalizó con código {status}.")
                s.reported_exits.add(proc.pid)
        if not s.pending:
            return
        candidates = enumerate_windows()
        handles = []
        for player, proc in enumerate(s.processes, start=1):
            matched = [w for w in candidates if w.pid == proc.pid]
            if s.kind == "demo":
                matched = [w for w in matched if w.title == f"SplitForge DEMO - Jugador {player}"]
            elif s.kind == "shared_demo":
                matched = [w for w in matched if w.title == f"SplitForge MUNDO COMPARTIDO - Jugador {player}"]
            if matched:
                handles.append(matched[0].handle)
        if len(handles) == 2:
            try:
                self._place(s, handles)
            except Exception as exc:
                self.log(f"No se pudieron acomodar las ventanas: {exc}")
                s.pending = False
            return
        if now >= s.deadline:
            s.pending = False
            self.log(f"No se encontraron las dos ventanas a tiempo ({len(handles)}/2). "
                     "Puedes seleccionarlas manualmente cuando estén visibles. "
                     "Los launchers que crean procesos hijos requieren selección manual.")

    def route_demo_inputs(self, states: dict[int, PadState]) -> None:
        s = self.active
        if s is None or s.kind != "demo":
            return
        for idx, writer in enumerate(s.input_writers):
            device_id = s.assigned_ids[idx] if idx < len(s.assigned_ids) else None
            writer.send(states.get(device_id) if device_id is not None else None)

    def stop(self) -> None:
        s = self.active
        if s is None:
            return
        self.active = None
        for previous in reversed(list(s.saved.values())):
            try:
                restore_window(previous)
            except Exception as exc:
                self.log(f"Aviso al restaurar una ventana: {exc}")
        if s.kind in ("demo", "shared_demo"):
            for writer in s.input_writers:
                writer.stop()
            candidates = enumerate_windows()
            for w in candidates:
                if w.pid in {p.pid for p in s.processes}:
                    request_close(w.handle)
            self._closing_demo.append((time.monotonic() + 2.0, s.processes))
            self.log("Sesión de prueba detenida; cerrando ventanas de SplitForge.")
        else:
            self.log("Distribución restaurada. Los juegos externos permanecen abiertos para evitar perder partidas.")

    def _cleanup_failed_launch(self, s: ActiveSession) -> None:
        # Aquí solo se llega desde start_demo, con procesos propios y sin partidas.
        for writer in s.input_writers:
            writer.stop()
        for proc in s.processes:
            if proc.poll() is None:
                try:
                    proc.terminate()
                except OSError:
                    pass
        self.active = None
