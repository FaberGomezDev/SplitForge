"""Dos ventanas muestran el MISMO mundo: sincronización local sin red."""
from __future__ import annotations
import math
import tkinter as tk
import time
from .shared_io import WorldReader

COLORS = {1: "#5ce0ca", 2: "#ffac83"}


class SharedDemo:
    def __init__(self, player: int):
        self.player = player
        self.reader = WorldReader()
        self.root = tk.Tk()
        self.root.title(f"SplitForge MUNDO COMPARTIDO - Jugador {player}")
        self.root.geometry("660x450")
        self.root.minsize(250, 180)
        self.root.configure(bg="#081322")
        self.canvas = tk.Canvas(self.root, bg="#102138", highlightthickness=0)
        self.canvas.pack(fill="both", expand=True)
        self.last_frame = -1
        self.root.after(16, self.frame)

    def frame(self):
        if not self.root.winfo_exists():
            return
        s = self.reader.read()
        c = self.canvas
        w = max(250, c.winfo_width())
        h = max(180, c.winfo_height())
        c.delete("all")
        for i in range(0, w, max(25, w // 14)):
            c.create_line(i, 0, i, h, fill="#1d3550")
        for j in range(0, h, max(25, h // 8)):
            c.create_line(0, j, w, j, fill="#1d3550")
        if not s:
            c.create_text(w / 2, h / 2, text="Esperando sincronización de SplitForge...",
                          fill="#dceaf5", font=("Segoe UI", 14, "bold"))
            self.root.after(33, self.frame)
            return
        self.last_frame = s["frame"]
        mx, my = s["orb"]
        r = 10 + 3 * math.sin(time.monotonic() * 5)
        c.create_oval(mx * w-r, my * h-r, mx * w+r, my * h+r, fill="#eac15e", outline="#fff4c6", width=2)
        for shot in s["shots"]:
            x, y = shot["x"] * w, shot["y"] * h
            c.create_oval(x-7, y-4, x+7, y+4, fill=COLORS[shot["owner"]], outline="")
        for number in (1, 2):
            a = s["actors"][str(number)]
            x, y = a["x"] * w, a["y"] * h
            color = COLORS[number]
            if number == self.player:
                c.create_oval(x-26, y-26, x+26, y+26, outline="#ffffff", width=2)
                c.create_text(x, y+36, text="TÚ", fill="#ffffff", font=("Segoe UI", 9, "bold"))
            if a.get("shield"):
                c.create_oval(x-39, y-39, x+39, y+39, outline="#83bdff", width=4)
            c.create_oval(x-20, y-20, x+20, y+20, fill=color, outline="#ffffff", width=2)
            c.create_text(x, y, text=str(number), font=("Segoe UI", 15, "bold"), fill="#0b1930")
            if not a.get("connected"):
                c.create_text(x, y-34, text="SIN MANDO", fill="#ffd2c9", font=("Segoe UI", 9))
        c.create_rectangle(0, 0, w, 67, fill="#09192e", outline="")
        c.create_text(12, 8, anchor="nw", fill="#ffffff", font=("Segoe UI", 15, "bold"),
                      text=f"MUNDO COMPARTIDO  ·  TU PANTALLA: JUGADOR {self.player}")
        a, b = s["actors"]["1"], s["actors"]["2"]
        c.create_text(12, 38, anchor="nw", fill="#bfcbdb", font=("Segoe UI", 10),
                      text=f"P1 núcleos: {a['score']}  impactos: {a['hits']}    |    P2 núcleos: {b['score']}  impactos: {b['hits']}")
        c.create_rectangle(0, h-40, w, h, fill="#09192e", outline="")
        c.create_text(12, h-21, anchor="w", fill="#b5d7e4", font=("Segoe UI", 10),
                      text=s.get("event", ""))
        self.root.after(16, self.frame)

    def run(self):
        self.root.mainloop()


def run_shared_demo(player: int):
    SharedDemo(player).run()
