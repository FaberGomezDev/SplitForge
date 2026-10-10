"""Punto de entrada SplitForge (Windows 10/11)."""
from __future__ import annotations
import argparse
import os
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="SplitForge - laboratorio y diagnóstico Steam")
    parser.add_argument("--demo-player", type=int, choices=(1, 2))
    parser.add_argument("--shared-player", type=int, choices=(1, 2))
    args = parser.parse_args()
    if os.name != "nt":
        print("SplitForge funciona solamente en Windows 10/11.", file=sys.stderr)
        return 2
    from splitforge.windows import enable_dpi_awareness
    enable_dpi_awareness()
    if args.shared_player:
        from splitforge.shared_demo import run_shared_demo
        run_shared_demo(args.shared_player)
    elif args.demo_player:
        from splitforge.demo import run_demo
        run_demo(args.demo_player)
    else:
        from splitforge.gui import run_app
        run_app(str(Path(__file__).resolve()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
