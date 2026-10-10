"""Instala/verifica dependencias de SplitForge en el Python que ejecuta el programa.

pygame-ce 2.5.8 ya incluye SDL2 en el wheel de Windows. No instalar SDL por separado.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import sys

PIN = "2.5.8"
DOWNLOAD = "https://pypi.org/project/pygame-ce/2.5.8/#files"


def target_tag(version=None, bits=None):
    version = version or sys.version_info[:2]
    bits = bits or (64 if sys.maxsize > 2**32 else 32)
    abitag = f"cp{version[0]}{version[1]}"
    platform = "win_amd64" if bits == 64 else "win32"
    return f"-{abitag}-{abitag}-{platform}.whl"


def matching_wheels(folders, version=None, bits=None):
    tag = target_tag(version, bits)
    found = []
    for folder in folders:
        if folder.is_dir():
            found.extend(p for p in folder.glob("pygame_ce-*.whl")
                         if p.is_file() and p.name.lower().endswith(tag))
    return sorted(found, reverse=True)


def inspect():
    try:
        import pygame
        # Importar 'pygame' no garantiza que sea el fork pygame-ce.
        from importlib.metadata import PackageNotFoundError, version
        try:
            version("pygame-ce")
        except PackageNotFoundError:
            print("[FALTA] Se importó pygame pero no consta pygame-ce; hay posible conflicto de instalaciones.")
            return False
        pygame.joystick.init()
        version = pygame.version.ver
        sdl = pygame.get_sdl_version()
        try:
            from pygame._sdl2 import controller
            controller.init()
            controller_api = True
        except (ImportError, OSError, AttributeError, pygame.error) as exc:
            controller_api = False
            print(f"[AVISO] Joystick SDL funciona, API GameController no: {exc}")
        print(f"[OK] pygame-ce {version}; SDL {sdl}; GameController={controller_api}")
        print("[OK] Instalado en este Python:", sys.executable)
        return True
    except (ImportError, OSError, RuntimeError, AttributeError) as exc:
        print(f"[FALTA] pygame-ce/SDL no se puede importar en {sys.executable}: {exc}")
        return False


def pip_install(args, timeout=100):
    cmd = [sys.executable, "-m", "pip", "install", "--disable-pip-version-check"] + args
    print("[INFO] Ejecutando instalación con tiempo máximo de", timeout, "segundos")
    try:
        done = subprocess.run(cmd, timeout=timeout, check=False)
        return done.returncode == 0
    except subprocess.TimeoutExpired:
        print("[AVISO] Descarga lenta. Se detuvo el intento sin bloquear el arranque.")
        return False
    except OSError as exc:
        print("[ERROR] No se pudo ejecutar pip:", exc)
        return False


def run(check_only=False, offline=False):
    print("=== SplitForge v0.3.2 | Dependencias / SDL ===")
    print("Python activo:", sys.executable)
    print("Python versión:", sys.version.split()[0], "Arquitectura:", 64 if sys.maxsize > 2**32 else 32, "bits")
    if inspect():
        return 0
    if check_only:
        return 2
    base = Path(__file__).resolve().parent
    found = matching_wheels([base / "wheels", base, Path.home() / "Downloads"])
    for wheel in found:
        print("[INFO] Encontrado wheel compatible:", wheel)
        if pip_install(["--no-index", "--no-deps", str(wheel)], timeout=30) and inspect():
            return 0
    if not offline:
        print("[INFO] Instalando pygame-ce con SDL incluido, desde PyPI ...")
        if pip_install(["--only-binary=:all:", "--retries", "1", "--timeout", "12",
                        f"pygame-ce=={PIN}"], timeout=100) and inspect():
            return 0
    print("[PENDIENTE] No se instaló pygame-ce en este entorno.")
    print("[INFO] Para descarga manual oficial:", DOWNLOAD)
    print("[INFO] Nombre necesario: pygame_ce-2.5.8" + target_tag())
    print("[INFO] Pon el .whl en la carpeta 'wheels' y ejecuta Instalar_Todo.bat nuevamente.")
    print("[INFO] El modo nativo WinMM puede abrirse, pero no detecta todos los PS4 Bluetooth.")
    return 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--offline", action="store_true")
    args = parser.parse_args()
    sys.exit(run(args.check, args.offline))
