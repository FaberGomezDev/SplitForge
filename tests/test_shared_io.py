import io
import json
import queue
import time
import unittest
from splitforge.shared_world import SharedWorld
from splitforge.shared_io import WorldWriter, WorldReader, validate_world

class Pipes:
    def __init__(self):
        self.q = queue.Queue()
    def write(self, text):
        self.q.put(text)
    def flush(self):
        pass
    def __iter__(self):
        while True:
            v = self.q.get()
            if v is None:
                return
            yield v

class SharedIoTests(unittest.TestCase):
    def test_validate_corrupted(self):
        for val in ({'type': 'world', 'frame': 2}, {'type': 'foo'}, [], None):
            self.assertIsNone(validate_world(val))

    def test_read_write_local_snapshot(self):
        pipe = Pipes()
        w = WorldWriter(pipe)
        r = WorldReader(pipe)
        expected = SharedWorld().snapshot()
        w.send(expected)
        deadline = time.monotonic() + 1
        while r.read() is None and time.monotonic() < deadline:
            time.sleep(.01)
        try:
            self.assertEqual(r.read()['actors'], expected['actors'])
        finally:
            w.stop()
            pipe.q.put(None)

    def test_latest_only_buffer(self):
        w = WorldWriter.__new__(WorldWriter)
        w.queue = queue.Queue(maxsize=1)
        w.closed = False
        for frame in range(10):
            w.send({'frame': frame})
        self.assertEqual(json.loads(w.queue.get_nowait())['frame'], 9)

if __name__ == '__main__':
    unittest.main()
