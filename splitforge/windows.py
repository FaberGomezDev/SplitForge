"""Adaptador Win32: enumera, mueve y restaura ventanas normales.

No inyecta DLL, no altera memoria del juego ni elude antitrampas.
"""
from __future__ import annotations
import ctypes
import os
from ctypes import wintypes
from dataclasses import dataclass
from .layout import Region


@dataclass(frozen=True)
class WindowInfo:
    handle: int
    pid: int
    title: str


@dataclass(frozen=True)
class WindowSnapshot:
    handle: int
    x: int
    y: int
    width: int
    height: int
    maximized: bool
    minimized: bool


if os.name == "nt":
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    ENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)

    class RECT(ctypes.Structure):
        _fields_ = [("left", wintypes.LONG), ("top", wintypes.LONG),
                    ("right", wintypes.LONG), ("bottom", wintypes.LONG)]

    user32.EnumWindows.argtypes = [ENUMPROC, wintypes.LPARAM]
    user32.EnumWindows.restype = wintypes.BOOL
    user32.GetWindowTextLengthW.argtypes = [wintypes.HWND]
    user32.GetWindowTextLengthW.restype = ctypes.c_int
    user32.GetWindowTextW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
    user32.GetWindowTextW.restype = ctypes.c_int
    user32.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
    user32.GetWindowThreadProcessId.restype = wintypes.DWORD
    user32.IsWindowVisible.argtypes = [wintypes.HWND]
    user32.IsWindowVisible.restype = wintypes.BOOL
    user32.IsWindow.argtypes = [wintypes.HWND]
    user32.IsWindow.restype = wintypes.BOOL
    user32.IsZoomed.argtypes = [wintypes.HWND]
    user32.IsZoomed.restype = wintypes.BOOL
    user32.IsIconic.argtypes = [wintypes.HWND]
    user32.IsIconic.restype = wintypes.BOOL
    user32.GetWindowRect.argtypes = [wintypes.HWND, ctypes.POINTER(RECT)]
    user32.GetWindowRect.restype = wintypes.BOOL
    user32.ShowWindow.argtypes = [wintypes.HWND, ctypes.c_int]
    user32.ShowWindow.restype = wintypes.BOOL
    user32.SetWindowPos.argtypes = [wintypes.HWND, wintypes.HWND, ctypes.c_int,
                                     ctypes.c_int, ctypes.c_int, ctypes.c_int, wintypes.UINT]
    user32.SetWindowPos.restype = wintypes.BOOL
    user32.PostMessageW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
    user32.PostMessageW.restype = wintypes.BOOL
    user32.GetSystemMetrics.argtypes = [ctypes.c_int]
    user32.GetSystemMetrics.restype = ctypes.c_int


def enable_dpi_awareness() -> None:
    if os.name != "nt":
        return
    try:
        fn = user32.SetProcessDpiAwarenessContext
        fn.argtypes = [ctypes.c_void_p]
        fn.restype = wintypes.BOOL
        if fn(ctypes.c_void_p(-4)):
            return
    except (AttributeError, OSError):
        pass
    try:
        user32.SetProcessDPIAware()
    except (AttributeError, OSError):
        pass


def screen_size() -> tuple[int, int]:
    if os.name != "nt":
        raise RuntimeError("SplitForge requiere Windows para organizar ventanas.")
    return user32.GetSystemMetrics(0), user32.GetSystemMetrics(1)


def enumerate_windows() -> list[WindowInfo]:
    if os.name != "nt":
        return []
    found: list[WindowInfo] = []

    @ENUMPROC
    def callback(hwnd, _):
        if not user32.IsWindowVisible(hwnd):
            return True
        size = user32.GetWindowTextLengthW(hwnd)
        if size < 1:
            return True
        titlebuf = ctypes.create_unicode_buffer(size + 1)
        user32.GetWindowTextW(hwnd, titlebuf, size + 1)
        title = titlebuf.value.strip()
        if title:
            pid = wintypes.DWORD()
            user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
            found.append(WindowInfo(int(hwnd), int(pid.value), title))
        return True

    user32.EnumWindows(callback, 0)
    return found


def snapshot(hwnd: int) -> WindowSnapshot:
    if os.name != "nt" or not user32.IsWindow(hwnd):
        raise ValueError("La ventana seleccionada ya no existe.")
    rect = RECT()
    if not user32.GetWindowRect(hwnd, ctypes.byref(rect)):
        raise OSError(ctypes.get_last_error(), "No se puede obtener la posición de la ventana")
    return WindowSnapshot(hwnd, rect.left, rect.top,
                          rect.right - rect.left, rect.bottom - rect.top,
                          bool(user32.IsZoomed(hwnd)), bool(user32.IsIconic(hwnd)))


def move_window(hwnd: int, region: Region) -> None:
    if os.name != "nt" or not user32.IsWindow(hwnd):
        raise ValueError("La ventana ya no existe.")
    user32.ShowWindow(hwnd, 9)  # SW_RESTORE (si estaba maximizada/minimizada)
    # SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED
    flags = 0x0004 | 0x0010 | 0x0040 | 0x0020
    if not user32.SetWindowPos(hwnd, None, region.x, region.y,
                               region.width, region.height, flags):
        raise OSError(ctypes.get_last_error(), "Windows impidió mover esta ventana")


def restore_window(previous: WindowSnapshot) -> None:
    if os.name != "nt" or not user32.IsWindow(previous.handle):
        return
    region = Region(previous.x, previous.y, previous.width, previous.height)
    move_window(previous.handle, region)
    if previous.maximized:
        user32.ShowWindow(previous.handle, 3)  # SW_MAXIMIZE
    elif previous.minimized:
        user32.ShowWindow(previous.handle, 6)  # SW_MINIMIZE


def request_close(hwnd: int) -> None:
    if os.name == "nt" and user32.IsWindow(hwnd):
        user32.PostMessageW(hwnd, 0x0010, 0, 0)  # WM_CLOSE (petición no forzada)
