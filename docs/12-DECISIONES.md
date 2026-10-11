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

## ADR-017 · Aceptada (experimental) · Bloqueo de ratón por ventana con ClipCursor

**Decisión:** un atajo global (Ctrl+Alt+L) y un botón confinan el cursor al área de cliente de la ventana del jugador en primer plano, usando `ClipCursor`, que se reaplica en el tick de la interfaz porque Windows lo reinicia al cambiar el foco. **Por qué:** en pantalla dividida el ratón se escapa a la otra mitad; es la técnica de Nucleus Co-op, sin inyección ni hooks. **Límites:** solo confina el ratón; el teclado sigue yendo a la ventana con foco (el aislamiento por dispositivo exigiría Raw Input, aún no implementado); Windows podría ignorar `ClipCursor` desde una app en segundo plano. **Garantías:** solo bloquea ventanas que SplitForge colocó; se suelta al desactivar, detener la sesión o cerrar. **Pendiente:** validar en hardware (W18).

## ADR-020 · Aceptada (experimental) · Foco simulado desde fuera del juego (FakeFocus)

**Contexto:** el propietario informa (2026-10-10) de que, con «Lanzar J1 y J2», los mandos no son independientes: ningún mando mueve solo su ventana. El filtro XInput sí entrega a cada cliente su mando (sus «lecturas» suben en los dos, también sin foco), así que la hipótesis principal es que 7DTD (Unity) no procesa la entrada en la ventana sin foco. El handler de 7DTD de Nucleus Co-op funciona porque su `xinput1_3.dll` propia (x360ce, opción `ForceFocus` en `ncoop.ini`) hace que cada ventana se crea siempre activa.

**Decisión:** primer paso sin tocar el juego: la técnica `FakeFocus` de Nucleus (`WindowFakeFocus.cs`), con los mensajes del `FocusLoop` de ProtoInput. Un hilo propio de SplitForge (`WindowFakeFocus`) publica con `PostMessageW`, cada 100 ms, `WM_ACTIVATE(WA_CLICKACTIVE)`, `WM_ACTIVATEAPP(TRUE)`, `WM_NCACTIVATE(TRUE)`, `WM_SETFOCUS` y `WM_MOUSEACTIVATE(HTCLIENT)` a cada ventana de la sesión activa (dos o más). Activado por defecto, con casilla en la pestaña 07 (`settings.v2.json`: `FakeFocus`, `FakeFocusIntervalMs`, 5–2000 ms). La lógica es pura y está probada (`FakeFocusPlan`).

**Por qué así:** no inyecta código, no modifica archivos del juego y no bloquea la interfaz (mensajes publicados en un hilo propio). Nucleus reenvía cada 1000 ms y ProtoInput cada 5 ms; 100 ms recupera el foco enseguida sin saturar la cola del juego.

**Límites (sin comprobar):** no está probado con 7DTD ni con mandos. Si el juego consulta el foco real (`GetForegroundWindow`) o recibe el mando ajeno por DirectInput/Raw Input en la ventana con foco, los mensajes externos no bastan; Nucleus lo resuelve dentro del juego. Siguiente evidencia: W16 con el foco simulado activado y desactivado.

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

