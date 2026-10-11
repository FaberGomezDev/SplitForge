# CLAUDE.md — Guía obligatoria de desarrollo de SplitForge

> **Leer primero.** Esta guía describe el repositorio REAL. No confundir los objetivos del roadmap con características implementadas.
> Versión de código: **0.7.0** (reconstrucción en .NET 10 + SDL3). El prototipo Python **0.6.0** sigue en la raíz como referencia validada en hardware.

## Reglas de Git (decisión del propietario, obligatorias)

- Trabajar y hacer push **directamente en `main`**. **Nunca** crear ramas `claude/*` ni ramas con nombre de asistente.
- Los commits y PR **no** llevan firmas de asistente: nada de `Co-Authored-By`, `Claude-Session`, "Generated with" ni similares.
- Mensajes de commit en español, describiendo el cambio real.

## Contexto del proyecto

SplitForge es una aplicación para Windows que aspira a **pantalla dividida en una sola PC** para juegos de Steam compatibles: cada jugador con su mando y su perspectiva, en la misma partida local/LAN cuando el título lo permita. El propietario eligió explícitamente **las mejores tecnologías para eficiencia y fluidez**: por eso la 0.7 abandona Python/Tkinter (ADR-010).

Evidencia física: dos PS4 por Bluetooth funcionaron en el laboratorio del **prototipo Python** (pygame-ce 2.5.8 / SDL2). La versión .NET **aún no se ha validado con mandos en el hardware del propietario**; en su PC solo se ha medido (2026-10-10) el hilo de entrada SDL3 3.5.0 a 1000 Hz, sin mandos conectados. Ningún juego comercial está validado con dos jugadores independientes.

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
| `src/SplitForge.Core` | Lógica pura y probada: distribución 2–4, Steam (VDF, biblioteca, preflight, informes), ventanas de juego, mandos, laboratorio, IPC binario, sesiones, adaptadores (7DTD, filtro XInput, emulador de Steam, No Man's Sky sin validar), catálogo de la biblioteca del lanzador (`LauncherGames`, ADR-027), configuración, registro |
| `src/SplitForge.Windows` | Win32 (`LibraryImport`): ventanas sin bordes, monitores, procesos/módulos solo lectura, temporizador de alta resolución, Steam, Sandboxie-Plus, ranuras XInput, bloqueo de ratón (`ClipCursor`), bloqueo de entrada «solo mandos» (ganchos de bajo nivel, Ctrl+Alt+B), foco simulado (`WindowFakeFocus`, ADR-020), aviso superpuesto del bloqueo (`OverlayWindow`), ventanas sobre la barra de tareas, atajos globales, diagnósticos |
| `src/SplitForge.Input` | `SdlInputService`: SDL3 en hilo dedicado, eventos en segundo plano, luz/vibración por jugador |
| `src/SplitForge.LabViewer` | Visor del mundo compartido con el renderizador de SDL3 (Direct3D) y vsync |
| `src/SplitForge.App` | Lanzador WPF (`SplitForge.exe`, ADR-024): JUGAR (biblioteca con 7 Days to Die y No Man's Sky, ADR-027), MANDOS y AVANZADO (con una pestaña por juego); tema en `Theme.xaml`; con `--lab-viewer N` actúa como visor |
| `src/SplitForge.Diagnostico` | Consola de solo lectura: `steam`, `ventanas`, `7dtd`, `xinput`, `mandos` |
| `native/SplitForge.XInputFilter` | C++20: filtro XInput por proceso (experimental, ADR-015), con la XInput real interceptada por su tabla de exportación (ADR-021), foco simulado, un puntero propio por cliente (ADR-025), teclas que se sueltan al perder el foco y mando de prueba para ensayos (ADR-026); CMake + prueba de humo |
| `native/SplitForge.SteamEmu` | C++20: emulador de `steam_api64.dll` (ADR-016), **componente principal y permanente**: una identidad distinta por instancia del juego sin un segundo Steam; se gestiona en AVANZADO › 7 Days to Die y va en el paquete (`native\SplitForge.SteamEmu`); API plana generada con `tools/generar_api_plana.py` (las 944 entradas de Steamworks.NET); CMake + prueba de humo. Con 7DTD real llega al menú con dos instancias |
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
powershell -ExecutionPolicy Bypass -File tools\ensayo_7dtd.ps1   # ensayo sin manos con 7DTD real (ver docs/07-PRUEBAS.md)
powershell -ExecutionPolicy Bypass -File tools\ensayo_nms.ps1 -SinJuego   # pantallas de No Man's Sky en el lanzador, sin abrir el juego
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

Estado real (2026-10-10, PC del propietario, 7DTD build `25661859` = V 3.3.0 b18):
- **Dos clientes de 7DTD abiertos a la vez en el menú principal** con el emulador de Steam, cada uno con su perfil (`SPLITFORGEJ1` / `SPLITFORGEJ2`).
- Lanzados como el handler de Nucleus Co-op: sin juego cruzado (`-crossplatform=None -serverplatforms=Steam`), ventana sin marco con el tamaño de su panel, pantalla dividida automática (arriba/abajo, la preferida del propietario; 16:9 como mínimo).
- **Filtro XInput cargado en los dos** por `xinput1_3.dll`, con J1 → mando 1 y J2 → mando 2, y con la `xinput1_4.dll` real interceptada (ADR-021): **cada cliente enumera un solo mando** (InControl: `XInput::1`).
- Todo lo hace el botón **JUGAR** de la pantalla principal (antes «Lanzar J1 y J2» de la pestaña 07; ver `docs/05-INTEGRACION-STEAM.md`). La interfaz es un lanzador (ADR-024): JUGAR, MANDOS y AVANZADO, que recoge lo técnico. La sesión tapa la barra de tareas y **Ctrl+Alt+B** bloquea teclado y ratón para jugar solo con mandos.

W16 (2026-10-10, noche, propietario): con el foco simulado en los dos clientes (desde fuera, ADR-020, y dentro del filtro), **cada stick mueve las dos ventanas**. Causa comprobada (desensamblado y `Player.log`): Unity usa la copia `xinput1_3` del filtro, pero InControlNative, que es quien mueve al jugador, pide `XInput1_4.dll` por nombre y recibía la real de System32; la había cargado el propio filtro a través de la `XInput1_3.dll` del sistema, que en este Windows reenvía a la 1.4 en ejecución. **Corregido (ADR-021):** la DLL real del filtro es siempre la 1.4 y el filtro redirige su tabla de exportación hacia sí mismo. Comprobado con dos clientes reales y los dos mandos conectados: InControl lista un solo mando en cada uno (antes, dos). **W16 superado:** el propietario lo probó con los mandos y cada uno controla solo su ventana.

**W14 superado (2026-10-10, ADR-022):** J2 entra sola en la partida de J1. El emulador responde al ticket por el canal del servidor de juego, anuncia la partida y el lobby de J1 a las demás instancias y le entrega a J2 la misma petición que Steam al pulsar «Unirse» (`GameServerChangeRequested_t` → `127.0.0.1:26900`). Con las casillas «J2 entra sola en la partida de J1» y «J1 arranca cargando su última partida» (pestaña 07), «Lanzar J1 y J2» deja a los dos en el mismo mundo sin tocar los menús (comprobado con el juego real: 111 s).

Preferencias del propietario (2026-10-10): **nada automático sin pedirlo**. J1 elige su mundo; la unión automática y la carga automática de J1 son casillas desactivadas por defecto, y la unión vale en los dos sentidos. J2 usa su propia carpeta de datos (`%LOCALAPPDATA%\SplitForge\7dtd\clientes\J2`, `-UserDataFolder`): no comparte guardados con J1. Un fondo negro (ADR-023) tapa el escritorio detrás de los juegos. Tras una prueba, restaurar cualquier ajuste del propietario que se haya cambiado.

**Ventana sin foco (2026-10-11, propietario):** con los mandos ya independientes, en la ventana sin foco no funcionaban LB/RB para cambiar de objeto (sí en el inventario). **Causa comprobada y corregida (ADR-026):** al cambiar con Alt+Tab, la ventana que pierde el foco se quedaba con Alt pulsada (el foco simulado le oculta a Unity la desactivación, que es cuando suelta las teclas) y 7DTD, con Alt, hace que LB/RB pasen de página. El filtro suelta ahora las teclas al perder el foco de verdad. **Reproducido y corregido con el juego real** en un ensayo sin manos (`tools/ensayo_7dtd.ps1`, con el mando de prueba del filtro): casilla 8 → 9 sin teclas pegadas, 9 → 9 con Alt pegada y la corrección apagada, 9 → 10 con la corrección. **Falta que el propietario lo confirme jugando (W23).**

También del 2026-10-11: un puntero propio por cliente (ADR-025; apoyado en el código, no era la causa de lo anterior); el foco simulado externo ya no se envía a los clientes que lo simulan por dentro; los juegos se mantienen por encima de la barra de tareas; Ctrl+Alt+B avisa en pantalla (cartel al cambiar, insignia mientras dura; W24). A veces el juego abre un cuadro «Inicio de sesión de Discord» al arrancar: es del juego y al propietario no le molesta; no tocar.

**Siguiente juego pedido por el propietario: No Man's Sky** (AppID 275850). Inspeccionado en solo lectura el 2026-10-11 (`docs/plans/no-mans-sky.md`): **no se puede hacer como 7DTD**. Su multijugador va por servicios en línea (PlayFab) y el ejecutable lleva la protección de Steam; no hay partida por red local y el emulador no sirve. Para la misma partida hacen falta dos copias con dos cuentas. No quitar la protección del ejecutable ni usar una versión antigua. **Decisión del propietario: J2 con una copia en GOG (aún sin comprar).** Sonda con la copia de Steam (`tools/sonda_nms.ps1`, una instancia, hasta el menú): el juego carga el filtro como `xinput9_1_0.dll` pero no responde al mando de prueba; supuesto: con Steam abierto recibe el mando por Steam Input. Siguiente: que el propietario desactive Steam Input para este juego y repetir la sonda antes de comprar. **Ya está en la biblioteca del lanzador (ADR-027) como «EN PREPARACIÓN · SIN VALIDAR»:** J1 se abre a través de Steam y J2 desde su copia (indicada con «Elegir copia de J2…» o detectada si es de GOG), un mando por proceso y pantalla dividida con dos ventanas; el filtro (`xinput9_1_0.dll`) solo se instala con «Preparar ahora» y confirmación. Comprobada la pantalla con UI Automation; **JUGAR y «Cerrar juegos» no se han ejecutado con el juego** (`tools/ensayo_nms.ps1`, avisando antes). El propietario lo esperaba «igual, tal cual» 7DTD: tiene su pestaña en AVANZADO › NO MAN'S SKY (copias, filtro con «Instalar / actualizar», lanzar, dividir, cerrar), **sin emulador**: el juego inicia sesión en su servicio en línea con el pase de la cuenta de Steam (hecho, leído en el ejecutable) y con una identidad inventada cada ventana quedaría sola (supuesto, sin probar). Explicérselo con ejemplos sencillos.

Siguiente: sesión de juego real de los dos jugadores; con sus registros, comprobar lo que falta (unirse con la carpeta propia de J2, unión J2 → J1, «Cerrar J1 y J2» dentro de una partida) y si «Unirse a partida» lista la del otro jugador.

Pendiente en el PC: no están instalados el servidor dedicado (AppID 294420) ni Sandboxie-Plus (solo hacen falta para la vía del servidor oficial).

## Memoria de desarrollo

`CLAUDE.md` es el punto de entrada y `docs/01-MEMORIA.md` la fuente persistente del contexto. Es documentación versionada, **no** memoria automática del asistente. En cada entrega: fecha, cambio, prueba, evidencia y siguiente paso, sin datos personales.
