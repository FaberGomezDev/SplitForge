# Integración Steam: estado, restricciones y protocolo de compatibilidad

## Estado real (v0.6)

1. `steam_library.py` localiza instalaciones Steam y lee manifiestos instalados.
2. `steam_preflight.py` inspecciona carpetas/ejecutables de manera conservadora, crea un informe que elimina rutas personales e incluye candidatos de investigación.
3. `steam://rungameid/<appid>` puede solicitar apertura **normal de un juego**, una sola instancia.
4. `steam_windows.py` relaciona ventanas Win32 con procesos dentro del directorio del juego, y **permite reorganizar dos ventanas solo si pertenecen a dos PID del mismo exe**.

**No existe todavía** un adaptador capaz de iniciar dos clientes legítimos, conectar ambos a una partida o hacer entrada aislada de terceros. El apartado **05 Steam** y **06 Ventanas Steam** son herramientas diagnósticas.

## Estados formales propuestos por juego

`DESCUBIERTO` → `INSPECCIONADO` → `DOS_CLIENTES_VALIDOS` → `SESION_COMUN_VALIDADA` → `INPUT_AISLADO_VALIDADO` → `RENDIMIENTO_VALIDADO` → `COMPATIBLE`.

En cualquier punto se puede pasar a `BLOQUEADO` (con motivo verificable: licencia, sin LAN, antitrampas, etc.). **Nunca** inferir `COMPATIBLE` desde detectar una carpeta, versión o dos ventanas.

## Matriz preliminar (no certificada)

| Juego | ID Steam | Estado | Restricción por validar |
| --- | --- | --- | --- |
| 7 Days to Die | 251570 | Candidato principal de investigación | Clientes autorizados simultáneos, LAN/offline, integridad de guardados, antitrampas |
| No Man's Sky | 275850 | Investigación posterior | Dependencias de autenticación, sesiones multijugador, disponibilidad offline |
| State of Decay 2 | 495420 | Investigación posterior | Cuentas Xbox, servicios online, cliente concurrente |
| GTA V Legacy | 271590 | Investigación posterior | Cooperativo inexistente en campaña nativa, Rockstar, protecciones de GTA Online |

Estas descripciones no demuestran ni garantizan compatibilidad en versiones actuales. Registrar la versión realmente instalada y la fecha de prueba antes de declararla.

## Pruebas mínimas para aprobar un adaptador

- **Instalación:** AppID, build ID, ruta segura dentro de biblioteca, ejecutable legítimo comprobado.
- **Lanzamiento:** dos clientes autorizados y funcionales; no duplicar identidad ni forzar sesiones prohibidas.
- **Partida:** se ven en el mismo mundo y pueden interactuar; LAN sin Internet demostrado **si se quiere etiquetar como offline**.
- **Mandos:** cada mando afecta exclusivamente a su cliente; sin fuga de controles.
- **Pantallas:** 2 ventanas en dos regiones, DPI/monitor, limpieza al cerrar.
- **Persistencia:** no hay corrupción de datos ni escritura concurrente peligrosa.
- **Recursos:** benchmark con P50/P95/P99 frametime, 1% low, RAM, VRAM, carga CPU, mediciones repetidas.
- **Resiliencia:** cierre abrupto, reconexión mando, juego no responde, restauración; revisar logs sin secretos.
- **Compatibilidad por versión:** evidencia para build exacta; marcar como no verificado al cambiar build o backend.

## Contrato propuesto del adaptador (NO implementado)

```python
@dataclass(frozen=True)
class AdapterManifest:
    app_id: str
    supported_builds: tuple[str, ...]
    verified_modes: tuple[str, ...]  # p.ej. 'lan-online', 'lan-offline'
    min_players: int
    max_players: int
    requires_separate_accounts: bool
    needs_anti_cheat_disabled_by_official_option: bool
    notes: str

class GameAdapter(Protocol):
    def preflight(self, installation, session_request) -> Result: ...
    def prepare(self, session_request) -> PreparedSession: ...
    def start(self, prepared) -> RunningSession: ...
    def health(self, running) -> HealthReport: ...
    def stop(self, running) -> None: ...
    def restore(self, prepared) -> None: ...
```

No implementar `needs_anti_cheat_disabled_by_official_option` como bypass. Solo refleja si una opción **oficialmente ofrecida por el juego** es requerida; sin opción autorizada, bloquear integración.

### Informes requeridos

- `Escanear_Steam.bat` → `SplitForge_Reporte_Steam.json`.
- `Analizar_Ventanas_Steam.bat` → `SplitForge_Reporte_Ventanas_Steam.json` (enfocado en 7DTD).
- No exponer nombres de ventana, rutas absolutas, identificadores de cuenta ni GUID persistentes de hardware.

Ver plantilla [`templates/ADAPTADOR.md`](templates/ADAPTADOR.md) y plan [`plans/7-days-to-die-original.md`](plans/7-days-to-die-original.md).
