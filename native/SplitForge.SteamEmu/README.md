# SplitForge.SteamEmu — emulador de Steam (EXPERIMENTAL)

> **Componente principal y permanente de SplitForge** (ADR-016, decisión del propietario del
> 2026-10-10). Se publica en el paquete como `native\SplitForge.SteamEmu\steam_api64.dll` y se
> gestiona desde la pestaña 07 «EMULADOR STEAM». Funciona con el juego abierto sin EAC (opción
> oficial). Sigue siendo experimental: con 7DTD real (V 3.3.0 b18, 2026-10-10) dos instancias
> llegan al menú a la vez con identidades distintas (ver `docs/05-INTEGRACION-STEAM.md`).

## Qué es

Una DLL nativa (C++20) que **reimplementa el contrato ABI de `steam_api64.dll`**: misma lista
de exportaciones, mismas vtables por interfaz y mismos callbacks. Cuando un juego la carga
con el nombre `steam_api64.dll`, cree que habla con Steam y recibe en su lugar una identidad
local definida en un JSON. Así una sola copia del juego puede arrancar con **identidades
distintas** por proceso (la técnica que usa Nucleus Co-op con Goldberg), p. ej. dos instancias
de 7DTD en la misma PC que se descubren por LAN local.

Nada de esto toca Steam: no hay autenticación real, no hay tienda, no hay servidores de Valve.

## Qué implementa de verdad

| Pieza | Comportamiento |
| --- | --- |
| API plana | `SteamAPI_Init/InitSafe/Shutdown/RunCallbacks`, dispatch manual, registros de callbacks y call results, `SteamInternal_*`, API de servidor de juego |
| `ISteamClient` 020 | getters por versión para todas las interfaces |
| `ISteamUser` 023 | identidad del JSON, tickets de autenticación falsos (LAN: `BeginAuthSession` = OK), carpeta de datos propia |
| `ISteamFriends` 017 | nombre de persona del JSON, rich presence local, sin amigos reales |
| `ISteamUtils` 010 | AppID, país, idioma, hora, `GetAPICallResult` para las llamadas simuladas |
| `ISteamApps` 008 | app y DLC del JSON como "suscritos" |
| `ISteamUserStats` 012 | stats y logros en memoria + `stats.json`; callbacks `UserStatsReceived` etc. |
| `ISteamMatchmaking` 009 | lobbies en memoria (datos, chat, dueño, game server) |
| `ISteamNetworking` 006 | **P2P real entre procesos del mismo PC/LAN** por UDP multicast: presencia, canales, colas |
| `ISteamRemoteStorage` 016 | archivos de nube en `%LOCALAPPDATA%\SplitForge\steamemu\<appid>\remote\`, streams y async |
| Control, música, vídeo, parental, app list, game server | stubs seguros (0 mandos, todo desactivado) |
| UGC, inventario, HTTP, screenshots | stubs vacíos seguros |

**Interfaces sin lógica propia** (objetos con métodos neutros): `SteamNetworkingSockets`, `SteamNetworkingMessages`,
HTMLSurface, Parties, RemotePlay, Timeline, GameSearch. Los juegos que dependan de
NetworkingSockets para su multiplayer no conectarán con este emulador (7DTD usa su propio
transporte con Steam P2P o sockets según la versión: pendiente de comprobar con 7DTD real).

## API plana (generada)

Los juegos que usan Steamworks.NET (como 7DTD) llaman **solo** a la API plana del SDK: cada método
de interfaz es una exportación `SteamAPI_<Interfaz>_<Método>(self, …)` que recibe primero el
puntero a la interfaz. `src/flat_generated.cpp` se genera con `tools/generar_api_plana.py` a partir
de las vtables (`src/interfaces.h`) y de la lista de entradas de Steamworks.NET
(`tools/steamworksnet_api.json`, extraída con `tools/extraer_api_steamworksnet.cs`):

- reenvío tipado a la vtable para cada método implementado (sobrecargas con el sufijo del SDK:
  `GetStatInt32`/`GetStatFloat`, `SetPropertyString`…), comprobando la clase de registro x64 de cada
  argumento frente a la firma de Steamworks.NET;
- entrada neutra (0, false, nullptr o "") para lo que el emulador no implementa;
- registro en `steamemu.log` de la primera llamada a cada entrada (`[sin implementar]` en las neutras).

Para otro juego: extraer su lista con `dotnet run tools/extraer_api_steamworksnet.cs -- <dll> tools/steamworksnet_api.json`
y regenerar con `python tools/generar_api_plana.py`. La CI comprueba que las salidas están al día
(`--comprobar`) y la prueba de humo, que la DLL exporta todas las entradas.

## Configuración

`SplitForge.SteamEmu.json` junto a la DLL (o en la ruta de la variable de entorno
`SPLITFORGE_STEAMEMU_CONFIG`, que **es la vía para dar identidad distinta a cada instancia**):

```json
{
  "app_id": 251570,
  "steam_id": "76561198000000001",
  "persona_name": "SplitForgeJ1",
  "language": "spanish",
  "ip_country": "ES",
  "dlc": [251570],
  "storage_dir": "%LOCALAPPDATA%\\SplitForge\\steamemu\\251570-J1",
  "net_port": 27117,
  "net_discovery_seconds": 2,
  "net_multicast_address": "239.255.69.69"
}
```

Dos instancias del mismo juego con el **mismo** `net_port` y `net_multicast_address` se
descubren por presencia multicast y se envían paquetes P2P directamente. Para dos instancias:
lanzar la segunda con `SPLITFORGE_STEAMEMU_CONFIG` apuntando a otro JSON con otro `steam_id`
y otro `storage_dir`.

## Instalar / restaurar (reversible)

```powershell
tools\SteamEmu_Instalar.ps1 -Juego "C:\Steam\steamapps\common\7 Days To Die" -AppId 251570 -SteamId "76561198000000001" -Persona "SplitForgeJ1" -Accion Instalar
tools\SteamEmu_Instalar.ps1 -Juego "C:\Steam\steamapps\common\7 Days To Die" -Accion Restaurar
```

En juegos Unity (como 7DTD) la DLL real vive en `<juego>\<Nombre>_Data\Plugins\x86_64\steam_api64.dll`,
no en la raíz: el instalador (y la pestaña 07 de SplitForge) la buscan en la raíz, en los
patrones de Unity y por subcarpetas, y trabajan siempre junto a la DLL encontrada. El instalador
hace copia de seguridad de la DLL original como `steam_api64_splitforge_original.dll` (con SHA-256
en el registro) y solo escribe la DLL emulada, el JSON y un archivo de estado. `Restaurar` devuelve
la original y borra lo instalado. **Nunca** toca otros archivos del juego. Antes de verificar la
integridad en Steam, restaura siempre.

## Compilar y probar

```powershell
cmake -S native/SplitForge.SteamEmu -B native/build-steamemu -A x64
cmake --build native/build-steamemu --config Release
ctest --test-dir native/build-steamemu -C Release --output-on-failure
```

La prueba de humo (`steamemu_humo`) lanza dos procesos que cargan la DLL con identidades
distintas y verifican: identidad, callback de stats + persistencia, nube y un intercambio
P2P local. Registro de actividad: `%LOCALAPPDATA%\SplitForge\steamemu.log`.

Resultados: superada en el PC del propietario (2026-10-10) y en la CI de Windows desde la run #14
(`2c20d4a`).

## Límites técnicos

- No es un clon de Goldberg: es una reimplementación propia de un subconjunto de Steamworks.
- El orden de las vtables sigue los encabezados públicos del SDK; los juegos con el SDK más
  reciente y funciones añadidas al final de interfaces no probadas pueden diferir.
- P2P es UDP sin cifrado ni fiabilidad garantizada (se ignora la marca "reliable").
- Nada compatible con EAC/VAC; usarlo con el juego abierto **sin EAC** (opción oficial).
- Los logros/stats son locales al proceso y al `storage_dir`: no sincronizan con Steam.
