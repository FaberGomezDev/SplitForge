"""Canal unidireccional desde SplitForge hacia los juegos DEMO.

Sin sockets, drivers ni hooks: el proceso padre envía solo estados de mandos a
sus procesos de demostración. Una cola de 1 mensaje evita retardo acumulado.
"""
from __future__ import annotations
import json
import queue
import sys
import threading
from .gamepads import PadState


def encode_state(state: PadState | None) -> str:
    return json.dumps({"lx": state.lx if state else 0.0,
                       "ly": state.ly if state else 0.0,
                       "a": bool(state.a_pressed) if state else False,
                       "dash": bool(state.dash_pressed) if state else False,
                       "shield": bool(state.shield_pressed) if state else False,
                       "connected": bool(state.connected) if state else False}, separators=(",", ":")) + "\n"


def decode_state(message: str) -> PadState | None:
    try:
        data = json.loads(message)
        if not isinstance(data, dict) or not data.get("connected", False):
            return None
        return PadState(-1, max(-1.0, min(1.0, float(data["lx"]))),
                        max(-1.0, min(1.0, float(data["ly"]))), bool(data["a"]), True,
                        bool(data.get("dash", False)), bool(data.get("shield", False)))
    except (KeyError, TypeError, ValueError, json.JSONDecodeError):
        return None


class InputWriter:
    """La escritura a stdin nunca bloquea el bucle tkinter."""
    def __init__(self, stream):
        self.stream = stream
        self.queue: queue.Queue[str | None] = queue.Queue(maxsize=1)
        self.closed = False
        self.thread = threading.Thread(target=self._run, name="SplitForgeInput", daemon=True)
        self.thread.start()

    def send(self, state: PadState | None):
        if self.closed:
            return
        item = encode_state(state)
        try:
            self.queue.put_nowait(item)
        except queue.Full:
            try:
                self.queue.get_nowait()
            except queue.Empty:
                pass
            try:
                self.queue.put_nowait(item)
            except queue.Full:
                pass

    def _run(self):
        while True:
            item = self.queue.get()
            if item is None:
                return
            try:
                self.stream.write(item)
                self.stream.flush()
            except (OSError, ValueError, BrokenPipeError):
                self.closed = True
                return

    def stop(self):
        self.closed = True
        try:
            self.queue.put_nowait(None)
        except queue.Full:
            try:
                self.queue.get_nowait()
                self.queue.put_nowait(None)
            except (queue.Empty, queue.Full):
                pass
        # No wait/join: el proceso podría estar bloqueado y no debe congelar la UI.


class InputReader:
    """Hilo de la ventana DEMO; su estado más reciente se lee sin bloquear."""
    def __init__(self, stream=None):
        self.state: PadState | None = None
        self.stream = stream if stream is not None else sys.stdin
        self.last_time = 0.0
        self.thread = threading.Thread(target=self._run, name="SplitForgeDemoInput", daemon=True)
        self.thread.start()

    def _run(self):
        import time
        for line in self.stream:
            self.state = decode_state(line)
            self.last_time = time.monotonic()
        self.state = None

    def read(self) -> PadState | None:
        import time
        # Un proceso padre bloqueado NO debe dejar los controles presionados.
        if time.monotonic() - self.last_time > 0.75:
            return None
        return self.state
