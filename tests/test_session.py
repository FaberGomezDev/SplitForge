import unittest
from unittest import mock

from splitforge.session import SessionManager, ActiveSession
from splitforge.windows import WindowSnapshot, WindowInfo


def snap(h):
    return WindowSnapshot(h, 10, 20, 640, 480, False, False)


class SessionTests(unittest.TestCase):
    def setUp(self):
        self.logs = []
        self.manager = SessionManager(self.logs.append)

    @mock.patch("splitforge.session.restore_window")
    @mock.patch("splitforge.session.move_window")
    @mock.patch("splitforge.session.snapshot", side_effect=snap)
    @mock.patch("splitforge.session.screen_size", return_value=(1920, 1080))
    def test_attach_and_restore(self, _size, _snap, move, restore):
        self.manager.attach([11, 22], "vertical", 4)
        self.assertEqual(move.call_count, 2)
        self.assertEqual(self.manager.active.window_handles, [11, 22])
        self.manager.stop()
        self.assertIsNone(self.manager.active)
        self.assertEqual(restore.call_count, 2)

    def test_reject_duplicate_windows(self):
        with self.assertRaises(ValueError):
            self.manager.attach([11, 11], "vertical", 4)
        self.assertIsNone(self.manager.active)

    @mock.patch("splitforge.session.restore_window")
    @mock.patch("splitforge.session.move_window", side_effect=[None, RuntimeError("Ventana no redimensionable")])
    @mock.patch("splitforge.session.snapshot", side_effect=snap)
    @mock.patch("splitforge.session.screen_size", return_value=(1920, 1080))
    def test_rollback_on_move_error(self, _size, _snap, _move, restore):
        with self.assertRaisesRegex(RuntimeError, "no redimensionable"):
            self.manager.attach([11, 22], "horizontal", 4)
        self.assertIsNone(self.manager.active)
        self.assertEqual(restore.call_count, 2)

    @mock.patch("splitforge.session.restore_window")
    @mock.patch("splitforge.session.move_window")
    @mock.patch("splitforge.session.snapshot", side_effect=snap)
    @mock.patch("splitforge.session.screen_size", return_value=(1920, 1080))
    def test_manual_window_binding_after_launcher(self, _size, _snap, move, _restore):
        session = ActiveSession("external", "vertical", 4, pending=False)
        self.manager.active = session
        self.manager.attach([101, 102], "horizontal", 8)
        self.assertIs(self.manager.active, session)
        self.assertEqual(session.orientation, "horizontal")
        self.assertEqual(session.window_handles, [101, 102])
        self.assertEqual(move.call_count, 2)


if __name__ == "__main__":
    unittest.main()
