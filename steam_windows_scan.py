"""Escáner de ventanas de Steam en modo lectura para diagnóstico compartible."""
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path
from splitforge.steam_library import scan_steam
from splitforge.steam_windows import inspect_game_windows


def main() -> int:
    parser = argparse.ArgumentParser(description='Analiza ventanas abiertas asociadas a Steam')
    parser.add_argument('--appid', default='251570', help='AppID Steam (por defecto 7 Days to Die)')
    parser.add_argument('--all', action='store_true', help='Analizar todos los juegos instalados')
    parser.add_argument('--output', type=Path, help='Archivo JSON de salida')
    args = parser.parse_args()
    if os.name != 'nt':
        print('Este diagnóstico de ventanas requiere Windows 10/11.')
        return 2
    games = scan_steam()
    selected = games if args.all else [g for g in games if g.app_id == args.appid]
    reports = [inspect_game_windows(g).public_dict() for g in selected]
    result = {'schema': 'SplitForge Steam Windows v1',
              'installed_count': len(games),
              'requested_appid': None if args.all else args.appid,
              'games': reports,
              'notice': ('Solo enumera procesos. Dos instancias no garantizan multijugador, '
                         'offline ni aislamiento de mandos.')}
    desktop = Path(os.path.join(os.environ.get('USERPROFILE', str(Path.home())), 'Desktop'))
    target = args.output or (desktop / 'SplitForge_Reporte_Ventanas_Steam.json')
    if not target.parent.exists():
        target = Path.cwd() / target.name
    target.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
    print('Reporte guardado:', target)
    print(f"Juegos encontrados: {len(games)} | analizados: {len(reports)}")
    for r in reports:
        print(f"{r['juego']}: {r['estado']} ({r['ventanas']} ventanas; {r['procesos_distintos']} procesos)")
    if not reports:
        print('El juego solicitado no aparece instalado en la biblioteca detectada.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
