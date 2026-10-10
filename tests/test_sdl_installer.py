"""Comprobación de archivos SDL offline sin red ni Windows."""
import tempfile
import unittest
from pathlib import Path
from setup_deps import target_tag, matching_wheels


class TestSdlInstaller(unittest.TestCase):
    def test_filename_windows_python_313(self):
        self.assertEqual(target_tag((3, 13), 64), '-cp313-cp313-win_amd64.whl')

    def test_matches_local_wheel(self):
        with tempfile.TemporaryDirectory() as td:
            good = Path(td) / 'pygame_ce-2.5.8-cp313-cp313-win_amd64.whl'
            bad = Path(td) / 'pygame_ce-2.5.8-cp312-cp312-win_amd64.whl'
            good.touch()
            bad.touch()
            self.assertEqual(matching_wheels([Path(td)], (3, 13), 64), [good])

    def test_missing_folder_no_crash(self):
        self.assertEqual(matching_wheels([Path('/nonexistent-splitforge-wheel-dir')], (3, 13), 64), [])

if __name__ == '__main__':
    unittest.main()
