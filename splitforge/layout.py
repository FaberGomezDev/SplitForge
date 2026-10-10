"""Cálculos puros y verificables para pantalla dividida."""
from dataclasses import dataclass


@dataclass(frozen=True)
class Region:
    x: int
    y: int
    width: int
    height: int


def two_player_regions(width: int, height: int, orientation: str = "vertical", gutter: int = 4) -> tuple[Region, Region]:
    """vertical = lado a lado; horizontal = arriba / abajo.

    Unidades físicas en píxeles; el proceso configura DPI awareness.
    """
    if width < 200 or height < 200:
        raise ValueError("La pantalla debe tener al menos 200 × 200 píxeles.")
    if orientation not in ("vertical", "horizontal"):
        raise ValueError("Orientación no reconocida.")
    if not 0 <= gutter <= 64:
        raise ValueError("Separación fuera del rango 0–64.")
    if orientation == "vertical":
        left = (width - gutter) // 2
        return (Region(0, 0, left, height), Region(left + gutter, 0, width - left - gutter, height))
    top = (height - gutter) // 2
    return (Region(0, 0, width, top), Region(0, top + gutter, width, height - top - gutter))
