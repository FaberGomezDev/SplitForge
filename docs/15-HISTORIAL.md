# Historial del proyecto y de la documentación

Los hitos provienen de los paquetes del prototipo, del repositorio Git y de las verificaciones comunicadas por el usuario. No son benchmarks de juegos comerciales.

## Hitos

- **0.1–0.2:** primer laboratorio; de XInput exclusivo a SDL/WinMM; instaladores.
- **0.3:** diagnóstico reproducible SDL/Windows; enumeración separada de lectura.
- **0.3.1:** lectura directa joystick de PS4.
- **0.3.2:** aprendizaje de botones; disparo, escudo, impulso.
- **0.4:** laboratorio de mundo compartido. **Usuario confirmó que funcionó.**
- **0.5:** biblioteca Steam e informes sin rutas personales. **Usuario confirmó funcionamiento.**
- **0.6:** inspección HWND/PID/EXE y distribución conservadora; 72 pruebas.
- **0.6.0+docs:** `CLAUDE.md`, `docs/`, plantillas, validación de enlaces; 75 pruebas.

## 2026-10-10 · Repositorio y diagnóstico 7DTD

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 180218f
OBJETIVO: subir el código a GitHub y obtener evidencia de cómo lee 7DTD los mandos.
CAMBIOS EFECTIVOS: diagnostico_7dtd.py + Diagnostico_7DTD.bat (módulos de entrada por proceso, solo lectura);
  README sin la línea UTF-16 corrupta; reglas de Git del propietario en CLAUDE.md.
EVIDENCIA AUTOMÁTICA: 77 pruebas Python OK.
EVIDENCIA EN WINDOWS REAL: el propietario inicia 7DTD sin EAC con la opción oficial.
PRÓXIMO PASO: ejecutar el diagnóstico con el juego abierto.
```

## 2026-10-10 · 0.7.0 · Reconstrucción en .NET 10 + SDL3

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 3018ffa (+ correcciones posteriores del mismo día)
OBJETIVO: tecnologías más eficientes (decisión del propietario) y camino hacia 7DTD con dos jugadores.
CAMBIOS EFECTIVOS: solución SplitForge.slnx (Core, Windows, Input, LabViewer, App WPF, Diagnostico, Tests);
  entrada SDL3 en hilo dedicado ≤1000 Hz; simulación 120 Hz; IPC binario; ventanas 2–4 sin bordes;
  adaptador 7DTD (servidor dedicado oficial aislado, Sandboxie-Plus, pantalla dividida, diagnóstico);
  CI Windows con paquete autónomo y releases por etiqueta; documentación actualizada (ADR-010…014).
EVIDENCIA AUTOMÁTICA: 112 xUnit + 77 unittest; CI windows-latest verde (pruebas, publicación,
  humo: SDL3 3.5.0 inicializa en Windows); Linux: hilo de entrada 1000 Hz, visor dibujado bajo Xvfb.
EVIDENCIA EN WINDOWS (VM de la CI): interfaz WPF arranca, 6 pestañas capturadas, cierre ordenado,
  EnumWindows real, SDL3 3.5.0 a 1000 Hz, filtro XInput C++ cargando la DLL real de System32.
EVIDENCIA EN WINDOWS REAL (propietario, con mandos): pendiente.
NUEVOS RIESGOS: perfiles de botones a recalibrar (GUID SDL3); Sandboxie y Steam; firewall al abrir el servidor.
LIMITACIONES EXPLÍCITAS: sin aislamiento de mandos validado dentro del juego.
  Retirados a propósito: lanzamiento doble genérico de .exe y demo de dos mundos separados (siguen en el prototipo).
PRÓXIMO PASO: validar W01–W07; SplitForge_Reporte_7DTD.json; validar el filtro XInput (W16).
AMPLIACIÓN (mismo día): recuperación de ventanas tras cierre inesperado; filtro XInput nativo C++20
  (native/SplitForge.XInputFilter, ADR-015) con instalación reversible desde la pestaña 06 y prueba de humo en la CI.
```

## 2026-10-10 · Emulador de Steam (nació como experimento temporal; hoy componente principal)

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 2c20d4a (en main; CI run #14 verde)
OBJETIVO: experimento educativo a petición del propietario: reimplementar la técnica de Goldberg
  (sustituir steam_api64.dll) para estudiar cómo una copia se hace pasar por varias identidades.
CAMBIOS EFECTIVOS: native/SplitForge.SteamEmu (C++20): API plana completa, ISteamClient/User/Friends/
  Utils/Apps/UserStats/Matchmaking/Networking/RemoteStorage/UGC/Inventory/HTTP/Screenshots/Controller/
  Input/GameServer con vtables ABI del SDK; callbacks y call results; P2P UDP multicast local;
  stats/logros y nube en %LOCALAPPDATA%\SplitForge\steamemu; instalador reversible
  tools/SteamEmu_Instalar.ps1; CI: build+ctest del módulo.
  Integración en la interfaz (petición del propietario): Core.Adapters.SteamEmulator (instalar/
  restaurar con copia de seguridad + SHA-256, config JSON, LaunchSpec con variables de entorno),
  pestaña 07 «EMULADOR STEAM» (instalar/restaurar, identidades J1/J2, lanzar instancias), DLL
  publicada en native\SplitForge.SteamEmu del paquete.
  Documentación: ADR-016, MEMORIA, HISTORIAL.
EVIDENCIA AUTOMÁTICA: ctest steamemu_humo: dos procesos con identidades distintas, callback de stats,
  stats.json persistido, nube ida y vuelta, P2P entre procesos OK. También verde en la CI (run #14).
EVIDENCIA EN WINDOWS REAL: la prueba de humo se ejecutó en el PC del propietario (2026-10-10 03:51,
  el de referencia: Ryzen 7 7700X, RTX 4060; corregido: antes decía "no en el del propietario").
  Con 7DTD real: ver la entrada «Memoria contrastada» más abajo.
NUEVOS RIESGOS: sustituir la DLL puede incumplir los términos de Steam; nada de EAC/VAC; restaurar
  antes de verificar archivos en Steam; el orden de vtables sigue el SDK público (colas de interfaces
  nuevas no probadas).
LIMITACIONES EXPLÍCITAS: NetworkingSockets/Messages y otras interfaces sin implementar (nullptr);
  P2P sin cifrado ni fiabilidad; alcance misma PC/LAN por multicast.
PRÓXIMO PASO (original): instalar en 7DTD sin EAC y probar dos instancias; eliminar el módulo tras el
  experimento. SUSTITUIDO el mismo día: el propietario lo declaró componente principal y permanente.
```

## 2026-10-10 · Informe real de 7DTD e instrumentación del filtro XInput

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 59674e9, en la rama 7dtd-diagnostico-xinput (CI run #11 verde); integrada en main en fd98ba2
OBJETIVO: aprovechar el primer SplitForge_Reporte_7DTD.json real para preparar la prueba W16.
EVIDENCIA RECIBIDA: 1 cliente 7DaysToDie.exe sin EAC (módulos legibles, sin módulos de EAC);
  XInput: xinput1_3.dll + xinput1_4.dll; DirectInput (dinput8), HID, InControl (incontrolnative),
  steam_api64, superposición de Steam, EOS SDK; SDL3 ve 2 × "Xbox 360 Controller (xbox360)".
CAMBIOS EFECTIVOS:
  - Filtro nativo: estado-PID-<dll>.txt por copia (antes las dos copias escribían el mismo archivo
    y solo una vez, así que tras asignar seguía apareciendo "sin asignar"); se reescribe al cambiar
    la asignación y cada 2 s con el contador de lecturas; reemplazo atómico; DLL real cargada por ruta
    absoluta y verificada en System32 (la alternativa xinput1_4 ya no puede resolver a otra copia).
  - XInputFilter.Install: valida nombres antes de copiar, omite archivos ajenos sin abortar,
    actualiza solo copias propias sin modificar y guarda el manifiesto aunque falle una copia.
  - Detección de la ranura XInput de cada mando («Detectar» en la pestaña 06 y comando xinput).
  - Informe 7DTD v3: origen de cada módulo de mando (sistema/juego/steam/otro, sin rutas),
    filtro_xinput_cargado (SHA-256), motor, ranuras XInput conectadas.
  - Estado del filtro y de las ranuras refrescado en segundo plano en la interfaz.
EVIDENCIA AUTOMÁTICA: 126 xUnit + 77 unittest; filtro y prueba de humo compilados sin avisos con
  MinGW (-Wall -Wextra -Werror); la prueba de humo (dos copias, reasignación en vivo) corre en la CI de Windows.
EVIDENCIA EN WINDOWS REAL: el informe del propietario; nada de lo nuevo probado aún en su PC.
PRÓXIMO PASO: W16 con dos clientes (lecturas, un mando por ventana, ventana sin foco) e informe v3.
```

## 2026-10-10 · Tipo y carga de batería de los mandos

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 665b760, en la rama 7dtd-diagnostico-xinput (CI run #12 verde)
OBJETIVO: ver de un vistazo qué mando es cada uno y cuánta carga le queda.
CAMBIOS EFECTIVOS: pestaña 01 con columnas «Tipo» (PlayStation 4/5, Xbox 360/One, estándar, genérico
  HID) y «Carga» (SDL_GetJoystickPowerInfo una vez por segundo: «Batería 80%», «Cargando 45%»,
  «Cargado», «Cable», «?» sin porcentaje; aviso de batería baja).
EVIDENCIA AUTOMÁTICA: pruebas xUnit del tipo legible y de la etiqueta de batería.
EVIDENCIA EN WINDOWS REAL: no realizada.
PRÓXIMO PASO: W02 con los dos PS4.
```

## 2026-10-10 · Bloqueo de ratón por ventana (ADR-017)

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 819a45b, en la rama 7dtd-diagnostico-xinput (cubierto por la CI run #13)
OBJETIVO: que el ratón no se escape a la mitad del otro jugador en pantalla dividida.
CAMBIOS EFECTIVOS: CursorLock (ClipCursor al área de cliente de la ventana del jugador en primer
  plano, reaplicado en el tick de la interfaz) y GlobalHotkey (Ctrl+Alt+L), con botón en la interfaz;
  se suelta al desactivar, detener la sesión o cerrar. Solo actúa sobre ventanas que SplitForge colocó.
EVIDENCIA AUTOMÁTICA: pruebas del atajo (etiqueta y modificadores MOD_* de Win32).
EVIDENCIA EN WINDOWS REAL: no realizada.
LIMITACIONES EXPLÍCITAS: solo ratón; el teclado sigue yendo a la ventana con foco.
PRÓXIMO PASO: W18.
```

## 2026-10-10 · Ajuste de cada juego a su ventana

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 5d1c748, en la rama 7dtd-diagnostico-xinput (CI run #13 verde)
OBJETIVO: que cada juego encaje en su región sin deformarse.
CAMBIOS EFECTIVOS: modos «Rellenar» y «Mantener proporción» (16:9, 16:10, 21:9, 4:3) centrados en la
  región, resolución sugerida por panel y persistencia en settings.v2.json. SplitForge solo cambia la
  ventana; la resolución interna la fija el usuario en el juego.
EVIDENCIA AUTOMÁTICA: pruebas xUnit de distribución (letterbox/pillarbox, proporción inválida) y de
  configuración (ida y vuelta, saneado).
EVIDENCIA EN WINDOWS REAL: no realizada.
PRÓXIMO PASO: W19.
```

## 2026-10-10 · Fusión en main

```text
FECHA: 2026-10-10
RELEASE/COMMIT: fd98ba2 (fusión de 7dtd-diagnostico-xinput) + 783da1d (reglas de Git de main)
OBJETIVO: unir en main la rama del informe 7DTD con el emulador de Steam.
CAMBIOS EFECTIVOS: todo lo de la rama, conservando las reglas de main (trabajar en main; CI solo en
  push a main, PR y manual).
EVIDENCIA AUTOMÁTICA: 158 xUnit + 77 unittest; CI run #15 cancelada (la sustituyó la #16) y run #16
  verde, con las pruebas de humo de los dos módulos nativos y la prueba de arranque de 7 pestañas.
EVIDENCIA EN WINDOWS REAL: no aplica.
PRÓXIMO PASO: validar en hardware.
```

## 2026-10-10 · Memoria contrastada con el código y con el PC del propietario

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 0f99505 (main)
OBJETIVO: que la memoria diga la verdad: contrastar docs con el código, las pruebas y el PC real.
CAMBIOS EFECTIVOS:
  - Docs al día: 158 xUnit, 7 pestañas, comando xinput, CursorLock/GlobalHotkey/SessionRecovery,
    carpetas xinput\ y steamemu\, CI completa, estructura native/ y tools/, W20.
  - Emulador de Steam = componente principal y permanente (decisión del propietario): ADR-016 y
    textos de la interfaz, LEEME, README nativo, CI y Compilar_SplitForge.bat sin "educativo",
    "temporal" ni "eliminar".
  - Quitados jsontest.obj y minijson.obj (objetos de compilación subidos por error); *.obj ignorado.
EVIDENCIA AUTOMÁTICA: 158 xUnit + 77 unittest; check_docs OK; la app WPF compila; el emulador
  recompilado con MSVC (CMake) y steamemu_humo superada en este PC (el primer enlace falló con
  LNK1104 pasajero; al reintentar, todo verde).
EVIDENCIA EN WINDOWS REAL (PC del propietario, solo lectura):
  - hardware = el de referencia; hilo SDL3 3.5.0 a 1000 Hz sin mandos conectados;
  - 7DTD build 25661859 (V 3.3.0 b18); servidor dedicado 294420 y Sandboxie-Plus NO instalados;
    ViGEmBus y HidHide instalados; filtro XInput no instalado;
  - emulador INSTALADO en 7DTD. Sus registros muestran la primera prueba real (02:50–03:55):
    las instancias cargan J1/J2, pero 7DTD se cierra en SteamAPI_ISteamClient_GetISteamUser.
    Causa en el código: la API plana del emulador no recibe self y solo exporta 32
    SteamAPI_ISteamClient_*; el Steamworks.NET de 7DTD referencia 866 métodos SteamAPI_ISteam*.
NUEVOS RIESGOS: con el emulador instalado, 7DTD se cierra al iniciar. Abierto desde Steam carga la
  misma DLL emulada, así que previsiblemente también falla (supuesto). Restaurar para jugar.
PRÓXIMO PASO: corregir la API plana del emulador y probarla; restaurar la DLL original para W16
  con un segundo Steam; instalar el servidor dedicado para W12/W14.
```

## 2026-10-10 · Emulador funcional con 7DTD y lanzador con la configuración de Nucleus

```text
FECHA: 2026-10-10 (tarde)
RELEASE/COMMIT: dd6170e (main)
OBJETIVO: que 7DTD arranque con el emulador, dos jugadores en pantalla dividida, cada uno con su mando,
  sin barra de tareas y con un atajo para jugar solo con mandos (peticiones del propietario).
CAMBIOS EFECTIVOS:
  - Emulador: API plana generada (tools/generar_api_plana.py, comprobada en la CI) con las 944 entradas
    de Steamworks.NET; ContextInit con el contrato del SDK; 3 firmas planas y 7 de vtable como el SDK;
    estructuras de entrada pack(1); call results por SteamAPICallCompleted_t (703) con sus datos (antes
    se perdían al mover el evento); RequestCurrentStats como callback; respuestas de tickets (154, 163,
    168); cola de eventos segura (deque por índice, sin crecer sin fin); actualización en el sitio.
  - Referencia de Nucleus Co-op (su handler de 7DTD): clientes sin juego cruzado por argumentos
    (-crossplatform=None -serverplatforms=Steam, que 7DTD admite), ventana sin marco del tamaño de su
    panel, pantalla dividida automática con mínimo 16:9 y asignación de mandos por PID (ADR-018).
  - Tapar la barra de tareas (monitor completo, ventanas siempre encima, restaurable) y bloqueo de
    entrada «solo mandos» con Ctrl+Alt+B (ganchos de bajo nivel; ADR-019).
EVIDENCIA AUTOMÁTICA: 166 xUnit + 77 unittest; pruebas de humo del emulador (API plana, ContextInit,
  despacho manual) y del filtro (primera vez en el PC del propietario); check_docs; generador --comprobar.
EVIDENCIA EN WINDOWS REAL (PC del propietario):
  - 7DTD V 3.3.0 b18 con el emulador: SteamAPI_Init ok y Login ok; con juego cruzado, EOS rechaza el
    ticket local y «Jugar sin conexión» queda desactivado (exige EUserStatus.OfflineMode, visto en el IL);
    sin juego cruzado: menú principal.
  - Dos clientes a la vez con perfiles SPLITFORGEJ1 / SPLITFORGEJ2; 960×1032 corta el menú, 16:9 y
    arriba/abajo lo muestran entero.
  - Filtro cargado en los dos por xinput1_3, J1 → mando 1, J2 → mando 2, lecturas subiendo en ambos.
  - Arriba/abajo tapando la barra: la franja de la barra pertenece a la ventana de J2.
NUEVOS RIESGOS: sin juego cruzado no hay partidas con consolas; el bloqueo de entrada también bloquea el
  escritorio mientras está activo; ventanas «siempre encima» durante la sesión.
LIMITACIONES EXPLÍCITAS: falta la confirmación física de los mandos (W16, ventana sin foco) y del
  bloqueo (W21); partida común por IP sin probar (W14).
PRÓXIMO PASO: W16 y W21 con el propietario; si la ventana sin foco no reacciona, simulación de foco;
  después W14 (J2 se une a J1 en 127.0.0.1).
```

## 2026-10-10 · Foco simulado (FakeFocus)

```text
FECHA: 2026-10-10
RELEASE/COMMIT: (este commit)
OBJETIVO: que cada mando controle solo su ventana; el propietario informa de que no son independientes.
CAMBIOS EFECTIVOS: Core.Windows.FakeFocusPlan (mensajes del FocusLoop de ProtoInput, ventanas objetivo,
  intervalo 5–2000 ms); SplitForge.Windows.WindowFakeFocus (hilo propio, PostMessageW cada 100 ms a cada
  ventana de la sesión); casilla «Simular foco» en la pestaña 07 con estado (mensajes publicados/rechazados);
  settings.v2.json: FakeFocus (true) y FakeFocusIntervalMs (100). ADR-020.
EVIDENCIA AUTOMÁTICA: compilación de la solución sin avisos (dotnet 10, Linux); 171 xUnit OK y 1 fallo
  previo de rutas de Windows en SteamEmulatorTests, ajeno al cambio (6 pruebas nuevas en FakeFocusTests).
EVIDENCIA EN WINDOWS REAL: ninguna todavía.
NUEVOS RIESGOS: un juego podría reaccionar mal a mensajes de activación repetidos; se desactiva con la casilla.
LIMITACIONES EXPLÍCITAS: si 7DTD consulta el foco real o recibe el mando ajeno por DirectInput/Raw Input,
  los mensajes externos no bastan.
PRÓXIMO PASO: W16 con la casilla activada y desactivada.
```

## 2026-10-10 · Foco simulado: cada mando en su ventana

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: commit siguiente a 7ee4c38 en main
OBJETIVO: el propietario vio que los mandos no eran independientes (solo atendía la ventana activa).
CAMBIOS EFECTIVOS: el filtro XInput simula el foco (técnica ForceFocus/FocusHooks de Nucleus) en cada
  cliente con mando asignado: subclase de la ventana de Unity (WM_KILLFOCUS bloqueado; WM_ACTIVATE,
  WM_ACTIVATEAPP y WM_NCACTIVATE siempre activos), GetForegroundWindow/GetActiveWindow/GetFocus
  redirigidos en la tabla de importaciones de UnityPlayer.dll y del ejecutable, activación inicial;
  estado «foco=forzado». SplitForge recupera el foco real tras dividir (SetForegroundWindowElsewhere).
EVIDENCIA AUTOMÁTICA: 166 xUnit; filtro compilado sin avisos (MSVC /W4 /WX) y su prueba de humo.
EVIDENCIA EN WINDOWS REAL: IL de 7DTD: OnApplicationFocus → GameIsFocused; puntero propio por instancia
  (SoftCursor). «Lanzar J1 y J2» real (pulsado por UI Automation): J1 → mando 1, J2 → mando 2, los dos
  con foco simulado, arriba/abajo sobre la barra de tareas.
PRÓXIMO PASO: confirmación física del propietario (W16) y partida común por IP (W14).
```

## 2026-10-10 · W16: los dos mandos mueven las dos ventanas (causa encontrada)

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: este commit (solo documentación)
EVIDENCIA EN WINDOWS REAL (propietario + diagnóstico v3 con los dos clientes):
  - Con el foco simulado en ambos clientes, cada stick mueve LAS DOS ventanas (antes solo atendía la activa).
  - Los dos clientes tienen cargada la xinput1_4.dll REAL de System32 (origen «sistema»); el filtro solo
    está cargado como xinput1_3 («juego+sistema»). El juego lee los mandos por la 1.4 sin filtrar.
  - Descartado: la XInput1_3.dll de System32 no depende de la 1.4 (no la importa).
  - InControlNative.dll pide XInput1_3/XInput1_4/XInput9_1_0 por nombre; UnityPlayer.dll también.
PRÓXIMO PASO: averiguar qué módulo carga la 1.4 desde System32 y hacer que esa vía pase por el filtro.
  Referencia: Nucleus usa Game.Hook.XInputEnabled (XInput interceptado dentro del proceso, sea cual sea
  la DLL), además de XInputPlusDll con los mismos nombres que SplitForge.
```

## 2026-10-10 · XInput real interceptada: cada cliente enumera un solo mando

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: 56fe872 en main
OBJETIVO: que la vía xinput1_4 de 7DTD pase por el filtro (W16: cada stick movía las dos ventanas).
CAUSA COMPROBADA (corrige la entrada anterior):
  - UnityPlayer.dll (con símbolos): win::RawInput::Open hace LoadLibraryA("xinput1_3.dll") → copia del
    filtro. El sistema nuevo (NewInput, Windows.Gaming.Input) no se usa: no aparece en Player.log.
  - La XInput1_3.dll de System32 (10.0.26200.1) es un reenviador: carga xinput1_4.dll en ejecución
    (LoadLibraryExW + GetProcAddress), no por importación. «No depende de la 1.4» era falso.
  - InControlNative.dll: LoadLibraryW("XInput1_4.dll") y GetProcAddress (ordinal 100, SetState,
    GetCapabilities, GetBatteryInformation). Con la 1.4 real ya en memoria, Windows le daba esa.
  - Player.log real: «NativeInputDeviceManager enabled» y dos mandos (XInput::1, XInput::2) por cliente.
CAMBIOS EFECTIVOS: filtro: DLL real = xinput1_4.dll de System32 siempre; tabla de exportación de la DLL
  real redirigida al filtro (ADR-021); segunda copia delega; copia fijada en memoria; ordinales 104, 108
  y 109; XInputGetDSoundAudioDeviceGuids emulada; estado con intercepcion/interceptadas/previa.
  Aplicación: «Lanzar J1 y J2» asigna cada mando al crear el proceso y pone al día el filtro instalado;
  pestaña 06 y diagnóstico `xinput` muestran la intercepción.
EVIDENCIA AUTOMÁTICA: 173 xUnit; filtro sin avisos (MSVC /W4 /WX); prueba de humo ampliada: la XInput
  real de System32 queda interceptada, lecturas contadas aparte, segunda copia «delegada».
EVIDENCIA EN WINDOWS REAL: dos clientes de 7DTD lanzados como «Lanzar J1 y J2», con los dos mandos
  conectados (ranuras 1 y 2). Orden de carga en ambos: xinput1_3.dll (juego) → xinput1_4.dll (System32) →
  InControlNative.dll. Estado: intercepcion=activa, previa=no, 702 y 603 lecturas por la DLL real, foco
  forzado. Player.log de cada cliente: UN solo «XInput Controller» (XInput::1). Con J1 asignado después
  de arrancar, su InControl aún listaba dos: por eso ahora se asigna al crear el proceso.
  Las copias del filtro de la carpeta del juego quedaron actualizadas (solo copias propias, por SHA-256).
LIMITACIONES EXPLÍCITAS: nadie movió los sticks: falta la confirmación física (W16). Quien resuelva la
  XInput real antes que el filtro no queda cubierto (previa=si lo avisa). Ordinal 109: firma supuesta.
PRÓXIMO PASO: W16 con el propietario; después W14 (J2 se une a J1 en 127.0.0.1).
```

## 2026-10-10 · W16 superado: cada mando controla solo su ventana

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: este commit (solo documentación); código en 56fe872
EVIDENCIA EN WINDOWS REAL: paquete generado con Compilar_SplitForge.bat (173 xUnit) y «Lanzar J1 y J2»
  pulsado en la aplicación real. J1 → mando XInput 1, J2 → mando XInput 2; en ambos: intercepcion=activa,
  previa=no, foco forzado, un solo mando listado por InControl. El propietario prueba con los dos mandos
  y confirma que funciona: cada mando controla solo su ventana.
LIMITACIONES EXPLÍCITAS: comprobado en el menú del juego; aún no dentro de una partida común.
PRÓXIMO PASO: W14, que J1 y J2 se encuentren en la misma partida en este PC.
```

## 2026-10-10 · W14 superado: J2 entra sola en la partida de J1

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: 0c13111 en main
OBJETIVO: que J1 y J2 se encuentren en la misma partida en este PC (el propietario: «la conexión local»;
  después: «averigua cómo lo hace Nucleus y lo mejoramos»).
CAUSA DEL INTENTO FALLIDO DEL PROPIETARIO (registros de 20:14–20:20, «Conectar a IP» a mano):
  - Dos intentos: «Server is still initializing» (el anfitrión aún cargaba).
  - Tercero: todos los autorizadores correctos y BeginAuthSession = OK, pero nunca llegó
    ValidateAuthTicketResponse_t (7DTD lo registra con CreateGameServer): «authstate_nativeplatform».
  - El anfitrión no recibía LobbyCreated_t (lo registra como callback) y su IP pública salía 1.0.0.127.
CÓMO LO HACE NUCLEUS: emulador Goldberg y, según su handler de 7DTD, conexión manual por IP 127.0.0.1.
CAMBIOS EFECTIVOS (ADR-022): emulador con dos canales de callbacks; ValidateAuthTicketResponse_t por el
  canal correcto y con el ABI del SDK (20 bytes; también UserStatsReceived_t); LobbyCreated_t/LobbyEnter_t
  como callbacks en el orden de Steam; GetPublicIP = 127.0.0.1; partida y lobbies anunciados por multicast;
  RequestLobbyList con los lobbies de otras instancias; auto_join → GameServerChangeRequested_t.
  Aplicación: casillas «J2 entra sola en la partida de J1» (activada) y «J1 arranca cargando su última
  partida» (-quick-continue); el emulador instalado se pone al día al lanzar; ajustes en settings.v2.json.
EVIDENCIA AUTOMÁTICA: 176 xUnit (3 nuevas); prueba de humo del emulador ampliada (canales, ABI, lobby del
  otro proceso, unión automática a 127.0.0.1:26900); API plana al día.
EVIDENCIA EN WINDOWS REAL: «Lanzar J1 y J2» pulsado en el paquete nuevo con las dos casillas. J1 carga su
  partida (31 s), crea servidor y lobby (68 s); J2 recibe la petición y conecta (70–76 s); autenticación
  k_EAuthSessionResponseOK (77 s); «Player 'SplitForgeJ2' joined the game» y PlayerSpawnedInWorld
  (JoinMultiplayer) a los 107 s. Las dos sesiones siguen abiertas y estables.
NUEVOS RIESGOS: J1 y J2 comparten guardados: en la prueba, en J2 se pulsó «Continuar» y tuvo abierta 45 s
  la misma partida que alojaba J1 antes de unirse. Pendiente decidir si J2 usa su propia -UserDataFolder.
LIMITACIONES EXPLÍCITAS: sin sesión de juego real de los dos; la lista «Unirse a partida» sin comprobar
  (ISteamMatchmakingServers sigue vacío); el descubrimiento necesita una interfaz de red (multicast). Al
  enlazar la prueba de humo, dos veces LNK1104 (ejecutable bloqueado); al repetir, enlazó.
PRÓXIMO PASO: sesión real con los dos mandos; decidir el aislamiento de guardados de J2.
```

## 2026-10-10 · Fondo negro, guardados propios de J2 y automatizaciones como opción

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: d9e8401 en main
OBJETIVO: lo que pidió el propietario tras probar W14: la unión automática le resultó inesperada; J1 no
  debe cargar partida sin elegir mundo y la unión debe funcionar en los dos sentidos; se ven las uniones
  entre ventanas y pide un fondo negro eficiente.
CAMBIOS EFECTIVOS:
  - Fondo negro (ADR-023): SessionBackdrop, ventana Win32 sin dibujo propio justo debajo de las ventanas
    de la sesión; BackdropPlan en Core; casilla «Fondo negro» (activada).
  - Unión automática y carga automática de J1: opciones desactivadas por defecto; la unión funciona para
    J1 y para J2. Yo había dejado activada la carga automática tras mi prueba: restaurada a desactivada.
  - J2 con carpeta de datos propia (-UserDataFolder) y copia inicial de perfiles; casilla activada.
  - «Cerrar J1 y J2»: cierre normal, invitados primero y anfitrión al final.
EVIDENCIA AUTOMÁTICA: 183 xUnit (7 nuevas: fondo, carpeta de J2, orden de cierre, valores por defecto).
EVIDENCIA EN WINDOWS REAL: laboratorio (dos visores) y dos clientes de 7DTD lanzados con el paquete nuevo:
  el fondo (1920×1080) queda inmediatamente debajo de las dos ventanas y la unión pasa a negro (000000);
  desaparece al detener. J2 arranca con -UserDataFolder, crea ahí serveradmin.xml, tiene sus perfiles y
  llega al menú; J1 arranca sin -quick-continue.
LIMITACIONES EXPLÍCITAS: sin comprobar con el juego: entrar en partida con la carpeta propia de J2, la
  unión automática J2 → J1 y «Cerrar J1 y J2» dentro de una partida. Una primera comprobación del fondo usó
  por error una compilación antigua de la aplicación (dotnet test no recompila bin\Release de la App).
PRÓXIMO PASO: que el propietario juegue una sesión; con sus registros, cerrar esas tres comprobaciones.
```

## 2026-10-10 · El fondo negro se quedaba al salir de los juegos

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: ba5d95e en main
FALLO (propietario): al salir de los dos juegos desde su menú, la pantalla negra seguía y tapaba el
  escritorio; había que cerrar SplitForge.
CAUSA: una sesión de ventanas existentes seguía «activa» con sus ventanas ya destruidas; el fondo,
  a pantalla completa y por encima de todo, la seguía.
CAMBIOS EFECTIVOS: la sesión termina sola cuando no queda ninguna de sus ventanas (se quita el fondo y
  se sueltan ratón y bloqueo de entrada); el fondo no se muestra sin ventanas de la sesión. Unión
  automática desactivada en los ajustes del propietario (la señaló como inesperada tres veces).
EVIDENCIA AUTOMÁTICA: 183 xUnit.
EVIDENCIA EN WINDOWS REAL: laboratorio, terminando los visores desde fuera: con un visor el fondo sigue;
  sin ninguno, desaparece, queda «Los juegos se cerraron: sesión terminada» y SplitForge sigue abierto.
  Del propietario, con 7DTD: J1 abrió su mundo como público y J2 entró (sola, y después a mano por
  «Conectar a IP» con 127.0.0.1); al volver a entrar, J2 conserva posición y progreso.
LIMITACIONES EXPLÍCITAS: el arreglo no se ha visto aún con los juegos reales (sí con el laboratorio).
PRÓXIMO PASO: sesión de juego normal; lista «Unirse a partida»; unión J2 → J1; «Cerrar J1 y J2».
```

## 2026-10-10 · Rediseño de la interfaz como lanzador de juegos

```text
FECHA: 2026-10-10 (noche)
RELEASE/COMMIT: 7ae5845 en main
OBJETIVO: el propietario pide un diseño propio de juegos, quitando lo innecesario (7DTD es el primero
  de varios).
CAMBIOS EFECTIVOS (ADR-024): navegación JUGAR / MANDOS / AVANZADO; pantalla JUGAR con biblioteca,
  portada, jugadores, preparación, opciones y pasos; tema nuevo (Theme.xaml); carátulas desde la caché
  local de Steam; PlayReadiness y SteamArt en Core; mensajes que citaban «pestaña 06/07» actualizados;
  prueba de arranque de la CI adaptada (3 secciones + 6 pestañas).
EVIDENCIA AUTOMÁTICA: 187 xUnit (4 nuevas); los 130 enlaces de la ventana tienen propiedad en el modelo.
EVIDENCIA EN WINDOWS REAL: el paquete publicado arranca, enumera las 3 secciones y las 6 pestañas de
  AVANZADO por UI Automation y se cierra de forma ordenada. La pantalla JUGAR se vio con la portada, los
  dos mandos conectados y «Todo listo para jugar».
ERRORES PROPIOS CORREGIDOS: (1) el paquete se cerraba al arrancar porque el estado del filtro se
  refrescaba en el constructor, sin contexto de sincronización; lo detectó la comprobación del paquete
  publicado, no la compilación de desarrollo. (2) Una captura de comprobación recogió otra ventana del
  propietario en vez de SplitForge: se borraron todas las capturas y no se guarda ninguna en el repositorio.
LIMITACIONES EXPLÍCITAS: sin partida completa lanzada con el botón JUGAR nuevo; la nueva prueba de la CI
  aún no se ha ejecutado en GitHub.
PRÓXIMO PASO: opinión del propietario sobre el diseño; partida real desde JUGAR.
```

## 2026-10-11 · Un puntero por cliente y aviso de bloqueo en pantalla

```text
FECHA: 2026-10-11
RELEASE/COMMIT: 619f4c2 en main
OBJETIVO: el propietario informa de que la ventana sin foco solo deja moverse y golpear (el resto de
  botones no responde hasta darle el foco), de que su puntero a veces parpadea y de que con Ctrl+Alt+B
  no sabe si el teclado y el ratón están bloqueados.
CAUSA ENCONTRADA EN EL CÓDIGO (desensamblado de UnityPlayer.dll con símbolos e IL del juego): con el foco
  simulado, los dos clientes creen estar en primer plano y cada uno recoloca y encierra el único puntero
  de Windows (SetLockCursorInternal); Unity lee ese puntero en cada fotograma sin mirar el foco, y el
  puntero de los menús con mando de 7DTD (SoftCursor) salta a esa posición y se oculta cuando cambia.
  El foco simulado externo (ADR-020) lo provocaba 10 veces por segundo. SUPUESTO: que sea la causa de
  los síntomas; no se pudo reproducir sin los mandos.
CAMBIOS EFECTIVOS (ADR-025):
  - Filtro XInput: puntero propio por cliente (GetCursorPos / SetCursorPos / ClipCursor redirigidos
    mientras el cliente no es la ventana activa de verdad); estado «puntero=propio» y contador.
  - Aplicación: el foco simulado externo ya no publica mensajes a los clientes cuyo filtro simula el
    foco por dentro.
  - Aviso en pantalla del bloqueo «solo mandos»: cartel de 3 s al bloquear y al desbloquear, e insignia
    fija mientras dura (LockNotice, LockNoticePlan, OverlayWindow).
EVIDENCIA AUTOMÁTICA: 192 xUnit (5 nuevas), 77 unittest; prueba de humo del filtro ampliada (el proceso
  cree tener el foco, mueve y encierra «su» puntero y el de Windows no cambia).
EVIDENCIA EN WINDOWS REAL: el aviso, mostrado solo y medido con Win32 (sin bloquear nada): arriba y
  centrado, por encima, sin clics ni foco, pasa a insignia a los 3 s y desaparece al desbloquear.
  Paquete publicado con Compilar_SplitForge.bat.
LIMITACIONES EXPLÍCITAS: nada de esto se ha visto con 7DTD ni con los mandos (el propietario estaba
  usando el PC: no se abrieron los juegos). Sin compilar con MinGW; la CI aún no lo ha ejecutado.
PRÓXIMO PASO: el propietario prueba con los dos mandos (W23 y W24). Si la ventana sin foco siguiera
  fallando, lanzar con -debuginput=verbose y leer en Player.log el foco, el cursor y los conjuntos de
  acciones.
```

## 2026-10-11 · LB/RB en la ventana sin foco: teclas pegadas, barra de tareas y ensayo sin manos

```text
FECHA: 2026-10-11 (tarde)
RELEASE/COMMIT: 64c33ac en main
OBJETIVO: el propietario probó el paquete de la mañana. Dentro de la partida los dos se mueven bien, pero
  LB/RB (cambiar de objeto) solo funcionan en la ventana con foco; en el inventario funcionan en las dos.
  Además la barra de tareas se quedó visible hasta que eligió la ventana con Alt+Tab. Pide ensayarlo.
CAUSA COMPROBADA: al cambiar de ventana con Alt+Tab, la que pierde el foco se queda con Alt pulsada (el
  foco simulado le oculta a Unity la desactivación, que es cuando suelta las teclas). 7DTD, con Alt
  pulsada, hace que LB/RB pasen de página del cinturón (XUiC_Radial.HandleToolbeltCommand). La corrección
  de la mañana (puntero propio, ADR-025) no era la causa de este síntoma.
CAMBIOS EFECTIVOS (ADR-026):
  - Filtro XInput: al dejar de ser la ventana activa, el juego recibe el «soltar» de las teclas que tenía
    pulsadas; las pulsaciones atrasadas no se entregan; GetAsyncKeyState/GetKeyState «sin pulsar» en
    segundo plano; estado «teclas_soltadas».
  - Filtro XInput: mando de prueba (prueba.txt) para ensayar sin manos.
  - Aplicación: los juegos se mantienen por encima de la barra de tareas (TaskbarPlan, TaskbarGuard).
  - Aplicación: corregido que el estado del filtro dejara de actualizarse (y con él la retirada del foco
    simulado externo, que no había llegado a aplicarse).
  - tools/ensayo_7dtd.ps1: ensayo de extremo a extremo con el juego real.
EVIDENCIA AUTOMÁTICA: 195 xUnit (3 nuevas), 77 unittest; prueba de humo del filtro ampliada (Alt se suelta
  al perder el foco, una pulsación atrasada no llega al juego, mando de prueba sin mando en la ranura).
EVIDENCIA EN WINDOWS REAL (juego real, dos clientes, J1 en su partida y SIN foco, RB con el mando de
  prueba, casilla del cinturón leída en la captura de su ventana):
  fase A sin teclas pegadas 8 → 9; fase B Alt pegada con la corrección apagada 9 → 9 (fallo reproducido);
  fase C con la corrección 9 → 10 (corregido), «teclas_soltadas=1». Juegos por encima de la barra de
  tareas en todas las muestras. Registro: «Foco simulado: 2 ventanas lo hacen dentro del juego».
  «Cerrar juegos» dentro de la partida guarda (SaveAndCleanupWorld).
ERRORES PROPIOS: (1) el diagnóstico de la mañana era un supuesto y no explicaba el síntoma; (2) la retirada
  del foco externo no funcionaba y no lo comprobé con el juego; (3) el mando de prueba metía pulsaciones
  dobles al reescribirse su archivo: en ese ensayo J1 gastó un frasco de agua de la partida del
  propietario. Corregido antes del ensayo válido.
LIMITACIONES EXPLÍCITAS: falta que el propietario lo confirme jugando con los dos mandos. A veces el juego
  abre un cuadro «Inicio de sesión de Discord» al arrancar (es del juego; al propietario no le molesta).
PRÓXIMO PASO: confirmación del propietario (W23, W24) y siguiente juego: No Man's Sky.
```

## 2026-10-11 · No Man's Sky: inspección y por qué no sirve la vía de 7DTD

```text
FECHA: 2026-10-11 (tarde)
RELEASE/COMMIT: faa4773 en main
OBJETIVO: el propietario pide seguir con No Man's Sky (AppID 275850, build 25732212).
HECHO (solo lectura, sin ejecutar el juego): el multijugador va por PlayFab (el ejecutable importa
  PartyWin y PlayFabMultiplayer y no pide ninguna interfaz SteamNetworking); el ejecutable lleva la
  protección de Steam (sección .bind con el punto de entrada); mandos por XINPUT9_1_0 y SteamInput006;
  ajustes de pantalla en la carpeta del juego; guardados por cuenta. En el PC solo hay la copia de Steam.
  El handler de Nucleus Co-op solo vale para la versión antigua y usa un parcheador de la protección.
CONSECUENCIA: no se puede hacer como 7DTD. No hay partida por red local ni identidad emulable: cada
  jugador necesita su copia y su cuenta. No se quitará la protección del ejecutable ni se usará una
  versión antigua.
CAMBIOS EFECTIVOS: docs/plans/no-mans-sky.md (plantilla de adaptador, estado NO VALIDADO) y fila de la
  matriz de 05-INTEGRACION-STEAM.md. Ningún cambio de código.
LIMITACIONES EXPLÍCITAS: nada ejecutado; los supuestos (XInput con Steam Input desactivado, foco, VRAM
  para dos instancias, riesgo del filtro en juego en línea) están sin comprobar.
PRÓXIMO PASO: decisión del propietario sobre la segunda copia (GOG recomendada) o cambiar de juego.
```

## 2026-10-11 · No Man's Sky: ruta elegida y sonda con una instancia

```text
FECHA: 2026-10-11 (tarde)
RELEASE/COMMIT: (ver el registro de git de esta fecha)
DECISIÓN DEL PROPIETARIO: J1 en Steam y J2 con una copia en GOG. Se le pide no comprar hasta comprobar
  que el mando se puede aislar en la copia de Steam.
SONDA (una instancia por Steam, solo hasta el menú; tools/sonda_nms.ps1): el filtro, puesto de forma
  temporal como xinput9_1_0.dll, se carga y el juego lee XInput unas 12 veces por segundo; se instalan
  el foco simulado y el puntero propio; la ventana (clase GLFW30) se captura con PrintWindow y el juego
  cierra con WM_CLOSE. PERO los botones del mando de prueba (cruceta, A mantenida) no hacen nada y el
  juego sigue mostrando iconos de ratón.
SUPUESTO: con Steam abierto recibe el mando por Steam Input (activado para mandos Xbox en ese Steam; el
  juego trae sus configuraciones de Steam Input). Por ahí el filtro no puede aislarlo.
CAMBIOS EFECTIVOS: documentación y tools/sonda_nms.ps1. Ningún cambio de código. El filtro se retiró de
  la carpeta del juego al terminar; Steam quedó abierto (estaba cerrado antes de la sonda).
PRÓXIMO PASO: que el propietario desactive Steam Input solo para este juego y repetir la sonda. Si
  responde, seguir con la ruta A1; si no, replantear antes de comprar nada.
```

## Siguiente entrada (plantilla)

```text
FECHA:
RELEASE/COMMIT:
OBJETIVO:
CAMBIOS EFECTIVOS:
EVIDENCIA AUTOMÁTICA:
EVIDENCIA EN WINDOWS REAL:
NUEVOS RIESGOS:
LIMITACIONES EXPLÍCITAS:
PRÓXIMO PASO:
```

