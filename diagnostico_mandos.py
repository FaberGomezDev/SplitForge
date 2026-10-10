"""Diagnóstico de SDL, GameController, WinMM y entradas de PS4 Bluetooth.

No instala nada, no requiere Steam, no modifica drivers ni emparejamientos.
"""
from __future__ import annotations

import argparse
from datetime import datetime
import os
from pathlib import Path
import platform
import sys
import time


def run(seconds=20):
    text = []

    def emit(s):
        s = str(s)
        print(s, flush=True)
        text.append(s)

    emit("=== SplitForge v0.3 - Diagnóstico de mandos ===")
    emit("Fecha local: " + datetime.now().isoformat(timespec="seconds"))
    emit("Windows: " + platform.platform())
    emit("Python: " + platform.python_version() + (" 64 bits" if sys.maxsize > 2**32 else " 32 bits"))
    emit("Ejecutable relativo: " + str(Path(sys.executable).name))
    emit("HIDAPI_PS4 = " + os.getenv("SDL_JOYSTICK_HIDAPI_PS4", "por defecto"))
    emit("SDL_ALLOW_BACKGROUND = " + os.getenv("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS", "por defecto"))
    emit("Aparecer en Bluetooth no equivale a emitir eventos joystick.\n")

    from splitforge.gamepads import ControllerManager
    from splitforge.winmm_gamepads import WinMMControllerManager
    os.environ.setdefault("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS", "1")
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")

    manager = None
    winmm = None
    try:
        manager = ControllerManager()
        emit("Motor SplitForge: " + manager.backend_name)
        if manager.warning:
            emit("AVISO SDL: " + manager.warning)
        try:
            import pygame
            emit("pygame-ce: " + pygame.version.ver)
            emit("SDL cargado: " + str(pygame.get_sdl_version()))
            emit("SDL joysticks conectados: " + str(pygame.joystick.get_count()))
            if manager.controller_api is not None:
                emit("SDL GameController disponibles: " + str(manager.controller_api.get_count()))
                for i in range(pygame.joystick.get_count()):
                    emit(f"Joystick index {i}: GameController={manager.controller_api.is_controller(i)}")
            else:
                emit("API GameController: no disponible, SDL Joystick debería seguir funcionando")
        except (ImportError, OSError, RuntimeError) as exc:
            emit("ERROR al importar pygame/SDL: " + repr(exc))
        for x in manager.devices():
            emit(f"SDL/Activo: {x.name} | id={x.instance_id} | {x.kind} | axes={x.axes} | botones={x.buttons}")
        if os.name == "nt":
            try:
                winmm = WinMMControllerManager()
                emit("WinMM detectó: " + str(len(winmm.devices())))
                for x in winmm.devices():
                    emit(f"WinMM: {x.name} | id={x.instance_id} | axes={x.axes} | botones={x.buttons}")
            except Exception as exc:
                emit("AVISO WinMM: " + repr(exc))

        emit(f"\nMueve cada stick y pulsa CRUZ (X) en los DOS mandos durante {seconds}s")
        emit("Solo se muestran cambios importantes para facilitar el diagnóstico.")
        before = {}
        start = time.monotonic()
        last_rescan = 0
        while time.monotonic() - start < seconds:
            if time.monotonic() - last_rescan > 2:
                manager.refresh()
                if winmm:
                    winmm.refresh()
                last_rescan = time.monotonic()
            readings = []
            for d in manager.devices():
                axes, btns = manager.raw_axes(d.instance_id)
                sample = manager.poll().get(d.instance_id)
                state = (tuple(round(a, 1) for a in axes), tuple(btns),
                         None if sample is None else sample.a_pressed)
                readings.append(("SDL", d.instance_id, d.name, state))
            if winmm:
                for d in winmm.devices():
                    axes, btns = winmm.raw_axes(d.instance_id)
                    state = (tuple(round(a, 1) for a in axes), tuple(btns), None)
                    readings.append(("WINMM", d.instance_id, d.name, state))
            for backend, iid, name, state in readings:
                key = (backend, iid)
                if before.get(key) != state:
                    emit(f"{backend} id={iid} {name}: ejes={state[0]} botones={state[1]} accion={state[2]}")
                    before[key] = state
            time.sleep(0.07)
        emit("\nFin de prueba: si hubo lecturas cambientes en SDL, el mando ENVIA datos.")
    except Exception as exc:
        emit("ERROR inesperado: " + repr(exc))
    finally:
        if manager:
            manager.close()
        if winmm:
            winmm.close()
        target = Path(__file__).resolve().parent / "Reporte_Mandos.txt"
        try:
            target.write_text("\n".join(text) + "\n", encoding="utf-8")
            emit("Reporte guardado en: Reporte_Mandos.txt (junto al BAT)")
        except OSError as exc:
            emit("No se pudo guardar Reporte_Mandos.txt: " + str(exc))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--seconds", type=int, default=20)
    args = parser.parse_args()
    run(max(1, min(120, args.seconds)))
