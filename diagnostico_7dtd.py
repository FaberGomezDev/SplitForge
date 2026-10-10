"""Diagnóstico de 7 Days to Die en ejecución: qué APIs de mando carga cada proceso.

Solo lectura: no inyecta, no modifica el juego ni cierra procesos. Requiere el juego
abierto (idealmente sin EAC). El informe no incluye rutas, títulos de ventana ni cuentas.
"""
from __future__ import annotations

import argparse
import ctypes
import json
import ntpath
import os
from ctypes import wintypes
from pathlib import Path

GAME_EXE = "7DaysToDie.exe"
SERVER_EXE = "7DaysToDieServer.exe"

# Familias de módulos que delatan cómo lee el juego los mandos.
INPUT_FAMILIES = {
    "xinput": ("xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll", "xinputuap.dll"),
    "directinput": ("dinput8.dll", "dinput.dll"),
    "hid": ("hid.dll",),
    "windows_gaming_input": ("windows.gaming.input.dll",),
    "gameinput": ("gameinput.dll",),
    "sdl": ("sdl2.dll", "sdl3.dll"),
    "steam_api": ("steam_api64.dll", "steam_api.dll"),
    "steam_overlay_input": ("gameoverlayrenderer64.dll", "gameoverlayrenderer.dll"),
    "easy_anti_cheat": ("easyanticheat_x64.dll", "easyanticheat.dll", "eosovh-win64-shipping.dll"),
}
KEYWORDS = ("input", "xinput", "hid", "pad", "joy", "rewired", "incontrol", "steam", "anticheat", "eos")


def classify(module_names: list[str]) -> dict:
    """Agrupa nombres de módulo (sin rutas) por familia de entrada. Función pura y probada."""
    lowered = sorted({name.lower() for name in module_names})
    families = {family: sorted(n for n in lowered if n in names)
                for family, names in INPUT_FAMILIES.items()}
    relevant = sorted(n for n in lowered if any(k in n for k in KEYWORDS))
    return {
        "familias": {k: v for k, v in families.items() if v},
        "modulos_relevantes": relevant,
        "total_modulos": len(lowered),
    }


def _api():
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    psapi = ctypes.WinDLL("psapi", use_last_error=True)
    k32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
    k32.OpenProcess.restype = wintypes.HANDLE
    k32.CloseHandle.argtypes = [wintypes.HANDLE]
    psapi.EnumProcesses.argtypes = [ctypes.POINTER(wintypes.DWORD), wintypes.DWORD, ctypes.POINTER(wintypes.DWORD)]
    psapi.EnumProcessModulesEx.argtypes = [wintypes.HANDLE, ctypes.POINTER(wintypes.HMODULE), wintypes.DWORD,
                                           ctypes.POINTER(wintypes.DWORD), wintypes.DWORD]
    psapi.GetModuleBaseNameW.argtypes = [wintypes.HANDLE, wintypes.HMODULE, wintypes.LPWSTR, wintypes.DWORD]
    return k32, psapi


def list_pids(psapi) -> list[int]:
    size = 4096
    while True:
        arr = (wintypes.DWORD * size)()
        needed = wintypes.DWORD()
        if not psapi.EnumProcesses(arr, ctypes.sizeof(arr), ctypes.byref(needed)):
            return []
        count = needed.value // ctypes.sizeof(wintypes.DWORD)
        if count < size:
            return [int(arr[i]) for i in range(count) if arr[i]]
        size *= 2


def module_names(k32, psapi, pid: int) -> list[str] | None:
    """PROCESS_QUERY_INFORMATION | PROCESS_VM_READ: lectura de la lista de módulos, nada más."""
    handle = k32.OpenProcess(0x0400 | 0x0010, False, pid)
    if not handle:
        return None
    try:
        size = 1024
        while True:
            mods = (wintypes.HMODULE * size)()
            needed = wintypes.DWORD()
            if not psapi.EnumProcessModulesEx(handle, mods, ctypes.sizeof(mods), ctypes.byref(needed), 0x03):
                return None
            count = needed.value // ctypes.sizeof(wintypes.HMODULE)
            if count <= size:
                break
            size = count
        names = []
        buf = ctypes.create_unicode_buffer(260)
        for i in range(count):
            if psapi.GetModuleBaseNameW(handle, mods[i], buf, 260):
                names.append(buf.value)
        return names
    finally:
        k32.CloseHandle(handle)


def main() -> int:
    parser = argparse.ArgumentParser(description="SplitForge - diagnóstico de 7 Days to Die en ejecución")
    parser.add_argument("--output", type=Path, help="Archivo JSON de salida")
    args = parser.parse_args()
    if os.name != "nt":
        print("Este diagnóstico requiere Windows 10/11.")
        return 2
    from splitforge.steam_windows import process_image_path
    from splitforge.windows import enumerate_windows

    k32, psapi = _api()
    windows_by_pid: dict[int, int] = {}
    for w in enumerate_windows():
        windows_by_pid[w.pid] = windows_by_pid.get(w.pid, 0) + 1

    clients, servers = [], []
    for pid in list_pids(psapi):
        path = process_image_path(pid)
        if not path:
            continue
        exe = ntpath.basename(path).lower()
        if exe not in (GAME_EXE.lower(), SERVER_EXE.lower()):
            continue
        names = module_names(k32, psapi, pid)
        entry = {
            "ejecutable": ntpath.basename(path),
            "ventanas_visibles": windows_by_pid.get(pid, 0),
            "modulos_legibles": names is not None,
        }
        if names is None:
            entry["nota"] = "Windows denegó la lectura de módulos (¿EAC activo o proceso elevado?)."
        else:
            entry.update(classify(names))
        (clients if exe == GAME_EXE.lower() else servers).append(entry)

    result = {
        "schema": "SplitForge 7DTD Diagnostico v1",
        "clientes_7dtd": len(clients),
        "servidores_7dtd": len(servers),
        "clientes": clients,
        "servidores": servers,
        "aviso": "Solo lectura. No incluye rutas, títulos de ventana, cuentas ni identificadores de mandos.",
    }
    desktop = Path(os.environ.get("USERPROFILE", str(Path.home()))) / "Desktop"
    target = args.output or ((desktop if desktop.is_dir() else Path.cwd()) / "SplitForge_Reporte_7DTD.json")
    target.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")

    print(f"Clientes 7DTD abiertos: {len(clients)} | Servidores: {len(servers)}")
    for i, c in enumerate(clients, 1):
        if not c["modulos_legibles"]:
            print(f"  Cliente {i}: {c['nota']}")
            continue
        fams = ", ".join(c["familias"]) or "ninguna detectada"
        print(f"  Cliente {i}: ventanas={c['ventanas_visibles']} | APIs de entrada: {fams}")
    if not clients:
        print("No hay 7 Days to Die abierto. Ábrelo (sin EAC) y vuelve a ejecutar este diagnóstico.")
    print("Reporte guardado:", target)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
