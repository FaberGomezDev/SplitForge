# Flujos de ejecución, hilos, IPC y persistencia

## Hilos de SplitForge.exe (0.7)

| Hilo | Frecuencia | Responsabilidad |
| --- | --- | --- |
| Interfaz WPF | temporizador 33 ms; tareas lentas ~2 Hz | Reflejar estado, comandos del usuario, `SessionManager.Tick`, estado del servidor |
| `SplitForge.Input` | ≤1000 Hz | Único que llama a SDL3 para mandos; publica `InputSnapshot` |
| `SplitForge.Lab` | 120 Hz | `LabSession.Step` + envío a visores (solo con laboratorio activo) |
| `SplitForge.LabWriter.Pn` | según llegan fotogramas | Escribir el último fotograma en el stdin de cada visor |

## Iniciar Mundo Compartido

```mermaid
sequenceDiagram
    participant U as Usuario
    participant G as UI WPF
    participant I as Hilo de entrada
    participant S as SessionManager
    participant L as Hilo de simulación
    participant V1 as Visor J1
    participant V2 as Visor J2
    U->>G: Elige dos mandos + Iniciar
    G->>S: StartLab(viewerSpec, j1, j2, layout)
    S->>V1: SplitForge.exe --lab-viewer 1 (stdin redirigido)
    S->>V2: SplitForge.exe --lab-viewer 2
    G->>L: FixedRateLoop 120 Hz
    loop cada 8,3 ms
        L->>I: Latest.States (lectura sin bloqueo)
        L->>S: RouteLab(states, dt) → SharedWorld.Step
        S->>V1: fotograma binario (último gana)
        S->>V2: mismo fotograma
    end
    G->>S: Tick (~2 Hz) → encuentra ventanas por PID + título → coloca
    U->>G: Restaurar / detener
    G->>L: Dispose
    G->>S: Stop → restaura → WM_CLOSE a visores → terminar tras 2 s si siguen
```

## Instantánea del mundo (binario SFW1 v1)

```
u32 longitud | u32 "SFW1" | u16 versión | i64 fotograma
J1: f32 x, f32 y, i32 núcleos, i32 impactos, u8 banderas(escudo|conectado)
J2: ídem
f32 orbe_x, f32 orbe_y | u16 n_disparos | n × (f32 x, f32 y, u8 dueño)
u16 n_bytes_evento | evento UTF-8 (≤512 bytes)
```

El visor descarta cargas inválidas (magia, versión, rangos, NaN, bytes sobrantes) y considera caducado un fotograma de más de 1 s.

## 7 Days to Die

```mermaid
flowchart LR
    PF[Preflight solo lectura] -->|ok| CFG[serverconfig.xml propio\nplantilla + ajustes]
    CFG --> SRV[7DaysToDieServer.exe -configfile …]
    SRV -->|telnet 127.0.0.1 abierto| READY[LISTO]
    READY --> C1[Cliente J1]
    READY --> C2[Cliente J2 · Sandboxie]
    C1 --> TILE[Dividir pantalla\nrevalidar PID/exe · sin bordes]
    C2 --> TILE
    TILE --> STOP[Detener: restaurar ventanas]
    SRV --> SHUT[shutdown por telnet → esperar ≤45 s → terminar solo si no responde]
```

## Persistencia

- `%LOCALAPPDATA%\SplitForge\settings.v2.json`: distribución, modo de lectura, frecuencia de entrada, perfiles de mandos por GUID, opciones del servidor 7DTD. Escritura atómica.
- `%LOCALAPPDATA%\SplitForge\settings.json`: del prototipo Python; la 0.7 solo lo lee una vez para importar preferencias.
- `%LOCALAPPDATA%\SplitForge\splitforge-net.log`: eventos con identificador de sesión; rota a `.old` al superar 2 MB.
- `%LOCALAPPDATA%\SplitForge\7dtd\`: `serverconfig.xml`, `UserData\` (mundos y guardados del servidor local), `logs\`.
- `%LOCALAPPDATA%\SplitForge\sesion-ventanas.json`: solo mientras hay ventanas externas modificadas; se borra al detener. Si existe al arrancar (cierre inesperado), se restauran las ventanas cuyo handle sigue siendo del mismo PID.
- Los juegos guardan sus partidas según su sistema; SplitForge no las toca.
- `SplitForge_Reporte_*.json`: salidas compartibles ya saneadas.
