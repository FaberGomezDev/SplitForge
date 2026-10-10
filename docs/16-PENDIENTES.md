# Riesgos y preguntas abiertas

Resolver con evidencia concreta antes de programar sobre suposiciones.

## 7 Days to Die (prioritario)

- ~~¿Qué APIs de mando carga 7DTD en el PC del propietario?~~ → **Resuelto con un cliente (2026-10-10):** `xinput1_3` + `xinput1_4`, DirectInput, HID, InControl. Falta repetir con dos clientes (informe v3).
- ¿Por qué DLL lee 7DTD los mandos: XInput, DirectInput o HID? → «lecturas» de cada copia del filtro en W16.
- Los mandos llegan como `Xbox 360 Controller` y están instalados ViGEmBus y HidHide (2026-10-10). ¿HidHide oculta de verdad los PS4 originales al juego, sin duplicados por HID/DirectInput? ¿Qué programa crea los mandos virtuales?
- ¿La ventana **sin foco** sigue simulando el juego y leyendo mandos? (Unity puede pausar o ignorar la entrada en segundo plano).
- ~~¿Abre 7DTD una segunda instancia en la misma sesión de Windows?~~ → **Sí** con el emulador (2026-10-10): dos clientes en el menú a la vez desde la misma carpeta. Con Sandboxie-Plus u otro usuario, sin probar.
- ~~¿Procesa la ventana sin foco la entrada del mando?~~ → **No** (el propietario, 2026-10-10): 7DTD ignora la entrada sin foco (`GameIsFocused`). Hecho: foco simulado desde fuera (ADR-020) y dentro del filtro.
- **Bloqueante actual (W16):** con foco simulado, cada stick mueve las dos ventanas. Los dos clientes cargan la `xinput1_4.dll` **real** de System32 y el filtro solo está en `xinput1_3`. ¿Qué módulo la carga desde System32 (¿InControl con búsqueda solo en System32?) y cómo hacer que pase por el filtro? Nucleus usa `Game.Hook.XInputEnabled` (interceptar XInput dentro del proceso). La `XInput1_3.dll` del sistema no depende de la 1.4 (comprobado).
- ¿Steam Input o la superposición de Steam (`gameoverlayrenderer64.dll`, cargada en 7DTD) interfieren con el filtro? Si las lecturas no suben, probar con Steam Input desactivado para 7DTD.
- ¿Coinciden versión de cliente y servidor tras las actualizaciones de Steam? Cliente: build `25661859` (V 3.3.0 b18); el servidor dedicado aún no está instalado.

## Emulador de Steam (componente principal)

- ~~Bloqueante: API plana sin `self` e incompleta~~ → resuelto (2026-10-10): API plana generada con las 944 entradas de Steamworks.NET; `steamemu.log` registra la primera llamada a cada entrada, con «[sin implementar]» en las neutras.
- ¿Acepta el servidor dedicado oficial identidades emuladas, o debe alojar la partida uno de los clientes? (W14 por IP 127.0.0.1 con J1 de anfitrión).
- ¿Usa 7DTD `SteamNetworkingSockets` (en el emulador, objeto con métodos neutros) para su multijugador? Lo dirá `steamemu.log` al unirse J2.
- Las vtables con `CSteamID` devuelto por valor siguen la convención de las funciones libres; MSVC devuelve las clases de los métodos virtuales por puntero oculto. Afecta a juegos C++ que usen las vtables directamente, no a Steamworks.NET.
- «Verificar archivos» de Steam (o una actualización que traiga otra `steam_api64.dll`) sustituye la DLL emulada. En ese caso «Restaurar» se niega (SHA-256 distinto) y quedan la copia de seguridad, el JSON y el estado en la carpeta del juego. Falta decidir cómo limpiar ese caso desde la pestaña 07.

## Bloqueo de ratón y teclado

- `ClipCursor` desde SplitForge (en segundo plano, con el juego en primer plano): ¿lo respeta Windows 11 o lo ignora por seguridad? Validar en hardware (W18).
- Bloqueo de entrada «solo mandos» (Ctrl+Alt+B): validar en uso real (W21) que no deja teclas pegadas y que los juegos siguen leyendo los mandos.
- Aislamiento de teclado por dispositivo: hoy el teclado va a la ventana con foco; separar dos teclados físicos exigiría Raw Input por dispositivo o un módulo nativo. Pendiente de decidir si hace falta.

## Diseño técnico

- Filtro XInput implementado (experimental): ¿lo carga 7DTD? → el informe v3 lo dice (`filtro_xinput_cargado`). ¿Basta para aislar los mandos? → W16.
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
