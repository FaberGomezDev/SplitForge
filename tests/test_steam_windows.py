"""Regresiones: identificación correcta de los procesos de juegos Steam."""
from __future__ import annotations
from unittest import TestCase
from splitforge.steam_library import InstalledGame
from splitforge.windows import WindowInfo
from splitforge.steam_windows import (
    is_game_executable, inspect_game_windows, verify_pair,
)

GAME = InstalledGame('251570', '7 Days to Die',
                     'C:\\SteamLibrary\\steamapps\\common\\7 Days To Die',
                     'C:\\SteamLibrary', build_id='12345')
EXE = r'C:\SteamLibrary\steamapps\common\7 Days To Die\7DaysToDie.exe'


class TestSteamWindows(TestCase):
    def test_path_boundaries(self):
        self.assertTrue(is_game_executable(EXE, GAME.folder))
        self.assertTrue(is_game_executable(EXE.upper(), GAME.folder.lower()))
        self.assertFalse(is_game_executable(r'C:\SteamLibrary\steamapps\common\7 Days To Die HACK\x.exe', GAME.folder))
        self.assertFalse(is_game_executable(r'C:\SteamLibrary\steamapps\common\Other\game.exe', GAME.folder))
        self.assertFalse(is_game_executable('relative\\7DaysToDie.exe', GAME.folder))
        self.assertFalse(is_game_executable('\\\\host\\share\\7DaysToDie.exe', GAME.folder))
        self.assertFalse(is_game_executable(None, GAME.folder))

    def test_no_windows(self):
        report = inspect_game_windows(GAME, [], lambda _: EXE)
        self.assertEqual(report.state, 'SIN_VENTANAS')
        self.assertEqual(report.chosen_handles, ())

    def test_single_process_two_windows_not_two_clients(self):
        visible = [WindowInfo(100, 9, 'Main'), WindowInfo(101, 9, 'Settings')]
        r = inspect_game_windows(GAME, visible, lambda _: EXE)
        self.assertEqual(len(r.windows), 2)
        self.assertEqual(r.distinct_pids, 1)
        self.assertEqual(r.chosen_handles, ())

    def test_two_clients_same_binary(self):
        visible = [WindowInfo(100, 9, '7DTD P1'), WindowInfo(101, 10, '7DTD P2')]
        r = inspect_game_windows(GAME, visible, lambda _: EXE)
        self.assertEqual(r.state, 'DOS_CLIENTES_DETECTADOS')
        self.assertEqual(r.chosen_handles, (100, 101))
        self.assertEqual(verify_pair(GAME, r, visible, lambda _: EXE), (100, 101))

    def test_different_exes_are_not_two_game_clients(self):
        visible = [WindowInfo(100, 9, '7DTD P1'), WindowInfo(101, 10, 'Launcher')]
        def lookup(pid):
            return EXE if pid == 9 else EXE.replace('7DaysToDie.exe', '7dLauncher.exe')
        r = inspect_game_windows(GAME, visible, lookup)
        self.assertEqual(r.distinct_pids, 2)
        self.assertEqual(r.chosen_handles, ())

    def test_different_game_windows_ignored(self):
        visible = [WindowInfo(100, 9, '7DTD'), WindowInfo(101, 10, 'Other')]
        r = inspect_game_windows(GAME, visible, lambda pid: EXE if pid == 9 else 'C:\\other\\game.exe')
        self.assertEqual(len(r.windows), 1)

    def test_revalidates_window_handles_before_attach(self):
        r = inspect_game_windows(GAME, [WindowInfo(100, 9, 'P1'), WindowInfo(101, 10, 'P2')], lambda _: EXE)
        with self.assertRaises(ValueError):
            verify_pair(GAME, r, [WindowInfo(100, 9, 'P1'), WindowInfo(102, 10, 'P2')], lambda _: EXE)
        with self.assertRaises(ValueError):
            verify_pair(GAME, r, [WindowInfo(100, 9, 'P1'), WindowInfo(101, 10, 'P2')],
                        lambda pid: EXE if pid == 9 else 'C:\\other\\game.exe')

    def test_public_report_never_exposes_paths_or_window_titles(self):
        r = inspect_game_windows(GAME, [WindowInfo(100, 9, 'Sala de Fulanito')], lambda _: EXE)
        import json
        data = json.dumps(r.public_dict(), ensure_ascii=False)
        self.assertNotIn('C:\\Steam', data)
        self.assertNotIn('Fulanito', data)
        self.assertNotIn('100', data)
        self.assertIn('7DaysToDie.exe', data)

    def test_lookup_errors_do_not_crash(self):
        r = inspect_game_windows(GAME, [WindowInfo(100, 9, 'P1')], lambda _: (_ for _ in ()).throw(OSError('permission')))
        self.assertEqual(r.windows, ())

    def test_same_handle_no_duplicate(self):
        r = inspect_game_windows(GAME, [WindowInfo(100, 9, 'P1'), WindowInfo(100, 10, 'P2')], lambda _: EXE)
        self.assertEqual(len(r.windows), 1)
