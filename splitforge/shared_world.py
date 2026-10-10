"""Estado autoritativo de una pequeña arena CO-OP local.

Es un laboratorio para validar DOS controles y DOS cámaras del mismo mundo.
NO ofrece pantalla dividida a juegos comerciales ni emula Steam Input.
"""
from __future__ import annotations
from dataclasses import dataclass
from math import hypot
from .gamepads import PadState


def clamp(value: float, lo: float, hi: float) -> float:
    return max(lo, min(hi, value))


@dataclass
class Actor:
    x: float
    y: float
    score: int = 0
    hits: int = 0
    shield: bool = False
    last_fire: bool = False
    last_dash: bool = False
    connected: bool = False


class SharedWorld:
    """Toda la simulación ocurre en el proceso principal, una sola vez por frame."""

    def __init__(self):
        self.actors = {1: Actor(.20, .50), 2: Actor(.80, .50)}
        self.projectiles: list[dict] = []
        self.orb = (.5, .5)
        self.time = 0.0
        self.frame = 0
        self.event = "Conecta ambos mandos. Acerca las naves al orbe central."
        self._event_age = 0.0

    def step(self, dt: float, inputs: dict[int, PadState | None]) -> dict:
        dt = clamp(dt, 0, .05)
        self.time += dt
        self.frame += 1
        self._event_age += dt
        if self._event_age > 4:
            self.event = "Misma arena en las dos mitades: puedes ver al otro jugador."
        for num, actor in self.actors.items():
            pad = inputs.get(num)
            actor.connected = bool(pad is not None and pad.connected)
            xdir = clamp(pad.lx, -1, 1) if actor.connected else 0.0
            ydir = -clamp(pad.ly, -1, 1) if actor.connected else 0.0
            speed = .46
            length = max(1., hypot(xdir, ydir))
            actor.x = clamp(actor.x + xdir / length * speed * dt, .045, .955)
            actor.y = clamp(actor.y + ydir / length * speed * dt, .09, .91)
            actor.shield = bool(pad and pad.shield_pressed)
            dash = bool(pad and pad.dash_pressed)
            if dash and not actor.last_dash:
                dx, dy = (xdir, ydir) if abs(xdir) + abs(ydir) > .15 else ((1., 0.) if num == 1 else (-1., 0.))
                d = max(1., hypot(dx, dy))
                actor.x = clamp(actor.x + dx / d * .11, .045, .955)
                actor.y = clamp(actor.y + dy / d * .11, .09, .91)
            actor.last_dash = dash
            firing = bool(pad and pad.a_pressed)
            if firing and not actor.last_fire:
                direction = 1. if num == 1 else -1.
                self.projectiles.append({"x": actor.x, "y": actor.y, "vx": direction * .67, "owner": num})
            actor.last_fire = firing
            if hypot(actor.x - self.orb[0], actor.y - self.orb[1]) < .07:
                actor.score += 1
                self.orb = (.15 + ((actor.score * 17 + num * 11) % 70) / 100,
                            .20 + ((actor.score * 13 + num * 7) % 55) / 100)
                self.event = f"Jugador {num} recogió un núcleo compartido"
                self._event_age = 0
        remaining = []
        for shot in self.projectiles:
            shot["x"] += shot["vx"] * dt
            target = 3 - shot["owner"]
            enemy = self.actors[target]
            if hypot(enemy.x - shot["x"], enemy.y - shot["y"]) < .055:
                if not enemy.shield:
                    enemy.hits += 1
                    self.event = f"Jugador {shot['owner']} alcanzó a Jugador {target}"
                else:
                    self.event = f"Jugador {target} bloqueó el disparo con el escudo"
                self._event_age = 0
            elif .01 <= shot["x"] <= .99:
                remaining.append(shot)
        self.projectiles = remaining[:60]
        return self.snapshot()

    def snapshot(self) -> dict:
        return {
            "type": "world", "frame": self.frame,
            "actors": {str(i): {"x": round(a.x, 5), "y": round(a.y, 5),
                                  "score": a.score, "hits": a.hits, "shield": a.shield,
                                  "connected": a.connected} for i, a in self.actors.items()},
            "shots": [{"x": round(p["x"], 5), "y": round(p["y"], 5),
                       "owner": p["owner"]} for p in self.projectiles],
            "orb": [round(v, 5) for v in self.orb], "event": self.event,
        }
