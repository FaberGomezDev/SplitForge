"""Valida referencias markdown locales del repositorio, sin dependencias externas.

Uso: python tools/check_docs.py
No accede a Internet ni modifica archivos.
"""
from __future__ import annotations

from pathlib import Path
import re
import sys
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
LINK = re.compile(r"(?<!!)\[[^\]]*\]\(([^)]+)\)")


def check() -> list[str]:
    errors: list[str] = []
    required = [ROOT / "CLAUDE.md", ROOT / "README.md", ROOT / "docs" / "README.md",
                ROOT / "docs" / "01-MEMORIA.md", ROOT / "docs" / "02-ARQUITECTURA.md"]
    errors.extend(f"Falta archivo obligatorio: {path.relative_to(ROOT)}" for path in required if not path.is_file())
    for path in sorted((ROOT / "docs").rglob("*.md")) + [ROOT / "CLAUDE.md"]:
        if not path.is_file():
            continue
        content = path.read_text(encoding="utf-8")
        for link in LINK.findall(content):
            href = link.strip().split(maxsplit=1)[0].strip("<>\"'")
            if href.startswith(("https://", "http://", "mailto:", "#")):
                continue
            local = unquote(href.split("#", 1)[0])
            if not local:
                continue
            destination = (path.parent / local).resolve()
            if not destination.is_relative_to(ROOT) or not destination.exists():
                errors.append(f"{path.relative_to(ROOT)}: enlace inválido: {link}")
    return errors


if __name__ == "__main__":
    issues = check()
    for problem in issues:
        print("ERROR:", problem)
    if issues:
        print(f"Documentación con {len(issues)} problema(s)")
        sys.exit(1)
    print("OK: documentación obligatoria y enlaces Markdown locales")
