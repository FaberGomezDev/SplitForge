"""Juego de demostración que recibe exclusivamente el mando asignado por SplitForge.

La separación es real para estas ventanas de prueba; no afecta a juegos externos.
"""
from __future__ import annotations
import math
import random
import time
import tkinter as tk
from .input_stream import InputReader


class DemoGame:
    def __init__(self, player: int):
        self.player = player
        self.pad = InputReader()
        self.root = tk.Tk()
        self.root.title(f"SplitForge DEMO - Jugador {player}")
        self.root.geometry("650x430")
        self.root.minsize(250, 180)
        self.root.configure(bg="#0b1220")
        self.canvas = tk.Canvas(self.root, bg="#10192a", highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)
        self.root.bind("<KeyPress>", self.keydown)
        self.root.bind("<KeyRelease>", self.keyup)
        self.root.protocol("WM_DELETE_WINDOW", self.root.destroy)
        self.keys: set[str] = set()
        self.x, self.y = 0.5, 0.55
        self.shots = 0
        self.last_a = False
        self.last_space = False
        self.last_dash = False
        self.dashes = 0
        self.shield_active = False
        self.projectiles: list[tuple[float, float, float]] = []
        self.t = time.monotonic()
        self.color = "#44d7c7" if player == 1 else "#f59e77"
        random.seed(player)
        self.stars = [(random.random(), random.random(), random.randint(1, 3)) for _ in range(45)]
        self.root.after(50, self.frame)

    def keydown(self, event):
        self.keys.add(event.keysym.lower())

    def keyup(self, event):
        self.keys.discard(event.keysym.lower())

    def frame(self):
        if not self.root.winfo_exists():
            return
        now = time.monotonic()
        dt = min(0.05, now - self.t)
        self.t = now
        w = max(240, self.canvas.winfo_width())
        h = max(150, self.canvas.winfo_height())
        pad = self.pad.read()
        lx = pad.lx if pad else 0.0
        ly = -pad.ly if pad else 0.0
        if not pad:
            lx += float("d" in self.keys or "right" in self.keys) - float("a" in self.keys or "left" in self.keys)
            ly += float("s" in self.keys or "down" in self.keys) - float("w" in self.keys or "up" in self.keys)
        self.x = max(0.05, min(0.95, self.x + lx * dt * 0.50))
        self.y = max(0.12, min(0.92, self.y + ly * dt * 0.50))
        a_down = bool(pad.a_pressed) if pad else ("space" in self.keys)
        dash_down = bool(pad.dash_pressed) if pad else ("shift_l" in self.keys or "shift_r" in self.keys)
        self.shield_active = bool(pad.shield_pressed) if pad else ("e" in self.keys)
        if a_down and not self.last_a:
            self.shots += 1
            self.projectiles.append((self.x, self.y - 0.055, 1.0))
        self.last_a = a_down
        if dash_down and not self.last_dash:
            self.dashes += 1
            # La nave avanza en su rumbo horizontal o hacia arriba.
            advance_x = lx if abs(lx) > 0.15 else 0.0
            advance_y = ly if abs(ly) > 0.15 else -1.0
            scale = max(1.0, math.hypot(advance_x, advance_y))
            self.x = max(0.05, min(0.95, self.x + advance_x * 0.14 / scale))
            self.y = max(0.12, min(0.92, self.y + advance_y * 0.14 / scale))
        self.last_dash = dash_down
        self.projectiles = [(x, y - dt * 0.75, life - dt)
                            for x, y, life in self.projectiles if y > -0.03 and life > 0.0]
        c = self.canvas
        c.delete("all")
        for px, py, r in self.stars:
            c.create_oval(px*w, py*h, px*w+r, py*h+r, fill="#31425a", outline="")
        for gx in range(0, w, 60):
            c.create_line(gx, 0, gx, h, fill="#1b2c43")
        for gy in range(0, h, 60):
            c.create_line(0, gy, w, gy, fill="#1b2c43")
        for bx, by, _life in self.projectiles:
            c.create_line(bx*w, by*h, bx*w, by*h-19, fill="#ffd16a", width=4)
        cx, cy = self.x * w, self.y * h
        radius = 24 + math.sin(now * 3) * 3
        if self.shield_active:
            c.create_oval(cx-39, cy-43, cx+39, cy+43, outline="#78bfff", width=4)
        c.create_oval(cx-radius-9, cy-radius-9, cx+radius+9, cy+radius+9, outline=self.color, width=1)
        c.create_polygon(cx,cy-25,cx-20,cy+17,cx,cy+9,cx+20,cy+17,fill=self.color,outline="#e9f1ff",width=2)
        c.create_text(14, 13, anchor="nw", fill="#ffffff", font=("Segoe UI", 17, "bold"), text=f"JUGADOR {self.player}")
        if pad:
            status = "Stick: mover | Disparo: Cruz/R2 configurado | Circulo: impulso | Cuadrado: escudo"
        else:
            status = "Sin mando | WASD: mover | ESPACIO: disparar | SHIFT: impulso | E: escudo"
        c.create_text(14, 45, anchor="nw", fill="#a9bcd6", font=("Segoe UI", 10), text=status)
        c.create_text(14, h-14, anchor="sw", fill=self.color, font=("Segoe UI", 11, "bold"),
                      text=f"Disparos: {self.shots}  |  Impulsos: {self.dashes}  |  Escudo: {'ACTIVO' if self.shield_active else 'OFF'}")
        self.root.after(16, self.frame)

    def run(self):
        self.root.mainloop()


def run_demo(player: int):
    DemoGame(player).run()
