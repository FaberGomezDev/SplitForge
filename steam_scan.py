"""Solo lectura: lista instalaciones Steam y genera diagnóstico compartible."""
from __future__ import annotations
import argparse
import os
from pathlib import Path
from splitforge.steam_library import scan_steam
from splitforge.steam_preflight import report_json, evaluate


def main() -> int:
    parser = argparse.ArgumentParser(description='SplitForge - inspección local de juegos Steam')
    parser.add_argument('--output', help='Archivo JSON de destino; predeterminado en el escritorio')
    parser.add_argument('--appid', help='Inspeccionar solo este juego, si está instalado')
    args = parser.parse_args()
    games = scan_steam()
    if args.appid:
        games = [game for game in games if game.app_id == args.appid]
    if args.output:
        output = Path(args.output)
    else:
        desktop = Path(os.environ.get('USERPROFILE', str(Path.home()))) / 'Desktop'
        output = (desktop if desktop.is_dir() else Path.cwd()) / 'SplitForge_Reporte_Steam.json'
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(report_json(games), encoding='utf-8')
    print(f'Instalaciones encontradas: {len(games)}')
    for game in games:
        p = evaluate(game)
        print(f' - {p.name} [AppID {p.app_id}] | {p.phase} | doble jugador verificado: NO')
    print(f'Archivo guardado en: {output}')
    print('El reporte no incluye rutas absolutas, cuentas de Steam ni datos de mandos.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
