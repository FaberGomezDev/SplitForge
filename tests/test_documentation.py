"""Protege el contrato de memoria/documentación del repositorio."""
from pathlib import Path
import subprocess
import sys
import unittest


ROOT = Path(__file__).resolve().parents[1]


class DocumentationTests(unittest.TestCase):
    def test_claude_memory_exists(self):
        self.assertTrue((ROOT / "CLAUDE.md").is_file())
        self.assertTrue((ROOT / "docs" / "01-MEMORIA.md").is_file())

    def test_required_architecture_docs_exist(self):
        for name in ("02-ARQUITECTURA.md", "05-INTEGRACION-STEAM.md", "06-PLAN-DE-DESARROLLO.md", "08-SEGURIDAD.md"):
            with self.subTest(name=name):
                self.assertTrue((ROOT / "docs" / name).is_file())

    def test_internal_document_links(self):
        outcome = subprocess.run([sys.executable, str(ROOT / "tools" / "check_docs.py")],
                                 capture_output=True, text=True, timeout=10)
        self.assertEqual(outcome.returncode, 0, outcome.stdout + outcome.stderr)


if __name__ == "__main__":
    unittest.main()
