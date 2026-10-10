# Integración Steam: estado, restricciones y protocolo de compatibilidad

## Estado real (0.7)

1. `SteamLibrary` localiza bibliotecas (registro + rutas estándar) y lee manifiestos con un parser KeyValues propio, tolerante y sin escritura.
2. `PreflightEvaluator` inspecciona carpetas/ejecutables con rutas confinadas a `steamapps/common` (incluidos enlaces simbólicos); el informe público elimina rutas personales.
3. `steam://rungameid/<appid>` abre **una** instancia normal a petición del usuario.
4. `GameWindowInspector` relaciona ventanas con procesos de la carpeta del juego y permite distribuir **2–4 clientes** solo si son procesos distintos del mismo exe, revalidando justo antes de mover.
5. **Adaptador experimental de 7 Days to Die** (pestaña 06): preflight, servidor dedicado oficial local, Steam del jugador 2 en Sandboxie-Plus, pantalla dividida sin bordes y diagnóstico de APIs de mando.

## Adaptador 7 Days to Die (AppID 251570)

Camino **legítimo** elegido (ADR-013):

| Paso | Qué hace SplitForge | Qué necesita el jugador |
| --- | --- | --- |
| Preflight | Juego y servidor instalados, mandos, clientes abiertos, Sandboxie-Plus; estado máximo `INSPECCIONADO` | — |
| Servidor | Lanza `7DaysToDieServer.exe` (AppID 294420, gratuito) con `-configfile` propio en `%LOCALAPPDATA%\SplitForge\7dtd`: plantilla del servidor + `ServerVisibility=0`, puerto, jugadores, `EACEnabled` según opción, telnet solo en 127.0.0.1, `UserDataFolder` aislado | Instalar «7 Days to Die Dedicated Server» desde Steam → Herramientas |
| Clientes | Abre, si se pide, un segundo Steam en el sandbox `SplitForge_J2` de Sandboxie-Plus | **Una cuenta de Steam con el juego por jugador**; unirse a `127.0.0.1:puerto` |
| EAC | Servidor sin EAC por defecto (los clientes deben coincidir) | Opción oficial de Steam: Propiedades → General → Preguntar al iniciar → «Launch game without EAC» |
| Pantalla | Distribuye las ventanas de los clientes (orden por PID, intercambiable), sin bordes, y las restaura al detener | Juego en modo ventana / ventana sin bordes |
| Apagado | `shutdown` por telnet local, espera hasta 45 s y solo entonces termina el proceso propio | — |
| Mandos | Identifica físicamente cada mando (luz/vibración). **Filtro XInput por proceso (experimental, ADR-015)**: instalar, asignar J1/J2 a ranuras y retirar, sin sobrescribir archivos del juego | Clientes sin EAC; PS4 como XInput (Steam Input o DS4Windows); diagnóstico `7dtd` para confirmar que carga XInput |

### Alternativas descartadas para 7DTD

- **Modo sin conexión con una sola cuenta**: misma identidad dos veces; la partida vería al mismo jugador dos veces.
- **Remote Play Together**: requiere multijugador local en PC; 7DTD solo tiene pantalla dividida en consola.

## Estados formales por juego

`DESCUBIERTO` → `INSPECCIONADO` → `DOS_CLIENTES_VALIDOS` → `SESION_COMUN_VALIDADA` → `INPUT_AISLADO_VALIDADO` → `RENDIMIENTO_VALIDADO` → `COMPATIBLE`.

En cualquier punto: `BLOQUEADO` con motivo verificable. **Nunca** inferir `COMPATIBLE` desde una carpeta, versión o ventanas. El código solo llega a `INSPECCIONADO`; los estados superiores se registran aquí con evidencia física.

## Matriz preliminar (no certificada)

| Juego | ID Steam | Estado | Restricción por validar |
| --- | --- | --- | --- |
| 7 Days to Die | 251570 | `INSPECCIONADO` · adaptador experimental | Segunda cuenta con el juego; aislamiento de mandos; ventana sin foco; guardados |
| No Man's Sky | 275850 | Investigación posterior | Autenticación, sesiones multijugador, offline |
| State of Decay 2 | 495420 | Investigación posterior | Cuentas Xbox, servicios online |
| GTA V Legacy | 271590 | Investigación posterior | Sin cooperativo en campaña, protecciones de GTA Online |

Registrar la versión instalada y la fecha de prueba antes de cambiar un estado.

## Pruebas mínimas para aprobar un adaptador

- **Instalación:** AppID, build ID, ruta dentro de la biblioteca, ejecutable comprobado.
- **Lanzamiento:** dos clientes, una cuenta cada uno.
- **Partida:** se ven en el mismo mundo e interactúan; LAN sin Internet demostrada **solo** si se quiere etiquetar como offline.
- **Mandos:** cada mando afecta exclusivamente a su cliente.
- **Pantallas:** regiones correctas, DPI/monitor, restauración al cerrar.
- **Persistencia:** sin corrupción de guardados (el servidor usa su carpeta propia).
- **Recursos:** P50/P95/P99 de frametime, 1 % low, RAM, VRAM, CPU, repetido.
- **Resiliencia:** cierre abrupto, reconexión de mando, juego que no responde, restauración.

## Contrato del adaptador (implementado, mínimo)

```csharp
public sealed record AdapterManifest(string AppId, string Title, IReadOnlyList<string> SupportedBuilds,
    IReadOnlyList<string> VerifiedModes, int MinPlayers, int MaxPlayers, bool RequiresSeparateAccounts,
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
- `SplitForge.Diagnostico.exe 7dtd` → `SplitForge_Reporte_7DTD.json` (APIs de mando por cliente).
- Sin títulos de ventana, rutas absolutas, cuentas ni identificadores únicos de hardware.

Ver plantilla [`templates/ADAPTADOR.md`](templates/ADAPTADOR.md) y plan [`plans/7-days-to-die-original.md`](plans/7-days-to-die-original.md).
