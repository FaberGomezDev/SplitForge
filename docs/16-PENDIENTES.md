# Riesgos y preguntas abiertas

Resolver con evidencia concreta antes de programar sobre suposiciones.

## 7 Days to Die (prioritario)

- ~~¿Qué APIs de mando carga 7DTD en el PC del propietario?~~ → **Resuelto con un cliente (2026-10-10):** `xinput1_3` + `xinput1_4`, DirectInput, HID, InControl. Falta repetir con dos clientes (informe v3).
- ~~¿Por qué DLL lee 7DTD los mandos: XInput, DirectInput o HID?~~ → **Resuelto (2026-10-10):** por XInput en dos vías. Unity usa `xinput1_3` (copia del filtro) e InControlNative, que es quien mueve al jugador, usa `xinput1_4`. InControl no lista mandos por DirectInput ni HID.
- Los mandos llegan como `Xbox 360 Controller` y están instalados ViGEmBus y HidHide (2026-10-10). ¿HidHide oculta de verdad los PS4 originales al juego, sin duplicados por HID/DirectInput? ¿Qué programa crea los mandos virtuales?
- ¿La ventana **sin foco** sigue simulando el juego y leyendo mandos? (Unity puede pausar o ignorar la entrada en segundo plano).
- ~~¿Abre 7DTD una segunda instancia en la misma sesión de Windows?~~ → **Sí** con el emulador (2026-10-10): dos clientes en el menú a la vez desde la misma carpeta. Con Sandboxie-Plus u otro usuario, sin probar.
- ~~¿Procesa la ventana sin foco la entrada del mando?~~ → **No** (el propietario, 2026-10-10): 7DTD ignora la entrada sin foco (`GameIsFocused`). Hecho: foco simulado desde fuera (ADR-020) y dentro del filtro.
- ~~Bloqueante (W16): cada stick mueve las dos ventanas~~ → **causa encontrada y corregida (ADR-021, 2026-10-10):** la 1.4 real la cargaba el propio filtro a través de la `XInput1_3.dll` de System32 (un reenviador), e InControlNative la recibía por nombre. Ahora el filtro la intercepta y cada cliente enumera un solo mando. **Confirmado por el propietario con los mandos.**
- ~~Ventana sin foco: LB/RB no cambiaban de objeto~~ → **causa comprobada y corregida (ADR-026, 2026-10-11):** Alt se quedaba pegada tras Alt+Tab. Reproducido y corregido con el juego real en `tools/ensayo_7dtd.ps1`. **Falta la confirmación del propietario jugando con los dos mandos (W23).** Si apareciera otro botón que solo funciona con foco:
  - ¿Depende de una tecla modificadora (Alt, Mayús, Ctrl) en el código del juego? El estado del filtro (`teclas_soltadas`) dice si se soltó alguna.
  - ¿Qué dice `Player.log` con `-debuginput=verbose` (foco, cursor y conjuntos de acciones)?
  - ¿Envía algún botón del mando teclas o ratón (herramienta que convierte los PS4, Steam Input en modo escritorio)? Eso solo llega a la ventana activa.
- El puntero propio por cliente (ADR-025) está apoyado en el código, pero ningún síntoma del propietario lo ha confirmado ni desmentido.
- Cuadro «Inicio de sesión de Discord» al arrancar (a veces): el juego no sigue hasta aceptarlo y, con «J1 carga su última partida», la carga no empieza. Es del juego (se lanza sin EOS). El propietario indica que no le molesta (2026-10-11): no se toca.
- Si en otro juego (o tras una actualización) el estado del filtro dice «ya estaba cargada antes del filtro» (`previa=si`), alguien resolvió la XInput real antes: habría que enganchar el código de sus funciones, como `Game.Hook.XInputEnabled` de Nucleus.
- ¿Steam Input o la superposición de Steam (`gameoverlayrenderer64.dll`, cargada en 7DTD) interfieren con el filtro? Si las lecturas no suben, probar con Steam Input desactivado para 7DTD.
- ¿Coinciden versión de cliente y servidor tras las actualizaciones de Steam? Cliente: build `25661859` (V 3.3.0 b18); el servidor dedicado aún no está instalado.

## No Man's Sky (siguiente juego pedido)

Detalle en [`plans/no-mans-sky.md`](plans/no-mans-sky.md). El propietario eligió la ruta A1 (2026-10-11): J1 en Steam y J2 con una copia en GOG. **Aún no comprada, a propósito.**

- **Bloqueante:** con Steam abierto, la copia de Steam no responde al mando de prueba del filtro (sonda de una instancia, 2026-10-11). Supuesto: recibe el mando por Steam Input. Falta que el propietario desactive Steam Input para este juego y repetir la sonda. Si tampoco responde, la copia de Steam no sirve para un jugador con mando propio.
- ¿Lee la copia de GOG los mandos por XInput? Se espera que sí (no lleva Steam Input). Sin comprobar hasta tenerla.
- ¿Atiende el mando la ventana sin foco? El foco simulado del filtro busca primero la ventana de Unity y no cubre `GetKeyboardState` ni Raw Input.
- ¿Caben dos instancias en 8 GB de VRAM? Sin medir.
- ¿Acepta el propietario usar el filtro de mandos en un juego en línea? No hay antitrampas y el juego admite mods; riesgo bajo, no nulo.
- El lanzador solo conoce 7DTD: un segundo juego exige generalizar adaptador, biblioteca y botón JUGAR.

## Emulador de Steam (componente principal)

- ~~Bloqueante: API plana sin `self` e incompleta~~ → resuelto (2026-10-10): API plana generada con las 944 entradas de Steamworks.NET; `steamemu.log` registra la primera llamada a cada entrada, con «[sin implementar]» en las neutras.
- ¿Acepta el servidor dedicado oficial identidades emuladas? Sin probar (no está instalado). Ya no hace falta: J1 aloja la partida y J2 entra sola (W14, ADR-022).
- ~~¿Usa 7DTD `SteamNetworkingSockets` para su multijugador?~~ → **No** en la conexión local: usa LiteNetLib por UDP (puerto 26900) y `ISteamNetworking` (P2P); la autenticación va por `ISteamGameServer`.
- ~~Guardados compartidos~~ → **resuelto (2026-10-10):** J2 se lanza con su propia carpeta de datos (`-UserDataFolder`), con una copia inicial de los perfiles de personaje. Comprobado en la sesión del propio propietario (21:16–21:18): J1 eligió el mundo a mano, J2 entró sola con su carpeta propia (autenticación correcta y con el dueño ya bien: `owner: 76561198000000002`) y la copia local del mundo quedó en `clientes\J2\SavesLocal`; salió y volvió a entrar sin problema. **Sin comprobar aún:** la unión automática en el sentido J2 → J1 y «Cerrar J1 y J2» dentro de una partida (¿guarda 7DTD al recibir WM_CLOSE?).
- **Partida sin conexión (visto el 2026-10-10, 21:14):** el propietario cargó «FirstGame» desde el menú de J1 y el juego arrancó con `ServerEnabled = False` («Starting offline server»): J2 no puede entrar ni sola ni por IP. Es la opción de multijugador de la pantalla de partida del propio juego (`XUiC_NewContinueGameSettings`), que recuerda su último valor. Los clientes se lanzan ahora con `-ServerEnabled=true` para preajustarla; falta comprobar con el juego que ese valor se respeta al continuar una partida (si no, hay que encenderla a mano una vez). Con «J1 carga directamente su última partida» no ocurre: ese camino abre siempre en línea.
- ¿Muestra la pantalla «Unirse a partida» de J2 la partida de J1? El lobby de J1 se anuncia y J2 puede unirse por su ID, pero `ISteamMatchmakingServers` (lista LAN/Internet) sigue vacío. Falta mirarlo en la interfaz del juego.
- El descubrimiento entre instancias usa multicast UDP: sin ninguna interfaz de red activa podría no funcionar. Sin probar.
- Las vtables con `CSteamID` devuelto por valor siguen la convención de las funciones libres; MSVC devuelve las clases de los métodos virtuales por puntero oculto. Afecta a juegos C++ que usen las vtables directamente, no a Steamworks.NET.
- «Verificar archivos» de Steam (o una actualización que traiga otra `steam_api64.dll`) sustituye la DLL emulada. En ese caso «Restaurar» se niega (SHA-256 distinto) y quedan la copia de seguridad, el JSON y el estado en la carpeta del juego. Falta decidir cómo limpiar ese caso desde la pestaña 07.

## Bloqueo de ratón y teclado

- `ClipCursor` desde SplitForge (en segundo plano, con el juego en primer plano): ¿lo respeta Windows 11 o lo ignora por seguridad? Validar en hardware (W18).
- Bloqueo de entrada «solo mandos» (Ctrl+Alt+B): validar en uso real (W21) que no deja teclas pegadas y que los juegos siguen leyendo los mandos. Desde el 2026-10-11 avisa en pantalla al bloquear y al desbloquear; falta ver que el cartel y la insignia quedan por encima de los juegos y no molestan.
- Aislamiento de teclado por dispositivo: hoy el teclado va a la ventana con foco; separar dos teclados físicos exigiría Raw Input por dispositivo o un módulo nativo. Pendiente de decidir si hace falta.

## Diseño técnico

- Filtro XInput (experimental): 7DTD lo carga y, con la intercepción de ADR-021, cada cliente enumera un solo mando. W16 superado en el menú; falta que el propietario juegue una sesión con los dos mandos dentro de la partida común.
- Clientes en Sandboxie: el estado del filtro queda dentro del sandbox (escritura virtualizada); SplitForge lo lee de la carpeta por defecto de `SplitForge_J2`. Si el sandbox usa otra ruta raíz, no se verá (la asignación sí se lee).
- Identificador estable tras reconexión Bluetooth sin guardar seriales en reportes (hoy: reasignación explícita).
- Fullscreen exclusivo, DPI mixto y varios monitores (hoy: área útil del monitor principal).
- ~~Cierre de golpe con clientes sin bordes~~ → resuelto: `SessionRecovery` guarda posiciones/estilos y los restaura al siguiente arranque si el handle sigue siendo del mismo proceso.
- Medir el rendimiento con dos instancias en 8 GB de VRAM antes de proponer perfiles.
- ¿Ruta Gamepad de SDL3 para PS4 como valor por defecto? (probar en hardware).

## Prioridad

- **P0:** validar la 0.7 con los dos PS4 en el PC del propietario.
- **P1:** dos clientes de 7DTD; requiere el emulador corregido o Sandboxie-Plus, y el servidor dedicado.
- **P2:** módulo nativo de un mando por ventana (W16).
- **P3:** métricas.
- **P4:** firma y distribución.
