"""Interfaz de SplitForge: orquestación de procesos y mandos SDL."""
from __future__ import annotations
import json
import logging
import os
import time
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

from .gamepads import ControllerManager, PadInfo, validate_mapping, new_button_presses
from .session import SessionManager
from .steam_library import scan_steam
from .steam_preflight import evaluate, report_json, steam_uri
from .steam_windows import inspect_game_windows, verify_pair
from .windows import WindowInfo, enumerate_windows

BG = "#0a1322"
CARD = "#142339"
TEXT = "#edf4ff"
MUTED = "#9eb0c8"
ACCENT = "#30c7b3"


def app_dir() -> Path:
    base = os.getenv("LOCALAPPDATA")
    path = Path(base) / "SplitForge" if base else Path.home() / ".splitforge"
    path.mkdir(parents=True, exist_ok=True)
    return path


class SplitForgeApp:
    def __init__(self, script: str):
        self.script = script
        self.data_dir = app_dir()
        self.settings_path = self.data_dir / "settings.json"
        self.logger = logging.getLogger("splitforge")
        self.logger.setLevel(logging.INFO)
        self.logger.propagate = False
        if not self.logger.handlers:
            handler = logging.FileHandler(self.data_dir / "splitforge.log", encoding="utf-8")
            handler.setFormatter(logging.Formatter("%(asctime)s %(levelname)s: %(message)s"))
            self.logger.addHandler(handler)
        self.settings = self._load_settings()
        self.root = tk.Tk()
        self.root.title("SplitForge 0.6.0 • Laboratorio de pantalla dividida")
        self.root.geometry("970x825")
        self.root.minsize(830, 690)
        self.root.configure(bg=BG)
        self.pad = ControllerManager(self.settings.get("controller_mappings", {}),
                                     input_mode=self.settings.get("input_mode", "auto"))
        self.pad_labels: dict[str, int] = {}
        self._last_slow_tick = 0.0
        self._last_pad_tick = 0.0
        self._last_world_tick = time.monotonic()
        self.shared_input_status = tk.StringVar(value="P1: pendiente\nP2: pendiente")
        self.steam_summary = tk.StringVar(value="Escanea tu biblioteca Steam; ningún juego tiene aún un adaptador validado.")
        self.steam_detail = tk.StringVar(value="Selecciona un juego para ver el diagnóstico.")
        self.game_window_title = tk.StringVar(value="Selecciona un juego instalado para inspeccionar las ventanas abiertas.")
        self.game_window_choice = tk.StringVar(value="")
        self._game_window_options = {}
        self._game_window_report = None
        self._game_window_game = None
        self.input_mode_var = tk.StringVar(value=self.pad.input_mode)
        self.session = SessionManager(self.log)
        self.orientation = tk.StringVar(value=self.settings.get("orientation", "vertical"))
        self.gutter = tk.IntVar(value=self.settings.get("gutter", 4))
        self.exe = tk.StringVar(value=self.settings.get("executable", ""))
        self.args_1 = tk.StringVar(value=self.settings.get("args_1", ""))
        self.args_2 = tk.StringVar(value=self.settings.get("args_2", ""))
        self.player_1 = tk.StringVar()
        self.player_2 = tk.StringVar()
        self.status = tk.StringVar(value="Listo para iniciar")
        self.controllers = tk.StringVar(value="Buscando mandos de Windows…")
        self.pad_1 = tk.StringVar(value="")
        self.pad_2 = tk.StringVar(value="")
        self.selected_pad = tk.StringVar(value="")
        self.pad_report = tk.StringVar(value="Conecta un mando y mueve el stick para probarlo.")
        self.map_x = tk.StringVar(value="0")
        self.map_y = tk.StringVar(value="1")
        self.map_button = tk.StringVar(value="0")
        self.map_dash = tk.StringVar(value="1")
        self.map_shield = tk.StringVar(value="2")
        self.capture_status = tk.StringVar(value="Pulsa 'Aprender' y después el botón de tu mando PS4.")
        self._capture_action = None
        self._capture_device_id = None
        self._capture_previous: set[int] = set()
        self._capture_axis_baseline: list[float] = []
        self._fire_axis: int | None = None
        self._fire_rest = 0.0
        self._fire_direction = 1
        self.invert_x = tk.BooleanVar(value=False)
        self.invert_y = tk.BooleanVar(value=False)
        self.visible_windows: dict[str, WindowInfo] = {}
        self._build_style()
        self._build_ui()
        self._refresh_windows()
        self._tick()
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        self.log("SplitForge 0.6.0: mundo compartido y evaluador conservador de instalaciones Steam.")

    def _build_style(self):
        style = ttk.Style(self.root)
        style.theme_use("clam")
        style.configure("TFrame", background=BG)
        style.configure("Panel.TFrame", background=CARD)
        style.configure("TLabel", foreground=TEXT, background=BG, font=("Segoe UI", 10))
        style.configure("Secondary.TLabel", foreground=MUTED, background=BG, font=("Segoe UI", 10))
        style.configure("Panel.TLabel", foreground=TEXT, background=CARD, font=("Segoe UI", 10))
        style.configure("TButton", padding=(12, 9), background="#243b54", foreground=TEXT, font=("Segoe UI", 10))
        style.map("TButton", background=[("active", "#34546d"), ("disabled", "#1c2d40")], foreground=[("disabled", "#7a8999")])
        style.configure("Primary.TButton", padding=(14, 11), background="#168f87", foreground="white", font=("Segoe UI", 11, "bold"))
        style.map("Primary.TButton", background=[("active", "#1bafa0")])
        style.configure("TEntry", fieldbackground="#eaf2fb", padding=7)
        style.configure("TCombobox", fieldbackground="#eaf2fb", padding=6)
        style.configure("TRadiobutton", background=BG, foreground=TEXT, font=("Segoe UI", 10))
        style.map("TRadiobutton", background=[("active", BG)], foreground=[("active", ACCENT)])
        style.configure("TNotebook", background=BG, borderwidth=0)
        style.configure("TNotebook.Tab", background="#1c3049", foreground=MUTED, padding=(16, 10))
        style.map("TNotebook.Tab", background=[("selected", CARD)], foreground=[("selected", "white")])

    def _build_ui(self):
        parent = ttk.Frame(self.root, padding=20)
        parent.pack(fill="both", expand=True)
        header = ttk.Frame(parent)
        header.pack(fill="x", pady=(0, 14))
        tk.Label(header, text="⚡  SplitForge", fg=ACCENT, bg=BG, font=("Segoe UI", 24, "bold")).pack(anchor="w")
        tk.Label(header, text="PROTOTIPO 0.6.0  ·  Mundo local compartido  ·  Windows 10/11",
                 fg=MUTED, bg=BG, font=("Segoe UI", 10)).pack(anchor="w", pady=(3, 0))
        ttk.Label(parent, textvariable=self.controllers, style="Secondary.TLabel").pack(anchor="w", pady=(0, 10))

        self.tabs = ttk.Notebook(parent)
        self.tabs.pack(fill="both", expand=True)
        # Los controles de calibración necesitan desplazamiento vertical.
        # Sin scroll el botón Guardar quedaba fuera de vista a 720/768 px.
        controls_host = ttk.Frame(self.tabs)
        controls_canvas = tk.Canvas(controls_host, background=BG, highlightthickness=0)
        controls_scroll = ttk.Scrollbar(controls_host, orient="vertical", command=controls_canvas.yview)
        controls_canvas.configure(yscrollcommand=controls_scroll.set)
        controls_scroll.pack(side="right", fill="y")
        controls_canvas.pack(side="left", fill="both", expand=True)
        controls = ttk.Frame(controls_canvas, padding=16)
        controls_window = controls_canvas.create_window((0, 0), window=controls, anchor="nw")
        controls.bind("<Configure>", lambda event: controls_canvas.configure(
            scrollregion=controls_canvas.bbox("all")))
        controls_canvas.bind("<Configure>", lambda event: controls_canvas.itemconfigure(
            controls_window, width=event.width))
        self._controls_canvas = controls_canvas
        demo = ttk.Frame(self.tabs, padding=16)
        real = ttk.Frame(self.tabs, padding=16)
        attach = ttk.Frame(self.tabs, padding=16)
        steam = ttk.Frame(self.tabs, padding=16)
        sessions_steam = ttk.Frame(self.tabs, padding=16)
        self.tabs.add(controls_host, text="01  CONTROLES")
        self.root.bind_all("<MouseWheel>", lambda event: controls_canvas.yview_scroll(
            -int(event.delta / 120), "units") if self.tabs.select() == str(controls_host) else None)
        self.tabs.add(demo, text="02  MUNDO COMPARTIDO")
        self.tabs.add(real, text="03  EJECUTABLES")
        self.tabs.add(attach, text="04  VENTANAS ABIERTAS")
        self.tabs.add(steam, text="05  STEAM + DIAGNÓSTICO")
        self.tabs.add(sessions_steam, text="06  VENTANAS STEAM")
        self._controls_tab(controls)
        self._demo_tab(demo)
        self._real_tab(real)
        self._attach_tab(attach)
        self._steam_tab(steam)
        self._steam_windows_tab(sessions_steam)

        options = ttk.Frame(parent)
        options.pack(fill="x", pady=(12, 4))
        ttk.Label(options, text="Distribución:").pack(side="left")
        ttk.Radiobutton(options, text="Lado a lado", variable=self.orientation, value="vertical").pack(side="left", padx=(10, 14))
        ttk.Radiobutton(options, text="Arriba / abajo", variable=self.orientation, value="horizontal").pack(side="left")
        ttk.Label(options, text="Separación:").pack(side="left", padx=(20, 8))
        ttk.Combobox(options, textvariable=self.gutter, values=[0, 2, 4, 8, 12, 16], width=4, state="readonly").pack(side="left")

        actions = ttk.Frame(parent)
        actions.pack(fill="x", pady=(8, 6))
        ttk.Button(actions, text="Restaurar / detener sesión", command=self._stop).pack(side="right")
        ttk.Label(actions, textvariable=self.status, style="Secondary.TLabel").pack(side="left")
        tk.Label(parent, text="REGISTRO DE SESIÓN", bg=BG, fg=MUTED, font=("Segoe UI", 9, "bold")).pack(anchor="w", pady=(5, 5))
        self.log_box = tk.Text(parent, height=5, bg="#07101e", fg="#c5d6eb", insertbackground="white",
                               relief="flat", borderwidth=0, padx=12, pady=8, state="disabled",
                               font=("Consolas", 9), wrap="word")
        self.log_box.pack(fill="x")

    def _controls_tab(self, pane):
        tk.Label(pane, text="Detección de mandos: Xbox, PS4 y genéricos", font=("Segoe UI", 15, "bold"),
                 fg=TEXT, bg=BG).pack(anchor="w", pady=(4, 8))
        tk.Label(pane, text="Usamos SDL cuando está instalado o WinMM nativo de Windows sin descargas para reconocer "
                          "mandos compatibles. Pulsa botones y mueve el stick para comprobar respuesta.",
                 fg=MUTED, bg=BG, wraplength=770, justify="left").pack(anchor="w", pady=(0, 12))
        tools = ttk.Frame(pane)
        tools.pack(fill="x", pady=(0, 9))
        ttk.Button(tools, text="↻  Volver a buscar mandos", command=self._rescan_pads).pack(side="left")
        ttk.Button(tools, text="🔎  Diagnóstico SDL / Bluetooth (20 s)",
                   command=self._open_pad_diagnostic).pack(side="left", padx=(9, 0))
        row_mode = ttk.Frame(pane)
        row_mode.pack(fill="x", pady=(4, 10))
        ttk.Label(row_mode, text="Lector SDL:").pack(side="left", padx=(0, 8))
        ttk.Combobox(row_mode, textvariable=self.input_mode_var, state="readonly", width=16,
                     values=("auto", "joystick", "controller")).pack(side="left")
        ttk.Button(row_mode, text="Aplicar modo", command=self._apply_input_mode).pack(side="left", padx=(8, 0))
        ttk.Label(row_mode, text="auto = PS4 directo / Xbox estándar",
                  style="Secondary.TLabel").pack(side="left", padx=(14, 0))
        ttk.Label(pane, text="Mando a inspeccionar").pack(anchor="w")
        self.pad_select = ttk.Combobox(pane, textvariable=self.selected_pad, state="readonly", width=72)
        self.pad_select.pack(fill="x", pady=(4, 8))
        self.pad_select.bind("<<ComboboxSelected>>", self._load_controller_mapping)
        tk.Label(pane, textvariable=self.pad_report, bg=BG, fg=ACCENT, font=("Consolas", 10),
                 justify="left", wraplength=760).pack(anchor="w", pady=(0, 12))
        tk.Label(pane, text="Configurar botones PS4 o controles genéricos", font=("Segoe UI", 11, "bold"),
                 fg=TEXT, bg=BG).pack(anchor="w", pady=(4, 8))
        tk.Label(pane, text="Para cada acción pulsa Aprender y después el botón físico del mando seleccionado. "
                          "Acepta Cruz o R2 (gatillo analógico) para disparar. Los mandos del mismo modelo comparten el perfil.",
                 bg=BG, fg=MUTED, wraplength=770, justify="left").pack(anchor="w", pady=(0, 10))
        group = ttk.Frame(pane)
        group.pack(anchor="w")
        ttk.Label(group, text="Eje X").grid(row=0, column=0, sticky="w")
        ttk.Label(group, text="Eje Y").grid(row=0, column=1, sticky="w", padx=(10, 0))
        ttk.Combobox(group, textvariable=self.map_x, values=list(range(20)), width=6, state="readonly").grid(row=1, column=0, sticky="w")
        ttk.Combobox(group, textvariable=self.map_y, values=list(range(20)), width=6, state="readonly").grid(row=1, column=1, sticky="w", padx=(10, 0))
        button_group = ttk.Frame(pane)
        button_group.pack(anchor="w", pady=(10, 2))
        for col, (name, key, variable) in enumerate((
            ("Disparo (Cruz/R2)", "button", self.map_button),
            ("Impulso (Círculo)", "button_dash", self.map_dash),
            ("Escudo (Cuadrado)", "button_shield", self.map_shield),
        )):
            cell = ttk.Frame(button_group)
            cell.grid(row=0, column=col, padx=(0, 16), sticky="w")
            ttk.Label(cell, text=name).pack(anchor="w")
            inner = ttk.Frame(cell)
            inner.pack(anchor="w", pady=(4, 0))
            ttk.Combobox(inner, textvariable=variable, values=[""] + list(range(32)),
                         width=5, state="readonly").pack(side="left")
            ttk.Button(inner, text="Aprender", command=lambda action=key: self._learn_button(action)).pack(side="left", padx=(6, 0))
        tk.Label(pane, textvariable=self.capture_status, bg=BG, fg=ACCENT,
                 wraplength=765, justify="left", font=("Segoe UI", 10)).pack(anchor="w", pady=(8, 0))
        options = ttk.Frame(pane)
        options.pack(anchor="w", pady=(9, 2))
        ttk.Checkbutton(options, text="Invertir X", variable=self.invert_x).pack(side="left", padx=(0, 25))
        ttk.Checkbutton(options, text="Invertir Y", variable=self.invert_y).pack(side="left")
        ttk.Button(pane, text="Guardar configuración de controles", command=self._save_controller_mapping).pack(anchor="w", pady=(8, 0))

    def _open_pad_diagnostic(self):
        import subprocess
        import sys
        script = Path(self.script).resolve().parent / "diagnostico_mandos.py"
        try:
            subprocess.Popen([sys.executable, str(script), "--seconds", "20"],
                             cwd=str(script.parent),
                             creationflags=getattr(subprocess, "CREATE_NEW_CONSOLE", 0))
            self.log("Diagnóstico abierto: mueve los dos mandos durante 20 segundos.")
        except OSError as exc:
            self._error(exc)

    def _apply_input_mode(self):
        try:
            self.pad.set_input_mode(self.input_mode_var.get())
            self._sync_pad_lists()
            self._save_settings()
            self.log(f"Lector de mandos cambiado a {self.pad.input_mode}.")
        except Exception as exc:
            self._error(exc)

    def _rescan_pads(self):
        try:
            self.pad.refresh()
            self._sync_pad_lists()
        except Exception as exc:
            self._error(exc)

    def _sync_pad_lists(self):
        old_ids = (self.pad_labels.get(self.pad_1.get()),
                   self.pad_labels.get(self.pad_2.get()),
                   self.pad_labels.get(self.selected_pad.get()))
        infos = self.pad.devices()
        names = [f"{p.name}  |  {p.kind}  |  ID {p.instance_id}" for p in infos]
        new_labels = dict(zip(names, (p.instance_id for p in infos)))
        if new_labels == self.pad_labels:
            return
        self.pad_labels = new_labels
        self.demo_pad_1["values"] = names
        self.demo_pad_2["values"] = names
        self.pad_select["values"] = names
        def match(wanted):
            return next((name for name, id_ in new_labels.items() if id_ == wanted), "")
        self.pad_1.set(match(old_ids[0]) or (names[0] if names else ""))
        self.pad_2.set(match(old_ids[1]) or (names[1] if len(names) > 1 else ""))
        self.selected_pad.set(match(old_ids[2]) or (names[0] if names else ""))
        self._load_controller_mapping()

    def _selected_info(self) -> PadInfo | None:
        wanted = self.pad_labels.get(self.selected_pad.get())
        return next((info for info in self.pad.devices() if info.instance_id == wanted), None)

    def _load_controller_mapping(self, _event=None):
        info = self._selected_info()
        if info is None:
            return
        m = self.pad.mappings.get(info.guid, {})
        self.map_x.set(str(m.get("axis_x", 0)))
        self.map_y.set(str(m.get("axis_y", 1)))
        self.map_button.set(str(m.get("button", 0)))
        self._fire_axis = m.get("fire_axis")
        self._fire_rest = m.get("fire_rest", 0.0)
        self._fire_direction = m.get("fire_direction", 1)
        self.capture_status.set("Disparo adicional R2: " +
                                (f"eje {self._fire_axis} calibrado" if self._fire_axis is not None
                                 else "no configurado; pulsa Aprender y después Cruz o R2"))
        self.map_dash.set(str(m.get("button_dash", 1 if info.buttons > 1 else "")))
        self.map_shield.set(str(m.get("button_shield", 2 if info.buttons > 2 else "")))
        self.invert_x.set(bool(m.get("invert_x", False)))
        self.invert_y.set(bool(m.get("invert_y", False)))

    def _learn_button(self, action: str) -> None:
        info = self._selected_info()
        if info is None:
            messagebox.showwarning("Mandos", "Selecciona primero un mando.")
            return
        if self.pad.read_path(info.instance_id) == "controller":
            messagebox.showinfo("Lector SDL", "Para calibrar botones físicos, pon Lector SDL en joystick "
                                "y pulsa Aplicar modo. En mandos PS4 el modo auto ya usa joystick.")
            return
        if info.buttons == 0:
            messagebox.showwarning("Mandos", "Este dispositivo no publica botones digitales.")
            return
        _axes, pressed = self.pad.raw_axes(info.instance_id)
        self._capture_axis_baseline = list(_axes)
        self._capture_action = action
        self._capture_device_id = info.instance_id
        self._capture_previous = set(pressed)
        label = {"button": "DISPARAR (Cruz)", "button_dash": "IMPULSO (Círculo)",
                 "button_shield": "ESCUDO (Cuadrado)"}[action]
        hint = "También puedes apretar R2 para disparar." if action == "button" else ""
        self.capture_status.set(f"Esperando {label} del mando ID {info.instance_id}. "
                                f"Suelta cualquier botón y púlsalo ahora. {hint}")

    def _capture_tick(self) -> None:
        if not self._capture_action:
            return
        if self.pad_labels.get(self.selected_pad.get()) != self._capture_device_id:
            self.capture_status.set("Se cambió el mando; vuelve a pulsar Aprender.")
            self._capture_action = None
            return
        axes, pressed = self.pad.raw_axes(self._capture_device_id)
        latest = set(pressed)
        fresh = new_button_presses(latest, self._capture_previous)
        self._capture_previous = latest
        if fresh:
            idx = fresh[0]
            target = {"button": self.map_button, "button_dash": self.map_dash,
                      "button_shield": self.map_shield}[self._capture_action]
            target.set(str(idx))
            if self._capture_action == "button":
                self._fire_axis = None  # Recalibrar Cruz anula el gatillo opcional previo.
            self._capture_action = None
            self.capture_status.set(f"Detectado botón físico {idx}. Guardando configuración...")
            self._save_controller_mapping(notify=False)
            return
        if self._capture_action == "button":
            # Algunos PS4 exponen R2 como eje analógico, no como botón.
            # No capturamos los ejes X/Y del stick izquierdo.
            ignored = {int(self.map_x.get()), int(self.map_y.get())}
            for idx, (initial, current) in enumerate(zip(self._capture_axis_baseline, axes)):
                delta = current - initial
                if idx not in ignored and abs(delta) >= .65:
                    self._fire_axis = idx
                    self._fire_rest = initial
                    self._fire_direction = 1 if delta > 0 else -1
                    self._capture_action = None
                    self.capture_status.set(f"Detectado gatillo analógico en eje {idx}. Guardando...")
                    self._save_controller_mapping(notify=False)
                    return

    def _save_controller_mapping(self, notify=True):
        info = self._selected_info()
        if info is None:
            if notify:
                messagebox.showwarning("Mandos", "Selecciona un mando conectado.")
            return
        try:
            mapping = validate_mapping({"axis_x": self.map_x.get(), "axis_y": self.map_y.get(),
                                        "button": self.map_button.get(),
                                        "button_dash": self.map_dash.get(),
                                        "button_shield": self.map_shield.get(),
                                        "fire_axis": self._fire_axis, "fire_rest": self._fire_rest,
                                        "fire_direction": self._fire_direction,
                                        "invert_x": self.invert_x.get(), "invert_y": self.invert_y.get()},
                                        info.axes, info.buttons)
            self.pad.mappings[info.guid] = mapping
            self._save_settings()
            self.log(f"Botones configurados para {info.name}: disparo={mapping['button']}, "
                     f"gatillo_eje={mapping['fire_axis']}, impulso={mapping['button_dash']}, "
                     f"escudo={mapping['button_shield']}.")
            if notify:
                messagebox.showinfo("Mandos", "Configuración guardada. Prueba las acciones en la demostración.")
            else:
                self.capture_status.set("Botón aprendido y guardado. Repite con las demás acciones.")
        except (ValueError, OSError) as exc:
            self.capture_status.set(f"No se guardó: {exc}")
            if notify:
                self._error(exc)

    def _demo_tab(self, pane):
        tk.Label(pane, text="Mundo compartido: 2 perspectivas, 1 misma partida",
                 font=("Segoe UI", 15, "bold"), bg=BG, fg=TEXT).pack(anchor="w", pady=(5, 8))
        tk.Label(pane, text="LABORATORIO LOCAL: dos ventanas independientes muestran simultáneamente "
                            "ambos jugadores, un núcleo recogible y los mismos proyectiles. "
                            "No es todavía un juego de Steam ni redirige controles a otros juegos.",
                 bg=BG, fg=MUTED, justify="left", wraplength=780).pack(anchor="w", pady=(0, 12))
        for line in [
            "1. Asigna PS4 Controller ID 0 al jugador 1 y PS4 Controller ID 1 al jugador 2.",
            "2. Pulsa INICIAR MUNDO COMPARTIDO. Ambos personajes estarán en el MISMO escenario.",
            "3. Stick: mover · Cruz/R2 calibrado: proyectil · Círculo: impulso · Cuadrado: escudo.",
            "4. Si P2 deja de moverse, comprueba abajo su X/Y. Reasigna los mandos sin reiniciar.",
        ]:
            tk.Label(pane, text=line, bg=BG, fg=TEXT, font=("Segoe UI", 10),
                     justify="left", wraplength=780).pack(anchor="w", pady=4)
        ttk.Label(pane, text="Mando del jugador 1").pack(anchor="w", pady=(10, 0))
        self.demo_pad_1 = ttk.Combobox(pane, textvariable=self.pad_1, state="readonly", width=72)
        self.demo_pad_1.pack(fill="x", pady=(3, 8))
        ttk.Label(pane, text="Mando del jugador 2").pack(anchor="w")
        self.demo_pad_2 = ttk.Combobox(pane, textvariable=self.pad_2, state="readonly", width=72)
        self.demo_pad_2.pack(fill="x", pady=(3, 8))
        tools = ttk.Frame(pane)
        tools.pack(fill="x", pady=(9, 8))
        ttk.Button(tools, text="▶  INICIAR MUNDO COMPARTIDO",
                   style="Primary.TButton", command=self._start_shared).pack(side="left")
        ttk.Button(tools, text="Reasignar mandos en vivo",
                   command=self._reassign_shared).pack(side="left", padx=(9, 0))
        tk.Label(pane, text="Señal real que llega al motor (actualización continua):",
                 bg=BG, fg=MUTED, font=("Segoe UI", 10, "bold")).pack(anchor="w", pady=(16, 5))
        tk.Label(pane, textvariable=self.shared_input_status, bg="#081827", fg="#8ae8d4",
                 font=("Consolas", 11), justify="left", padx=10, pady=10,
                 anchor="w").pack(fill="x", pady=(0, 9))
        ttk.Button(pane, text="Iniciar demostración antigua (dos mundos separados)",
                   command=self._start_demo).pack(anchor="w", pady=3)
        tk.Label(pane, text="Esta prueba valida sincronización e independencia de controles. "
                            "Los juegos Steam necesitarán adaptadores específicos y modos de red compatibles.",
                 bg=BG, fg="#e9be97", justify="left", wraplength=780).pack(anchor="w", pady=(10, 0))

    def _start_shared(self):
        try:
            one = self.pad_labels.get(self.pad_1.get())
            two = self.pad_labels.get(self.pad_2.get())
            if one is None or two is None or one == two:
                messagebox.showwarning("Asignación", "Selecciona dos controles diferentes y conectados.")
                return
            self.session.start_shared_demo(self.script, [one, two], *self._options())
            self.status.set("Arrancando sesión local compartida…")
            self._save_settings()
        except Exception as exc:
            self._error(exc)

    def _reassign_shared(self):
        try:
            one = self.pad_labels.get(self.pad_1.get())
            two = self.pad_labels.get(self.pad_2.get())
            self.session.reassign_shared([one, two], {p.instance_id for p in self.pad.devices()})
        except Exception as exc:
            self._error(exc)

    def _steam_tab(self, pane):
        tk.Label(pane, text="Steam: instalaciones y preparación para multijugador local",
                 font=("Segoe UI", 14, "bold"), bg=BG, fg=TEXT).pack(anchor="w", pady=(4, 8))
        tk.Label(pane, text="Este asistente solo INSPECCIONA tus juegos. No habilita dos cuentas, "
                            "no separa mandos dentro de juegos comerciales y NO certifica modo offline. "
                            "El laboratorio de dos jugadores sigue disponible en la pestaña 02.",
                 bg=BG, fg=MUTED, justify="left", wraplength=780).pack(anchor="w", pady=(0, 9))
        tools = ttk.Frame(pane)
        tools.pack(fill="x", pady=(0, 9))
        ttk.Button(tools, text="↻  Escanear Steam", command=self._refresh_steam).pack(side="left")
        ttk.Button(tools, text="Guardar reporte sin datos personales",
                   command=self._export_steam).pack(side="left", padx=(8, 0))
        ttk.Label(pane, textvariable=self.steam_summary, style="Secondary.TLabel").pack(anchor="w", pady=(0, 8))
        tree = ttk.Treeview(pane, columns=("appid", "status"), show="headings", height=7)
        tree.heading("appid", text="AppID")
        tree.heading("status", text="Juego / estado de investigación")
        tree.column("appid", width=90, stretch=False)
        tree.column("status", width=600)
        tree.pack(fill="both", expand=True)
        self.steam_tree = tree
        self._steam_rows = {}
        self.steam_detail.set("Escanea Steam y selecciona un título para revisar sus requisitos.")
        tk.Label(pane, textvariable=self.steam_detail, bg="#091a2b", fg="#d7e5f5",
                 justify="left", anchor="nw", wraplength=755, padx=10, pady=9,
                 font=("Consolas", 9), height=9).pack(fill="x", pady=(9, 8))
        actions = ttk.Frame(pane)
        actions.pack(fill="x", pady=(2, 0))
        ttk.Button(actions, text="Abrir carpeta del juego",
                   command=self._open_steam_folder).pack(side="left")
        ttk.Button(actions, text="Abrir en Steam (solo 1 instancia)",
                   command=self._launch_steam_single).pack(side="left", padx=(8, 0))
        tree.bind("<<TreeviewSelect>>", self._select_steam)

    def _refresh_steam(self):
        try:
            games = scan_steam()
            for row in self.steam_tree.get_children():
                self.steam_tree.delete(row)
            self._steam_rows.clear()
            for game in games:
                assessment = evaluate(game)
                row = self.steam_tree.insert("", "end", values=(
                    game.app_id, f"{game.name} — {assessment.phase}"))
                self._steam_rows[row] = game
            self.steam_summary.set(
                f"{len(games)} instalaciones encontradas. 0 adaptadores de juego verificados en v0.5.")
            self.steam_detail.set("Selecciona un juego para ver el diagnóstico.")
            self.log(f"Steam: {len(games)} títulos enumerados sin escribir en sus carpetas.")
        except Exception as exc:
            self._error(exc)

    def _selected_steam_game(self):
        selection = self.steam_tree.selection()
        return self._steam_rows.get(selection[0]) if selection else None

    def _select_steam(self, _event=None):
        game = self._selected_steam_game()
        if not game:
            return
        p = evaluate(game)
        lines = [
            f"{p.name}  |  AppID {p.app_id}  |  Steam build {p.build_id or 'desconocida'}",
            f"Instalación: {'ENCONTRADA' if p.folder_present else 'NO ENCONTRADA'}"
            f"  |  Perfil: {p.phase}",
            "Dos instancias / controles independientes / offline: NO VERIFICADOS",
            "Ejecutables detectados en raíz: " + (", ".join(p.executables_found[:4]) or "ninguno"),
            "Ejecutable candidato: " + (" / ".join(p.expected_executables) if p.expected_executables else "sin especificar"),
            "OBSERVACIONES:",
        ]
        lines += [" - " + x for x in p.notes[:3]]
        self.steam_detail.set("\n".join(lines))

    def _export_steam(self):
        try:
            games = scan_steam()
            target = filedialog.asksaveasfilename(
                title="Guardar reporte Steam (sin rutas personales)",
                defaultextension=".json", initialfile="SplitForge_Reporte_Steam.json",
                filetypes=[("JSON", "*.json")])
            if not target:
                return
            Path(target).write_text(report_json(games), encoding="utf-8")
            self.log(f"Diagnóstico Steam exportado ({len(games)} entradas).")
            messagebox.showinfo("Reporte guardado", "El reporte NO incluye rutas absolutas, "
                                "cuentas de Steam ni identificadores Bluetooth.")
        except Exception as exc:
            self._error(exc)

    def _open_steam_folder(self):
        game = self._selected_steam_game()
        if game is None:
            messagebox.showwarning("Steam", "Selecciona un juego primero.")
            return
        p = evaluate(game)
        if not p.folder_present or not p.inside_steam_library:
            messagebox.showwarning("Steam", "No hay una carpeta de instalación válida.")
            return
        try:
            os.startfile(game.folder)
        except (OSError, AttributeError) as exc:
            self._error(exc)

    def _launch_steam_single(self):
        game = self._selected_steam_game()
        if game is None:
            messagebox.showwarning("Steam", "Selecciona un juego primero.")
            return
        p = evaluate(game)
        if not p.folder_present:
            messagebox.showwarning("Steam", "El título no tiene una instalación válida.")
            return
        if not messagebox.askyesno(
            "Abrir un juego a través de Steam",
            f"Se solicitará a Steam abrir UNA instancia de {game.name}.\n\n"
            "Esto NO activa pantalla dividida, no garantiza modo offline y puede "
            "requerir una cuenta o un launcher. ¿Continuar?"):
            return
        try:
            os.startfile(steam_uri(game.app_id))
            self.log(f"Solicitud enviada al cliente Steam para AppID {game.app_id} (una instancia).")
        except (OSError, AttributeError, ValueError) as exc:
            self._error(exc)

    def _steam_windows_tab(self, pane):
        tk.Label(pane, text="Dos ventanas de un mismo juego Steam (fase experimental)",
                 font=("Segoe UI", 15, "bold"), fg=TEXT, bg=BG).pack(anchor="w", pady=(4, 8))
        tk.Label(pane, text="Inspecciona procesos que YA estén abiertos desde Steam. "
                            "No inicia una segunda cuenta, no crea multijugador y no redirige mandos. "
                            "Solo organiza DOS ventanas independientes verificadas.",
                 bg=BG, fg=MUTED, wraplength=790, justify="left").pack(anchor="w", pady=(0, 12))
        ttk.Label(pane, text="Juego Steam instalado").pack(anchor="w")
        self.game_window_combo = ttk.Combobox(
            pane, textvariable=self.game_window_choice, state="readonly", width=90)
        self.game_window_combo.pack(fill="x", pady=(5, 10))
        buttons = ttk.Frame(pane)
        buttons.pack(fill="x", pady=(0, 12))
        ttk.Button(buttons, text="1 · Actualizar biblioteca", command=self._load_steam_window_choices).pack(side="left")
        ttk.Button(buttons, text="2 · Analizar ventanas", command=self._scan_game_windows).pack(side="left", padx=(8, 0))
        ttk.Button(buttons, text="3 · Distribuir 2 clientes", style="Primary.TButton",
                   command=self._tile_steam_game).pack(side="left", padx=(8, 0))
        tk.Label(pane, textvariable=self.game_window_title, bg="#091a2b", fg="#d7e5f5",
                 justify="left", anchor="nw", wraplength=795, padx=12, pady=12,
                 font=("Consolas", 10), height=12).pack(fill="x", pady=(4, 13))
        ttk.Button(pane, text="Restaurar posiciones originales (sin cerrar los juegos)",
                   command=self._stop).pack(anchor="w", pady=(0, 10))
        tk.Label(pane, text="Para jugar juntos, ambos clientes deben admitir una conexión LAN o una "
                            "sesión multijugador legítima. Dos ventanas no garantizan dos jugadores "
                            "ni funcionamiento sin Internet. Sin modificar ni inyectar código en Steam.",
                 bg=BG, fg="#eabda1", wraplength=785, justify="left").pack(anchor="w")
        self._load_steam_window_choices()

    def _load_steam_window_choices(self):
        try:
            game_list = scan_steam()
            old_game = self._game_window_options.get(self.game_window_choice.get())
            self._game_window_options = {
                f"{game.name} (AppID {game.app_id})": game for game in game_list
            }
            self.game_window_combo["values"] = list(self._game_window_options)
            previous = next((label for label, game in self._game_window_options.items()
                             if old_game and game.app_id == old_game.app_id), "")
            self.game_window_choice.set(previous)
            self._game_window_report = None
            self._game_window_game = None
            self.game_window_title.set(
                f"Biblioteca: {len(game_list)} instalaciones. Selecciona un juego y pulsa Analizar ventanas.")
        except Exception as exc:
            self._error(exc)

    def _scan_game_windows(self):
        game = self._game_window_options.get(self.game_window_choice.get())
        if game is None:
            messagebox.showwarning("Ventanas Steam", "Selecciona un juego en la lista.")
            return
        try:
            report = inspect_game_windows(game)
            self._game_window_report = report
            self._game_window_game = game
            lines = [
                f"Juego: {game.name} (AppID {game.app_id})",
                f"Estado: {report.state}",
                f"Ventanas asociadas: {len(report.windows)} | Clientes (PID) distintos: {report.distinct_pids}",
                report.detail,
                "",
            ]
            for w in report.windows[:12]:
                lines.append(f"  PID {w.pid} | {w.exe_name} | {w.title[:60]}")
            if not report.windows:
                lines.append("  (Sin ventanas coincidentes; inicia un cliente normalmente desde Steam.)")
            if len(report.windows) > 12:
                lines.append(f"  +{len(report.windows)-12} ventanas omitidas")
            lines.extend(["", "ATENCIÓN: posición solamente. Entrada y partida compartida NO verificadas."])
            self.game_window_title.set("\n".join(lines))
            self.log(f"Ventanas Steam AppID {game.app_id}: {len(report.windows)} ventanas, "
                     f"{report.distinct_pids} PID(s). Estado {report.state}.")
        except Exception as exc:
            self._error(exc)

    def _tile_steam_game(self):
        game, report = self._game_window_game, self._game_window_report
        if game is None or report is None:
            messagebox.showwarning("Ventanas Steam", "Pulsa Analizar ventanas antes de distribuir.")
            return
        selected_game = self._game_window_options.get(self.game_window_choice.get())
        if selected_game is None or game.app_id != selected_game.app_id:
            messagebox.showwarning("Ventanas Steam", "El juego seleccionado cambió. Analiza otra vez.")
            return
        try:
            handles = list(verify_pair(game, report))
            if not messagebox.askyesno(
                "Colocar dos clientes Steam",
                "Hay dos ventanas del MISMO ejecutable en procesos distintos.\n\n"
                "SplitForge modificará su posición y tamaño; no tocará los archivos del juego.\n"
                "NO separará los controles ni conectará a los jugadores.\n\n"
                "¿Distribuir las ventanas en pantalla?"):
                return
            self.session.attach(handles, *self._options())
            self.status.set("Dos ventanas Steam distribuidas · multijugador no verificado")
            self.log(f"Steam AppID {game.app_id}: ventanas asociadas; no se alteran procesos.")
            self._save_settings()
        except Exception as exc:
            self._error(exc)

    def _real_tab(self, pane):
        tk.Label(pane, text="Lanzamiento experimental de 2 procesos", font=("Segoe UI", 15, "bold"),
                 bg=BG, fg=TEXT).pack(anchor="w", pady=(5, 6))
        tk.Label(pane, text="Solo para juegos y programas que admitan varias instancias legal y técnicamente. "
                            "Steam, Rockstar o Xbox pueden impedir abrir una segunda sesión.",
                 bg=BG, fg=MUTED, wraplength=775, justify="left").pack(anchor="w", pady=(0, 15))
        ttk.Label(pane, text="Ruta del ejecutable (.exe)").pack(anchor="w")
        row = ttk.Frame(pane)
        row.pack(fill="x", pady=(4, 12))
        ttk.Entry(row, textvariable=self.exe).pack(side="left", fill="x", expand=True)
        ttk.Button(row, text="Examinar…", command=self._browse).pack(side="left", padx=(8, 0))
        ttk.Label(pane, text="Argumentos opcionales jugador 1").pack(anchor="w")
        ttk.Entry(pane, textvariable=self.args_1).pack(fill="x", pady=(4, 12))
        ttk.Label(pane, text="Argumentos opcionales jugador 2").pack(anchor="w")
        ttk.Entry(pane, textvariable=self.args_2).pack(fill="x", pady=(4, 14))
        ttk.Button(pane, text="▶  LANZAR DOS PROCESOS (EXPERIMENTAL)",
                   style="Primary.TButton", command=self._start_external).pack(anchor="w", pady=5)
        tk.Label(pane, text="No incluye aislamiento de archivos/cuentas, redirección de mandos de otros juegos, "
                            "multijugador forzado ni evasión de DRM/antitrampas. No uses partidas importantes "
                            "para las primeras pruebas.",
                 fg="#eabda1", bg=BG, justify="left", wraplength=760).pack(anchor="w", pady=(17, 0))

    def _attach_tab(self, pane):
        tk.Label(pane, text="Organizar juegos que ya estén abiertos", font=("Segoe UI", 15, "bold"),
                 fg=TEXT, bg=BG).pack(anchor="w", pady=(5, 8))
        tk.Label(pane, text="Puedes iniciar dos ventanas manualmente (por ejemplo desde Steam) y seleccionar "
                            "cada una aquí. Solo cambiamos tamaño y posición; no controlamos su entrada.",
                 bg=BG, fg=MUTED, justify="left", wraplength=775).pack(anchor="w", pady=(0, 16))
        ttk.Button(pane, text="↻  Actualizar ventanas visibles", command=self._refresh_windows).pack(anchor="w", pady=(0, 16))
        ttk.Label(pane, text="Ventana del jugador 1").pack(anchor="w")
        self.combo_1 = ttk.Combobox(pane, textvariable=self.player_1, state="readonly", width=80)
        self.combo_1.pack(fill="x", pady=(4, 12))
        ttk.Label(pane, text="Ventana del jugador 2").pack(anchor="w")
        self.combo_2 = ttk.Combobox(pane, textvariable=self.player_2, state="readonly", width=80)
        self.combo_2.pack(fill="x", pady=(4, 14))
        ttk.Button(pane, text="▦  DISTRIBUIR VENTANAS", style="Primary.TButton",
                   command=self._attach).pack(anchor="w", pady=5)
        tk.Label(pane, text="Recomendado: juegos en modo ventana. Pantalla completa exclusiva y ventanas "
                            "protegidas podrían ignorar el cambio de tamaño.",
                 bg=BG, fg=MUTED, justify="left", wraplength=770).pack(anchor="w", pady=(17, 0))

    def _load_settings(self) -> dict:
        try:
            data = json.loads(self.settings_path.read_text(encoding="utf-8"))
            return data if isinstance(data, dict) else {}
        except (OSError, ValueError):
            return {}

    def _save_settings(self):
        values = {"orientation": self.orientation.get(), "gutter": self.gutter.get(),
                  "executable": self.exe.get(), "args_1": self.args_1.get(), "args_2": self.args_2.get(),
                  "controller_mappings": self.pad.mappings,
                  "input_mode": self.pad.input_mode}
        tmp = self.settings_path.with_suffix(".tmp")
        tmp.write_text(json.dumps(values, indent=2, ensure_ascii=False), encoding="utf-8")
        tmp.replace(self.settings_path)

    def log(self, msg: str):
        self.logger.info(msg)
        # Al iniciar SessionManager aún no existe self.log_box
        if not hasattr(self, "log_box"):
            return
        self.log_box.configure(state="normal")
        self.log_box.insert("end", msg + "\n")
        self.log_box.see("end")
        self.log_box.configure(state="disabled")

    def _browse(self):
        path = filedialog.askopenfilename(title="Seleccionar ejecutable", filetypes=[("Programas Windows", "*.exe")])
        if path:
            self.exe.set(path)

    def _options(self) -> tuple[str, int]:
        return self.orientation.get(), int(self.gutter.get())

    def _start_demo(self):
        try:
            first = self.pad_labels.get(self.pad_1.get())
            second = self.pad_labels.get(self.pad_2.get())
            if first is not None and second is not None and first == second:
                messagebox.showwarning("Mandos duplicados", "Asigna un mando distinto a cada jugador.")
                return
            if first is None or second is None:
                if not messagebox.askyesno(
                    "Faltan mandos", "No hay dos mandos asignados. Puedes probar sin ellos "
                    "con teclado, una ventana con foco cada vez. ¿Iniciar igualmente?"):
                    return
            self.session.start_demo(self.script, [first, second], *self._options())
            self.status.set("Preparando dos ventanas de prueba…")
            self._save_settings()
        except Exception as exc:
            self._error(exc)

    def _start_external(self):
        # No permitir dos lanzamientos directos de un titulo Steam detectado:
        # no existe aislamiento de identidad, ficheros, licencias ni entradas.
        requested = Path(self.exe.get().strip().strip('"')).expanduser()
        try:
            resolved = requested.resolve()
            for game in scan_steam():
                install = Path(game.folder).resolve()
                if resolved == install or install in resolved.parents:
                    messagebox.showwarning(
                        "Juego Steam sin adaptador",
                        "En v0.5 está bloqueado el doble lanzamiento directo de juegos Steam. "
                        "Usa el asistente de la pestaña 05 para revisar compatibilidad. "
                        "El lanzamiento doble no equivale a pantalla dividida y podría "
                        "afectar archivos de partida o una cuenta.")
                    return
        except (OSError, RuntimeError, ValueError):
            pass
        if not messagebox.askyesno(
            "Función experimental",
            "Se intentará abrir DOS veces el ejecutable seleccionado.\n\n"
            "Solo debes hacerlo si su licencia, launcher y multijugador permiten varias instancias.\n"
            "SplitForge NO aislará automáticamente los mandos ni los archivos de guardado.\n\n"
            "¿Quieres continuar?"
        ):
            return
        try:
            self.session.start_external(self.exe.get().strip().strip('"'), self.args_1.get(),
                                        self.args_2.get(), *self._options())
            self.status.set("Buscando ventanas de los procesos lanzados…")
            self._save_settings()
        except Exception as exc:
            self._error(exc)

    def _refresh_windows(self):
        selected_1, selected_2 = self.player_1.get(), self.player_2.get()
        self.visible_windows.clear()
        for w in enumerate_windows():
            if w.pid == os.getpid():
                continue
            label = f"0x{w.handle:X} | {w.title[:74]} [PID {w.pid}]"
            self.visible_windows[label] = w
        names = list(self.visible_windows)
        self.combo_1["values"] = names
        self.combo_2["values"] = names
        if selected_1 in self.visible_windows:
            self.player_1.set(selected_1)
        else:
            self.player_1.set("")
        if selected_2 in self.visible_windows:
            self.player_2.set(selected_2)
        else:
            self.player_2.set("")

    def _attach(self):
        one = self.visible_windows.get(self.player_1.get())
        two = self.visible_windows.get(self.player_2.get())
        if not one or not two:
            messagebox.showwarning("Faltan ventanas", "Selecciona las dos ventanas que quieras distribuir.")
            return
        try:
            self.session.attach([one.handle, two.handle], *self._options())
            self.status.set("Dos ventanas distribuidas")
            self._save_settings()
        except Exception as exc:
            self._error(exc)

    def _stop(self):
        self.session.stop()
        self.status.set("Sesión detenida")

    def _error(self, exc: Exception):
        self.log(f"ERROR: {exc}")
        messagebox.showerror("SplitForge", str(exc))

    def _tick(self):
        try:
            states = self.pad.poll()
            self._capture_tick()
            self.session.route_demo_inputs(states)
            now = time.monotonic()
            world_dt = min(.05, max(0., now - self._last_world_tick))
            self._last_world_tick = now
            self.session.route_shared_inputs(states, world_dt)
            if now - self._last_pad_tick > 0.07:
                self._last_pad_tick = now
                self._sync_pad_lists()
                active = self.session.active
                if active and active.kind == "shared_demo":
                    self.shared_input_status.set("\n".join(active.last_input_status))
                else:
                    labels = []
                    for i, label in enumerate((self.pad_1.get(), self.pad_2.get()), 1):
                        device_id = self.pad_labels.get(label)
                        sample = states.get(device_id)
                        labels.append(f"P{i} (ID {device_id}): X={sample.lx:+.2f} Y={sample.ly:+.2f} "
                                      f"D={'SI' if sample.a_pressed else 'NO'}" if sample else
                                      f"P{i} (ID {device_id}): SIN LECTURA")
                    self.shared_input_status.set("\n".join(labels))
                selected = self._selected_info()
                if selected:
                    axes, pressed = self.pad.raw_axes(selected.instance_id)
                    sample = states.get(selected.instance_id)
                    self.pad_report.set(
                        f"{selected.name} | Lectura: {self.pad.read_path(selected.instance_id)}\n"
                        f"Ejes crudos: {axes}    Botones presionados: {pressed}\n"
                        f"Movimiento: X={sample.lx:+.2f} Y={sample.ly:+.2f} "
                        f"Disparo={'SI' if sample.a_pressed else 'NO'} "
                        f"Impulso={'SI' if sample.dash_pressed else 'NO'} "
                        f"Escudo={'SI' if sample.shield_pressed else 'NO'}" if sample else
                        f"{selected.name}: sin lectura. Comprueba el modo SDL y usa Escanear_Movimiento.bat.")
                else:
                    self.pad_report.set("No hay mandos detectados. Comprueba joy.cpl y pulsa Volver a buscar.")
                infos = self.pad.devices()
                status = f"Mandos ({self.pad.backend_name}) detectados: {len(infos)}  ·  " + (
                    "  |  ".join(f"{i.name} (ID {i.instance_id})" for i in infos)
                    if infos else "ninguno: ejecuta Diagnóstico SDL / Bluetooth")
                if getattr(self.pad, "warning", ""):
                    status += " · " + self.pad.warning
                self.controllers.set(status)
            if now - self._last_slow_tick > 0.55:
                self._last_slow_tick = now
                self.session.tick()
                active = self.session.active
                if active and not active.pending and len(active.window_handles) == 2:
                    self.status.set("Sesión activa · 2 ventanas organizadas")
                elif active and not active.pending and not active.window_handles:
                    self.status.set("Sin detección automática · prueba selección manual")
        except Exception as exc:
            self.log(f"Error de supervisión: {exc}")
        finally:
            # 60 Hz para enviar estados a las ventanas de prueba con baja latencia.
            self.root.after(16, self._tick)

    def _on_close(self):
        self.session.stop()
        try:
            self._save_settings()
        except OSError:
            pass
        self.pad.close()
        self.root.destroy()

    def run(self):
        self.root.mainloop()


def run_app(script: str):
    SplitForgeApp(script).run()
