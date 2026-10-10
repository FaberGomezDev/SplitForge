# CLAUDE.md — Guía obligatoria de desarrollo de SplitForge

> **Leer primero.** Esta guía describe el repositorio REAL. No confundir los objetivos del roadmap con características implementadas.
> Versión de código: **0.7.0** (reconstrucción en .NET 10 + SDL3). El prototipo Python **0.6.0** sigue en la raíz como referencia validada en hardware.

## Reglas de Git (decisión del propietario, obligatorias)

- Trabajar y hacer push **directamente en `main`**. **Nunca** crear ramas `claude/*` ni ramas con nombre de asistente.
- Los commits y PR **no** llevan firmas de asistente: nada de `Co-Authored-By`, `Claude-Session`, "Generated with" ni similares.
- Mensajes de commit en español, describiendo el cambio real.

## Contexto del proyecto

SplitForge es una aplicación para Windows que aspira a **pantalla dividida en una sola PC** para juegos de Steam compatibles: cada jugador con su mando y su perspectiva, en la misma partida local/LAN cuando el título lo permita. El propietario eligió explícitamente **las mejores tecnologías para eficiencia y fluidez**: por eso la 0.7 abandona Python/Tkinter (ADR-010).

Evidencia física: dos PS4 por Bluetooth funcionaron en el laboratorio del **prototipo Python** (pygame-ce 2.5.8 / SDL2). La versión .NET **aún no se ha validado en el hardware del propietario**. Ningún juego comercial está validado con dos jugadores independientes.

Hardware de referencia: Ryzen 7 7700X, RTX 4060 (8 GB), 32 GB RAM, dos PS4, Windows 11. No almacenar nombres de cuenta, MAC, rutas de perfil ni IDs únicos de dispositivos en docs ni reportes.

## Orden de lectura para una sesión nueva

1. [`docs/README.md`](docs/README.md) — índice y estado.
2. [`docs/01-MEMORIA.md`](docs/01-MEMORIA.md) — avances comprobados, fallos y contexto.
3. [`docs/02-ARQUITECTURA.md`](docs/02-ARQUITECTURA.md) — arquitectura **actual** vs **propuesta**.
4. [`docs/06-PLAN-DE-DESARROLLO.md`](docs/06-PLAN-DE-DESARROLLO.md) — prioridad y criterios de aceptación.
5. [`docs/05-INTEGRACION-STEAM.md`](docs/05-INTEGRACION-STEAM.md) — límites reales por juego y adaptador 7DTD.
6. [`docs/07-PRUEBAS.md`](docs/07-PRUEBAS.md) y [`docs/08-SEGURIDAD.md`](docs/08-SEGURIDAD.md) antes de tocar procesos, entrada o guardados.

## Reglas de implementación

- **Hilos:** la entrada (SDL3, hasta 1000 Hz) y la simulación del laboratorio (120 Hz) viven en hilos propios. La interfaz WPF **solo refleja** estado; nunca meter lectura de mandos ni simulación en el hilo de UI.
- **Solo el hilo de `SdlInputService` llama a SDL** para mandos. El resto lee `Latest` (instantánea inmutable).
- Detección ≠ lectura ≠ asignación. PS4 usa joystick directo en modo Auto (ADR-002); el índice de Cruz se aprende, no se asume.
- Separar siempre **hecho comprobado / supuesto / objetivo futuro** en código, interfaz, docs y commits.
- Nunca declarar compatible un juego por detectar su instalación, un proceso o dos ventanas. Estado máximo automático: `INSPECCIONADO`.
- **EAC:** los componentes que lo requieran (filtro XInput) se usan con el juego abierto mediante la opción oficial «Launch game without EAC». Ver `docs/08-SEGURIDAD.md`.
- No cerrar a la fuerza procesos externos. Solo se terminan procesos **creados por SplitForge** (visores, servidor 7DTD) tras un cierre normal fallido.
- No modificar carpetas de juegos. El servidor 7DTD usa configuración y guardados **propios** en `%LOCALAPPDATA%\SplitForge\7dtd`.
- No tocar `.venv` ni `%LOCALAPPDATA%\SplitForge\settings.json` (versión Python). La versión .NET usa `settings.v2.json`.
- No afirmar mediciones de FPS/VRAM no recogidas en una ejecución controlada.
- C++ solo para piezas nativas justificadas por evidencia (aislamiento de entrada dentro del juego, captura de frametimes), con ADR.

## Mapa de código (.NET 10)

| Proyecto | Propósito |
| --- | --- |
| `src/SplitForge.Core` | Lógica pura y probada: distribución 2–4, Steam (VDF, biblioteca, preflight, informes), ventanas de juego, mandos, laboratorio, IPC binario, sesiones, adaptadores (7DTD), configuración, registro |
| `src/SplitForge.Windows` | Win32 (`LibraryImport`): ventanas sin bordes, monitores, procesos/módulos solo lectura, temporizador de alta resolución, Steam, Sandboxie-Plus, diagnósticos |
| `src/SplitForge.Input` | `SdlInputService`: SDL3 en hilo dedicado, eventos en segundo plano, luz/vibración por jugador |
| `src/SplitForge.LabViewer` | Visor del mundo compartido con el renderizador de SDL3 (Direct3D) y vsync |
| `src/SplitForge.App` | Interfaz WPF (`SplitForge.exe`); con `--lab-viewer N` actúa como visor |
| `src/SplitForge.Diagnostico` | Consola de solo lectura: `steam`, `ventanas`, `7dtd`, `mandos` |
| `native/SplitForge.XInputFilter` | C++20: filtro XInput por proceso (experimental, ADR-015); CMake + prueba de humo |
| `native/SplitForge.SteamEmu` | C++20: emulador educativo temporal de `steam_api64.dll` (ADR-016); integrado en la pestaña 07 y publicado como `native\SplitForge.SteamEmu`; **eliminar tras el experimento** |
| `tests/SplitForge.Tests` | xUnit (Core) |
| `launcher.py`, `splitforge/`, `tests/test_*.py` | Prototipo Python 0.6 (referencia; no ampliar salvo diagnósticos) |

Ver [`docs/03-MODULOS.md`](docs/03-MODULOS.md) para contratos y riesgos.

## Entorno y comandos

```powershell
dotnet test SplitForge.slnx -c Release                     # pruebas .NET
dotnet run --project src/SplitForge.App                    # interfaz (Windows)
dotnet run --project src/SplitForge.Diagnostico -- 7dtd    # diagnóstico 7DTD
Compilar_SplitForge.bat                                    # pruebas + publish\SplitForge autónomo
python -m unittest discover -s tests -v                    # prototipo Python
python tools/check_docs.py                                 # enlaces de documentación
```

En Linux se compila todo (incluida la app WPF, con `EnableWindowsTargeting`) y se ejecutan las pruebas de Core; SDL3 funciona (hay `libSDL3.so`), así que el hilo de entrada y el visor pueden probarse con `xvfb-run`. Eso **no** valida Win32 ni el hardware. La CI (`.github/workflows/build.yml`) prueba y publica en `windows-latest`; el paquete `SplitForge-win-x64` queda como artefacto.

## Antes de dar un trabajo por terminado

1. Revisar impacto en mandos J1/J2, IPC, posiciones de ventanas, guardados, Steam y servidor 7DTD.
2. Agregar/ajustar pruebas; `dotnet test` y `python -m unittest` en verde.
3. `python tools/check_docs.py` si cambian docs.
4. Documentar lo implementado y lo que sigue sin comprobarse en `docs/01-MEMORIA.md` y `docs/15-HISTORIAL.md`.
5. Si cambia la arquitectura, ADR en `docs/12-DECISIONES.md`; si se verifica un juego, evidencias en `docs/05-INTEGRACION-STEAM.md`.
6. Probar en **Windows con ambos mandos** antes de llamar a una versión "verificada en hardware".

## Próxima meta explícita

Validar la 0.7 en el PC del propietario (mandos, laboratorio, pantalla dividida) y obtener `SplitForge_Reporte_7DTD.json` con 7DTD abierto **sin EAC**. Con esa evidencia, validar el **filtro XInput nativo** (ADR-015) o añadir simulación de foco si el juego la exige (ver `docs/06-PLAN-DE-DESARROLLO.md`).

## Memoria de desarrollo

`CLAUDE.md` es el punto de entrada y `docs/01-MEMORIA.md` la fuente persistente del contexto. Es documentación versionada, **no** memoria automática del asistente. En cada entrega: fecha, cambio, prueba, evidencia y siguiente paso, sin datos personales.
