"""Comparador SDL crudo / normalizado para controles PS4 (sin instalaciones).

Reutiliza la misma ruta de SplitForge; no registra MAC ni IDs seriales.
"""
from __future__ import annotations
import os
import sys
import time
from datetime import datetime
from pathlib import Path

from splitforge.gamepads import ControllerManager


def main():
    root = Path(__file__).resolve().parent
    output = root / "Reporte_Movimiento.txt"
    def write(line=""):
        print(line, flush=True)
        with output.open("a", encoding="utf-8") as out:
            out.write(line + "\n")

    output.write_text("", encoding="utf-8")
    write("SPLITFORGE 0.3.1 — COMPARADOR ENTRADA PS4")
    write("Fecha: " + datetime.now().isoformat(timespec="seconds"))
    write("Python: " + sys.version.split()[0] + " | SO: " + os.name)
    manager = ControllerManager(input_mode="auto")
    try:
        write("Motor: " + manager.backend_name)
        write("Aviso: " + (manager.warning or "ninguno"))
        detected = manager.devices()
        for info in detected:
            write(f"Mando ID {info.instance_id}: {info.name}; tipo={info.kind}, axes={info.axes}, buttons={info.buttons}; ruta={manager.read_path(info.instance_id)}")
        if not detected:
            write("SIN MANDOS: no se pudo probar movimiento")
            return 2
        write("Mueve los sticks de AMBOS mandos y pulsa la Cruz/X durante 12 segundos.")
        write("Se guardan cambios significativos (no cada fotograma).")
        last = {}
        t0 = time.monotonic()
        while time.monotonic() - t0 < 12:
            states = manager.poll()
            for info in manager.devices():
                axes, buttons = manager.raw_axes(info.instance_id)
                state = states.get(info.instance_id)
                signature = (tuple(round(a, 1) for a in axes[:4]), tuple(buttons),
                             round(state.lx, 1) if state else None,
                             round(state.ly, 1) if state else None,
                             bool(state.a_pressed) if state else None)
                if last.get(info.instance_id) != signature:
                    last[info.instance_id] = signature
                    write(f"{time.monotonic()-t0:05.2f}s | ID {info.instance_id} | "
                          f"raw={axes} botones={buttons} | "
                          f"motor=({state.lx:+.2f},{state.ly:+.2f},A={int(state.a_pressed)})" if state else
                          f"{time.monotonic()-t0:05.2f}s | ID {info.instance_id} | sin estado motor | raw={axes}")
            time.sleep(0.08)
        write("FIN. Si raw cambia y motor no cambia, reporta ese contraste.")
        return 0
    finally:
        manager.close()


if __name__ == "__main__":
    raise SystemExit(main())
