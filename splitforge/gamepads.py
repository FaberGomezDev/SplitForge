"""Lectura de controles: SDL preferente y Windows nativo sin descargas.

SDL GameController proporciona ejes/botones estandarizados. Si un joystick
HID no tiene mapeo SDL se exponen sus ejes y botones con un perfil editable.
Solo este proceso abre los mandos; las demos reciben entrada por IPC.
"""
from __future__ import annotations

from dataclasses import dataclass
import os
import time


@dataclass(frozen=True)
class PadInfo:
    instance_id: int
    name: str
    guid: str
    kind: str
    axes: int
    buttons: int


@dataclass(frozen=True)
class PadState:
    instance_id: int
    lx: float = 0.0
    ly: float = 0.0  # Y positiva = arriba, como XInput
    a_pressed: bool = False
    connected: bool = True
    dash_pressed: bool = False
    shield_pressed: bool = False


def normalize_axis(value: float, deadzone: float = 0.14) -> float:
    """Normalización radial simplificada con zona muerta por eje."""
    value = max(-1.0, min(1.0, float(value)))
    if abs(value) <= deadzone:
        return 0.0
    return round((abs(value) - deadzone) / (1 - deadzone) * (1 if value > 0 else -1), 4)


def analog_trigger_pressed(value: float, rest: float, direction: int, threshold: float = .60) -> bool:
    """Disparo de gatillo calibrado contra el nivel en reposo, no un índice fijo."""
    return (float(value) - float(rest)) * int(direction) >= threshold


def new_button_presses(current: set[int], previous: set[int]) -> list[int]:
    """Solo flancos de subida; útil para aprender botones sin falsos positivos."""
    return sorted(current - previous)


def choose_read_path(info: PadInfo, has_standard_controller: bool, mode: str = "auto") -> str:
    """Elige el lector efectivo (no confundir deteccion con recepcion de datos).

    PS4/DualShock emplea SDL Joystick directo para no perder ejes.
    El índice del botón Cruz NO está garantizado y se puede calibrar desde UI. Xbox puede usar el mapeo SDL.
    El modo manual ayuda a verificar dispositivos no estandarizados.
    """
    if mode not in ("auto", "joystick", "controller"):
        raise ValueError(f"Modo de entrada no valido: {mode}")
    if mode == "joystick":
        return "joystick"
    if mode == "controller":
        return "controller" if has_standard_controller else "joystick"
    name = info.name.lower()
    if any(token in name for token in ("ps4", "dualshock", "wireless controller", "playstation")):
        return "joystick"
    return "controller" if has_standard_controller else "joystick"


def validate_mapping(mapping: dict, axes: int, buttons: int) -> dict:
    """Descarta entradas erróneas antes de leer HID y evita índices fuera de rango."""
    a = int(mapping.get("axis_x", 0))
    b = int(mapping.get("axis_y", 1))
    action = int(mapping.get("button", 0))
    if not 0 <= a < axes or not 0 <= b < axes:
        raise ValueError(f"Los ejes deben estar entre 0 y {axes - 1}.")
    if not 0 <= action < buttons:
        raise ValueError(f"El botón debe estar entre 0 y {buttons - 1}.")
    # Acciones adicionales son opcionales: controladores pequeños pueden
    # tener un solo botón. Ningún índice inválido debe romper el movimiento.
    extras = {}
    for key, default in (("button_dash", 1), ("button_shield", 2)):
        raw = mapping.get(key, default if default < buttons else None)
        if raw is None or raw == "":
            extras[key] = None
        else:
            value = int(raw)
            if not 0 <= value < buttons:
                raise ValueError(f"{key}: botón fuera del rango 0..{buttons - 1}")
            extras[key] = value
    fire_axis = mapping.get("fire_axis", None)
    fire_rest, fire_direction = 0.0, 1
    if fire_axis not in (None, ""):
        fire_axis = int(fire_axis)
        if not 0 <= fire_axis < axes:
            raise ValueError(f"Eje de disparo fuera del rango 0..{axes - 1}")
        fire_rest = float(mapping.get("fire_rest", 0.0))
        fire_direction = int(mapping.get("fire_direction", 1))
        if not -1 <= fire_rest <= 1 or fire_direction not in (-1, 1):
            raise ValueError("Calibración del gatillo inválida")
    else:
        fire_axis = None
    return {"axis_x": a, "axis_y": b, "button": action,
            "invert_x": bool(mapping.get("invert_x", False)),
            "invert_y": bool(mapping.get("invert_y", False)),
            "fire_axis": fire_axis, "fire_rest": fire_rest,
            "fire_direction": fire_direction, **extras}


class ControllerManager:
    """Dispositivos abiertos una sola vez con SDL, por instance_id de SDL.

    `poll` y `refresh` se llaman desde el hilo de UI, sin lecturas concurrentes
    del mismo dispositivo. El identificador es estable mientras esté conectado,
    pero puede cambiar tras reconectarlo.
    """

    backend_name = "SDL2 (pygame-ce)"

    def __init__(self, mappings: dict | None = None, input_mode: str = "auto"):
        self.input_mode = input_mode if input_mode in ("auto", "joystick", "controller") else "auto"
        self._fallback = None
        self._winmm = None
        self.warning = ""
        os.environ.setdefault("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS", "1")
        # No forzar el controlador de video "dummy" bajo Windows.
        # Utilizar el backend nativo y una ventana SDL oculta para actualizar
        # el estado de entrada sin modificar el foco de la interfaz Tk.
        try:
            import pygame
        except (ImportError, OSError, RuntimeError) as exc:
            self._use_windows_fallback(mappings, f"pygame-ce no se pudo cargar: {exc}")
            return
        self.pg = pygame
        self.controller_api = None
        try:
            pygame.display.init()
            pygame.display.set_mode((1, 1), pygame.HIDDEN)
            pygame.joystick.init()
        except (pygame.error, OSError) as exc:
            self._use_windows_fallback(mappings, f"SDL no pudo iniciar joystick/video: {exc}")
            return
        # La API _sdl2.controller es experimental: su fallo NO debe inutilizar
        # pygame.joystick, que sí reconoce mandos HID genéricos / DS4.
        try:
            from pygame._sdl2 import controller
            controller.init()
            self.controller_api = controller
        except (ImportError, AttributeError, pygame.error, OSError) as exc:
            self.warning = f"Sin mapeo GameController, se usará SDL Joystick: {exc}"
            self.backend_name = "SDL2 Joystick (sin GameController)"
        self.mappings = mappings if isinstance(mappings, dict) else {}
        self._devices: dict[int, tuple[PadInfo, object, object | None]] = {}
        self._last_scan = 0.0
        # WinMM es un respaldo SOLO cuando SDL no informa ningún joystick.
        # Nunca mezcla ambos para no duplicar un mismo mando.
        if os.name == "nt":
            try:
                from .winmm_gamepads import WinMMControllerManager
                self._winmm = WinMMControllerManager(self.mappings)
            except (OSError, RuntimeError) as exc:
                self.warning += f" Respaldo WinMM indisponible: {exc}"
        self.refresh()

    def _use_windows_fallback(self, mappings: dict | None, reason: str):
        from .winmm_gamepads import WinMMControllerManager
        self.warning = reason
        self._fallback = WinMMControllerManager(mappings)
        self.backend_name = self._fallback.backend_name + " (SDL no disponible)"
        self.mappings = self._fallback.mappings

    def refresh(self):
        if getattr(self, "_fallback", None) is not None:
            return self._fallback.refresh()
        self.pg.event.pump()
        found: dict[int, tuple[PadInfo, object, object | None]] = {}
        for index in range(self.pg.joystick.get_count()):
            joy = None
            ctrl = None
            try:
                joy = self.pg.joystick.Joystick(index)
                if not joy.get_init():
                    joy.init()
                instance_id = joy.get_instance_id()
                if instance_id in self._devices:
                    found[instance_id] = self._devices[instance_id]
                    # La instancia ya tiene su dispositivo abierto.
                    continue
                is_standard = False
                if self.controller_api is not None:
                    try:
                        is_standard = bool(self.controller_api.is_controller(index))
                        if is_standard and choose_read_path(
                            PadInfo(instance_id, joy.get_name(), joy.get_guid(),
                                    "SDL estándar (PS4/Xbox/otros)",
                                    joy.get_numaxes(), joy.get_numbuttons()),
                            True, self.input_mode) == "controller":
                            ctrl = self.controller_api.Controller(index)
                    except (self.pg.error, OSError, ValueError):
                        # El dispositivo puede seguir sirviendo como joystick HID.
                        is_standard = False
                        ctrl = None
                info = PadInfo(instance_id=instance_id, name=joy.get_name(),
                               guid=joy.get_guid(),
                               kind="SDL estándar (PS4/Xbox/otros)" if is_standard else "HID genérico",
                               axes=joy.get_numaxes(), buttons=joy.get_numbuttons())
                found[instance_id] = (info, joy, ctrl)
            except (self.pg.error, ValueError, OSError):
                if ctrl is not None:
                    try:
                        ctrl.quit()
                    except Exception:
                        pass
        for instance_id, (_info, _joy, ctrl) in self._devices.items():
            if instance_id not in found and ctrl is not None:
                try:
                    ctrl.quit()
                except Exception:
                    pass
        self._devices = found
        if not found and self._winmm is not None:
            self._winmm.refresh()
        self._last_scan = time.monotonic()

    def devices(self) -> list[PadInfo]:
        if getattr(self, "_fallback", None) is not None:
            return self._fallback.devices()
        if not self._devices and getattr(self, "_winmm", None) is not None:
            self.backend_name = "Windows WinMM (SDL: cero mandos)"
            return self._winmm.devices()
        if self._devices:
            self.backend_name = ("SDL2 (pygame-ce)" if self.controller_api is not None
                                 else "SDL2 Joystick (sin GameController)")
        return [entry[0] for entry in self._devices.values()]

    def poll(self) -> dict[int, PadState]:
        if getattr(self, "_fallback", None) is not None:
            return self._fallback.poll()
        # Hotplug sin cambiar el dispositivo del jugador silenciosamente.
        if time.monotonic() - self._last_scan > 0.9:
            self.refresh()
        else:
            self.pg.event.pump()
        if not self._devices and getattr(self, "_winmm", None) is not None:
            return self._winmm.poll()
        result = {}
        for instance_id, (info, joy, ctrl) in self._devices.items():
            try:
                if not joy.get_init():
                    continue
                standard_ok = bool(ctrl is not None and ctrl.attached())
                read_path = choose_read_path(info, standard_ok, self.input_mode)
                if read_path == "controller":
                    api = self.controller_api
                    lx = normalize_axis(ctrl.get_axis(api.CONTROLLER_AXIS_LEFTX) / 32767.0)
                    ly = -normalize_axis(ctrl.get_axis(api.CONTROLLER_AXIS_LEFTY) / 32767.0)
                    action = bool(ctrl.get_button(api.CONTROLLER_BUTTON_A))
                    dash = bool(ctrl.get_button(api.CONTROLLER_BUTTON_B))
                    shield = bool(ctrl.get_button(api.CONTROLLER_BUTTON_X))
                else:
                    if info.axes < 2:
                        continue
                    m = validate_mapping(self.mappings.get(info.guid, {}), info.axes, info.buttons)
                    lx = normalize_axis(joy.get_axis(m["axis_x"]))
                    ly = -normalize_axis(joy.get_axis(m["axis_y"]))
                    if m["invert_x"]:
                        lx = -lx
                    if m["invert_y"]:
                        ly = -ly
                    if m["fire_axis"] is not None:
                        # Disparo con R2: evitar que el botón digital por defecto
                        # (posiblemente otro botón físico) también dispare.
                        action = analog_trigger_pressed(joy.get_axis(m["fire_axis"]),
                                                        m["fire_rest"], m["fire_direction"])
                    else:
                        action = bool(joy.get_button(m["button"]))
                    dash = bool(m["button_dash"] is not None and joy.get_button(m["button_dash"]))
                    shield = bool(m["button_shield"] is not None and joy.get_button(m["button_shield"]))
                result[instance_id] = PadState(instance_id, lx, ly, action, True, dash, shield)
            except (ValueError, IndexError, self.pg.error, OSError):
                # Hot-unplug durante la lectura: siguiente ciclo emite neutral.
                continue
        return result

    def set_input_mode(self, mode: str) -> None:
        if mode not in ("auto", "joystick", "controller"):
            raise ValueError("Modo de entrada no válido")
        if mode == self.input_mode:
            return
        self.input_mode = mode
        if getattr(self, "_fallback", None) is not None:
            return
        for _info, _joy, ctrl in self._devices.values():
            if ctrl is not None:
                try:
                    ctrl.quit()
                except Exception:
                    pass
        self._devices = {}
        self.refresh()

    def read_path(self, instance_id: int) -> str:
        if getattr(self, "_fallback", None) is not None:
            return self.backend_name
        entry = self._devices.get(instance_id)
        if entry is None:
            return self.backend_name
        info, _joy, ctrl = entry
        return choose_read_path(info, bool(ctrl is not None and ctrl.attached()), self.input_mode)

    def raw_axes(self, instance_id: int) -> tuple[list[float], list[int]]:
        """Lecturas para descubrir manualmente mapeos de mandos genéricos."""
        if getattr(self, "_fallback", None) is not None:
            return self._fallback.raw_axes(instance_id)
        entry = self._devices.get(instance_id)
        if entry is None:
            if not self._devices and getattr(self, "_winmm", None) is not None:
                return self._winmm.raw_axes(instance_id)
            return [], []
        _info, joy, _ctrl = entry
        try:
            self.pg.event.pump()
            return ([round(joy.get_axis(i), 2) for i in range(joy.get_numaxes())],
                    [i for i in range(joy.get_numbuttons()) if joy.get_button(i)])
        except (self.pg.error, OSError):
            return [], []

    def close(self):
        if getattr(self, "_fallback", None) is not None:
            return self._fallback.close()
        for _info, _joy, ctrl in self._devices.values():
            if ctrl is not None:
                try:
                    ctrl.quit()
                except Exception:
                    pass
        self._devices.clear()
        if getattr(self, "_winmm", None) is not None:
            self._winmm.close()
        self.pg.joystick.quit()
        self.pg.display.quit()
