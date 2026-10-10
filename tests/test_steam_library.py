import tempfile
import unittest
from pathlib import Path
from splitforge.steam_library import scan_steam, kv_value, library_paths

class SteamScannerTests(unittest.TestCase):
    def test_value_parser(self):
        self.assertEqual(kv_value('"name" "7 Days to Die"', 'name'), '7 Days to Die')

    def test_steam_library_uses_acf_no_changes(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp) / 'Steam'
            apps = root / 'steamapps'
            apps.mkdir(parents=True)
            man = apps / 'appmanifest_251570.acf'
            content = '"AppState"\n{\n"appid" "251570"\n"name" "7 Days to Die"\n"installdir" "7 Days To Die"\n}'
            man.write_text(content, encoding='utf-8')
            old = man.read_bytes()
            res = scan_steam([root])
            self.assertEqual(len(res), 1)
            self.assertEqual(res[0].app_id, '251570')
            self.assertEqual(res[0].status, 'Sin adaptador verificado')
            self.assertEqual(man.read_bytes(), old)

    def test_multiple_libraries(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp) / 'Steam'
            second = Path(temp) / 'SteamLibrary'
            (root / 'steamapps').mkdir(parents=True)
            second.mkdir()
            (root / 'steamapps' / 'libraryfolders.vdf').write_text(
                '"libraryfolders"\n{\n"1"\n{\n"path" "' + str(second) + '"\n}\n}', encoding='utf-8')
            self.assertIn(second, library_paths(root))

if __name__ == '__main__':
    unittest.main()
