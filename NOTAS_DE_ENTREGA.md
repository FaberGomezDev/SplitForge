# SplitForge v0.6.0 — Repositorio completo con documentación

**Contenido:** código fuente completo de SplitForge v0.6.0, arranque Windows, dependencias declaradas, 75 pruebas `unittest`, `CLAUDE.md`, documentación técnica `docs/`, plantillas de adaptador y memoria del proyecto.

## Resultado de esta entrega

- Se conserva la base v0.6.0 del launcher y laboratorio probada con los dos mandos PS4.
- Se añaden 3 pruebas automatizadas de documentación; suite completa: **75 tests correctos** en el entorno de generación (no constituye validación física Windows).
- Se crea memoria persistente versionable y arquitectura con separación explícita entre funciones actuales y propuestas.
- Se agrega `tools/check_docs.py` y `.gitignore`.
- **No** se incorpora modo pantalla dividida comercial ni se modifican controladores, DRM o ejecutables Steam.
- **No** se incluye `.venv`, un ejecutable `.exe` autónomo ni binarios pygame-ce; para una instalación nueva usar `Instalar_Todo.bat`.

## Lectura / comandos

1. Abrir `CLAUDE.md` y `docs/README.md`.
2. Windows: `Iniciar_SplitForge.bat` (o `Instalar_Todo.bat` si hace falta instalar entorno).
3. Pruebas: `run_tests.bat` y `python tools/check_docs.py`.
4. Para trabajar en un adaptador real, seguir `docs/05-INTEGRACION-STEAM.md` y `docs/06-PLAN-DE-DESARROLLO.md`.
