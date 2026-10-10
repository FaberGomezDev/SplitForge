"""Regression: un fallo GameController no debe desactivar SDL Joystick."""
import sys
import types
import unittest
from unittest.mock import patch
from splitforge.gamepads import ControllerManager


class SimpleStick:
    def get_init(self):
        return True
    def get_instance_id(self):
        return 444
    def get_name(self):
        return 'Wireless Controller'
    def get_guid(self):
        return 'GUID-PS4'
    def get_numaxes(self):
        return 2
    def get_numbuttons(self):
        return 1
    def get_axis(self, axis):
        return 0.7 if axis == 0 else -0.6
    def get_button(self, button):
        return True


class SDLFallbackTest(unittest.TestCase):
    def test_sdl_joystick_works_without_controller_module(self):
        fake = types.ModuleType('pygame')
        fake.error = RuntimeError
        fake.HIDDEN = 1
        fake.display = types.SimpleNamespace(init=lambda:None, set_mode=lambda *_:None, quit=lambda:None)
        fake.joystick = types.SimpleNamespace(init=lambda:None, get_count=lambda:1,
                                              Joystick=lambda _i:SimpleStick(),quit=lambda:None)
        fake.event = types.SimpleNamespace(pump=lambda:None)
        with patch.dict(sys.modules, {'pygame': fake, 'pygame._sdl2': None}):
            mgr = ControllerManager()
            try:
                self.assertIn('SDL', mgr.backend_name)
                self.assertEqual(len(mgr.devices()), 1)
                state = mgr.poll()[444]
                self.assertTrue(state.a_pressed)
                self.assertGreater(state.lx, 0.5)
                self.assertGreater(state.ly, 0.4)
            finally:
                mgr.close()

if __name__ == '__main__':
    unittest.main()
