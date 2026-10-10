"""Difusión de instantáneas JSON de la sesión local a dos ventanas de prueba.

Cada hijo es un visor de un único mundo que simula el anfitrión. Se usa stdin
porque no es necesario abrir puertos, instalar servicios o transmitir por Internet.
"""
from __future__ import annotations
import json
import queue
import sys
import threading
import time


def validate_world(data: object) -> dict | None:
    if not isinstance(data, dict) or data.get("type") != "world":
        return None
    if not isinstance(data.get("frame"), int):
        return None
    actors = data.get("actors")
    if not isinstance(actors, dict) or not all(str(i) in actors for i in (1, 2)):
        return None
    for i in (1, 2):
        a = actors[str(i)]
        if not isinstance(a, dict) or not all(isinstance(a.get(k), (int, float)) for k in ("x", "y")):
            return None
    return data


class WorldWriter:
    def __init__(self, stream):
        self.stream = stream
        self.queue: queue.Queue[str | None] = queue.Queue(maxsize=1)
        self.closed = False
        self.thread = threading.Thread(target=self._run, daemon=True, name="SplitForgeWorldWriter")
        self.thread.start()

    def send(self, snapshot: dict):
        if self.closed:
            return
        item = json.dumps(snapshot, ensure_ascii=False, separators=(",", ":")) + "\n"
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
            message = self.queue.get()
            if message is None:
                return
            try:
                self.stream.write(message)
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


class WorldReader:
    def __init__(self, stream=None):
        self.stream = stream if stream is not None else sys.stdin
        self.state: dict | None = None
        self.received_at = 0.0
        self.thread = threading.Thread(target=self._run, daemon=True, name="SplitForgeWorldReader")
        self.thread.start()

    def _run(self):
        for line in self.stream:
            try:
                candidate = validate_world(json.loads(line))
            except (ValueError, TypeError):
                candidate = None
            if candidate:
                self.state = candidate
                self.received_at = time.monotonic()
        self.state = None

    def read(self) -> dict | None:
        if time.monotonic() - self.received_at > 1.:
            return None
        return self.state
