# Integración Steam: estado, restricciones y protocolo de compatibilidad

## Estado real (0.7)

1. `SteamLibrary` localiza bibliotecas (registro + rutas estándar) y lee manifiestos con un parser KeyValues propio, tolerante y sin escritura.
2. `PreflightEvaluator` inspecciona carpetas/ejecutables con rutas confinadas a `steamapps/common` (incluidos enlaces simbólicos); el informe público elimina rutas personales.
3. `steam://rungameid/<appid>` abre **una** instancia normal a petición del usuario.
4. `GameWindowInspector` relaciona ventanas con procesos de la carpeta del juego y permite distribuir **2–4 clientes** solo si son procesos distintos del mismo exe, revalidando justo antes de mover.
5. **Adaptador experimental de 7 Days to Die** (pestaña 06): preflight, servidor dedicado oficial local, Steam del jugador 2 en Sandboxie-Plus, pantalla dividida sin bordes y diagnóstico de APIs de mando.

## Adaptador 7 Days to Die (AppID 251570)

Camino elegido (ADR-013):

| Paso | Qué hace SplitForge | Qué necesita el jugador |
| --- | --- | --- |
| Preflight | Juego y servidor instalados, mandos, clientes abiertos, Sandboxie-Plus; estado máximo `INSPECCIONADO` | — |
| Servidor | Lanza `7DaysToDieServer.exe` (AppID 294420, gratuito) con `-configfile` propio en `%LOCALAPPDATA%\SplitForge\7dtd`: plantilla del servidor + `ServerVisibility=0`, puerto, jugadores, `EACEnabled` según opción, telnet solo en 127.0.0.1, `UserDataFolder` aislado | Instalar «7 Days to Die Dedicated Server» desde Steam → Herramientas |
| Clientes | Abre, si se pide, un segundo Steam en el sandbox `SplitForge_J2` de Sandboxie-Plus | Unirse a `127.0.0.1:puerto` |
| EAC | Servidor sin EAC por defecto (los clientes deben coincidir) | Opción oficial de Steam: Propiedades → General → Preguntar al iniciar → «Launch game without EAC» |
| Pantalla | Distribuye las ventanas de los clientes (orden por PID, intercambiable), sin bordes, con ajuste configurable (rellenar o mantener proporción), y las restaura al detener | Juego en modo ventana a la resolución por panel que indica SplitForge |
| Apagado | `shutdown` por telnet local, espera hasta 45 s y solo entonces termina el proceso propio | — |
| Mandos | Identifica físicamente cada mando (luz/vibración). **Filtro XInput por proceso (experimental, ADR-015)**: se instala con los nombres que carga 7DTD (`xinput1_3`, `xinput1_4`) y actualiza solo copias antiguas propias. Además detecta la ranura XInput de cada mando con una pulsación, asigna J1/J2 y muestra las lecturas de cada cliente. Se retira sin tocar archivos del juego | Clientes sin EAC; cada mando en una sola ranura XInput; diagnósticos `7dtd` y `xinput` |

### Evidencia del PC del propietario (2026-10-10)

Primer `SplitForge_Reporte_7DTD.json` real (esquema v2), con **un** cliente abierto y sin servidor.

| Dato del informe | Valor | Qué significa |
| --- | --- | --- |
| Cliente | `7DaysToDie.exe`, 1 ventana, módulos legibles | **Hecho:** abierto sin EAC. Ningún módulo de EAC está cargado y Windows permitió leer los módulos |
| XInput | `xinput1_3.dll` y `xinput1_4.dll` | **Hecho:** el juego carga las dos, así que el filtro debe estar con ambos nombres (la instalación ya lo hace). **Pendiente:** saber por cuál lee los mandos; lo dirán las «lecturas» del estado de cada copia |
| DirectInput / HID | `dinput8.dll`, `hid.dll` | **Pendiente:** son otras vías posibles. Si el juego leyera los mandos por ellas, el filtro XInput no bastaría |
| InControl | `incontrolnative.dll` | **Hecho:** usa InControl, la biblioteca de mandos para Unity, con su módulo nativo |
| Steam | `steam_api64.dll`, `steamclient64.dll`, `gameoverlayrenderer64.dll` | **Hecho:** API de Steam y superposición cargadas |
| EOS | `eossdk-win64-shipping.dll` | SDK de Epic Online Services; no es el antitrampas |
| Otros | `inputhost.dll`, `textinputframework.dll`, `backtracecrashpadwindows.dll` | Entrada de texto de Windows e informes de fallos; no leen mandos |
| Mandos SDL3 | 2 × `Xbox 360 Controller (xbox360)` | **Supuesto:** los dos PS4 llegan a Windows como mandos XInput virtuales mediante una herramienta tipo DS4Windows, con los PS4 originales ocultos, porque SDL3 ya no los ve como PS4. Es justo lo que necesita el filtro |

Cambios hechos a partir de esta evidencia:

- **Estado por copia del filtro.** Cada copia escribe `estado-PID-<dll>.txt` con la ranura vigente y el número de lecturas, y se actualiza en vivo. Antes, las dos DLL escribían el mismo archivo y solo una vez, así que tras asignar seguía apareciendo «sin asignar».
- **DLL real comprobada.** Se carga por ruta absoluta y se verifica que viene de System32, nunca de otra copia del filtro ya cargada.
- **Instalación sin abortar.** Ya no se detiene a medias, actualiza solo copias propias sin modificar y guarda el manifiesto aunque falle una copia.
- **Detección de ranura.** «Detectar» averigua la ranura XInput de cada mando con una pulsación; desde consola, `SplitForge.Diagnostico.exe xinput`.
- **Informe v3.** Indica el origen de cada módulo de mando (`sistema`, `juego`, `steam`, `otro`) sin rutas, si el filtro de SplitForge está cargado, el motor y las ranuras XInput conectadas.

**Siguiente evidencia necesaria (W16):** dos clientes con el filtro asignado. Las lecturas de cada cliente deben subir y cada mando debe mover solo su ventana. Hay que anotar también si la ventana sin foco responde. Si las lecturas se quedan en 0 mientras el juego responde al mando, el juego lee por otra vía: guardar un informe v3 y decidir con él.

### Alternativas descartadas para 7DTD

- **Remote Play Together**: requiere multijugador local en PC; 7DTD solo tiene pantalla dividida en consola.

## Estados formales por juego

`DESCUBIERTO` → `INSPECCIONADO` → `DOS_CLIENTES_VALIDOS` → `SESION_COMUN_VALIDADA` → `INPUT_AISLADO_VALIDADO` → `RENDIMIENTO_VALIDADO` → `COMPATIBLE`.

En cualquier punto: `BLOQUEADO` con motivo verificable. **Nunca** inferir `COMPATIBLE` desde una carpeta, versión o ventanas. El código solo llega a `INSPECCIONADO`; los estados superiores se registran aquí con evidencia física.

## Matriz preliminar (no certificada)

| Juego | ID Steam | Estado | Restricción por validar |
| --- | --- | --- | --- |
| 7 Days to Die | 251570 | `INSPECCIONADO` · adaptador experimental · informe real con 1 cliente sin EAC (XInput 1_3 + 1_4) | Aislamiento de mandos con dos clientes; ventana sin foco; guardados |
| No Man's Sky | 275850 | Investigación posterior | Autenticación, sesiones multijugador, offline |
| State of Decay 2 | 495420 | Investigación posterior | Servicios online |
| GTA V Legacy | 271590 | Investigación posterior | Sin cooperativo en campaña, protecciones de GTA Online |

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
