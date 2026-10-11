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

**Foco simulado (2026-10-10, técnica `ForceFocus` / `FocusHooks` de Nucleus):** 7DTD solo atiende la entrada si Unity le notifica el foco (`GameManager.OnApplicationFocus` → `GameIsFocused`), así que en pantalla dividida solo respondía la ventana activa. Con mando asignado, el filtro (una vez por proceso) subclasifica la ventana de Unity: bloquea `WM_KILLFOCUS` y convierte `WM_ACTIVATE`/`WM_ACTIVATEAPP`/`WM_NCACTIVATE` inactivos en activos. Además redirige `GetForegroundWindow`/`GetActiveWindow`/`GetFocus` en la tabla de importaciones de `UnityPlayer.dll` y del ejecutable a su ventana, y le envía una activación inicial. Todo dentro del propio proceso del juego, sin parchear código. Tras dividir la pantalla, SplitForge recupera el foco real (`SetForegroundWindowElsewhere` de Nucleus). El puntero no necesita nada: 7DTD dibuja el suyo por instancia (`SoftCursor`).

**Ampliada por ADR-021** (2026-10-10): la DLL real pasa a ser siempre `xinput1_4.dll` y el filtro intercepta sus exportaciones. Con eso, W16 quedó superado (cada mando mueve solo su ventana).

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

**Aviso en pantalla (2026-10-11, petición del propietario: «no me doy ni cuenta cuando está desbloqueado o bloqueado»):** al cambiar el estado aparece un cartel arriba, centrado sobre los juegos, durante 3 s («TECLADO Y RATÓN BLOQUEADOS» / «DESBLOQUEADOS», con el atajo). Mientras dura el bloqueo queda una insignia pequeña («Bloqueado · Ctrl+Alt+B»). Es una ventana propia (`LockNotice`): no recibe clics, no toma el foco, no sale en Alt+Tab y se vuelve a subir cada 0,5 s por si un juego queda delante. Solo refleja el estado. Textos y posición son lógica pura (`LockNoticePlan`). Medido con Win32 sin bloquear nada; **sin ver aún sobre los juegos reales**.

## ADR-017 · Aceptada (experimental) · Bloqueo de ratón por ventana con ClipCursor

**Decisión:** un atajo global (Ctrl+Alt+L) y un botón confinan el cursor al área de cliente de la ventana del jugador en primer plano, usando `ClipCursor`, que se reaplica en el tick de la interfaz porque Windows lo reinicia al cambiar el foco. **Por qué:** en pantalla dividida el ratón se escapa a la otra mitad; es la técnica de Nucleus Co-op, sin inyección ni hooks. **Límites:** solo confina el ratón; el teclado sigue yendo a la ventana con foco (el aislamiento por dispositivo exigiría Raw Input, aún no implementado); Windows podría ignorar `ClipCursor` desde una app en segundo plano. **Garantías:** solo bloquea ventanas que SplitForge colocó; se suelta al desactivar, detener la sesión o cerrar. **Pendiente:** validar en hardware (W18).

## ADR-020 · Aceptada (experimental) · Foco simulado desde fuera del juego (FakeFocus)

**Contexto:** el propietario informa (2026-10-10) de que, con «Lanzar J1 y J2», los mandos no son independientes: ningún mando mueve solo su ventana. El filtro XInput sí entrega a cada cliente su mando (sus «lecturas» suben en los dos, también sin foco), así que la hipótesis principal es que 7DTD (Unity) no procesa la entrada en la ventana sin foco. El handler de 7DTD de Nucleus Co-op funciona porque su `xinput1_3.dll` propia (x360ce, opción `ForceFocus` en `ncoop.ini`) hace que cada ventana se crea siempre activa.

**Decisión:** primer paso sin tocar el juego: la técnica `FakeFocus` de Nucleus (`WindowFakeFocus.cs`), con los mensajes del `FocusLoop` de ProtoInput. Un hilo propio de SplitForge (`WindowFakeFocus`) publica con `PostMessageW`, cada 100 ms, `WM_ACTIVATE(WA_CLICKACTIVE)`, `WM_ACTIVATEAPP(TRUE)`, `WM_NCACTIVATE(TRUE)`, `WM_SETFOCUS` y `WM_MOUSEACTIVATE(HTCLIENT)` a cada ventana de la sesión activa (dos o más). Activado por defecto, con casilla en la pestaña 07 (`settings.v2.json`: `FakeFocus`, `FakeFocusIntervalMs`, 5–2000 ms). La lógica es pura y está probada (`FakeFocusPlan`).

**Por qué así:** no inyecta código, no modifica archivos del juego y no bloquea la interfaz (mensajes publicados en un hilo propio). Nucleus reenvía cada 1000 ms y ProtoInput cada 5 ms; 100 ms recupera el foco enseguida sin saturar la cola del juego.

**Límites (sin comprobar):** no está probado con 7DTD ni con mandos. Si el juego consulta el foco real (`GetForegroundWindow`) o recibe el mando ajeno por DirectInput/Raw Input en la ventana con foco, los mensajes externos no bastan; Nucleus lo resuelve dentro del juego. Siguiente evidencia: W16 con el foco simulado activado y desactivado.

**Revisión (2026-10-11, ADR-025):** estos mensajes ya no se publican a las ventanas cuyo juego simula el foco por dentro (filtro XInput con `foco=forzado`). En esos juegos sobran y, comprobado en el código de Unity, cada tanda hacía que el juego recolocara y encerrara el puntero de Windows. Siguen en uso para ventanas sin filtro.

## ADR-021 · Aceptada (experimental) · XInput real interceptada dentro del proceso (tabla de exportación)

**Contexto (2026-10-10, PC del propietario, Windows 11 compilación 26300):** en W16 cada stick movía las dos ventanas. Causa comprobada con el desensamblado de los binarios del juego y el `Player.log` real:

- Unity (`win::RawInput::Open`) carga `xinput1_3.dll` por nombre: recibe la copia del filtro.
- El filtro usaba como DLL real la `XInput1_3.dll` de System32. En este Windows esa DLL es un reenviador: carga `xinput1_4.dll` de System32 **en tiempo de ejecución** (no la importa; por eso se descartó por error).
- `InControlNative.dll`, la vía que mueve al jugador («NativeInputDeviceManager enabled»), hace `LoadLibraryW("XInput1_4.dll")`. Como ya hay un módulo con ese nombre en memoria, Windows le entrega la 1.4 real y no la copia del filtro. InControl listaba dos mandos (`XInput::1` y `XInput::2`) en cada cliente.

**Decisión:**

1. La DLL real del filtro es siempre `xinput1_4.dll` de System32, por ruta completa (la única implementación desde Windows 8), se llame como se llame la copia.
2. Al cargarla, el filtro guarda las direcciones reales y redirige su tabla de exportación hacia saltos a sus propias funciones: `XInputGetState`, `XInputSetState`, `XInputGetCapabilities`, `XInputGetBatteryInformation`, `XInputGetKeystroke`, `XInputGetAudioDeviceIds` y los ordinales 100–103, 108 y 109. Quien resuelva después esas funciones (`GetProcAddress` o importación) recibe las del filtro.
3. Una segunda copia del filtro reconoce los saltos (firma al principio de su página) y delega en la primera: el índice no se cambia dos veces.
4. La copia que intercepta queda fijada en memoria mientras viva el proceso.
5. «Lanzar J1 y J2» asigna cada mando nada más crear el proceso (antes de su primera lectura) y pone al día las copias instaladas si son de otra compilación.

**Por qué así y no como Nucleus:** `Game.Hook.XInputEnabled` engancha el código de `XInputGetState` con EasyHook e inyección. Redirigir la tabla de exportación no parchea código de Windows, no exige desensamblar prólogos ni suspender hilos, y cada escritura es atómica (32 bits). Cubre el caso comprobado porque la 1.4 real la carga el propio filtro antes que nadie.

**Límites:** quien resolviera las funciones antes de que el filtro cargara la DLL real conserva las direcciones originales (el estado lo avisa con `previa=si`); para ese caso haría falta enganchar el código. Solo x64. El ordinal 109 no está documentado: por su código se **supone** que recibe (índice de mando, puntero). `XInputGetDSoundAudioDeviceGuids` no existe en la 1.4: el filtro responde «sin dispositivo de audio» para el mando asignado.

**Evidencia (2026-10-10, dos clientes reales con los dos mandos conectados):** `intercepcion=activa`, `previa=no`, cientos de lecturas por la DLL real en cada cliente, e InControl lista **un** solo mando por cliente (`XInput::1`). Prueba de humo ampliada. **W16 superado:** el propietario confirma con los mandos que cada uno controla solo su ventana.

## ADR-022 · Aceptada (experimental) · Las instancias se encuentran solas: descubrimiento y unión automática en el emulador

**Contexto (2026-10-10, registros del intento real del propietario):** J1 llegó a conectar por IP a la partida de J2 y pasó todos los autorizadores, pero se quedó en «autenticando»: tras `SteamGameServer.BeginAuthSession` el anfitrión espera `ValidateAuthTicketResponse_t` por el canal del servidor de juego, y el emulador no lo enviaba. Además el anfitrión nunca recibía `LobbyCreated_t` (7DTD lo registra como callback, no como call result) y su «IP pública» salía como `1.0.0.127`. Referencia: con Nucleus Co-op (Goldberg) la conexión se hace a mano, «Conectar a IP» con 127.0.0.1.

**Decisión:**

1. **Dos canales de callbacks**, como Steam: el del usuario y el del servidor de juego (`SteamGameServer_GetHSteamPipe` = 2). Cada callback se entrega solo por el suyo. `BeginAuthSession` responde con `ValidateAuthTicketResponse_t` (correcto) por el canal que corresponde.
2. **ABI de `CSteamID`:** en el SDK no se alinea a 8 bytes. `ValidateAuthTicketResponse_t` mide 20 bytes con el dueño en el desplazamiento 12, y `UserStatsReceived_t` lleva el usuario en el 12.
3. **Lobbies como en Steam:** `LobbyCreated_t` llega también como callback y siempre antes que `LobbyEnter_t`; `JoinLobby` entrega `LobbyEnter_t` como callback y después `LobbyDataUpdate_t`.
4. **Descubrimiento por la red multicast que ya existía:** cada instancia anuncia su partida abierta (puerto del servidor de juego) y sus lobbies no privados con todos sus datos. `RequestLobbyList` devuelve los lobbies de las demás instancias (con los filtros de igualdad), y sus datos se pueden leer y unirse a ellos.
5. **Unión automática (`auto_join` en el JSON de la identidad):** cuando otra instancia abre una partida, el emulador entrega al juego `GameServerChangeRequested_t` con `127.0.0.1:puerto`, lo mismo que Steam al pulsar «Unirse a la partida» de un amigo. 7DTD lo atiende en `InviteManager` y se conecta. Una sola petición por partida abierta, con 3 s de margen.
6. **Aplicación:** casillas «J2 entra sola en la partida de J1» (activada) y «J1 arranca cargando su última partida» (`-quick-continue` del propio juego, desactivada por defecto). «Lanzar J1 y J2» pone al día la DLL emulada instalada.

**Por qué mejora a Nucleus:** nadie tiene que escribir una IP. Con las dos casillas, «Lanzar J1 y J2» termina con los dos jugadores en el mismo mundo sin tocar los menús.

**Evidencia (2026-10-10, 7DTD real):** lanzados con las dos casillas, J1 carga su partida y a los 111 s el anfitrión registra «Player 'SplitForgeJ2' joined the game» y `PlayerSpawnedInWorld (reason: JoinMultiplayer)`. Autenticación `k_EAuthSessionResponseOK`; J2 entra además en el lobby de J1 por su ID.

**Ajustes tras probarlo el propietario (2026-10-10, noche):** la unión automática le resultó inesperada y que J1 cargara partida sin elegir mundo, incorrecto. Queda así: las dos automatizaciones son opciones **desactivadas por defecto**; la unión automática funciona **en los dos sentidos** (entra solo quien esté en el menú, sea J1 o J2; quien aloja una partida no recibe la petición); y J2 usa por defecto **su propia carpeta de datos** (`-UserDataFolder` del juego, en `%LOCALAPPDATA%\SplitForge\7dtd\clientes\J2`, con una copia inicial de los perfiles de personaje), de modo que dos instancias no pueden abrir el mismo guardado. «Cerrar J1 y J2» pide el cierre normal (WM_CLOSE) primero a los invitados y, cuando han salido, al anfitrión.

**Límites:** la lista de servidores del juego (`ISteamMatchmakingServers`) sigue vacía: comprobado solo que el lobby de J1 es visible y unible desde el emulador, no que la pantalla «Unirse a partida» lo muestre. El descubrimiento usa multicast: hace falta una interfaz de red activa. **Riesgo resuelto:** J1 y J2 compartían la carpeta de guardados y en la primera prueba J2 tuvo abierta 45 s la partida que alojaba J1; con la carpeta de datos propia de J2 ya no puede ocurrir.

## ADR-023 · Aceptada · Fondo negro detrás de la pantalla dividida

**Contexto:** el propietario ve las uniones entre las ventanas de los juegos (por la separación entre paneles se ve el escritorio) y pide un fondo negro a pantalla completa «que disimule», óptimo y eficiente.

**Decisión:** `SessionBackdrop` (SplitForge.Windows): una ventana Win32 propia, sin marco, con el pincel negro del sistema. No tiene contenido, temporizadores ni dibujo propio; no toma el foco (`WS_EX_NOACTIVATE`) ni aparece en la barra de tareas o en Alt+Tab (`WS_EX_TOOLWINDOW`). Ocupa la zona de la sesión (monitor completo si tapa la barra de tareas; si no, el área útil) y se mantiene **justo debajo** de la ventana de la sesión que esté más abajo (`BackdropPlan`, lógica pura y probada), así nunca tapa un juego. La interfaz lo sincroniza a ≈2 Hz y solo actúa si algo cambió. Aparece con la sesión y desaparece al detenerla. Casilla «Fondo negro» junto a «Tapar barra de tareas», activada por defecto (`BlackBackdrop` en `settings.v2.json`).

**La sesión termina sola cuando ya no queda ninguna de sus ventanas** (el jugador salió de los juegos desde su menú): se quita el fondo y se sueltan el ratón y el bloqueo de entrada. La primera versión no lo hacía y el fondo, a pantalla completa y por encima de todo, se quedaba tapando el escritorio hasta cerrar SplitForge (lo encontró el propietario). Además, el fondo nunca se muestra si no existe ninguna ventana de la sesión.

**Por qué una ventana nativa y no una de WPF:** WPF crearía otra superficie de Direct3D a pantalla completa para no dibujar nada.

**Evidencia (2026-10-10):** con los dos visores del laboratorio y con dos clientes reales de 7DTD, el fondo queda inmediatamente debajo de las dos ventanas y los dos píxeles de la unión pasan del color del escritorio a negro (`000000`); al detener la sesión, desaparece.

## ADR-024 · Aceptada · La interfaz es un lanzador de juegos, no un panel técnico

**Contexto:** el propietario pide un diseño propio de juegos, sin lo innecesario: 7 Days to Die es el primero de varios. La interfaz anterior eran siete pestañas numeradas de herramientas, con una barra de opciones y el registro siempre visibles.

**Decisión:**

- **Tres secciones** en una barra lateral: **JUGAR**, **MANDOS** y **AVANZADO**.
- **JUGAR** es todo lo que hace falta para una partida: biblioteca (hoy un juego y un hueco «próximamente»), portada con el botón JUGAR, una tarjeta por jugador (nombre, mando, «Detectar»), la lista «Preparación» (juego, identidad, un mando por ventana, mandos) con «Preparar ahora» si falta algo instalable, las opciones de pantalla y de partida, y tres pasos de cómo se juega.
- **AVANZADO** recoge lo técnico sin quitar nada: distribución y bloqueos, emulador y filtro de 7DTD con su diagnóstico y el servidor dedicado, laboratorio, ventanas, Steam y registro.
- **Tema** (`Theme.xaml`): oscuro, un solo color de acento, un color por jugador (J1 azul, J2 rojo), interruptores, botones segmentados y desplegables oscuros. Barra de título oscura.
- **Carátulas:** se leen de la caché local de Steam del usuario (`appcache\librarycache\<AppID>`); SplitForge no incluye ni distribuye imágenes de juegos. Si no están, se muestra el título en texto.
- **Lógica nueva, pura y probada:** `PlayReadiness` (qué falta para jugar) y `SteamArt` (dónde están las carátulas).

**Reglas que se mantienen:** la interfaz solo refleja estado (la lista «Preparación» se recalcula cada ≈2 s y solo cambia si cambió algo); nada se declara compatible por estar instalado; «próximamente» no promete ningún juego concreto.

**Detalle a recordar:** en una plantilla propia de `TabControl`, el presentador del contenido debe llamarse `PART_SelectedContentHost`; sin ese nombre la automatización (lectores de pantalla y la prueba de la CI) no ve el contenido de las pestañas.

**Evidencia (2026-10-10):** 187 xUnit; los 130 enlaces de la ventana tienen su propiedad en el modelo; el paquete publicado arranca, muestra las 3 secciones y las 6 pestañas de AVANZADO y se cierra de forma ordenada. **Sin comprobar:** una partida completa lanzada desde el botón JUGAR nuevo (usa la misma orden que antes) y la opinión del propietario sobre el diseño.

## ADR-025 · Aceptada (experimental) · Un puntero propio por cliente

**Contexto (propietario, 2026-10-11):** los mandos ya son independientes, pero la ventana **sin foco** solo permite moverse y golpear; el resto de botones no responde hasta que se le da el foco, y su puntero a veces parpadea.

**Hechos comprobados** (desensamblado de `UnityPlayer.dll` del juego con sus símbolos e IL de `Assembly-CSharp.dll`, build `25661859`):

- Windows tiene **un** puntero. Unity lo lee en cada fotograma con `GetCursorPos`, sin mirar el foco (`win::Input::Process`, `InputProcess`).
- `ScreenManagerWin::SetLockCursorInternal`: con el cursor bloqueado, si `GetForegroundWindow()` es su ventana, lleva el puntero al centro (`SetCursorPos`) y lo encierra (`ClipCursor`); con el cursor libre, suelta el encierro (`ClipCursor(NULL)`), sea de quien sea.
- El foco simulado del filtro hace que `GetForegroundWindow` devuelva la ventana propia en **los dos** clientes: los dos movían y encerraban el mismo puntero.
- En `PlayerMainWndProc`, cada `WM_ACTIVATE` ejecuta `PlayerWindowActivated` (reactiva la entrada, vuelve a aplicar el bloqueo del cursor) y cada `WM_NCACTIVATE(TRUE)` llama a `SetLockCursorInternal` sin condiciones. El foco simulado **externo** (ADR-020) publicaba ambos mensajes 10 veces por segundo a cada ventana.
- 7DTD, con mando, dibuja el puntero de sus menús como un elemento propio (`SoftCursor`). En cuanto `Input.mousePosition` cambia, lo lleva a esa posición, oculta su puntero y la caja de selección ese fotograma y muestra el cursor de Windows (`SoftCursor.HandleMovement` / `SetVisible`). Moverse y golpear no usan el puntero.

**Supuesto (sin reproducir):** esa pelea por el puntero es la causa de los dos síntomas. No se ha podido comprobar sin los mandos del propietario.

**Decisión:**

- **Puntero propio.** Dentro de cada cliente con mando asignado, el filtro redirige `GetCursorPos`, `SetCursorPos` y `ClipCursor` de `UnityPlayer.dll` y del ejecutable (su tabla de importaciones, igual que el foco). Mientras el cliente **no** es la ventana activa de verdad:
  - `GetCursorPos` devuelve su última posición conocida (o el centro de su ventana), que no cambia aunque se mueva el puntero de Windows;
  - `SetCursorPos` solo la recuerda y `ClipCursor` no hace nada.
- El cliente que **sí** es la ventana activa usa el puntero real: teclado y ratón siguen igual que siempre.
- Al dejar de ser la ventana activa, si el encierro vigente es el que puso ese cliente, lo suelta; nunca quita el de otro.
- **El foco externo se retira donde sobra** (revisión de ADR-020): la aplicación lee cada ≈2 s qué clientes tienen `foco=forzado` y deja de publicarles mensajes.
- El estado del filtro añade `puntero=propio` y `puntero_propio=N` (llamadas atendidas sin tocar el puntero de Windows).

**Por qué así:** es la técnica de ProtoInput en Nucleus Co-op (ganchos de cursor por proceso), reducida a lo que hace falta. Solo cambia datos del propio proceso del juego; no se parchea código ni se toca otro proceso. No hay que dibujar un puntero falso: el de los menús con mando lo dibuja el juego.

**Límites:**

- El cliente en segundo plano no ve el ratón real (su puntero está quieto). Para usar el ratón en un cliente hay que hacer clic en él, como siempre.
- No se redirigen las llamadas administradas del juego a `user32` (`MouseLib` → `SetCursorPos`/`GetCursorPos` por P/Invoke): solo las usa `FlexibleCursor`, y por el código el puntero en uso es `SoftCursor` (`GameManager.setCursorEnabled` trata como tal a todos los punteros registrados). Deducción del código, sin ver en ejecución; otro juego podría necesitarlo.
- **Sin comprobar con el juego real y los mandos.** Si el síntoma siguiera, el propio juego tiene un registro para esto: lanzado con `-debuginput=verbose` anota los cambios de foco, de cursor y de conjuntos de acciones (`ActionSetManager`).

**Evidencia (2026-10-11):** prueba de humo del filtro con una ventana como la de Unity que no es la activa: el proceso cree tener el foco, mueve «su» puntero y el de Windows no cambia (ni su encierro); 192 xUnit.

**Corrección del mismo día (tarde):**

- **El supuesto era incompleto.** El propietario lo probó y precisó el síntoma: en la ventana sin foco no funcionaban LB/RB para cambiar de objeto, y sí dentro del inventario. La causa de eso era otra (teclas pegadas, ADR-026). El puntero propio sigue siendo correcto por lo que se lee en el código, pero no era lo que él notaba.
- **La retirada del foco externo no llegó a aplicarse** en el paquete de la mañana: el estado del filtro se publicaba con una continuación atada al contexto de sincronización y, si la primera lectura llegaba sin contexto, la marca «en curso» se quedaba puesta para toda la sesión (el registro nunca mostró «lo hacen dentro del juego»). Ahora la lectura deja el resultado en un campo y lo publica el tick de la interfaz. Comprobado con el juego real: «Foco simulado: 2 ventanas lo hacen dentro del juego (filtro XInput); no se les envían mensajes».

## ADR-026 · Aceptada (experimental) · Teclas que no se quedan pegadas al perder el foco, y un mando de prueba

**Contexto (propietario, 2026-10-11):** dentro de la partida los dos se mueven bien, pero solo al jugador de la ventana con foco le funcionan LB/RB para cambiar de objeto; al otro hay que darle el foco con Alt+Tab. En el inventario, LB/RB funcionan en los dos.

**Causa comprobada** (código del juego y de Unity, y reproducida con el juego real):

- Unity suelta todas las teclas cuando su ventana se desactiva (`win::Input::Activate(false)`). El foco simulado del filtro le oculta esa desactivación.
- Al cambiar de ventana con **Alt+Tab**, el juego ve pulsar Alt y el «soltar» llega a la otra ventana: Alt se queda pulsada para él.
- En 7DTD, LB/RB abren la rueda del cinturón (`XUiC_Radial`); el toque rápido llama a `HandleToolbeltCommand`, que con Alt pulsada (`InputUtils.AltKeyPressed`) pasa de página en vez de cambiar de objeto. El inventario no mira Alt.

**Decisión (en el filtro, junto al foco simulado):**

- Se apuntan las teclas que el juego vio pulsar. Cuando su ventana deja de ser la activa de verdad, el juego recibe el «soltar» (`WM_KEYUP`) de cada una.
- Una pulsación que llega cuando la ventana ya no es la activa no se entrega: nadie la soltaría. Pasa con las repeticiones de Alt que quedan en cola, porque Unity se devuelve el foco de teclado (`SetFocus`) al «activarse».
- `GetAsyncKeyState` y `GetKeyState` del juego responden «sin pulsar» mientras no es la ventana activa (Unity los consulta para Mayús).
- El estado del filtro añade `teclas_soltadas=N`.

**Mando de prueba (diagnóstico):** si existe `%LOCALAPPDATA%\SplitForge\xinput\prueba.txt`, cada línea `PID BOTONES_HEX [opciones]` mantiene pulsados esos botones en el mando de ese proceso (si la ranura no tiene mando, simula uno en reposo). Opciones solo para pruebas: `sin_soltar` apaga esta corrección (para reproducir el fallo) y `activa` trata la ventana como la activa (para la prueba de humo). Sin el archivo no hace nada: solo se mira si existe una vez por segundo. **Por qué:** permite ensayar con el juego real sin las manos del propietario; así se comprobó esta corrección.

**Barra de tareas (misma sesión):** con «Tapar barra de tareas», Windows volvía a subir la barra por encima del juego de abajo al cambiar la ventana activa (ninguna ventana de la pantalla dividida ocupa todo el monitor). Cada ≈0,5 s, si la barra está por encima de una ventana de la sesión a la que pisa, esa ventana se vuelve a subir sin activarla (`TaskbarPlan`, `TaskbarGuard`). No se oculta ni se modifica la barra.

**Evidencia (2026-10-11, juego real, `tools/ensayo_7dtd.ps1`):** J1 dentro de su partida y sin foco; el mando de prueba pulsa RB y se lee la casilla resaltada del cinturón en una captura de su ventana:

| Fase | Situación de J1 | Casilla antes → después |
| --- | --- | --- |
| A | Sin foco, sin teclas pegadas | 8 → 9 (cambia) |
| B | Perdió el foco con Alt pulsada, corrección apagada | 9 → 9 (**no cambia: el fallo**) |
| C | Perdió el foco con Alt pulsada, corrección encendida | 9 → 10 (**cambia: corregido**) |

En la fase C el estado de J1 pasa a `teclas_soltadas=1`. Los juegos quedaron por encima de la barra de tareas en todas las muestras. Además, «Cerrar juegos» dentro de una partida guarda antes de salir (`SaveAndCleanupWorld`, «Saving 46 of chunks»).

**Límites:**

- Falta que el propietario lo confirme jugando con los dos mandos (W23).
- El ensayo carga la última partida del propietario y manda Alt al juego: en un ensayo anterior, cuando el mando de prueba aún metía pulsaciones dobles, J1 gastó un frasco de agua.
- A veces el juego abre al arrancar un cuadro «Inicio de sesión de Discord» que hay que aceptar; mientras está abierto no carga la partida. Es del propio juego (va sin EOS). El propietario indica que no le molesta: no se toca.

## ADR-027 · Aceptada (experimental) · Biblioteca de varios juegos; No Man's Sky con una copia por jugador

**Contexto (propietario, 2026-10-11):** «agrega No Man's Sky en SplitForge, haremos que funcione tal cual lo hicimos con 7DTD». El lanzador (ADR-024) solo conocía 7DTD. No Man's Sky no admite la vía de 7DTD: su multijugador va por servicios en línea y el ejecutable lleva la protección de Steam ([`plans/no-mans-sky.md`](plans/no-mans-sky.md)). El propietario eligió J1 en Steam y J2 con otra copia (GOG).

**Decisión:**

- **Catálogo en Core** (`LauncherGames`): identificador, nombre, AppID, cartel, pasos de «Cómo se juega» y si usa identidad emulada. La biblioteca de la pantalla JUGAR es una lista seleccionable; el juego elegido se guarda en `settings.v2.json` (`SelectedGame`) y un identificador desconocido vuelve al primero.
- **JUGAR, «Cerrar juegos», «Preparar ahora» y la lista «Preparación» hacen lo que toque en el juego elegido.** Con dos juegos son dos ramas en el modelo de la interfaz; una interfaz común de lanzamiento se extraerá cuando un tercer juego enseñe qué tienen en común de verdad.
- **No Man's Sky** (`NoMansSkyAdapter`, sin emulador de Steam):
  - J1 usa la copia de Steam y se abre **a través de Steam** (`steam://rungameid/275850`), como al pulsar «Jugar» en su biblioteca. No se arranca el ejecutable protegido por otra vía.
  - J2 usa otra copia, con su cuenta: la que el usuario indica eligiendo su `NMS.exe`, o la de GOG si se detecta. La misma carpeta de Steam no vale como segunda copia.
  - Qué jugador es cada proceso se sabe por la carpeta de su ejecutable. Cada proceso recibe su mando en cuanto existe; la pantalla se divide cuando están las dos ventanas. Con una sola copia se abre solo ese jugador y se dice.
  - El filtro de mandos se instala como `xinput9_1_0.dll` (la que importa el juego) en `Binaries` de cada copia, **solo con «Preparar ahora» y tras una confirmación** que explica que es un juego en línea y que el filtro es un archivo añadido. No se sobrescribe nada del juego y se retira con «Quitar filtro de mandos».
  - «Cerrar juegos» pide el cierre normal de cada ventana, avisando antes de que el juego solo guarda en sus puntos de guardado. Nunca a la fuerza.
- **El cartel del juego dice «EN PREPARACIÓN · SIN VALIDAR»** y su ficha, «En preparación · 2 copias, 2 cuentas». Estar en la biblioteca no declara compatibilidad.
- **Filtro nativo:** en un juego que no es de Unity adopta su ventana a los 3 s (antes 20 s) y descarta las ventanas menores de 320 × 200.
- **Carátulas:** `SteamArt` lee también la disposición nueva de la caché de Steam (cada imagen en una subcarpeta con nombre de huella; la vertical se llama `library_capsule.jpg`).

- **AVANZADO › NO MAN'S SKY** (añadida el mismo día: el propietario esperaba encontrar el juego ahí, como 7DTD, con su «Instalar / actualizar» y el emulador). Tiene lo que aplica a este juego: copias, filtro de mandos (instalar o actualizar, asignar, quitar, estado por ventana) y abrir, dividir y cerrar. El bloque «Emulador de Steam» no ofrece instalarlo: explica en palabras llanas por qué aquí no sirve. Se mantiene al día aunque el juego elegido en la biblioteca sea otro.

**Alternativas descartadas:** el emulador de Steam, quitar la protección del ejecutable o usar la versión antigua (ver el plan); esperar a validar el juego para añadirlo al lanzador (el propietario lo pidió ahora, y el cartel dice el estado real). Sobre el emulador: es **hecho** que el juego inicia sesión en su servicio en línea con el pase de la cuenta de Steam (cadenas del ejecutable); que ese servicio rechace una identidad emulada es un supuesto razonable sin prueba propia. No se ha abierto el juego con el emulador.

**Evidencia (2026-10-11):** 203 pruebas xUnit (8 nuevas: instalación, copia de J2, jugador de cada proceso, preparación, catálogo, ajustes y carátulas). Con el paquete publicado y UI Automation (`tools/ensayo_nms.ps1 -SinJuego`): la biblioteca lista «7 Days to Die» y «No Man's Sky»; al elegir el segundo, la preparación refleja este PC (J1 en Steam: sí; copia de J2: falta; filtro: falta; mandos: 2) y aparecen «Preparar ahora» y «Elegir copia de J2…». Los ajustes del propietario se restauraron al terminar.

**Límites:**

- **JUGAR y «Cerrar juegos» de No Man's Sky no se han ejecutado con el juego** desde el lanzador. La sonda anterior hizo esos pasos con un guion (abrir por Steam, reconocer la ventana, cerrar con `WM_CLOSE`), no con este código.
- Nunca se han abierto dos instancias: no hay segunda copia.
- La detección de GOG (registro de GOG Galaxy) es un supuesto: en este PC no hay GOG.
- Sigue el bloqueante de la sonda: con Steam Input activo, la copia de Steam no responde al mando del filtro.

