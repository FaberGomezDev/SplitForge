"""Clasificación de módulos del diagnóstico de 7 Days to Die, sin Windows."""
import json
import unittest

from diagnostico_7dtd import classify


class Diagnostico7DTDTests(unittest.TestCase):
    def test_detects_input_families_case_insensitive(self):
        result = classify(["UnityPlayer.dll", "XINPUT1_3.dll", "hid.dll", "steam_api64.dll",
                           "GameOverlayRenderer64.dll", "kernel32.dll"])
        self.assertEqual(result["familias"]["xinput"], ["xinput1_3.dll"])
        self.assertEqual(result["familias"]["hid"], ["hid.dll"])
        self.assertIn("steam_overlay_input", result["familias"])
        self.assertNotIn("directinput", result["familias"])
        self.assertEqual(result["total_modulos"], 6)

    def test_keywords_capture_unknown_input_libraries(self):
        result = classify(["InControlNative.dll", "Rewired_Windows.dll", "d3d11.dll"])
        self.assertEqual(result["modulos_relevantes"], ["incontrolnative.dll", "rewired_windows.dll"])
        json.dumps(result)


if __name__ == "__main__":
    unittest.main()
