# Módulos y contratos existentes

Todos los módulos actuales son Python; la arquitectura C++/C# discutida anteriormente es una **posible evolución**, no algo implementado.

| Archivo | Entrada / contrato | Salida | Riesgo al modificar |
| --- | --- | --- | --- |
| `launcher.py` | CLI sin argumentos / `--demo-player N` / `--shared-player N` | GUI o ventana de demostración | Arranque en Windows |
| `splitforge/gui.py` | eventos Tk, configuración local, lecturas ControllerManager | cambios visuales y llamadas a sesión | Bloqueos de UI, asignación de mandos |
| `splitforge/gamepads.py` | SDL Joystick/GameController + perfil por GUID/nombre | `PadInfo`, `PadState` | Duplicados, pérdida de eventos, P1/P2 |
| `splitforge/winmm_gamepads.py` | API WinMM como fallback | estados compatibles | Ejes/botones HID heterogéneos |
| `splitforge/session.py` | sesión, ID mandos, tamaño pantalla | procesos de demostración, streams, restauración | Cierre inseguro de procesos, staleness |
| `splitforge/shared_world.py` | `step(dt, {1:PadState,2:PadState})` | instantánea completa JSON-compatible | Determinismo, movimiento, estado mutante |
| `splitforge/shared_io.py` | instantáneas validadas `type:'world'` | stream de texto a visores | Mensajes perdidos o formatos cambiantes |
| `splitforge/shared_demo.py` | `stdin` con instantáneas | render de P1/P2 | GUI, tiempo y robustez sin estado |
| `splitforge/input_stream.py` | `PadState` serializado | input a demo independiente | Coexistencia con shared world |
| `splitforge/demo.py` | entrada por stdin | juego de prueba aislado | Pruebas anteriores, no comercial |
| `splitforge/layout.py` | dimensiones y orientación | 2 regiones `Region` | Monitores DPI, coordenadas |
| `splitforge/windows.py` | Win32 HWND, screen | ventanas visibles, snapshots y mover/restaurar | Restauración, fullscreen, permisos |
| `splitforge/steam_library.py` | registro Steam/VDF/ACF | `InstalledGame` | Formato VDF, rutas maliciosas |
| `splitforge/steam_preflight.py` | `InstalledGame` | `Preflight`, JSON público, URI de una instancia | Confundir detección con compatibilidad |
| `splitforge/steam_windows.py` | ventanas + imagen del PID | `GameWindowReport`; `verify_pair()` | PIDs reciclados, permisos, rutas |
| `setup_deps.py` | Python activo, wheels locales/PyPI | verificación SDL/pygame-ce | Rutas de instalación, red lenta |
| `steam_scan.py` | biblioteca local | informe Steam | Privacidad |
| `steam_windows_scan.py` | Windows/Steam | informe de ventanas | Privacidad y falsos positivos |

## Tipos públicos importantes

```python
@dataclass(frozen=True)
class PadInfo:
    instance_id: int
    name: str
    guid: str
    kind: str
    axes: int
    buttons: int

@dataclass(frozen=True)
class PadState:
    instance_id: int
    lx: float = 0.0
    ly: float = 0.0
    a_pressed: bool = False
    connected: bool = True
    dash_pressed: bool = False
    shield_pressed: bool = False
```

- `SessionManager.start_shared_demo(script, assigned_ids, orientation, gutter)` necesita dos mandos distintos, no `None`.
- `SessionManager.route_shared_inputs(states, dt)` elige por ID, crea la instantánea y la emite a dos visores.
- `ControllerManager.refresh()` vuelve a enumerar SDL; `poll()` devuelve estados por `instance_id`.
- `inspect_game_windows(game)` solo compara pertenencia a carpeta y EXE de PID visible. Requiere 2 PID distintos ejecutando el mismo EXE para elegir un par.
- `verify_pair()` vuelve a inspeccionar antes de mover las ventanas. **No certifica la misma partida.**
- `SessionManager.stop()` finaliza únicamente *demos propias*; para ventanas/juegos externos restaura sin matar procesos.

## Reglas para cambios en interfaces

- Cambios de `PadState`: actualizar serialización en `input_stream.py`, lógica de `shared_world.py`, controles del GUI, tests y docs.
- Cambios de instantánea del mundo: actualizar productor, validación `shared_io.py`, consumidor `shared_demo.py`, tests.
- Cambios en el ciclo de sesiones: cubrir cierre normal, error a medio iniciar, ventana desaparecida, IDs de mandos cambiantes.
- Cambios en Steam: mantener evaluación *read-only*, informes sin secretos y puerta de compatibilidad conservadora.
