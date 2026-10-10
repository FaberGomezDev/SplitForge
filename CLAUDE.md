# CLAUDE.md — Guía obligatoria de desarrollo de SplitForge

> **Leer primero.** Esta guía describe el repositorio REAL. No confundir los objetivos del roadmap con características implementadas.
> Versión base de código: **0.6.0**. Esta entrega agrega documentación; **no incorpora soporte cooperativo comercial nuevo**.

## Contexto del proyecto

SplitForge es un prototipo para Windows que aspira a facilitar **pantalla dividida en una sola PC** para juegos de Steam compatibles: cada jugador con su mando y su perspectiva, participando en la misma partida local/LAN cuando el título lo permita. El usuario probó dos controles PS4 por Bluetooth, con `pygame-ce 2.5.8 / SDL2`, y confirmó funcionamiento del **laboratorio de mundo compartido**. Todavía **no** se ha validado ningún juego comercial con dos jugadores independientes.

Hardware de referencia de pruebas del proyecto: Ryzen 7 7700X, RTX 4060 (8 GB VRAM), 32 GB RAM, dos controles PS4, Windows 11, Python 3.13. No almacenar nombres de cuenta, MAC, rutas de perfil o IDs exclusivos de dispositivos en docs ni reportes compartidos.

## Orden de lectura para una sesión nueva

1. [`docs/README.md`](docs/README.md) — índice y estado.
2. [`docs/01-MEMORIA.md`](docs/01-MEMORIA.md) — avances comprobados, fallos y contexto.
3. [`docs/02-ARQUITECTURA.md`](docs/02-ARQUITECTURA.md) — arquitectura **actual** vs **propuesta**.
4. [`docs/06-PLAN-DE-DESARROLLO.md`](docs/06-PLAN-DE-DESARROLLO.md) — prioridad y criterios de aceptación.
5. [`docs/05-INTEGRACION-STEAM.md`](docs/05-INTEGRACION-STEAM.md) — límites reales por juego.
6. [`docs/07-PRUEBAS.md`](docs/07-PRUEBAS.md) y [`docs/08-SEGURIDAD.md`](docs/08-SEGURIDAD.md) antes de tocar procesos, entrada o guardados.

## Reglas de Git (decisión del propietario, obligatorias)

- Trabajar y hacer push **directamente en `main`**. **Nunca** crear ramas `claude/*` ni ramas con nombre de asistente.
- Los commits y PR **no** llevan firmas de asistente: nada de `Co-Authored-By`, `Claude-Session`, "Generated with" ni similares.
- Mensajes de commit en español, describiendo el cambio real.

## Reglas de implementación

- **No reescribir sin necesidad** la ruta probada `ControllerManager` → `SessionManager` → `SharedWorld` → `WorldWriter` → `shared_demo`.
- Primero diagnosticar con datos reales. En el pasado SDL **enumeraba** los PS4 pero no entregaba movimiento por la ruta elegida; se corrigió prefiriendo `pygame.joystick` y aprendiendo botones. Detección ≠ lectura ≠ asignación.
- Mantener comunicación local sin red para el laboratorio. Las dos ventanas comparten el estado de **una** simulación, no dos juegos sincronizados mágicamente.
- Separar siempre **hecho comprobado / supuesto / objetivo futuro** en código, interfaz, docs y changelog.
- Nunca declarar que un juego de Steam es compatible por detectar su instalación, un proceso o dos ventanas. Se requieren dos clientes autorizados + mandos independientes + mismo mundo + cierre seguro + prueba en hardware.
- No introducir bypass de DRM/licencias, suplantación de cuentas, evasión antitrampas ni desactivación de protecciones. Consultar `docs/08-SEGURIDAD.md`.
- No eliminar ni sobrescribir `.venv` o `%LOCALAPPDATA%\SplitForge\settings.json` del usuario. El archivo `.venv` **no se incluye** en el ZIP de código fuente: se prepara en Windows ejecutando `Instalar_Todo.bat`.
- No cerrar a la fuerza procesos de juegos externos. Solo se pueden terminar procesos **de demostración** creados por SplitForge después de un cierre normal fallido.
- No almacenar rutas absolutas personales, IDs de cuenta, nombres de ventanas o direcciones de dispositivos en informes públicos.
- No afirmar mediciones de FPS/VRAM que todavía no se hayan recogido en una ejecución controlada.
- Preferir cambios pequeños y pruebas de regresión; no hacer migración a C++/C# sin un ADR y pruebas de coste/beneficio.

## Mapa de código

| Archivo | Propósito |
| --- | --- |
| `launcher.py` | Entrada, GUI / visores demo, Windows obligatorio |
| `splitforge/gui.py` | Interfaz Tkinter, configuración, pestañas Steam y sesiones |
| `splitforge/gamepads.py` | Lectura SDL2/joystick, calibración y estados por mando |
| `splitforge/winmm_gamepads.py` | Respaldo de lectura Windows WinMM |
| `splitforge/session.py` | Ciclo de vida de procesos/ventanas e IPC |
| `splitforge/shared_world.py` | Mundo compartido autoritativo del laboratorio |
| `splitforge/shared_io.py` | Instantáneas JSON a visores por stdin |
| `splitforge/input_stream.py` | Entrada a demos simples por stdin |
| `splitforge/windows.py`, `layout.py` | Win32, colocación y restauración de ventanas |
| `splitforge/steam_library.py` | Lectura pasiva de manifestos y bibliotecas Steam |
| `splitforge/steam_preflight.py` | Evaluación de precondiciones, SIN certificar compatibilidad |
| `splitforge/steam_windows.py` | Asociar HWND ↔ PID ↔ ejecutable; no coop/mandos |
| `tests/` | Suite de regresión con `unittest` |

Ver [`docs/03-MODULOS.md`](docs/03-MODULOS.md) para contratos, dependencias y riesgos.

## Entorno y comandos

Windows 11 (probado por el usuario para el laboratorio):

```bat
Instalar_Todo.bat
Iniciar_SplitForge.bat
run_tests.bat
Diagnostico_Mandos.bat
Escanear_Steam.bat
Analizar_Ventanas_Steam.bat
```

Pruebas desde terminal con Python apropiado:

```powershell
.\.venv\Scripts\python.exe -m unittest discover -s tests -v
.\.venv\Scripts\python.exe tools\check_docs.py
```

En otro SO se pueden ejecutar pruebas puras con `python -m unittest discover -s tests -v`; **no significa** que el launcher Win32 funcione allí. Dependencia fijada: `pygame-ce==2.5.8`, la cual incluye SDL en su wheel de Windows; no instalar SDL aparte.

## Antes de dar un trabajo por terminado

1. Revisar impacto en controles P1/P2, IPC, posiciones de ventanas, guardados y Steam.
2. Agregar/ajustar pruebas. Ejecutar `python -m unittest discover -s tests -v`.
3. Si cambias docs o archivos, ejecutar `python tools/check_docs.py`.
4. Documentar exactamente lo implementado y lo que sigue sin comprobarse en `docs/01-MEMORIA.md` y `docs/15-HISTORIAL.md`.
5. Si se cambia arquitectura, añadir decisión a `docs/12-DECISIONES.md`; si se verifica un juego, registrar condiciones y evidencias en `docs/05-INTEGRACION-STEAM.md`.
6. No pedir al usuario reinstalar SDL/pygame-ce salvo diagnóstico específico que muestre que faltan o están rotos.
7. Probar manualmente en **Windows con ambos mandos** antes de llamar una versión “verificada en hardware”.

## Próxima meta explícita

**No otra simulación.** Preparar investigación reproducible del primer adaptador de un juego **real** instalado en Steam, candidato inicial 7 Days to Die, solo después de obtener los reportes locales y confirmar lanzamiento legal, sesión local/LAN, entradas independientes, guardados y rendimiento. La v0.6 **no** crea un segundo cliente Steam. Véanse los criterios de salida en `docs/06-PLAN-DE-DESARROLLO.md`.

## Memoria de desarrollo

`CLAUDE.md` funciona como punto de entrada y `docs/01-MEMORIA.md` como fuente persistente del contexto del repositorio. Es documentación versionada, **no** memoria automática del asistente. En cada entrega actualizar la memoria con fecha, modificación, prueba realizada, evidencia y siguiente paso, evitando registrar información personal.
