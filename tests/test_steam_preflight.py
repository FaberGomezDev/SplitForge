import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from splitforge.steam_library import InstalledGame, scan_steam, library_paths
from splitforge.steam_preflight import RESEARCH_PROFILES, evaluate, public_report, report_json, steam_uri


class PreflightTests(unittest.TestCase):
    def create_game(self, root: Path, installed=True, exe=True):
        apps = root / 'steamapps'
        apps.mkdir(parents=True, exist_ok=True)
        folder = apps / 'common' / '7 Days To Die'
        if installed:
            folder.mkdir(parents=True, exist_ok=True)
        if installed and exe:
            (folder / '7DaysToDie.exe').write_bytes(b'fixture')
        manifest = apps / 'appmanifest_251570.acf'
        manifest.write_text('"AppState"\n{\n  "appid" "251570"\n'
                            '  "name" "7 Days to Die"\n'
                            '  "installdir" "7 Days To Die"\n'
                            '  "buildid" "100234"\n  "StateFlags" "4"\n}', encoding='utf8')
        return manifest

    def test_preflight_detects_executable_without_declaring_coop(self):
        with TemporaryDirectory() as td:
            root = Path(td) / 'Steam'
            manifest = self.create_game(root)
            before = manifest.read_bytes()
            games = scan_steam([root])
            self.assertEqual(len(games), 1)
            p = evaluate(games[0])
            self.assertEqual(p.build_id, '100234')
            self.assertTrue(p.folder_present)
            self.assertTrue(p.inside_steam_library)
            self.assertTrue(p.expected_executable_found)
            self.assertFalse(p.ready_for_two_players)
            self.assertFalse(p.offline_coop_verified)
            self.assertFalse(p.input_isolation_verified)
            self.assertEqual(manifest.read_bytes(), before)

    def test_missing_executable_does_not_claim_supported(self):
        with TemporaryDirectory() as td:
            root = Path(td) / 'Steam'
            self.create_game(root, exe=False)
            p = evaluate(scan_steam([root])[0])
            self.assertFalse(p.expected_executable_found)
            self.assertFalse(p.ready_for_two_players)

    def test_report_excludes_sensitive_full_paths(self):
        with TemporaryDirectory() as td:
            root = Path(td) / 'Users' / 'private-name' / 'Steam'
            self.create_game(root)
            report = report_json(scan_steam([root]))
            data = json.loads(report)
            self.assertEqual(data['game_count'], 1)
            self.assertEqual(data['games'][0]['app_id'], '251570')
            self.assertNotIn('private-name', report)
            self.assertNotIn(str(root), report)
            self.assertNotIn('folder', data['games'][0])

    def test_steam_path_outside_library_rejected(self):
        with TemporaryDirectory() as td:
            root = Path(td) / 'Steam'
            (root / 'steamapps' / 'common').mkdir(parents=True)
            game = InstalledGame('251570', '7 Days to Die', str(root / '..' / 'secret'), str(root))
            assessment = evaluate(game)
            self.assertFalse(assessment.inside_steam_library)
            self.assertFalse(assessment.folder_present)

    def test_unrecognized_appid_not_verified(self):
        with TemporaryDirectory() as td:
            root = Path(td) / 'Steam'
            folder = root / 'steamapps' / 'common' / 'Unknown'
            folder.mkdir(parents=True)
            (folder / 'MyGame.exe').write_bytes(b'test')
            game = InstalledGame('99999999', 'Unknown', str(folder), str(root))
            p = evaluate(game)
            self.assertIn('Sin perfil', p.phase)
            self.assertIsNone(p.expected_executable_found)
            self.assertFalse(p.ready_for_two_players)

    def test_steam_uri(self):
        self.assertEqual(steam_uri('251570'), 'steam://rungameid/251570')
        for value in ('', 'abc', '-251570', '251570?x=y', '0', '1/../../'):
            with self.subTest(value=value):
                with self.assertRaises(ValueError):
                    steam_uri(value)

    def test_second_library_index_location(self):
        with TemporaryDirectory() as td:
            root = Path(td) / 'Steam'
            another = Path(td) / 'SteamLibrary'
            (root / 'config').mkdir(parents=True)
            another.mkdir()
            (root / 'config' / 'libraryfolders.vdf').write_text(
                '"libraryfolders" { "1" { "path" "' + str(another) + '" } }', encoding='utf-8')
            self.assertIn(another, library_paths(root))


if __name__ == '__main__':
    unittest.main()
