"""Windows-native joystick fallback, without external packages.

WinMM exposes many USB HID, XInput-compatible and PlayStation controllers when
Windows publishes them as legacy joysticks. It is NOT a full SDL replacement:
newer/specialized HID gamepads may require the optional SDL backend.
"""
from __future__ import annotations

import ctypes
from ctypes import wintypes
import os
import time

from .gamepads import PadInfo, PadState, normalize_axis, validate_mapping

JOY_RETURNALL = 0xFF
JOYCAPS_HASZ = 0x0001
JOYCAPS_HASR = 0x0002
JOYCAPS_HASU = 0x0004
JOYCAPS_HASV = 0x0008
JOYCAPS_POV4DIR = 0x0020
JOYERR_NOERROR = 0


class JOYINFOEX(ctypes.Structure):
    _fields_ = [
        ("dwSize", wintypes.DWORD), ("dwFlags", wintypes.DWORD),
        ("dwXpos", wintypes.DWORD), ("dwYpos", wintypes.DWORD),
        ("dwZpos", wintypes.DWORD), ("dwRpos", wintypes.DWORD),
        ("dwUpos", wintypes.DWORD), ("dwVpos", wintypes.DWORD),
        ("dwButtons", wintypes.DWORD), ("dwButtonNumber", wintypes.DWORD),
        ("dwPOV", wintypes.DWORD), ("dwReserved1", wintypes.DWORD),
        ("dwReserved2", wintypes.DWORD),
    ]


class JOYCAPSW(ctypes.Structure):
    _fields_ = [
        ("wMid", wintypes.WORD), ("wPid", wintypes.WORD),
        ("szPname", wintypes.WCHAR * 32),
        ("wXmin", wintypes.UINT), ("wXmax", wintypes.UINT),
        ("wYmin", wintypes.UINT), ("wYmax", wintypes.UINT),
        ("wZmin", wintypes.UINT), ("wZmax", wintypes.UINT),
        ("wNumButtons", wintypes.UINT),
        ("wPeriodMin", wintypes.UINT), ("wPeriodMax", wintypes.UINT),
        ("wRmin", wintypes.UINT), ("wRmax", wintypes.UINT),
        ("wUmin", wintypes.UINT), ("wUmax", wintypes.UINT),
        ("wVmin", wintypes.UINT), ("wVmax", wintypes.UINT),
        ("wCaps", wintypes.UINT), ("wMaxAxes", wintypes.UINT),
        ("wNumAxes", wintypes.UINT), ("wMaxButtons", wintypes.UINT),
        ("szRegKey", wintypes.WCHAR * 32),
        ("szOEMVxD", wintypes.WCHAR * 260),
    ]


def to_unit(raw: int, low: int, high: int) -> float:
    """Normalize a WinMM axis range (often 0..65535) to -1..+1."""
    if high <= low:
        return 0.0
    return max(-1.0, min(1.0, 2.0 * (int(raw) - int(low)) / (int(high) - int(low)) - 1.0))


def _axes(info: JOYINFOEX, caps: JOYCAPSW) -> list[float]:
    readings = [("dwXpos", "wXmin", "wXmax"), ("dwYpos", "wYmin", "wYmax")]
    for flag, prefix in ((JOYCAPS_HASZ, "Z"), (JOYCAPS_HASR, "R"),
                         (JOYCAPS_HASU, "U"), (JOYCAPS_HASV, "V")):
        if caps.wCaps & flag:
            readings.append((f"dw{prefix}pos", f"w{prefix}min", f"w{prefix}max"))
    return [round(to_unit(getattr(info, pos), getattr(caps, low), getattr(caps, high)), 4)
            for pos, low, high in readings]


class WinMMControllerManager:
    backend_name = "Windows nativo (WinMM)"

    def __init__(self, mappings: dict | None = None, api=None):
        if api is None:
            if os.name != "nt":
                raise RuntimeError("La entrada WinMM requiere Windows.")
            api = ctypes.WinDLL("winmm", use_last_error=True)
            api.joyGetNumDevs.argtypes = []
            api.joyGetNumDevs.restype = wintypes.UINT
            api.joyGetPosEx.argtypes = [wintypes.UINT, ctypes.POINTER(JOYINFOEX)]
            api.joyGetPosEx.restype = wintypes.UINT
            api.joyGetDevCapsW.argtypes = [ctypes.c_size_t, ctypes.POINTER(JOYCAPSW), wintypes.UINT]
            api.joyGetDevCapsW.restype = wintypes.UINT
        self.api = api
        self.mappings = mappings if isinstance(mappings, dict) else {}
        self._devices: dict[int, tuple[PadInfo, JOYCAPSW, int]] = {}
        self._last_scan = 0.0
        self.refresh()

    @staticmethod
    def _read_info() -> JOYINFOEX:
        info = JOYINFOEX()
        info.dwSize = ctypes.sizeof(JOYINFOEX)
        info.dwFlags = JOY_RETURNALL
        return info

    def refresh(self):
        found = {}
        # joyGetNumDevs reports available slots (not connected controllers).
        for idx in range(min(int(self.api.joyGetNumDevs()), 32)):
            state = self._read_info()
            if self.api.joyGetPosEx(idx, ctypes.byref(state)) != JOYERR_NOERROR:
                continue
            caps = JOYCAPSW()
            if self.api.joyGetDevCapsW(idx, ctypes.byref(caps), ctypes.sizeof(caps)) != JOYERR_NOERROR:
                continue
            iid = 100000 + idx
            key = f"WINMM:{caps.wMid:04X}:{caps.wPid:04X}:{caps.szPname}"
            axes = len(_axes(state, caps))
            name = caps.szPname or f"Joystick Windows #{idx + 1}"
            device = PadInfo(iid, name, key, "HID genérico", axes,
                             min(32, max(0, int(caps.wNumButtons))))
            found[iid] = (device, caps, idx)
        self._devices = found
        self._last_scan = time.monotonic()

    def devices(self) -> list[PadInfo]:
        return [entry[0] for entry in self._devices.values()]

    def _raw(self, instance_id: int):
        entry = self._devices.get(instance_id)
        if entry is None:
            return None
        info, caps, index = entry
        state = self._read_info()
        if self.api.joyGetPosEx(index, ctypes.byref(state)) != JOYERR_NOERROR:
            return None
        axes = _axes(state, caps)
        pressed = [i for i in range(info.buttons) if state.dwButtons & (1 << i)]
        return axes, pressed

    def poll(self) -> dict[int, PadState]:
        if time.monotonic() - self._last_scan > 0.9:
            self.refresh()
        states = {}
        for iid, (info, _, _) in self._devices.items():
            raw = self._raw(iid)
            if raw is None or info.axes < 2 or info.buttons < 1:
                continue
            axes, pressed = raw
            try:
                mapping = validate_mapping(self.mappings.get(info.guid, {}), info.axes, info.buttons)
            except (TypeError, ValueError):
                continue
            lx = normalize_axis(axes[mapping["axis_x"]])
            ly = -normalize_axis(axes[mapping["axis_y"]])
            if mapping["invert_x"]:
                lx = -lx
            if mapping["invert_y"]:
                ly = -ly
            if mapping["fire_axis"] is not None:
                from .gamepads import analog_trigger_pressed
                action = analog_trigger_pressed(axes[mapping["fire_axis"]],
                                                mapping["fire_rest"], mapping["fire_direction"])
            else:
                action = mapping["button"] in pressed
            states[iid] = PadState(iid, lx, ly, action, True,
                                   mapping["button_dash"] in pressed if mapping["button_dash"] is not None else False,
                                   mapping["button_shield"] in pressed if mapping["button_shield"] is not None else False)
        return states

    def raw_axes(self, instance_id: int) -> tuple[list[float], list[int]]:
        return self._raw(instance_id) or ([], [])

    def close(self):
        self._devices.clear()
