"""WinMM bridge tests with simulated Windows joystick API, without a controller."""
import ctypes
import unittest
from splitforge.winmm_gamepads import WinMMControllerManager, JOYINFOEX, JOYCAPSW, to_unit


class FakeWinMM:
    def __init__(self):
        self.connected = {0, 1}
        self.xs = {0: 65535, 1: 0}
        self.buttons = {0: 1, 1: 0}

    def joyGetNumDevs(self):
        return 3

    def joyGetPosEx(self, idx, pointer):
        if idx not in self.connected:
            return 165
        info = ctypes.cast(pointer, ctypes.POINTER(JOYINFOEX)).contents
        info.dwXpos = self.xs[idx]
        info.dwYpos = 0
        info.dwButtons = self.buttons[idx]
        return 0

    def joyGetDevCapsW(self, idx, pointer, size):
        if idx not in self.connected:
            return 165
        caps = ctypes.cast(pointer, ctypes.POINTER(JOYCAPSW)).contents
        caps.wMid = 111
        caps.wPid = 222
        caps.szPname = "USB Joystick"
        caps.wXmin = 0
        caps.wXmax = 65535
        caps.wYmin = 0
        caps.wYmax = 65535
        caps.wNumButtons = 4
        caps.wNumAxes = 2
        return 0


class TestWinMM(unittest.TestCase):
    def test_axis_normalization(self):
        self.assertEqual(to_unit(0, 0, 65535), -1)
        self.assertEqual(to_unit(65535, 0, 65535), 1)
        self.assertEqual(to_unit(50, 50, 50), 0)

    def test_independent_controllers(self):
        manager = WinMMControllerManager(api=FakeWinMM())
        infos = manager.devices()
        self.assertEqual(len(infos), 2)
        self.assertEqual(infos[0].name, "USB Joystick")
        self.assertNotEqual(infos[0].instance_id, infos[1].instance_id)
        states = manager.poll()
        self.assertEqual(set(states), {100000, 100001})
        self.assertTrue(states[100000].a_pressed)
        self.assertFalse(states[100001].a_pressed)
        self.assertEqual(states[100000].lx, 1)
        self.assertEqual(states[100001].lx, -1)
        axes, pressed = manager.raw_axes(100000)
        self.assertEqual(pressed, [0])
        self.assertEqual(axes[:2], [1, -1])
        manager.close()

    def test_disconnection(self):
        fake = FakeWinMM()
        manager = WinMMControllerManager(api=fake)
        fake.connected.remove(1)
        self.assertNotIn(100001, manager.poll())
        manager.refresh()
        self.assertEqual(len(manager.devices()), 1)

    def test_independent_mapping(self):
        fake = FakeWinMM()
        manager = WinMMControllerManager(api=fake)
        key = manager.devices()[0].guid
        manager.mappings[key] = {"axis_x": 1, "axis_y": 0, "button": 1}
        state = manager.poll()[100000]
        self.assertEqual(state.lx, -1)
        self.assertFalse(state.a_pressed)


if __name__ == "__main__":
    unittest.main()
