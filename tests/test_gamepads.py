"""Verificación de adaptador SDL con dispositivos simulados, sin hardware real."""
import time
import types
import unittest
from splitforge.gamepads import ControllerManager, PadInfo, choose_read_path


class FakeStick:
    def __init__(self, axes, buttons):
        self.axes = axes
        self.buttons = buttons

    def get_init(self):
        return True

    def get_axis(self, i):
        return self.axes[i]

    def get_button(self, i):
        return self.buttons[i]

    def get_numaxes(self):
        return len(self.axes)

    def get_numbuttons(self):
        return len(self.buttons)


class FakeController:
    def attached(self):
        return True

    def get_axis(self, index):
        return {0: 16383, 1: -32767}.get(index, 0)

    def get_button(self, index):
        return index == 8


class Tests(unittest.TestCase):
    def make_manager(self, devices, mappings=None):
        manager = ControllerManager.__new__(ControllerManager)
        manager.mappings = mappings or {}
        manager._devices = devices
        manager._last_scan = time.monotonic()
        manager.pg = types.SimpleNamespace(event=types.SimpleNamespace(pump=lambda: None), error=RuntimeError)
        manager.input_mode = "auto"
        manager.controller_api = types.SimpleNamespace(CONTROLLER_AXIS_LEFTX=0,
                                                        CONTROLLER_AXIS_LEFTY=1,
                                                        CONTROLLER_BUTTON_A=8,
                                                        CONTROLLER_BUTTON_B=9,
                                                        CONTROLLER_BUTTON_X=10)
        return manager

    def test_ps4_uses_real_joystick_values_not_stuck_controller_values(self):
        # El controlador normalizado puede notificar 0 aunque Joystick tenga
        # valores. Este test reproduce el caso que v0.3 no cubria.
        stick = FakeStick([0.75, -0.9], [True])
        info = PadInfo(17, "PS4 Controller", "ABC", "SDL estándar (PS4/Xbox/otros)", 2, 1)
        manager = self.make_manager({17: (info, stick, FakeController())})
        state = manager.poll()[17]
        self.assertTrue(state.a_pressed)
        self.assertGreater(state.ly, 0.8)
        self.assertGreater(state.lx, 0.65)
        self.assertEqual(manager.read_path(17), "joystick")

    def test_manual_standard_mode_still_works(self):
        stick = FakeStick([0, 0], [False])
        info = PadInfo(17, "PS4 Controller", "ABC", "SDL estándar (PS4/Xbox/otros)", 2, 1)
        manager = self.make_manager({17: (info, stick, FakeController())})
        manager.input_mode = "controller"
        state = manager.poll()[17]
        self.assertTrue(state.a_pressed)
        self.assertTrue(state.ly > .98)
        self.assertAlmostEqual(state.lx, .4186, places=2)

    def test_path_policy(self):
        ps4 = PadInfo(1, "PS4 Controller", "GUID", "standard", 6, 16)
        xb = PadInfo(2, "Xbox One Controller", "GUID", "standard", 6, 12)
        self.assertEqual(choose_read_path(ps4, True), "joystick")
        self.assertEqual(choose_read_path(xb, True), "controller")
        self.assertEqual(choose_read_path(xb, True, "joystick"), "joystick")
        self.assertEqual(choose_read_path(ps4, False, "controller"), "joystick")

    def test_two_ps4_same_model_stay_independent(self):
        a = FakeStick([0.75, -0.8], [True])
        b = FakeStick([-0.6, 0.4], [False])
        info_a = PadInfo(101, "PS4 Controller", "GUID", "SDL estándar", 2, 1)
        info_b = PadInfo(102, "PS4 Controller", "GUID", "SDL estándar", 2, 1)
        manager = self.make_manager({101: (info_a, a, FakeController()),
                                     102: (info_b, b, FakeController())})
        result = manager.poll()
        self.assertGreater(result[101].lx, 0.6)
        self.assertLess(result[102].lx, -0.5)
        self.assertTrue(result[101].a_pressed)
        self.assertFalse(result[102].a_pressed)

    def test_refresh_opens_joystick_only_for_ps4_default(self):
        ps4 = FakeStick([0.8, -0.8], [True])
        xbox = FakeStick([-0.5, 0.4], [False])
        class FakeInput(FakeStick):
            def __init__(self, name, index, axes, buttons):
                super().__init__(axes, buttons)
                self.name, self.index = name, index
            def get_instance_id(self): return 100 + self.index
            def get_name(self): return self.name
            def get_guid(self): return "GUID"
            def init(self): pass
        a = FakeInput("PS4 Controller", 0, ps4.axes, ps4.buttons)
        b = FakeInput("Xbox Controller", 1, xbox.axes, xbox.buttons)
        manager = self.make_manager({})
        opened = []
        class ControllerModule:
            CONTROLLER_AXIS_LEFTX = 0
            CONTROLLER_AXIS_LEFTY = 1
            CONTROLLER_BUTTON_A = 8
            CONTROLLER_BUTTON_B = 9
            CONTROLLER_BUTTON_X = 10
            def is_controller(self, i): return True
            def Controller(self, i):
                opened.append(i)
                return FakeController()
        manager.controller_api = ControllerModule()
        manager.pg = types.SimpleNamespace(
            event=types.SimpleNamespace(pump=lambda: None),
            joystick=types.SimpleNamespace(get_count=lambda: 2, Joystick=lambda i: [a,b][i]),
            error=RuntimeError)
        manager._last_scan = 0
        manager.refresh()
        self.assertEqual(opened, [1])
        self.assertEqual(set(manager.poll()), {100, 101})
        self.assertGreater(manager.poll()[100].lx, 0.7)

    def test_generic_custom_mapping(self):
        stick = FakeStick([.0, .0, -.8, .6], [False, False, True])
        info = PadInfo(30, "USB Generic", "XYZ", "HID genérico", 4, 3)
        mappings = {"XYZ": {"axis_x": 2, "axis_y": 3, "button": 2, "invert_y": True}}
        state = self.make_manager({30: (info, stick, None)}, mappings).poll()[30]
        self.assertTrue(state.a_pressed)
        self.assertLess(state.lx, 0)
        self.assertGreater(state.ly, 0)

    def test_independent_generic_same_model(self):
        stick1 = FakeStick([.7, .2], [True])
        stick2 = FakeStick([-.6, -.4], [False])
        info1 = PadInfo(10, "Generic", "GUID", "HID genérico", 2, 1)
        info2 = PadInfo(20, "Generic", "GUID", "HID genérico", 2, 1)
        states = self.make_manager({10: (info1, stick1, None), 20: (info2, stick2, None)}).poll()
        self.assertEqual(set(states), {10, 20})
        self.assertTrue(states[10].a_pressed)
        self.assertFalse(states[20].a_pressed)
        self.assertGreater(states[10].lx, 0)
        self.assertLess(states[20].lx, 0)


if __name__ == "__main__":
    unittest.main()
