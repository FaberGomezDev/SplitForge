"""Comprobar entrada SDL/WinMM sin exigir descargas."""
from __future__ import annotations
import platform
import sys


def main():
    print("=" * 60)
    print("SplitForge v0.3 - diagnostico de mandos")
    print("Sistema:", platform.platform())
    print("Python:", sys.version.split()[0])
    try:
        import tkinter as tk
        print("Tkinter:", tk.Tcl().eval("info patchlevel"))
    except Exception as exc:
        print("Tkinter ERROR:", exc)
    try:
        import pygame
        print("pygame-ce:", pygame.version.ver, "SDL:", pygame.get_sdl_version())
    except ImportError:
        print("pygame-ce: no instalado (se probara WinMM nativo)")
    try:
        from splitforge.gamepads import ControllerManager
        manager = ControllerManager()
        print("Motor de entrada:", manager.backend_name)
        print("Advertencia SDL:", getattr(manager, "warning", "") or "ninguna")
        devices = manager.devices()
        print("Mandos encontrados:", len(devices))
        for device in devices:
            print(f"  - {device.name} | ID={device.instance_id} | {device.kind}")
            print(f"    Ejes={device.axes} Botones={device.buttons}")
            print("    Lectura:", manager.raw_axes(device.instance_id))
        manager.close()
        if not devices:
            print("CONSEJO: Win+R, escribe joy.cpl. Si el mando no aparece ahi,")
            print("prueba por USB o instala SDL opcional con Instalar_SDL_Opcional.bat")
    except Exception as exc:
        print("ERROR de mandos:", type(exc).__name__, str(exc))
    print("=" * 60)


if __name__ == "__main__":
    main()
