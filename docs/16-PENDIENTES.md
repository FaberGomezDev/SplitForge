# Riesgos y preguntas abiertas

Resolver con evidencia concreta antes de programar sobre suposiciones.

## 7 Days to Die (prioritario)

- ~~¿Qué APIs de mando carga 7DTD en el PC del propietario?~~ → **Resuelto con un cliente (2026-10-10):** `xinput1_3` + `xinput1_4`, DirectInput, HID, InControl. Falta repetir con dos clientes (informe v3).
- ¿Por qué DLL lee 7DTD los mandos: XInput, DirectInput o HID? → «lecturas» de cada copia del filtro en W16.
- Los mandos llegan como `Xbox 360 Controller` y están instalados ViGEmBus y HidHide (2026-10-10). ¿HidHide oculta de verdad los PS4 originales al juego, sin duplicados por HID/DirectInput? ¿Qué programa crea los mandos virtuales?
- ¿La ventana **sin foco** sigue simulando el juego y leyendo mandos? (Unity puede pausar o ignorar la entrada en segundo plano).
- ¿Abre 7DTD una segunda instancia en la misma sesión de Windows? Con el emulador, dos procesos `7DaysToDie.exe` arrancan a la vez y cargan sus identidades (2026-10-10), pero se cierran al iniciar Steam. Con Sandboxie-Plus u otro usuario, sin probar (Sandboxie-Plus no está instalado).
- ¿Steam Input o la superposición de Steam (`gameoverlayrenderer64.dll`, cargada en 7DTD) interfieren con el filtro? Si las lecturas no suben, probar con Steam Input desactivado para 7DTD.
- ¿Coinciden versión de cliente y servidor tras las actualizaciones de Steam? Cliente: build `25661859` (V 3.3.0 b18); el servidor dedicado aún no está instalado.

## Emulador de Steam (componente principal)

- **Bloqueante:** la API plana no recibe `self` y solo exporta 32 funciones `SteamAPI_ISteamClient_*`. El Steamworks.NET de 7DTD referencia 866 métodos `SteamAPI_ISteam*`. ¿Qué subconjunto llama 7DTD realmente al arrancar y en partida? Se puede medir registrando en `steamemu.log` cada entrada plana que se llame.
- Una vez arranque: ¿se ven J1 y J2 por LAN/P2P del emulador? ¿Acepta el servidor dedicado oficial identidades emuladas, o debe alojar la partida uno de los clientes?
- ¿Usa 7DTD `SteamNetworkingSockets` (hoy devuelve `nullptr`) para su multijugador?
- La prueba de humo debe cubrir la API plana tal como la llama Steamworks.NET.
- «Verificar archivos» de Steam (o una actualización que traiga otra `steam_api64.dll`) sustituye la DLL emulada. En ese caso «Restaurar» se niega (SHA-256 distinto) y quedan la copia de seguridad, el JSON y el estado en la carpeta del juego. Falta decidir cómo limpiar ese caso desde la pestaña 07.

## Bloqueo de ratón y teclado

- `ClipCursor` desde SplitForge (en segundo plano, con el juego en primer plano): ¿lo respeta Windows 11 o lo ignora por seguridad? Validar en hardware (W18).
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
