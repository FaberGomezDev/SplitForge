"""Mundo compartido autoritativo, sin dispositivos reales."""
import unittest
from unittest.mock import Mock
from splitforge.gamepads import PadState
from splitforge.shared_world import SharedWorld
from splitforge.session import SessionManager, ActiveSession
from splitforge.shared_io import validate_world


class SharedWorldTests(unittest.TestCase):
    def test_two_users_in_same_snapshot(self):
        world = SharedWorld()
        before = world.snapshot()
        after = world.step(.05, {1: PadState(10, 1, 0), 2: PadState(20, -1, 0)})
        self.assertGreater(after['actors']['1']['x'], before['actors']['1']['x'])
        self.assertLess(after['actors']['2']['x'], before['actors']['2']['x'])
        self.assertTrue(after['actors']['1']['connected'])
        self.assertTrue(after['actors']['2']['connected'])
        self.assertIsNotNone(validate_world(after))

    def test_fire_is_edge_triggered_and_shared(self):
        world = SharedWorld()
        for _ in range(5):
            world.step(.016, {1: PadState(101, a_pressed=True), 2: PadState(102)})
        self.assertEqual(len(world.projectiles), 1)
        self.assertEqual(world.snapshot()['shots'][0]['owner'], 1)
        world.step(.016, {1: PadState(101, a_pressed=False), 2: PadState(102)})
        world.step(.016, {1: PadState(101, a_pressed=True), 2: PadState(102)})
        self.assertEqual(len(world.projectiles), 2)

    def test_unplug_stops_one_and_not_other(self):
        world = SharedWorld()
        first = world.step(.02, {1: PadState(1, .8), 2: PadState(2, -.8)})
        second = world.step(.02, {1: None, 2: PadState(2, -.8)})
        self.assertEqual(first['actors']['1']['x'], second['actors']['1']['x'])
        self.assertLess(second['actors']['2']['x'], first['actors']['2']['x'])
        self.assertFalse(second['actors']['1']['connected'])

    def test_step_clamps_untrusted_dt(self):
        world = SharedWorld()
        after = world.step(10000, {1: PadState(1, 1), 2: None})
        self.assertLess(after['actors']['1']['x'], .25)

    def test_no_spoofing_of_second_player(self):
        manager = SessionManager(lambda msg: None)
        a, b = Mock(), Mock()
        manager.active = ActiveSession('shared_demo', 'vertical', 4,
                                       assigned_ids=[101, 202], input_writers=[a, b],
                                       world=SharedWorld())
        manager.route_shared_inputs({101: PadState(101, .8),
                                     202: PadState(202, -.8)}, .05)
        a.send.assert_called_once()
        b.send.assert_called_once()
        world1 = a.send.call_args.args[0]
        world2 = b.send.call_args.args[0]
        self.assertEqual(world1, world2)  # un solo mundo
        self.assertNotEqual(world1['actors']['1']['x'], world1['actors']['2']['x'])
        self.assertIn('ID 101', manager.active.last_input_status[0])
        self.assertIn('ID 202', manager.active.last_input_status[1])
        manager.reassign_shared([202, 101], {101, 202})
        a.reset_mock()
        manager.route_shared_inputs({101: PadState(101, 1), 202: PadState(202, -1)}, .05)
        self.assertIn('ID 202', manager.active.last_input_status[0])
        with self.assertRaises(ValueError):
            manager.reassign_shared([101, 101], {101, 202})

    def test_shield_blocks_damage(self):
        world = SharedWorld()
        target = world.actors[2]
        target.x = .27
        target.y = .50
        world.step(.01, {1: PadState(1, a_pressed=True), 2: PadState(2, shield_pressed=True)})
        for _ in range(10):
            world.step(.01, {1: PadState(1), 2: PadState(2, shield_pressed=True)})
        self.assertEqual(target.hits, 0)


if __name__ == '__main__':
    unittest.main()
