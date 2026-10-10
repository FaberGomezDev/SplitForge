import tempfile
from pathlib import Path
import unittest
from setup_deps import target_tag, matching_wheels


class DependencyTests(unittest.TestCase):
    def test_tag_windows_313(self):
        self.assertEqual(target_tag((3, 13), 64), '-cp313-cp313-win_amd64.whl')
    def test_tag_windows_311_x86(self):
        self.assertEqual(target_tag((3, 11), 32), '-cp311-cp311-win32.whl')
    def test_find_compatible_wheel_only(self):
        with tempfile.TemporaryDirectory() as td:
            folder = Path(td)
            wanted = folder / 'pygame_ce-2.5.8-cp313-cp313-win_amd64.whl'
            unwanted = folder / 'pygame_ce-2.5.8-cp312-cp312-win_amd64.whl'
            wanted.touch()
            unwanted.touch()
            self.assertEqual(matching_wheels([folder], (3, 13), 64), [wanted])

if __name__ == '__main__':
    unittest.main()
