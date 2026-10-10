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
- Dos/cuatro ventanas de clientes distribuidas sin bordes: **implementado**, eficacia por juego no comprobada.
- Un mando por cliente dentro del juego: **filtro XInput nativo implementado (experimental)**. 7DTD carga XInput (informe real); el aislamiento con dos clientes está sin validar.
- Dos instancias con identidades distintas sin un segundo Steam: **emulador de Steam implementado** (componente principal). Superó su prueba de humo, pero **con 7DTD real se cierra al iniciar Steam** (API plana incompleta y sin `self`; ver `05-INTEGRACION-STEAM.md`).
- Dos clientes de 7DTD en la misma partida con dos mandos aislados: **NO** (falta validar el aislamiento de mandos).
- FPS/frametimes/VRAM medidos: **NO**.

## Decisiones tomadas con el propietario (2026-10-10)

- **Técnicas de Nucleus Co-op que adopta SplitForge:** filtrado de mandos por instancia, simulación de foco y colocación de ventanas (ADR-013, ADR-015).
- **Remote Play Together:** solo funciona con juegos con multijugador local en PC; 7DTD solo tiene pantalla dividida en consola.
- **Emulador de Steam = componente principal y permanente** (ADR-016): deja de ser temporal y no se eliminará; el propietario no quiere limitarlo.
- **Retirado en 0.7 a propósito:** el lanzamiento doble genérico de cualquier `.exe` (pestaña «03 EJECUTABLES» del prototipo; sin aislamiento de guardados, riesgo para partidas) y la demo antigua de dos mundos separados (sustituida por el mundo compartido). Siguen disponibles en el prototipo Python. Los juegos se integran mediante adaptadores con preflight.

## Estado para la siguiente sesión

**P0:** que el propietario descargue el artefacto `SplitForge-win-x64` de la CI (o ejecute `Compilar_SplitForge.bat`) y valide con los dos PS4 conectados: mandos en «01 MANDOS», laboratorio, distribución sin bordes y restauración. La frecuencia del hilo (1000 Hz) ya se midió en su PC sin mandos.
**P1:** hecho con un cliente (2026-10-10): 7DTD sin EAC carga `xinput1_3` y `xinput1_4` (ver `05-INTEGRACION-STEAM.md`). Falta el informe con dos clientes.
**Requisitos que faltan en el PC (2026-10-10):**
- **El emulador sigue instalado en 7DTD**, y con él el juego se cierra al iniciar. Para jugar o para W16 con un segundo Steam, primero «Restaurar DLL original» en la pestaña 07.
- El segundo cliente necesita el emulador corregido o Sandboxie-Plus (no instalado).
- Para unirse al servidor local (W12/W14) hay que instalar el servidor dedicado (Steam → Herramientas → «7 Days to Die Dedicated Server»).
**Emulador (bloqueante para su vía):** añadir `self` a la API plana y exportar los métodos `SteamAPI_ISteam*` que 7DTD usa al arrancar; ver la prueba del 2026-10-10 en `05-INTEGRACION-STEAM.md`.
**W16 (siguiente prueba física):**
1. Con los clientes cerrados, pulsa «Instalar filtro» en la pestaña 06.
2. Abre los dos clientes sin EAC (con la pestaña 07 o con un segundo Steam).
3. Pulsa «Detectar» para J1 y J2 y luego «Asignar mandos a los clientes».
4. Comprueba que las lecturas de cada cliente suben y que cada mando mueve solo su ventana.
5. Anota si la ventana sin foco responde.
6. Guarda un informe v3 (`7dtd`) con ambos clientes abiertos.
**P2:** prueba real de dos jugadores en la misma partida.
**Emulador de Steam con 7DTD (W20, repetir tras corregir la API plana):**
1. Instalar desde la pestaña 07 con 7DTD cerrado (o con `tools\SteamEmu_Instalar.ps1`).
2. Con Steam cerrado, como indica la pestaña, pulsar «Lanzar J1 y J2»: cada instancia recibe su JSON mediante `SPLITFORGE_STEAMEMU_CONFIG`.
3. Anotar si arrancan las dos, si se ven por LAN/P2P y si el servidor dedicado acepta las identidades emuladas.
4. Registrar el resultado en `05-INTEGRACION-STEAM.md`.

## Política de actualización de memoria

Después de un cambio: **fecha**, hash/versión, archivos afectados, comandos y salida de pruebas, hechos confirmados vs hipótesis, evidencias de hardware si las hay y riesgo pendiente. Usar [`templates/ENTREGA.md`](templates/ENTREGA.md). Nunca insertar secretos, IDs de Steam o información personal.
