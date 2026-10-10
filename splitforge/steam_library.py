"""Lectura pasiva y sin escritura de bibliotecas de Steam en Windows.

Descubrir juegos instalados NO implica que soporten modo offline, LAN,
multinstancia o split-screen; se requerirá un adaptador verificado por juego.
"""
from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import os
import re


def _unquote(raw: str) -> str:
    return raw.replace('\\\\', '\\').replace('\\"', '"')


def kv_value(text: str, key: str) -> str | None:
    match = re.search(r'"' + re.escape(key) + r'"\s*"((?:\\.|[^"\\])*)"', text, re.I)
    return _unquote(match.group(1)) if match else None


@dataclass(frozen=True)
class InstalledGame:
    app_id: str
    name: str
    folder: str
    library: str
    status: str = "Sin adaptador verificado"
    build_id: str = ""
    state_flags: str = ""


def steam_locations() -> list[Path]:
    candidates = []
    if os.name == "nt":
        try:
            import winreg
            for regkey in (r"Software\Valve\Steam", r"Software\WOW6432Node\Valve\Steam"):
                try:
                    with winreg.OpenKey(winreg.HKEY_CURRENT_USER, regkey) as h:
                        candidates.append(Path(winreg.QueryValueEx(h, "SteamPath")[0]))
                except OSError:
                    pass
        except ImportError:
            pass
        for variable in ("PROGRAMFILES(X86)", "PROGRAMFILES"):
            if os.getenv(variable):
                candidates.append(Path(os.environ[variable]) / "Steam")
    return list(dict.fromkeys(p for p in candidates if p.exists()))


def library_paths(steam_root: Path) -> list[Path]:
    libraries = [steam_root]
    # Steam mantiene el indice normalmente bajo steamapps; algunas versiones
    # tambien tienen una copia en config/. Leemos el primero disponible.
    indexes = (steam_root / "steamapps" / "libraryfolders.vdf",
               steam_root / "config" / "libraryfolders.vdf")
    text = ""
    for file in indexes:
        try:
            text = file.read_text(encoding="utf-8-sig", errors="replace")
            break
        except OSError:
            continue
    if not text:
        return libraries
    # Moderno: "path" "D:\\SteamLibrary". Legacy: "1" "D:\\SteamLibrary".
    paths = re.findall(r'"path"\s*"((?:\\.|[^"\\])*)"', text, re.I)
    paths += re.findall(r'^\s*"\d+"\s*"([^"]+)"', text, re.M)
    for raw in paths:
        p = Path(_unquote(raw))
        if p.exists() and p not in libraries:
            libraries.append(p)
    return libraries


def scan_steam(roots: list[Path] | None = None) -> list[InstalledGame]:
    games: dict[str, InstalledGame] = {}
    for root in roots if roots is not None else steam_locations():
        for library in library_paths(root):
            steamapps = library / "steamapps"
            try:
                manifests = list(steamapps.glob("appmanifest_*.acf"))
            except OSError:
                continue
            for file in manifests[:3000]:
                match = re.fullmatch(r"appmanifest_(\d+)\.acf", file.name)
                if not match:
                    continue
                try:
                    text = file.read_text(encoding="utf-8-sig", errors="replace")[:200000]
                except OSError:
                    continue
                appid = match.group(1)
                name = kv_value(text, "name")
                installdir = kv_value(text, "installdir")
                if not name or not installdir:
                    continue
                folder = str(steamapps / "common" / installdir)
                # No inferimos compatibilidad a partir de un manifesto instalado.
                games[appid] = InstalledGame(appid, name, folder, str(library),
                                            build_id=kv_value(text, "buildid") or "",
                                            state_flags=kv_value(text, "StateFlags") or "")
    return sorted(games.values(), key=lambda g: g.name.casefold())
