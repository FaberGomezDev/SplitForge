import unittest
from splitforge.layout import two_player_regions
from splitforge.gamepads import normalize_axis, validate_mapping


class LayoutTests(unittest.TestCase):
    def test_1080p_side_by_side(self):
        a, b = two_player_regions(1920, 1080, "vertical", 4)
        self.assertEqual((a.x, a.y, a.width, a.height), (0, 0, 958, 1080))
        self.assertEqual((b.x, b.y, b.width, b.height), (962, 0, 958, 1080))

    def test_1080p_stacked(self):
        a, b = two_player_regions(1920, 1080, "horizontal", 8)
        self.assertEqual((a.x, a.y, a.width, a.height), (0, 0, 1920, 536))
        self.assertEqual((b.x, b.y, b.width, b.height), (0, 544, 1920, 536))

    def test_odd_pixels_exhaust_screen(self):
        a, b = two_player_regions(1365, 767, "vertical", 3)
        self.assertEqual(b.x + b.width, 1365)
        self.assertEqual(a.width + b.width + 3, 1365)

    def test_invalid_input(self):
        with self.assertRaises(ValueError):
            two_player_regions(0, 1080)
        with self.assertRaises(ValueError):
            two_player_regions(1920, 1080, "diagonal")
        with self.assertRaises(ValueError):
            two_player_regions(1920, 1080, gutter=100)

    def test_deadzone_and_clamping(self):
        self.assertEqual(normalize_axis(0.10), 0)
        self.assertEqual(normalize_axis(1.2), 1)
        self.assertEqual(normalize_axis(-1.2), -1)
        self.assertTrue(0 < normalize_axis(.6) < 1)

    def test_generic_mapping_validation(self):
        result = validate_mapping({"axis_x": "2", "axis_y": "3", "button": "4", "invert_y": 1}, 6, 12)
        self.assertEqual((result["axis_x"], result["axis_y"], result["button"]), (2, 3, 4))
        self.assertTrue(result["invert_y"])
        with self.assertRaises(ValueError):
            validate_mapping({"axis_y": 9}, 3, 6)
        with self.assertRaises(ValueError):
            validate_mapping({"button": -1}, 3, 6)


if __name__ == "__main__":
    unittest.main()
