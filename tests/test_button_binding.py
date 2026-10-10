"""Sin hardware: aprendizaje de botones, acciones e IPC independiente."""
import time
import types
import unittest
from splitforge.gamepads import (ControllerManager, PadInfo, PadState,
                                 validate_mapping, new_button_presses)
from splitforge.gui import SplitForgeApp
from splitforge.input_stream import encode_state, decode_state
class FakeStick:
    def __init__(self, axes, buttons):
        self.axes, self.buttons = axes, buttons
    def get_init(self): return True
    def get_axis(self, i): return self.axes[i]
    def get_button(self, i): return self.buttons[i]
    def get_numaxes(self): return len(self.axes)
    def get_numbuttons(self): return len(self.buttons)


def mock_manager(devices, mappings=None):
    m = ControllerManager.__new__(ControllerManager)
    m.mappings = mappings or {}
    m._devices = devices
    m._last_scan = time.monotonic()
    m.pg = types.SimpleNamespace(event=types.SimpleNamespace(pump=lambda:None), error=RuntimeError)
    m.input_mode = 'auto'
    m.controller_api = None
    return m


class Variable:
    def __init__(self, value=None): self.value = value
    def get(self): return self.value
    def set(self, value): self.value = value


class ButtonBindings(unittest.TestCase):
    def test_edge_detection_ignores_held_buttons(self):
        self.assertEqual(new_button_presses({3}, {3}), [])
        self.assertEqual(new_button_presses({4}, set()), [4])
        self.assertEqual(new_button_presses({3, 4}, {3}), [4])

    def test_ps4_fire_can_be_nonzero_index_and_separate_from_dash_shield(self):
        info = PadInfo(101, "PS4 Controller", "GUID", "SDL estándar", 2, 8)
        # El botón Cruz es físicamente 5 en este dispositivo simulado.
        raw = FakeStick([0.2, 0], [False, True, False, False, False, True, False, False])
        manager = mock_manager({101: (info, raw, None)}, {
            "GUID": {"axis_x": 0, "axis_y": 1, "button": 5,
                     "button_dash": 1, "button_shield": 2}})
        state = manager.poll()[101]
        self.assertTrue(state.a_pressed)
        self.assertTrue(state.dash_pressed)
        self.assertFalse(state.shield_pressed)
        raw.buttons[5] = False
        raw.buttons[2] = True
        after = manager.poll()[101]
        self.assertFalse(after.a_pressed)
        self.assertTrue(after.shield_pressed)

    def test_optional_actions_for_one_button_device(self):
        m = validate_mapping({"button": 0}, 2, 1)
        self.assertIsNone(m["button_dash"])
        self.assertIsNone(m["button_shield"])
        with self.assertRaises(ValueError):
            validate_mapping({"button": 8}, 2, 1)

    def test_ipc_preserves_three_actions(self):
        state = PadState(2, 0.3, -0.4, True, True, True, False)
        received = decode_state(encode_state(state))
        self.assertTrue(received.a_pressed)
        self.assertTrue(received.dash_pressed)
        self.assertFalse(received.shield_pressed)

    def test_capture_assigns_pressed_physical_button(self):
        app = SplitForgeApp.__new__(SplitForgeApp)
        app.pad_labels = {"ps4": 10}
        app.selected_pad = Variable("ps4")
        app._capture_action = "button"
        app._capture_device_id = 10
        app._capture_previous = {1}
        app.map_button = Variable("0")
        app.map_dash = Variable("1")
        app.map_shield = Variable("2")
        app.capture_status = Variable()
        app.pad = types.SimpleNamespace(raw_axes=lambda _: ([], [1, 4]))
        saves = []
        app._save_controller_mapping = lambda notify=True: saves.append(notify)
        app._capture_tick()
        self.assertEqual(app.map_button.get(), '4')
        self.assertEqual(saves, [False])
        self.assertIsNone(app._capture_action)

    def test_two_ps4_profiles_do_not_mix_actions(self):
        p1 = FakeStick([.7, .1], [False, True, False, False])
        p2 = FakeStick([-.6, -.3], [False, False, False, True])
        info1 = PadInfo(10, 'PS4 Controller', 'same_guid', 'SDL', 2, 4)
        info2 = PadInfo(20, 'PS4 Controller', 'same_guid', 'SDL', 2, 4)
        manager = mock_manager({10: (info1,p1,None),20: (info2,p2,None)},
                                      {'same_guid': {'button':3,'button_dash':1,'button_shield':2}})
        st = manager.poll()
        self.assertFalse(st[10].a_pressed)
        self.assertTrue(st[10].dash_pressed)
        self.assertTrue(st[20].a_pressed)
        self.assertFalse(st[20].dash_pressed)


if __name__ == '__main__':
    unittest.main()

class AnalogTriggerTests(unittest.TestCase):
    def test_ps4_r2_axis_with_negative_rest(self):
        raw = FakeStick([0.1, 0.1, -1.0], [False] * 4)
        info = PadInfo(5, 'PS4 Controller', 'ps4', 'SDL', 3, 4)
        m = mock_manager({5: (info, raw, None)}, {'ps4': {
            'axis_x':0, 'axis_y':1, 'button': 0, 'fire_axis':2,
            'fire_rest':-1.0, 'fire_direction':1}})
        self.assertFalse(m.poll()[5].a_pressed)
        raw.axes[2] = 0.5
        self.assertTrue(m.poll()[5].a_pressed)
        raw.axes[2] = -1.0
        self.assertFalse(m.poll()[5].a_pressed)

    def test_other_face_button_doesnt_fire_if_analog_axis_bound(self):
        raw = FakeStick([0., 0., -1.], [True, False, False])
        info = PadInfo(5, 'PS4 Controller', 'ps4', 'SDL', 3, 3)
        m = mock_manager({5: (info, raw, None)}, {'ps4': {
            'button': 0, 'fire_axis':2, 'fire_rest':-1., 'fire_direction':1}})
        self.assertFalse(m.poll()[5].a_pressed)

    def test_capture_r2_axis(self):
        app = SplitForgeApp.__new__(SplitForgeApp)
        app.pad_labels = {'ps4': 10}
        app.selected_pad = Variable('ps4')
        app._capture_action = 'button'
        app._capture_device_id = 10
        app._capture_previous = set()
        app._capture_axis_baseline = [0., 0., -1.]
        app.map_x, app.map_y = Variable('0'), Variable('1')
        app.map_button, app.map_dash, app.map_shield = Variable('0'), Variable('1'), Variable('2')
        app.capture_status = Variable()
        app.pad = types.SimpleNamespace(raw_axes=lambda _: ([0., 0., .8], []))
        saves = []
        app._save_controller_mapping = lambda notify=True: saves.append(notify)
        app._capture_tick()
        self.assertEqual(app._fire_axis, 2)
        self.assertEqual(app._fire_rest, -1.)
        self.assertEqual(app._fire_direction, 1)
        self.assertEqual(saves, [False])
