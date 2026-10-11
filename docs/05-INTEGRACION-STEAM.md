# Integración Steam: estado, restricciones y protocolo de compatibilidad

## Estado real (0.7)

1. `SteamLibrary` localiza bibliotecas (registro + rutas estándar) y lee manifiestos con un parser KeyValues propio, tolerante y sin escritura.
2. `PreflightEvaluator` inspecciona carpetas/ejecutables con rutas confinadas a `steamapps/common` (incluidos enlaces simbólicos); el informe público elimina rutas personales.
3. `steam://rungameid/<appid>` abre **una** instancia normal a petición del usuario.
4. `GameWindowInspector` relaciona ventanas con procesos de la carpeta del juego y permite distribuir **2–4 clientes** solo si son procesos distintos del mismo exe, revalidando justo antes de mover.
5. **Adaptador experimental de 7 Days to Die** (pestaña 06): preflight, servidor dedicado oficial local, Steam del jugador 2 en Sandboxie-Plus, pantalla dividida sin bordes y diagnóstico de APIs de mando.
6. **Emulador de Steam, componente principal** (pestaña 07, ADR-016): sustituye `steam_api64.dll` de 7DTD con copia de seguridad y da a J1 y J2 identidades distintas sin un segundo Steam. «Lanzar J1 y J2» abre los dos clientes con la configuración del handler de Nucleus Co-op, asigna a cada uno su mando (filtro XInput) y divide la pantalla. **Comprobado el 2026-10-10: dos clientes de 7DTD en el menú principal a la vez** (ver más abajo).

## Adaptador 7 Days to Die (AppID 251570)

Camino elegido (ADR-013):

| Paso | Qué hace SplitForge | Qué necesita el jugador |
| --- | --- | --- |
| Preflight | Juego y servidor instalados, mandos, clientes abiertos, Sandboxie-Plus; estado máximo `INSPECCIONADO` | — |
| Servidor | Lanza `7DaysToDieServer.exe` (AppID 294420, gratuito) con `-configfile` propio en `%LOCALAPPDATA%\SplitForge\7dtd`: plantilla del servidor + `ServerVisibility=0`, puerto, jugadores, `EACEnabled` según opción, telnet solo en 127.0.0.1, `UserDataFolder` aislado | Instalar «7 Days to Die Dedicated Server» desde Steam → Herramientas |
| Clientes | Abre, si se pide, un segundo Steam en el sandbox `SplitForge_J2` de Sandboxie-Plus, o lanza J1 y J2 con identidades emuladas (pestaña 07, sin juego cruzado) | Unirse a `127.0.0.1:puerto` (servidor oficial) o a la partida de J1 en 127.0.0.1 |
| EAC | Servidor sin EAC por defecto (los clientes deben coincidir) | Opción oficial de Steam: Propiedades → General → Preguntar al iniciar → «Launch game without EAC» |
| Pantalla | Distribuye las ventanas de los clientes (orden por PID, intercambiable), sin bordes, con ajuste configurable (rellenar o mantener proporción), y las restaura al detener | Juego en modo ventana a la resolución por panel que indica SplitForge |
| Apagado | `shutdown` por telnet local, espera hasta 45 s y solo entonces termina el proceso propio | — |
| Mandos | Identifica físicamente cada mando (luz/vibración). **Filtro XInput por proceso (experimental, ADR-015)**: se instala con los nombres que carga 7DTD (`xinput1_3`, `xinput1_4`) y actualiza solo copias antiguas propias. Además detecta la ranura XInput de cada mando con una pulsación, asigna J1/J2 y muestra las lecturas de cada cliente. Se retira sin tocar archivos del juego | Clientes sin EAC; cada mando en una sola ranura XInput; diagnósticos `7dtd` y `xinput` |

### Evidencia del PC del propietario (2026-10-10)

Primer `SplitForge_Reporte_7DTD.json` real (esquema v2), con **un** cliente abierto y sin servidor.

| Dato del informe | Valor | Qué significa |
| --- | --- | --- |
| Cliente | `7DaysToDie.exe`, 1 ventana, módulos legibles | **Hecho:** abierto sin EAC. Ningún módulo de EAC está cargado y Windows permitió leer los módulos |
| XInput | `xinput1_3.dll` y `xinput1_4.dll` | **Hecho (2026-10-10, desensamblado y `Player.log`):** Unity pide `xinput1_3.dll` por nombre y recibe la copia del filtro. InControlNative, que es quien mueve al jugador, pide `XInput1_4.dll` y recibía la real de System32, ya cargada. El filtro intercepta esa DLL real (ADR-021) |
| DirectInput / HID | `dinput8.dll`, `hid.dll` | Las carga InControlNative. **Hecho:** con los dos mandos conectados, InControl solo lista mandos de tipo XInput (ninguno por DirectInput o HID) |
| InControl | `incontrolnative.dll` | **Hecho:** usa InControl, la biblioteca de mandos para Unity, con su módulo nativo |
| Steam | `steam_api64.dll`, `steamclient64.dll`, `gameoverlayrenderer64.dll` | **Hecho:** API de Steam y superposición cargadas |
| EOS | `eossdk-win64-shipping.dll` | SDK de Epic Online Services; no es el antitrampas |
| Otros | `inputhost.dll`, `textinputframework.dll`, `backtracecrashpadwindows.dll` | Entrada de texto de Windows e informes de fallos; no leen mandos |
| Mandos SDL3 | 2 × `Xbox 360 Controller (xbox360)` | **Hecho (2026-10-10):** ViGEmBus y HidHide están instalados en el PC. **Supuesto:** los dos PS4 llegan a Windows como mandos Xbox 360 virtuales (ViGEmBus) con los originales ocultos (HidHide), porque SDL3 ya no los ve como PS4. No se ha identificado qué programa los crea (DS4Windows no figura como instalado; puede ser portable). Es justo lo que necesita el filtro |

Cambios hechos a partir de esta evidencia:

- **Estado por copia del filtro.** Cada copia escribe `estado-PID-<dll>.txt` con la ranura vigente y el número de lecturas, y se actualiza en vivo. Antes, las dos DLL escribían el mismo archivo y solo una vez, así que tras asignar seguía apareciendo «sin asignar».
- **DLL real comprobada.** Se carga por ruta absoluta y se verifica que viene de System32, nunca de otra copia del filtro ya cargada.
- **Instalación sin abortar.** Ya no se detiene a medias, actualiza solo copias propias sin modificar y guarda el manifiesto aunque falle una copia.
- **Detección de ranura.** «Detectar» averigua la ranura XInput de cada mando con una pulsación; desde consola, `SplitForge.Diagnostico.exe xinput`.
- **Informe v3.** Indica el origen de cada módulo de mando (`sistema`, `juego`, `steam`, `otro`) sin rutas, si el filtro de SplitForge está cargado, el motor y las ranuras XInput conectadas.

### Estado de la instalación (2026-10-10, diagnóstico `steam` y registro, solo lectura)

| Componente | Estado |
| --- | --- |
| 7 Days to Die (251570) | Instalado, build `25661859`, actualizado el 2026-10-10; `7DaysToDie.exe` y `7DaysToDie_EAC.exe` presentes |
| 7 Days to Die Dedicated Server (294420) | **No instalado** (ninguna de las 2 bibliotecas tiene su manifiesto). Necesario para W11, W12 y W14 |
| Sandboxie-Plus | **No instalado**. Para el segundo cliente queda el emulador de Steam (pestaña 07) o instalarlo |
| ViGEmBus / HidHide | Instalados |
| Filtro XInput en la carpeta del juego | **Instalado** (2026-10-10, tarde): `xinput1_3.dll`, `xinput1_4.dll`, `xinput9_1_0.dll` y su manifiesto; se retira con «Quitar filtro» |
| Emulador de Steam en la carpeta del juego | **Instalado y actualizado** a la versión con API plana generada (2026-10-10, tarde): `7DaysToDie_Data\Plugins\x86_64\steam_api64.dll`; la original sigue respaldada como `steam_api64_splitforge_original.dll` |

### Emulador de Steam con 7DTD: primera prueba real (2026-10-10)

Evidencia: `splitforge-net.log`, `steamemu.log` y los registros de Unity (`Player.log` y `Player-prev.log`) del PC del propietario.

| Dato | Valor | Qué significa |
| --- | --- | --- |
| Intentos | Entre las 02:50 y las 03:55: 5 instalaciones, 4 restauraciones, 12 lanzamientos dobles y 2 sueltos desde la pestaña 07 | **Hecho:** la vía de la pestaña 07 se usó de verdad |
| Arranque | `steamemu.log`: cada instancia llama a `SteamAPI_Init` y el emulador carga su identidad (red multicast OK). Los 10 primeros arranques usaron AppID 480 y el mismo `steamid` para J1 y J2. Desde las 03:05, 16 arranques usaron AppID 251570 con `SplitForgeJ1` / `SplitForgeJ2` y `steamid` distintos | **Hecho:** 7DTD carga la DLL emulada, y J1 y J2 reciben identidades distintas |
| Versión | 7DTD `V 3.3.0 (b18)`, Unity 2022.3.62f2 | — |
| Resultado | Quedan dos registros de Unity, de los lanzamientos de las 03:55:02 y las 03:55:26. Los dos terminan en `Crash!!!` justo después de `[Platform] Initializing Steam`. Ningún proceso de 7DTD llega a `SteamAPI_Shutdown` | **Hecho:** el emulador, tal como está, **no permite arrancar 7DTD** |
| Traza | `steam_api64` ← `SteamAPI_ISteamClient_GetISteamUser` ← `Steamworks.CSteamAPIContext:Init` ← `Platform.Steam.Api:InitClientApis` | **Hecho:** el fallo ocurre dentro de la DLL emulada, en la primera llamada de Steamworks.NET a la API plana |
| Causa (código) | En el SDK, las funciones planas reciben primero el puntero `self` (`SteamAPI_ISteamClient_GetISteamUser(ISteamClient* self, HSteamUser, HSteamPipe, const char*)`), y la traza muestra que Steamworks.NET la llama con 4 argumentos. El emulador la declara sin `self` (`flat_api.cpp`): los argumentos se desplazan y la versión llega como un número tratado como puntero | **Hecho comprobado en el código.** Afecta a las 32 funciones `SteamAPI_ISteamClient_*` exportadas |
| Lo que pide 7DTD | `com.rlabrecque.steamworks.net.dll` referencia 931 entradas `SteamAPI_*`; 866 son métodos planos `SteamAPI_ISteam*` (Friends 80, UGC 89, RemoteStorage 59, UserStats 45, Utils 36…). Versiones: `SteamClient020`, `SteamUser023` | **Hecho:** Steamworks.NET usa la API plana para todo. El emulador solo exporta 32 entradas `SteamAPI_ISteamClient_*` y ninguna del resto de interfaces. Las versiones de interfaz sí coinciden |

### Emulador corregido y lanzador con la configuración de Nucleus: dos clientes (2026-10-10, tarde)

Corrección del emulador (ver ADR-016):
- API plana **generada** desde sus vtables (`tools/generar_api_plana.py`): 683 reenvíos con `self` y 234 entradas neutras, que cubren las 944 entradas de Steamworks.NET.
- Corregidos también `ContextInit`, 7 firmas de vtable, estructuras `pack(1)`, el despacho de call results (703) y las respuestas de tickets.

Resultado con 7DTD real, paso a paso:

| Paso | Evidencia | Qué significa |
| --- | --- | --- |
| Arranque | `[Steamworks.NET] SteamAPI_Init() ok` y `Login ok`; estadísticas y logros recibidos | **Hecho:** el emulador ya no tumba el juego |
| EOS | Con juego cruzado, EOS envía el ticket local a Epic: `Could not decrypt Steam ticket` → `[EOS] Login failed: ConnectExternalTokenValidationFailed`. El juego muestra «Inicio de sesión de servicios en línea de Epic» con «Jugar sin conexión» **desactivado** | **Hecho:** en el IL de `XUiC_LoginBase.updateState`, ese botón solo se activa si la plataforma está en `EUserStatus.OfflineMode` (1) |
| Referencia Nucleus | Su handler de 7DTD (v18) deja `crossplatform=` vacío y `serverplatforms=Steam` en `platform.cfg` de su copia del juego y lanza con `-screen-fullscreen 0 -popupwindow -noeac` y el tamaño de cada ventana | La técnica para evitar EOS |
| Sin tocar archivos | 7DTD admite `-crossplatform=` y `-serverplatforms=` como argumentos (`PlatformManager.GetCommandLineOverrides`, formato `-clave=valor`, valores vacíos ignorados). SplitForge lanza con `-crossplatform=None -serverplatforms=Steam` | **Hecho:** sin EOS, sin diálogo, **menú principal** |
| Dos clientes | J1 y J2 a la vez desde la misma carpeta; en pantalla «PERFIL: SPLITFORGEJ1» y «PERFIL: SPLITFORGEJ2» | **Hecho:** `DOS_CLIENTES_VALIDOS` (dos clientes simultáneos, identidades distintas) |
| Ventana | 960×1032 (lado a lado sin ajuste) corta el menú por la izquierda; 960×540 (16:9) y 1920×516 (arriba/abajo) lo muestran completo | **Hecho:** 7DTD necesita paneles de 16:9 o más anchos (`SevenDaysToDieAdapter.MinAspect`); el propietario prefiere arriba/abajo |
| Mandos | Filtro cargado en los dos clientes por `xinput1_3.dll`, DLL real cargada, J1 → mando 1, J2 → mando 2; las lecturas suben en ambos (~60 por segundo cada uno) aunque solo uno tenga el foco | **Hecho:** el juego consulta su mando también sin foco. Esas lecturas eran las de Unity; InControlNative leía aparte, sin filtrar (corregido, ver «Causa y corrección»). **Pendiente (W16):** que el propietario confirme que cada mando mueve solo su ventana |

**Foco (2026-10-10, noche):** el propietario confirmó que los mandos **no** eran independientes: solo atendía la ventana activa. El IL del juego lo explica: `GameManager.OnApplicationFocus` → `GameIsFocused`. El handler de Nucleus usa `ForceFocus`, `FocusHooks` y `SetForegroundWindowElsewhere`. SplitForge añade al filtro el foco simulado (ADR-015). Con «Lanzar J1 y J2» real: J1 → mando 1 y J2 → mando 2, los dos con «foco simulado» en su estado. El puntero de 7DTD es propio de cada instancia (`SoftCursor`), así que no hace falta puntero virtual.

**W16, resultado (propietario, 2026-10-10, noche):** con el foco simulado, **cada stick mueve las dos ventanas**. El diagnóstico v3 con los dos clientes lo explica: ambos tienen cargada la `xinput1_4.dll` **real** de System32 (origen «sistema») y el filtro solo está como `xinput1_3`. El juego (InControl pide `XInput1_3`/`XInput1_4`/`XInput9_1_0` por nombre) lee por la 1.4 sin filtrar. Siguiente: que la vía `xinput1_4` también pase por el filtro (Nucleus: `Game.Hook.XInputEnabled`).

**Causa y corrección (2026-10-10, noche; ADR-021):** la 1.4 real la cargaba el propio filtro. Su DLL real era la `XInput1_3.dll` de System32, que en este Windows reenvía a `xinput1_4.dll` cargándola en ejecución (no la importa: por eso se había descartado). InControlNative hace `LoadLibraryW("XInput1_4.dll")` y Windows le daba esa 1.4 ya cargada. Ahora el filtro usa siempre la 1.4 como DLL real y redirige sus exportaciones hacia sí mismo.

| Comprobación con dos clientes reales y los dos mandos conectados | Antes | Ahora |
| --- | --- | --- |
| Mandos que lista InControl en cada cliente (`Player.log`) | 2 (`XInput::1`, `XInput::2`) | **1** (`XInput::1`) |
| Estado del filtro | solo lecturas por `xinput1_3` | `intercepcion=activa`, `previa=no`, 702 y 603 lecturas por la DLL real |
| Orden de carga | — | `xinput1_3.dll` (juego) → `xinput1_4.dll` (System32) → `InControlNative.dll` |

**W16 superado (propietario, 2026-10-10, noche):** con «Lanzar J1 y J2» del paquete nuevo, el propietario confirma con los mandos que cada uno controla solo su ventana.

**Siguiente evidencia necesaria:**
- ~~**W14:** J2 se une a la partida de J1 en 127.0.0.1.~~ → superado, ver abajo.

### Misma partida: J2 entra sola en la de J1 (W14, 2026-10-10, noche)

**Primer intento del propietario (a mano, «Conectar a IP»):** la conexión llegó al anfitrión y pasó todos los autorizadores, pero el jugador se quedó en «autenticando». Causas en el emulador (corregidas, ADR-022): no respondía al ticket por el canal del servidor de juego, no entregaba `LobbyCreated_t` como callback y daba la IP pública al revés (`1.0.0.127`). Dos de los tres intentos fueron, además, demasiado pronto: «Server is still initializing».

**Cómo se encuentran ahora:** el emulador de J1 anuncia su partida y su lobby a las demás instancias; el de J2 le entrega al juego la misma petición que Steam al pulsar «Unirse a la partida» de un amigo, y 7DTD se conecta a `127.0.0.1:26900`. Opcionalmente J1 arranca con `-quick-continue` y carga su última partida.

| Paso (registros reales, lanzado con «Lanzar J1 y J2» y las dos casillas) | Momento |
| --- | --- |
| J1: «QuickContinue mode, loading last savegame» | 31 s |
| J1: servidor de juego y lobby creados («Lobby creation succeeded», puerto 26900) | 68 s |
| J2: `Friends_GameServerChangeRequested` → «Connecting to server 127.0.0.1:26900» | 70–76 s |
| J1: «Authentication callback … result: k_EAuthSessionResponseOK» | 77 s |
| J1 y J2: «Player 'SplitForgeJ2' joined the game» | 107 s |

**Sin comprobar:** una sesión de juego real de los dos; que la pantalla «Unirse a partida» de J2 liste la de J1 (el lobby es visible y unible desde el emulador, pero la lista de servidores sigue vacía). **Riesgo:** J1 y J2 comparten guardados; en esta prueba J2 tuvo abierta 45 s la misma partida que alojaba J1 (alguien pulsó «Continuar» en J2 antes de la unión).

### Alternativas descartadas para 7DTD

- **Remote Play Together**: requiere multijugador local en PC; 7DTD solo tiene pantalla dividida en consola.

## Estados formales por juego

`DESCUBIERTO` → `INSPECCIONADO` → `DOS_CLIENTES_VALIDOS` → `SESION_COMUN_VALIDADA` → `INPUT_AISLADO_VALIDADO` → `RENDIMIENTO_VALIDADO` → `COMPATIBLE`.

En cualquier punto: `BLOQUEADO` con motivo verificable. **Nunca** inferir `COMPATIBLE` desde una carpeta, versión o ventanas. El código solo llega a `INSPECCIONADO`; los estados superiores se registran aquí con evidencia física.

## Matriz preliminar (no certificada)

| Juego | ID Steam | Estado | Restricción por validar |
| --- | --- | --- | --- |
| 7 Days to Die | 251570 | `SESION_COMUN_VALIDADA` e `INPUT_AISLADO_VALIDADO` (2026-10-10, build `25661859`, V 3.3.0 b18): dos clientes con el emulador; cada mando controla solo su ventana (W16, ADR-021) y J2 entra sola en la partida de J1 (W14, ADR-022) | Una sesión de juego real de los dos; aislar los guardados de J2; lista de servidores |
| No Man's Sky | 275850 | Investigación posterior · instalado (build `25732212`) | Autenticación, sesiones multijugador, offline |
| State of Decay 2 | 495420 | Investigación posterior · no instalado en el PC de pruebas | Servicios online |
| GTA V Legacy | 271590 | Investigación posterior · no instalado (sí GTA V Enhanced, 3240220) | Sin cooperativo en campaña, protecciones de GTA Online |

Registrar la versión instalada y la fecha de prueba antes de cambiar un estado.

## Pruebas mínimas para aprobar un adaptador

- **Instalación:** AppID, build ID, ruta dentro de la biblioteca, ejecutable comprobado.
- **Lanzamiento:** dos clientes a la vez.
- **Partida:** se ven en el mismo mundo e interactúan; LAN sin Internet demostrada **solo** si se quiere etiquetar como offline.
- **Mandos:** cada mando afecta exclusivamente a su cliente.
- **Pantallas:** regiones correctas, DPI/monitor, restauración al cerrar.
- **Persistencia:** sin corrupción de guardados (el servidor usa su carpeta propia).
- **Recursos:** P50/P95/P99 de frametime, 1 % low, RAM, VRAM, CPU, repetido.
- **Resiliencia:** cierre abrupto, reconexión de mando, juego que no responde, restauración.

## Contrato del adaptador (implementado, mínimo)

```csharp
public sealed record AdapterManifest(string AppId, string Title, IReadOnlyList<string> SupportedBuilds,
    IReadOnlyList<string> VerifiedModes, int MinPlayers, int MaxPlayers,
    bool NeedsAntiCheatDisabledByOfficialOption, string Notes);

public interface IGameAdapter
{
    AdapterManifest Manifest { get; }
    AdapterPreflight Preflight(AdapterEnvironment environment); // solo lectura
}
```

`NeedsAntiCheatDisabledByOfficialOption` solo refleja una opción **oficial** del juego; sin ella, la integración se bloquea.

### Informes

- `SplitForge.Diagnostico.exe steam` → `SplitForge_Reporte_Steam.json` (mismo esquema que el prototipo).
- `SplitForge.Diagnostico.exe ventanas` → `SplitForge_Reporte_Ventanas_Steam.json`.
- `SplitForge.Diagnostico.exe 7dtd` → `SplitForge_Reporte_7DTD.json` (esquema v3: APIs de mando por cliente, origen de cada módulo de mando, filtro de SplitForge cargado, motor y ranuras XInput conectadas).
- `SplitForge.Diagnostico.exe xinput` → ranuras XInput con mando, pulsaciones en vivo y estado del filtro en cada cliente (sin archivo).
- Sin títulos de ventana, rutas absolutas, cuentas ni identificadores únicos de hardware.

Ver plantilla [`templates/ADAPTADOR.md`](templates/ADAPTADOR.md) y plan [`plans/7-days-to-die-original.md`](plans/7-days-to-die-original.md).
