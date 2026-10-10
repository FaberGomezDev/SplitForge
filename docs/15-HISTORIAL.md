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
RELEASE/COMMIT: commit siguiente a 42cfa1e en main
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

