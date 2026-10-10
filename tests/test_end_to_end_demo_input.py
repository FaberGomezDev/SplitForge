"""Prueba del circuito entrada -> asignacion -> IPC -> ventana demo, sin mandos."""
import queue
import time
import unittest
from splitforge.gamepads import PadState
from splitforge.input_stream import InputReader, InputWriter
from splitforge.session import SessionManager, ActiveSession


class Pipe:
    def __init__(self):
        self.q = queue.Queue()
    def write(self, text):
        self.q.put(text)
    def flush(self):
        pass
    def __iter__(self):
        while True:
            item = self.q.get()
            if item is None:
                return
            yield item


class TestDemoEndToEnd(unittest.TestCase):
    def test_two_independent_ps4_states_to_demo(self):
        pipes = [Pipe(), Pipe()]
        readers = [InputReader(p) for p in pipes]
        writers = [InputWriter(p) for p in pipes]
        manager = SessionManager(lambda _: None)
        manager.active = ActiveSession("demo", "vertical", 4,
                                       assigned_ids=[1, 2], input_writers=writers)
        states = {1: PadState(1, .7, -.4, True),
                  2: PadState(2, -.8, .65, False)}
        try:
            manager.route_demo_inputs(states)
            deadline = time.monotonic() + 1.0
            while time.monotonic() < deadline and any(r.read() is None for r in readers):
                time.sleep(.005)
            a, b = readers[0].read(), readers[1].read()
            self.assertIsNotNone(a)
            self.assertIsNotNone(b)
            self.assertAlmostEqual(a.lx, .7)
            self.assertAlmostEqual(b.lx, -.8)
            self.assertTrue(a.a_pressed)
            self.assertFalse(b.a_pressed)
        finally:
            for writer in writers:
                writer.stop()
            for pipe in pipes:
                pipe.q.put(None)

if __name__ == "__main__":
    unittest.main()
