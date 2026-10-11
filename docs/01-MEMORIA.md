# Memoria persistente del repositorio

**Propósito:** que otro desarrollador o una futura sesión continúe SplitForge sin volver a investigar los mismos fallos. Distingue lo observado por el usuario de lo probado en desarrollo.

## Objetivo no negociable

Una PC Windows con juegos de Steam compatibles, 2–4 jugadores, cada uno con mando y perspectiva propia, compartiendo **la misma sesión de juego**, incluso sin Internet **solo donde el juego soporte LAN/local y autenticación adecuada**. No basta con colocar ventanas ni abrir copias aisladas.

## Preferencias del propietario (vinculantes)

- **Mejores tecnologías para eficiencia, optimización y fluidez** → .NET 10 + SDL3 + C++ nativo donde haga falta (ADR-010). Tkinter descartado.
- Avanzar todo lo posible en cada entrega, no por etapas pequeñas.
- Git: trabajar en `main`, **sin ramas `claude/*` ni firmas de asistente** en commits.
- Objetivo inmediato: **7 Days to Die con dos mandos y dos jugadores en la misma partida**.

## Equipo de referencia y entorno conocido

- Ryzen 7 7700X, RTX 4060 de 8 GB VRAM, 32 GB RAM, Windows 11 (`10.0.26300.0`).
- Prototipo: Python 3.13.13, `pygame-ce 2.5.8`, SDL 2.32.10.
- Dos mandos Bluetooth `Wireless Controller` en Windows, `PS4 Controller` en SDL.
- 7 Days to Die instalado (build `25661859`, actualizado el 2026-10-10). El propietario consiguió iniciarlo **sin EAC** con la opción oficial de Steam ("Preguntar al iniciar" → "Launch game without EAC").
- 2026-10-10 (informe 7DTD): SDL3 ve los dos mandos como `Xbox 360 Controller` (XInput); antes aparecían como `PS4 Controller`.
- 2026-10-10 (comprobación de solo lectura en el PC):
  - Están instalados los controladores **ViGEmBus** (mandos virtuales) y **HidHide** (oculta dispositivos HID). Encaja con PS4 convertidos en mandos Xbox 360 virtuales y los originales ocultos.
  - DS4Windows no figura entre los programas instalados; puede ser una copia portable. El programa que crea los mandos virtuales sigue **sin identificar** (supuesto).
- 2026-10-10: **no** están instalados el servidor dedicado de 7DTD (AppID 294420) ni Sandboxie-Plus. Steam tiene 12 instalaciones en 2 bibliotecas.
- 2026-10-10: en este PC, la 0.7 mide el hilo de entrada SDL3 3.5.0 a **1000 Hz** (`mandos`, sin mandos conectados).

## Línea temporal de aprendizajes

| Etapa | Hecho observado o desarrollado | Estado de evidencia |
| --- | --- | --- |
| v0.1 | Laboratorio simple y organización de ventanas; mandos solo XInput. | Prototipo; problema con PS4 |
| v0.2–0.2.2 | SDL vía `pygame-ce`, alternativa WinMM, instaladores. | Falla original en descubrimiento PS4 |
| v0.3 | Windows mostró dos `Wireless Controller`; SDL registraba `0` al principio. | Reporte del usuario |
| Diagnóstico | `joy.cpl` los mostró; `pygame.joystick.get_count()` devolvió **2**. | Salida del usuario |
| v0.3.1 | Lectura `pygame.joystick` para PS4 en lugar de GameController. | Usuario confirmó movimiento independiente |
| v0.3.2 | Aprendizaje de botones y R2 analógico. | Regresión intermitente del 2.º mando antes de v0.4 |
| v0.4 | Un proceso simula `SharedWorld`, dos visores por stdin. | Usuario: «ya listo si funciona» |
| v0.5 | Biblioteca Steam, evaluación solo lectura, apertura de **una** instancia. | Usuario: «sí funcionó, siguiente» |
| v0.6 | Asociación HWND/PID/exe; dos procesos del mismo exe antes de distribuir. | 72 pruebas; sin aceptación física de dos clientes |
| 0.6.0+docs | `CLAUDE.md`, `docs/`, chequeo de documentación. | 75 pruebas |
| 2026-10-10 | Repositorio en GitHub; diagnóstico Python `diagnostico_7dtd.py` (APIs de mando de 7DTD). | 77 pruebas Python |
| **0.7.0** | Reconstrucción en .NET 10 + SDL3 + WPF; adaptador experimental 7DTD; filtro XInput C++; CI Windows. | 112 xUnit; en la VM Windows de la CI: interfaz arranca y se cierra, 6 pestañas capturadas, enumeración Win32 real, SDL3 a 1000 Hz, filtro C++ probado; **sin prueba en el PC del propietario ni con mandos** |
| 0.7.0+ · `2c20d4a` | **Emulador de Steam (ADR-016):** `native/SplitForge.SteamEmu`, reimplementación ABI de `steam_api64.dll` (técnica de Goldberg) para dar a cada instancia una identidad distinta; **integrado en la interfaz** (pestaña 07: instalar/restaurar, identidades J1/J2, lanzar instancias) y su DLL publicada en el paquete. | Prueba de humo `steamemu_humo` superada en el PC del propietario (2026-10-10) y en la CI de Windows (run #14): dos procesos con identidades distintas, stats+callback, nube y P2P multicast local. Con 7DTD real, ver la fila «memoria» |
| 2026-10-10 · informe 7DTD · `59674e9` | Primer `SplitForge_Reporte_7DTD.json` real: 1 cliente **sin EAC**; carga `xinput1_3` + `xinput1_4`, DirectInput, HID e InControl (Unity); mandos como `Xbox 360 Controller`. En consecuencia: estado del filtro por DLL y en vivo con lecturas, DLL real comprobada en System32, instalación que actualiza solo copias propias y no se detiene a medias, detección de ranura XInput, informe v3 con origen de módulos y comando `xinput`. | Informe real del propietario; 126 xUnit + 77 unittest en ese commit; filtro compilado sin avisos con MinGW; prueba de humo de dos copias en la CI de Windows |
| 2026-10-10 · `665b760` | Pestaña 01: tipo legible de cada mando (PS4/PS5/Xbox/estándar/genérico) y carga de batería en vivo (`SDL_GetJoystickPowerInfo`, una vez por segundo). | Pruebas xUnit de tipo y etiqueta de batería; CI run #12 verde; sin probar con mandos reales |
| 2026-10-10 · `819a45b` | **Bloqueo de ratón por ventana (ADR-017):** `ClipCursor` al área de cliente del jugador en primer plano, con botón y atajo global Ctrl+Alt+L; se suelta al desactivar, detener o cerrar. | Pruebas del atajo (modificadores `MOD_*`); sin validar en hardware (W18) |
| 2026-10-10 · `5d1c748` | **Ajuste de cada juego a su ventana:** «Rellenar» o «Mantener proporción» (16:9, 16:10, 21:9, 4:3) centrada en la región, con resolución sugerida por panel; guardado en `settings.v2.json`. | Pruebas xUnit de distribución y configuración; CI run #13 verde; sin validar en hardware (W19) |
| 2026-10-10 · `fd98ba2` + `783da1d` | Fusión en `main` de la rama `7dtd-diagnostico-xinput` (informe 7DTD, filtro, mandos, bloqueo de ratón, ajuste) con el emulador, conservando las reglas de Git de `main`. | **158 xUnit + 77 unittest**; CI run #16 verde con la prueba de arranque de 7 pestañas |
| 2026-10-10 · memoria | Docs contrastados con el código y con el PC del propietario. El emulador pasa a **componente principal** (decisión del propietario). Datos reales: hilo SDL3 a 1000 Hz en su PC, build de 7DTD, servidor dedicado y Sandboxie-Plus no instalados, ViGEmBus y HidHide instalados. **Primera prueba real del emulador con 7DTD (02:50–03:55):** carga las identidades J1/J2, pero el juego se cierra en `SteamAPI_ISteamClient_GetISteamUser` porque la API plana del emulador no recibe `self` y solo cubre 32 de las 866 entradas que usa Steamworks.NET. | Diagnósticos de solo lectura `steam`, `7dtd`, `mandos` y `xinput` (sin mandos conectados ni 7DTD abierto); registros `splitforge-net.log`, `steamemu.log` y `Player.log` de Unity; análisis de `flat_api.cpp` y del Steamworks.NET de 7DTD |
| 2026-10-10 · emulador + lanzador | **API plana generada** (`tools/generar_api_plana.py`: 683 reenvíos con `self` + 234 entradas neutras = las 944 de Steamworks.NET), `ContextInit` con el contrato del SDK, 7 firmas de vtable y estructuras `pack(1)` como el SDK, call results por `SteamAPICallCompleted_t` (703) con sus datos, respuestas de tickets. Con la referencia del handler de Nucleus Co-op para 7DTD: clientes sin juego cruzado (`-crossplatform=None -serverplatforms=Steam`), ventana del tamaño de su panel y pantalla dividida automática. «Lanzar J1 y J2» lo hace todo (pestaña 07). A petición del propietario: la sesión tapa la barra de tareas y **Ctrl+Alt+B** bloquea teclado y ratón para jugar solo con mandos (ADR-019). | **Dos clientes de 7DTD en el menú principal a la vez**, perfiles `SPLITFORGEJ1`/`SPLITFORGEJ2`; filtro XInput cargado en ambos por `xinput1_3`, J1 → mando 1, J2 → mando 2, lecturas subiendo en ambos; arriba/abajo con la interfaz completa y, tapando la barra, 1920×540 cada uno (la franja de la barra pasa a la ventana de J2). 166 xUnit, prueba de humo del emulador ampliada y del filtro en el PC del propietario. Falta que el propietario confirme con los mandos |
| 2026-10-10 · foco simulado | **FakeFocus (ADR-020):** el propietario informa de que los mandos no son independientes en 7DTD. SplitForge publica desde un hilo propio los mensajes de activación de Windows (`WM_ACTIVATE`, `WM_ACTIVATEAPP`, `WM_NCACTIVATE`, `WM_SETFOCUS`, `WM_MOUSEACTIVATE`) cada 100 ms a cada ventana de la sesión; casilla en la pestaña 07, activada por defecto. Sin inyección ni cambios en el juego. | 171 xUnit en Linux (1 fallo previo de rutas de Windows en `SteamEmulatorTests`, ajeno al cambio); compilación sin avisos; **sin probar con 7DTD ni con mandos** |
| 2026-10-10 · XInput interceptada | **Causa real de W16 y corrección (ADR-021).** La `XInput1_3.dll` de System32 de este Windows reenvía a `xinput1_4.dll` cargándola en ejecución; `InControlNative` pide `XInput1_4.dll` por nombre y recibía esa 1.4 real, sin filtrar. Ahora la DLL real del filtro es siempre la 1.4 y el filtro redirige su tabla de exportación hacia sí mismo. «Lanzar J1 y J2» asigna cada mando al crear el proceso y pone al día el filtro instalado. | Desensamblado de `UnityPlayer.dll` (con símbolos), `InControlNative.dll` y `XInput1_3.dll`; **dos clientes reales con los dos mandos conectados: InControl lista un solo mando en cada uno** (antes dos), `intercepcion=activa`, `previa=no`; 173 xUnit y prueba de humo del filtro. **Falta la confirmación física** (nadie movió los sticks en esta prueba) |

## Incidente PS4: causa, corrección y reglas (siguen vigentes en 0.7)

1. **Enumeración no es movimiento**: un mando puede aparecer sin que la ruta de lectura aporte X/Y.
2. **No instalar SDL aparte**: el prototipo usa el SDL del wheel de `pygame-ce`; la 0.7 trae `SDL3.dll` en su paquete.
3. **PS4**: en modo Auto se lee como joystick directo (ADR-002). En SDL3 la ruta Gamepad es mucho más sólida que el módulo experimental de pygame; probarla con «Lector: Gamepad» antes de cambiar el valor por defecto.
4. **Botones**: no asumir Cruz = 0; aprender en la interfaz y permitir R2 analógico. Con SDL3/HIDAPI el orden suele ser el estándar (0 Cruz, 1 Círculo, 2 Cuadrado), pero se mantiene el aprendizaje.
5. **Aislamiento del laboratorio**: los mandos se leen en el anfitrión y cada jugador recibe su estado; los visores no abren mandos.
6. **Reconexión**: los ID cambian; reasignación explícita, nunca intercambio silencioso.

## Qué ya se logró frente al objetivo final

- Laboratorio en una sola PC: **SÍ** (prototipo validado; 0.7 pendiente de validar).
- Dos perspectivas del mismo mundo simulado: **SÍ**.
- Steam detectado/diagnosticado: **SÍ, solo lectura**.
- Servidor dedicado 7DTD local con guardados aislados: **implementado**, no probado en el PC del propietario.
- Dos/cuatro ventanas de clientes distribuidas sin bordes: **SÍ con 7DTD** (2026-10-10, dos clientes, arriba/abajo y lado a lado 16:9).
- Un mando por cliente dentro del juego: **cada cliente de 7DTD enumera un solo mando** (InControl: `XInput::1`), con el filtro interceptando también la XInput real (ADR-021). Falta que el propietario confirme con los mandos que cada uno mueve solo su ventana.
- Dos instancias con identidades distintas sin un segundo Steam: **SÍ con 7DTD** (emulador de Steam, componente principal): las dos llegan al menú con perfiles distintos.
- Dos clientes de 7DTD en la misma partida con dos mandos aislados: **pendiente** (falta unir J2 a la partida de J1 y la confirmación de los mandos).
- FPS/frametimes/VRAM medidos: **NO**.

## Decisiones tomadas con el propietario (2026-10-10)

- **Técnicas de Nucleus Co-op que adopta SplitForge:** filtrado de mandos por instancia, simulación de foco y colocación de ventanas (ADR-013, ADR-015).
- **Remote Play Together:** solo funciona con juegos con multijugador local en PC; 7DTD solo tiene pantalla dividida en consola.
- **Emulador de Steam = componente principal y permanente** (ADR-016): deja de ser temporal y no se eliminará; el propietario no quiere limitarlo.
- **Nucleus Co-op como referencia** (instalado en el PC del propietario; su handler de 7DTD, versión 18): SplitForge adopta su configuración en el lanzador (sin juego cruzado, ventana por panel, colocación automática). En vez de editar `platform.cfg`, SplitForge usa los argumentos equivalentes, que el juego admite, y no toca archivos.
- **Mandos independientes (2026-10-10, noche):** el propietario vio que solo respondía la ventana activa. Se adopta el foco simulado de Nucleus dentro del filtro XInput.
- **Distribución preferida para 7DTD: arriba y abajo** (el propietario la prefiere; además da paneles más anchos que 16:9).
- **Retirado en 0.7 a propósito:** el lanzamiento doble genérico de cualquier `.exe` (pestaña «03 EJECUTABLES» del prototipo; sin aislamiento de guardados, riesgo para partidas) y la demo antigua de dos mundos separados (sustituida por el mundo compartido). Siguen disponibles en el prototipo Python. Los juegos se integran mediante adaptadores con preflight.

## Estado para la siguiente sesión

**P0:** que el propietario descargue el artefacto `SplitForge-win-x64` de la CI (o ejecute `Compilar_SplitForge.bat`) y valide con los dos PS4 conectados: mandos en «01 MANDOS», laboratorio, distribución sin bordes y restauración. La frecuencia del hilo (1000 Hz) ya se midió en su PC sin mandos.
**P1 (estado 2026-10-10):** emulador y filtro XInput instalados en 7DTD en el PC del propietario. «Lanzar J1 y J2» abre los dos clientes en pantalla dividida, cada uno con su mando asignado (ver `05-INTEGRACION-STEAM.md`).
**Foco simulado (2026-10-10, noche):** desde fuera (ADR-020, casilla de la pestaña 07) y dentro del filtro (estado «foco simulado» en ambos clientes).
**W16, resultado del propietario:** cada stick movía **las dos** ventanas. Causa y corrección: ADR-021 (el juego leía por la `xinput1_4.dll` real; ahora el filtro la intercepta). Comprobado sin manos en los mandos: cada cliente enumera un solo mando. Las copias del filtro en la carpeta del juego ya son las nuevas.
**W16 (repetir, pendiente del propietario):** antes, ejecutar `Compilar_SplitForge.bat` para que la aplicación traiga el filtro nuevo.
1. Con los dos clientes abiertos por «Lanzar J1 y J2», pulsar un botón en el mando 2: la ventana de J2 debe avanzar aunque no tenga el foco.
2. Mover el stick del mando 1: solo debe reaccionar la ventana de J1.
3. En la pestaña 06, el estado de cada cliente debe decir «XInput real interceptada (N lecturas por esa vía)». Si dijera «ya estaba cargada antes del filtro», anotarlo: haría falta enganchar el código (límite de ADR-021).
4. Guardar un informe v3 (`7dtd`) con los dos clientes abiertos.
**W14 (siguiente):** J1 crea o carga una partida y J2 se une conectando a la IP 127.0.0.1 (como indica el handler de Nucleus). Anotar si la identidad emulada de J2 entra y si se ven en el mismo mundo.
**Vía del servidor oficial (opcional):** instalar «7 Days to Die Dedicated Server» (Steam → Herramientas) para W11/W12; comprobar si acepta identidades emuladas.
**P2:** dos jugadores reales en la misma partida.

## Política de actualización de memoria

Después de un cambio: **fecha**, hash/versión, archivos afectados, comandos y salida de pruebas, hechos confirmados vs hipótesis, evidencias de hardware si las hay y riesgo pendiente. Usar [`templates/ENTREGA.md`](templates/ENTREGA.md). Nunca insertar secretos, IDs de Steam o información personal.
