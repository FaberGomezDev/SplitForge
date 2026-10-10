from io import StringIO
import unittest
from unittest.mock import Mock

from splitforge.gamepads import PadState
from splitforge.input_stream import decode_state, encode_state, InputWriter
from splitforge.session import SessionManager, ActiveSession


class InputTests(unittest.TestCase):
    def test_json_roundtrip(self):
        original = PadState(55, 0.77, -0.42, True)
        copy = decode_state(encode_state(original))
        self.assertAlmostEqual(copy.lx, .77)
        self.assertAlmostEqual(copy.ly, -.42)
        self.assertTrue(copy.a_pressed)

    def test_disconnected_or_corrupt_returns_none(self):
        self.assertIsNone(decode_state(encode_state(None)))
        self.assertIsNone(decode_state('{bad json'))
        self.assertIsNone(decode_state('{"connected":true,"lx":"oops","ly":0}'))

    def test_route_different_player_inputs(self):
        manager = SessionManager(lambda _: None)
        left, right = Mock(), Mock()
        manager.active = ActiveSession("demo", "vertical", 4,
                                       assigned_ids=[101, 202], input_writers=[left, right])
        states = {101: PadState(101, 1, 0, True), 202: PadState(202, -1, 0, False)}
        manager.route_demo_inputs(states)
        left.send.assert_called_once_with(states[101])
        right.send.assert_called_once_with(states[202])

    def test_unplug_sends_neutral_input(self):
        manager = SessionManager(lambda _: None)
        left, right = Mock(), Mock()
        manager.active = ActiveSession("demo", "vertical", 4,
                                       assigned_ids=[101, 202], input_writers=[left, right])
        manager.route_demo_inputs({101: PadState(101, .7, .4, True)})
        left.send.assert_called_once()
        right.send.assert_called_once_with(None)

    def test_latest_only_queue_is_bounded(self):
        writer = InputWriter.__new__(InputWriter)
        import queue
        writer.queue = queue.Queue(maxsize=1)
        writer.closed = False
        writer.send(PadState(1, .1, 0, False))
        writer.send(PadState(1, .9, 0, True))
        latest = decode_state(writer.queue.get_nowait())
        self.assertAlmostEqual(latest.lx, .9)
        self.assertTrue(latest.a_pressed)


if __name__ == "__main__":
    unittest.main()
