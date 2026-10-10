# Registro de decisiones (ADR)

Formato: **ID · estado · decisión · por qué · consecuencias**. Revisar antes de modificar límites arquitectónicos.

## ADR-001 · Sustituida por ADR-010 · Prototipo Python/Tkinter + SDL

Se usó Python 3.11+, Tkinter y pygame-ce/SDL2 en la fase de prototipo porque funcionó en el entorno del usuario. Sustituida el 2026-10-10: el propietario exigió las mejores tecnologías para eficiencia y fluidez. El prototipo se conserva como referencia validada.

## ADR-002 · Aceptada · Joystick directo por defecto para PS4

La ruta GameController del prototipo enumeraba sin entregar ejes. PS4/DualShock/Wireless Controller usa joystick directo en modo Auto; botones aprendidos. Se mantiene en la 0.7 con SDL3 hasta probar la ruta Gamepad en el hardware del propietario.

## ADR-003 · Aceptada · Estado autoritativo único del laboratorio

Un anfitrión calcula `SharedWorld` y difunde instantáneas a dos visores. Evita dos universos inconexos y no requiere red.

## ADR-004 · Aceptada · Inspección Steam sin modificación

Manifiestos, carpetas y procesos se leen; no se cambia contenido del juego. La detección no concede `COMPATIBLE`.

## ADR-005 · Aceptada (ampliada) · Organizar ventanas solo de procesos distintos del mismo EXE

Se rechazan dos HWND de un PID y ejecutables distintos. 0.7: hasta 4 clientes, ordenados por PID, revalidados justo antes de mover. No se cierran procesos externos.

## ADR-007 · Aceptada (mínima) · Adaptadores versionados por juego

Implementado el contrato mínimo `IGameAdapter` (manifiesto + preflight de solo lectura) y el primer adaptador (7DTD). Estado máximo automático `INSPECCIONADO`; los superiores requieren evidencia física registrada.

## ADR-008 · Propuesta · Telemetría reproducible antes de optimizar

Instrumentar frametimes, procesos y VRAM antes de compositor, afinidades o servicios nativos de rendimiento.

## ADR-009 · Aceptada (parcial) · Paquete ejecutable

Implementado como publicación .NET autónoma win-x64 en la CI (sin instalar nada). Pendiente: firma de código y actualizador.

## ADR-010 · Aceptada · .NET 10 + SDL3 + WPF, con C++ para piezas nativas

**Decisión:** reconstruir SplitForge en C# / .NET 10 (LTS) con WPF (renderizado por GPU) y SDL3; C++20 solo para componentes nativos justificados (aislamiento de entrada dentro del juego, captura de frametimes). **Por qué:** el propietario pidió las mejores tecnologías para eficiencia y fluidez; Tkinter acoplaba la entrada al temporizador de la interfaz (~60 Hz) y dibujaba por software; Python no sirve para hooks nativos. .NET da hilos reales, interop Win32 directa (`LibraryImport`), publicación autónoma y es el ecosistema habitual de herramientas de este tipo en Windows. **Consecuencias:** nueva solución `SplitForge.slnx`; el prototipo Python queda como referencia con sus pruebas en la CI; configuración propia `settings.v2.json`.

## ADR-011 · Aceptada · Entrada SDL3 en un hilo dedicado

Un único hilo llama a SDL3 (hasta 1000 Hz, plazos absolutos, eventos en segundo plano) y publica instantáneas inmutables. La UI y la simulación solo leen. **Consecuencia:** una ventana bloqueada no afecta a la entrada; cualquier llamada nueva a SDL de mandos debe ir por la cola de comandos del servicio.

## ADR-012 · Aceptada · IPC binario versionado "último fotograma gana"

`[u32 longitud][SFW1 v1 …]` con validación estricta; el escritor descarta fotogramas viejos. Sustituye a las líneas JSON. **Consecuencia:** cualquier cambio de formato sube `WorldCodec.Version`.

## ADR-013 · Aceptada · 7 Days to Die mediante servidor dedicado oficial

**Decisión:** el adaptador lanza el servidor dedicado oficial con configuración y guardados propios; el Steam del jugador 2 puede abrirse en otro usuario de Windows o en Sandboxie-Plus; EAC se desactiva con la opción oficial. **Descartado:** Remote Play Together (no aplica a 7DTD en PC). **Complemento (ADR-016):** la identidad del jugador 2 también puede venir del emulador de Steam, sin un segundo Steam. Falta comprobar si el servidor dedicado acepta identidades emuladas.

## ADR-014 · Aceptada · CI en Windows con artefacto autónomo

Cada push a `main` (y cada PR) ejecuta:
- los dos módulos nativos (filtro XInput y emulador de Steam) con sus pruebas de humo;
- las pruebas .NET y Python, y la validación de docs;
- la publicación win-x64 autónoma, con las dos DLL nativas;
- la prueba de humo del diagnóstico y la prueba de arranque de la interfaz (7 pestañas);
- la subida del artefacto `SplitForge-win-x64`.

Las etiquetas `v*` crean una release con ZIP y SHA-256 (prerelease para `v0.*`).

## ADR-015 · Aceptada (experimental) · Filtro XInput por proceso como primer módulo nativo

**Decisión:** primer componente C++ (`native/SplitForge.XInputFilter`): una DLL que se copia en la carpeta del juego con el nombre de la DLL de XInput que este carga y reenvía todo a la DLL real de System32, pero hace que cada proceso vea **un único mando**: la ranura física que SplitForge le asigna por PID en `%LOCALAPPDATA%\SplitForge\xinput\asignacion.txt`. Sin asignación es transparente. **Por qué:** XInput es la vía de mandos más común en juegos de PC y, a diferencia de Windows.Gaming.Input, funciona sin foco; es la técnica de aislamiento más simple y reversible (no inyecta en otros procesos, no escribe memoria ajena). **Límites:** solo con EAC desactivado mediante la opción oficial; los PS4 deben llegar como XInput (Steam Input o DS4Windows); si el juego exige foco hará falta además la simulación de foco. **Garantías:** nunca sobrescribe archivos del juego, retira solo lo que instaló (SHA-256), prueba de humo en la CI de Windows. **Evidencia (2026-10-10):** el informe real de 7DTD sin EAC muestra que el juego carga `xinput1_3.dll` **y** `xinput1_4.dll`, con los mandos como `Xbox 360 Controller`. Por eso el filtro se instala con ambos nombres. Cambios derivados:

- cada copia escribe `estado-PID-<dll>.txt` en vivo, con ranura y lecturas;
- la DLL real se verifica en System32;
- la instalación actualiza solo copias propias sin modificar y nunca deja el manifiesto incompleto.

**Pendiente:** prueba W16 con dos clientes (¿suben las lecturas en ambos?, ¿responde la ventana sin foco?).

## ADR-016 · Aceptada (experimental) · Emulador de Steam como componente principal (técnica de Goldberg)

**Decisión:** componente C++ (`native/SplitForge.SteamEmu`) con la técnica que usa Nucleus Co-op (Goldberg). Es una DLL que reimplementa el contrato ABI de `steam_api64.dll` para que una copia del juego arranque con **identidades distintas por proceso**, definidas en un JSON (`SplitForge.SteamEmu.json`, o `SPLITFORGE_STEAMEMU_CONFIG` por instancia). Reimplementa:
- exportaciones planas;
- vtables de `SteamClient020`, `SteamUser023`, `SteamFriends017`, `SteamUtils010`, `SteamUserStats012`, `SteamMatchmaking009`, `SteamNetworking006`, `SteamRemoteStorage016`…;
- callbacks y call results.

Incluye además:
- P2P local real por UDP multicast entre instancias del mismo PC/LAN;
- stats/logros y nube (`remote\`) en la carpeta de guardados de cada identidad, bajo `%LOCALAPPDATA%\SplitForge\steamemu\`;
- instalador/restaurador reversible (copia de seguridad + SHA-256).

Está **integrado en la interfaz** (pestaña 07 «EMULADOR STEAM»: instalar/restaurar, identidades J1/J2 y lanzamiento de las dos instancias), y su DLL va en el paquete como `native\SplitForge.SteamEmu\steam_api64.dll`.

**Estado (decisión del propietario, 2026-10-10):** pasa a ser un **componente principal y permanente** de SplitForge. Deja de ser temporal, no se eliminará y no se le añaden restricciones.

**Límites técnicos:** NetworkingSockets/Messages y otras interfaces sin implementar devuelven `nullptr`; P2P sin cifrado ni fiabilidad; requiere el juego sin EAC.

**Relación con ADR-013:** convive con el servidor dedicado oficial. El emulador aporta la identidad de cada instancia; falta comprobar si el servidor acepta esas identidades o si la partida debe alojarse por LAN/P2P.

**Evidencia (2026-10-10):**
- La prueba de humo pasa en el PC del propietario y en la CI.
- Con 7DTD V 3.3.0 (b18), por la mañana: las dos instancias cargaban sus identidades, pero el juego se cerraba en `SteamAPI_ISteamClient_GetISteamUser`. Causa: la API plana del emulador no recibía `self` y solo exportaba 32 de las 944 entradas de Steamworks.NET.
- Por la tarde: API plana **generada** desde las vtables (`tools/generar_api_plana.py`, comprobada en la CI) con las 944 entradas, `ContextInit` con el contrato del SDK, despacho de call results por `SteamAPICallCompleted_t`. Con el lanzador de ADR-018, **dos clientes de 7DTD llegan al menú a la vez** con perfiles distintos.

**Pendiente:** comprobar si el servidor dedicado oficial acepta identidades emuladas y la partida común por IP (W14).

## ADR-018 · Aceptada (experimental) · Lanzador de clientes con la configuración del handler de Nucleus Co-op

**Decisión:** «Lanzar J1 y J2» (pestaña 07) reproduce lo que hace el handler de 7DTD de Nucleus Co-op (versión 18, instalado en el PC del propietario y usado como referencia). Cada cliente:
- arranca sin juego cruzado (`-crossplatform=None -serverplatforms=Steam`), en ventana sin marco (`-screen-fullscreen 0 -popupwindow`, `-noeac`) y con el tamaño de su panel (`-screen-width`, `-screen-height`);
- recibe su mando con el filtro XInput (asignación por PID, si el filtro está instalado);
- se coloca en pantalla dividida en cuanto aparece su ventana (espera de 12 s, como `PauseBetweenProcessGrab`), con la proporción mínima 16:9 del juego (`LayoutOptions.MinAspect`).

**Por qué:** con juego cruzado, EOS rechaza la identidad local y bloquea el menú (el botón «Jugar sin conexión» exige `EUserStatus.OfflineMode`). Nucleus lo evita editando `platform.cfg` en su copia del juego; 7DTD admite lo mismo por argumentos (`PlatformManager.GetCommandLineOverrides`), así que SplitForge no toca archivos del juego para esto. La interfaz de 7DTD se corta en paneles más estrechos que 16:9.

**Consecuencias:** sin juego cruzado no hay partidas con jugadores de consola; la partida común es por Steam/IP. Pendiente: simulación de foco si la ventana sin foco no reacciona (Nucleus usa `ForceFocus` y `FocusHooks`).

**Barra de tareas (petición del propietario):** con «Tapar barra de tareas» (activada por defecto), la sesión usa el monitor completo y coloca las ventanas «siempre encima» (`HWND_TOPMOST`), por encima de la barra. Al detener se restaura también ese orden. Comprobado el 2026-10-10: la franja de la barra (esquinas incluidas) pasa a ser de la ventana de J2.

## ADR-019 · Aceptada (experimental) · Bloqueo de entrada «solo mandos» (Ctrl+Alt+B)

**Decisión:** un atajo global (Ctrl+Alt+B) y un botón activan ganchos de bajo nivel de teclado y ratón (`WH_KEYBOARD_LL` / `WH_MOUSE_LL`) que descartan toda la entrada de teclado y ratón, para los juegos y para el escritorio, salvo el propio atajo, que la suelta. Los mandos (XInput) no pasan por ahí. Es el equivalente al bloqueo de entrada de Nucleus Co-op (tecla Fin), pedido por el propietario para jugar solo con mandos.

**Cómo:** la decisión es pura y está probada (`InputLockFilter`): descarta pulsaciones, deja pasar las teclas soltadas (ningún modificador se queda «pegado») y sigue por sí misma el estado de Ctrl/Alt/Shift/Win, porque Windows no lo actualiza con las pulsaciones descartadas. `InputLock` (Win32) solo instala y retira los ganchos.

**Garantías:** se suelta con el atajo, al detener la sesión y al cerrar SplitForge. Ctrl+Alt+Supr no se puede bloquear. Windows retira el gancho si SplitForge deja de responder. Sin inyección en otros procesos.

**Límites:** bloquea todo el teclado y el ratón (también el escritorio) mientras está activo. No separa varios teclados o ratones por jugador.

## ADR-017 · Aceptada (experimental) · Bloqueo de ratón por ventana con ClipCursor

**Decisión:** un atajo global (Ctrl+Alt+L) y un botón confinan el cursor al área de cliente de la ventana del jugador en primer plano, usando `ClipCursor`, que se reaplica en el tick de la interfaz porque Windows lo reinicia al cambiar el foco. **Por qué:** en pantalla dividida el ratón se escapa a la otra mitad; es la técnica de Nucleus Co-op, sin inyección ni hooks. **Límites:** solo confina el ratón; el teclado sigue yendo a la ventana con foco (el aislamiento por dispositivo exigiría Raw Input, aún no implementado); Windows podría ignorar `ClipCursor` desde una app en segundo plano. **Garantías:** solo bloquea ventanas que SplitForge colocó; se suelta al desactivar, detener la sesión o cerrar. **Pendiente:** validar en hardware (W18).

## ADR-020 · Aceptada (experimental) · Foco simulado desde fuera del juego (FakeFocus)

**Contexto:** el propietario informa (2026-10-10) de que, con «Lanzar J1 y J2», los mandos no son independientes: ningún mando mueve solo su ventana. El filtro XInput sí entrega a cada cliente su mando (sus «lecturas» suben en los dos, también sin foco), así que la hipótesis principal es que 7DTD (Unity) no procesa la entrada en la ventana sin foco. El handler de 7DTD de Nucleus Co-op funciona porque su `xinput1_3.dll` propia (x360ce, opción `ForceFocus` en `ncoop.ini`) hace que cada ventana se crea siempre activa.

**Decisión:** primer paso sin tocar el juego: la técnica `FakeFocus` de Nucleus (`WindowFakeFocus.cs`), con los mensajes del `FocusLoop` de ProtoInput. Un hilo propio de SplitForge (`WindowFakeFocus`) publica con `PostMessageW`, cada 100 ms, `WM_ACTIVATE(WA_CLICKACTIVE)`, `WM_ACTIVATEAPP(TRUE)`, `WM_NCACTIVATE(TRUE)`, `WM_SETFOCUS` y `WM_MOUSEACTIVATE(HTCLIENT)` a cada ventana de la sesión activa (dos o más). Activado por defecto, con casilla en la pestaña 07 (`settings.v2.json`: `FakeFocus`, `FakeFocusIntervalMs`, 5–2000 ms). La lógica es pura y está probada (`FakeFocusPlan`).

**Por qué así:** no inyecta código, no modifica archivos del juego y no bloquea la interfaz (mensajes publicados en un hilo propio). Nucleus reenvía cada 1000 ms y ProtoInput cada 5 ms; 100 ms recupera el foco enseguida sin saturar la cola del juego.

**Límites (sin comprobar):** no está probado con 7DTD ni con mandos. Si el juego consulta el foco real (`GetForegroundWindow`) o recibe el mando ajeno por DirectInput/Raw Input en la ventana con foco, los mensajes externos no bastan; Nucleus lo resuelve dentro del juego. Siguiente evidencia: W16 con el foco simulado activado y desactivado.

