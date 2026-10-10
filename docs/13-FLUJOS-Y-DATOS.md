# Flujos de ejecución, IPC y persistencia

## Iniciar Mundo Compartido

```mermaid
sequenceDiagram
    participant U as Usuario
    participant G as GUI
    participant P as ControllerManager
    participant S as SessionManager
    participant W as SharedWorld
    participant V1 as Visor J1
    participant V2 as Visor J2
    U->>G: Selecciona dos IDs distintos + Iniciar
    G->>S: start_shared_demo(ids, orientation)
    S->>V1: Popen launcher --shared-player 1
    S->>V2: Popen launcher --shared-player 2
    loop Cada frame UI
        G->>P: poll()
        P-->>G: states[instance_id]
        G->>S: route_shared_inputs(states, dt)
        S->>W: step(dt, J1/J2)
        W-->>S: snapshot()
        S->>V1: WorldWriter(stdin, JSON)
        S->>V2: WorldWriter(stdin, JSON)
    end
    U->>G: Detener
    G->>S: stop()
    S->>V1: Cierre normal ventana prueba
    S->>V2: Cierre normal ventana prueba
    S-->>G: Restaura coordenadas guardadas
```

## Instantánea actual (ejemplo ilustrativo)

```json
{
  "type": "world",
  "frame": 120,
  "actors": {
    "1": {"x": 0.25, "y": 0.5, "score": 1, "hits": 0, "shield": false, "connected": true},
    "2": {"x": 0.75, "y": 0.5, "score": 0, "hits": 0, "shield": true, "connected": true}
  },
  "shots": [],
  "orb": [0.5, 0.5],
  "event": "Ambos jugadores en la misma arena"
}
```

La instantánea se envía en una línea JSON con `WorldWriter` y se valida parcialmente en `validate_world()`. La cola de salida tiene máximo **un mensaje pendiente** para preferir el último frame si el visor no consume a tiempo. Este esquema no tiene aún identificador de protocolo ni garantía de fiabilidad/replay: no usarlo sin revisión para partidas comerciales.

## Diagrama Steam (solo diagnóstico)

```mermaid
flowchart LR
    M[appmanifest Steam] --> LIB[InstalledGame]
    LIB --> PF[Preflight read-only]
    LIB --> G[Inspect Win32 HWND/PID/exe]
    G --> V{Dos PID del mismo exe?}
    V -- No --> BLOCK[No habilitar pareja]
    V -- Sí --> ARR[Permitir reubicar 2 ventanas]
    ARR --> REST[Restaurar posiciones]
```

## Persistencia

- `%LOCALAPPDATA%\SplitForge\settings.json`: asignaciones/calibración/opciones; no se deben borrar automáticamente.
- `%LOCALAPPDATA%\SplitForge\splitforge.log`: errores/eventos locales; revisar privacidad antes de compartir.
- Los juegos Steam guardan sus partidas según su propio sistema. SplitForge v0.6 **no** debe gestionarlas ni modificarlas.
- Los reportes `SplitForge_Reporte_*.json` son salidas compartibles tras sanitización.
