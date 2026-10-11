# No Man's Sky — adaptador · ESTADO: NO VALIDADO (`INSPECCIONADO`)

> Segundo juego pedido por el propietario (2026-10-11). Esta página recoge lo que se ha **leído** de la instalación, sin ejecutar el juego ni tocar su carpeta. Nada aquí declara compatibilidad.

## Identificación

- Título: No Man's Sky.
- Steam AppID: 275850.
- Build ID (observado): `25732212`.
- Fecha, sistema operativo y ejecutable real: 2026-10-11, Windows 11, `Binaries\NMS.exe` (x64, Vulkan).
- Requisitos de cada cliente: **una cuenta con licencia por jugador** (ver «Lo que cambia respecto a 7 Days to Die»).
- Restricciones oficiales/EULA/antitrampas: sin antitrampas junto al ejecutable. El ejecutable lleva la protección de Steam. El multijugador pasa por los servicios en línea de Hello Games.

## Hallazgos (solo lectura)

| Hecho | Evidencia |
| --- | --- |
| El multijugador va por PlayFab (servicios en la nube), no por Steam | `NMS.exe` importa `PartyWin.dll`, `PlayFabMultiplayerWin.dll`, `PlayFabCore.Win32.dll` y `PlayFabServices.Win32.dll` |
| No usa la red entre jugadores de Steam | Interfaces de Steam que pide: `SteamUser021`, `SteamFriends017`, `SteamUtils010`, `SteamMatchMaking009`, `STEAMUSERSTATS_INTERFACE_VERSION012`, `STEAMAPPS_INTERFACE_VERSION008`, `STEAMREMOTESTORAGE_INTERFACE_VERSION016`, `STEAMHTTP_INTERFACE_VERSION003`, `STEAMSCREENSHOTS_INTERFACE_VERSION003`, `SteamInput006`. Ninguna `SteamNetworking*` |
| El ejecutable lleva la protección de Steam | Sección `.bind` con el punto de entrada dentro (el envoltorio DRM de Steam) |
| Mandos por XInput y por Steam Input | Importa `XINPUT9_1_0.dll` (`XInputGetState`, `XInputSetState`) y pide `SteamInput006`; en sus cadenas aparecen también Raw Input, DirectInput y GameInput |
| Usa las funciones de foco y de puntero de Windows | Importa de `USER32`: `GetForegroundWindow`, `GetActiveWindow`, `GetCursorPos`, `SetCursorPos`, `ClipCursor`, `GetAsyncKeyState`, `GetKeyState`, `GetKeyboardState`, `RegisterRawInputDevices`, `GetRawInputData` |
| Los ajustes de pantalla están en la carpeta del juego | `Binaries\SETTINGS\TKGRAPHICSSETTINGS.MXML`, `TKGAMESETTINGS.MXML`, `GCUSERSETTINGSDATA.MXML` |
| Los guardados van por cuenta | `%APPDATA%\HelloGames\NMS\st_<id>` (una carpeta, de la cuenta de Steam del propietario) |
| En el PC solo hay la copia de Steam | Sin GOG Galaxy, sin copia de la Tienda de Microsoft ni carpeta `XboxGames`, sin Sandboxie-Plus |
| El handler de Nucleus Co-op no sirve para esta versión | Su propia descripción (2021): solo funciona con la versión antigua, que usaba la API de Steam para el multijugador. Emplea un emulador de Steam, un parcheador de la protección del ejecutable y una copia enlazada de la carpeta del juego por instancia |

## Lo que cambia respecto a 7 Days to Die

En 7DTD un jugador aloja la partida en su propio PC y el otro entra por la red local; por eso bastó con dar a cada instancia una identidad emulada. **En No Man's Sky eso no existe:**

- La sesión compartida la montan los servidores de Hello Games. Cada jugador tiene que iniciar sesión con una cuenta real que tenga el juego. Una identidad emulada no puede iniciar sesión ahí.
- El ejecutable está protegido: no arranca con el emulador de Steam si no se le quita la protección. **Eso no se va a hacer**: ni se salta la protección de un juego ni se modifican sus archivos.

Consecuencia: para dos jugadores en la misma partida hacen falta **dos copias del juego con dos cuentas**.

## Rutas

| Ruta | Qué exige | Valoración |
| --- | --- | --- |
| **A1. J1 en Steam + J2 con una copia en GOG** | Comprar el juego en GOG y tener GOG Galaxy abierto para J2. Los dos se encuentran con el juego cruzado del propio juego (códigos de amigo) | La más limpia: Steam y GOG conviven sin trucos; la copia de GOG tiene su propia carpeta (sus propios ajustes de pantalla) y no lleva la protección de Steam |
| A2. Dos cuentas de Steam, cada una con el juego | Segunda cuenta con el juego y **dos clientes de Steam a la vez** (Sandboxie-Plus u otro usuario de Windows) | Más frágil: Steam no admite dos clientes en la misma sesión; las dos instancias compartirían la carpeta de ajustes |
| A3. J2 con la copia de Xbox / Game Pass para PC | Suscripción o compra en la Tienda | Dudosa: su carpeta está protegida y el filtro de mandos no podría instalarse. Sin comprobar |
| B. Versión antigua (2019) con el emulador | Bajar una versión vieja y quitar la protección del ejecutable | **Descartada** |
| C. Dos instancias sin conexión | Lo mismo que B, y además cada uno estaría en su partida | **Descartada** |

**Qué haría SplitForge en la ruta A:** abrir o adjuntar las dos ventanas, dividir la pantalla, tapar la barra de tareas, dar un mando a cada ventana (el filtro, como `xinput9_1_0.dll`, en cada instalación) y mantener el foco, el puntero y las teclas por ventana, igual que en 7DTD. El emulador de Steam no interviene.

## Supuestos sin comprobar

- Con Steam Input desactivado para el juego, lee los mandos por XInput (y entonces el filtro puede aislarlos). Con Steam Input activo los leería por Steam y el filtro no los vería.
- La ventana sin foco deja de atender el mando (lo sugiere el handler de Nucleus, que fuerza el foco). El foco simulado del filtro debería servir, pero hoy busca la ventana de Unity primero y no cubre `GetKeyboardState` ni Raw Input.
- Dos instancias a la vez caben en 8 GB de VRAM con ajustes bajos. Sin medir.
- El juego en línea admite una DLL de mandos junto al ejecutable. No hay antitrampas y el juego admite mods, pero es juego en línea con servicios de terceros: riesgo bajo, no nulo, y la decisión es del propietario.

## Capacidades verificadas

| Prueba | Resultado | Evidencia / condiciones |
| --- | --- | --- |
| Iniciar 1 cliente oficial | Pendiente | El propietario tiene guardados (15 archivos), pero SplitForge no lo ha lanzado |
| Dos clientes en una PC | Pendiente | Requiere la segunda copia |
| Un mismo mundo/partida | Pendiente | |
| LAN sin Internet real | **No existe** | El multijugador pasa por servicios en línea |
| 2 mandos sin entrada cruzada | Pendiente | |
| Dos ventanas se restauran | Pendiente | |
| Guardados intactos tras cerrar/fallo | Pendiente | |
| Rendimiento medido | Pendiente | |

## Preflight y bloqueo

- Comprobaciones que nunca deben saltarse: que cada jugador tenga su propia copia y su sesión iniciada; que no haya emulador de Steam instalado en la carpeta de este juego.
- Qué mostrar cuando falta un cliente: que hace falta una segunda copia con otra cuenta, y por qué.
- Riesgos conocidos y cómo aislarlos: los ajustes de pantalla están en la carpeta del juego (no editarlos; pedir al usuario el modo ventana); juego en línea (ver supuestos).
- Modo offline: sin conexión no hay partida compartida.

## Datos de validación

- Configuración y fecha: 2026-10-11, inspección de solo lectura (`dumpbin`, cadenas del ejecutable, existencia de carpetas).
- Resultado final: `NO VALIDADO`. Bloqueado a la espera de la decisión del propietario sobre la segunda copia.
