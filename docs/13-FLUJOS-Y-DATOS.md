# Flujos de ejecución, hilos, IPC y persistencia

## Hilos de SplitForge.exe (0.7)

| Hilo | Frecuencia | Responsabilidad |
| --- | --- | --- |
| Interfaz WPF | temporizador 33 ms; tareas lentas ~2 Hz | Reflejar estado, comandos del usuario, `SessionManager.Tick`, estado del servidor |
| `SplitForge.Input` | ≤1000 Hz | Único que llama a SDL3 para mandos; publica `InputSnapshot` |
| `SplitForge.Lab` | 120 Hz | `LabSession.Step` + envío a visores (solo con laboratorio activo) |
| `SplitForge.LabWriter.Pn` | según llegan fotogramas | Escribir el último fotograma en el stdin de cada visor |
| Tareas en segundo plano de la pestaña 06 | a petición y en el refresco de estado | `XInputProbe` (ranuras XInput, «Detectar»), estado del filtro y del servidor; nunca en el hilo de la interfaz |

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

## Emulador de Steam (pestaña 07)

```mermaid
flowchart LR
    INST[Instalar] -->|copia de seguridad + SHA-256| DLL[steam_api64.dll emulada\njunto a la original del juego]
    DLL --> LJ1[Lanzar J1\nSPLITFORGE_STEAMEMU_CONFIG=j1.json]
    DLL --> LJ2[Lanzar J2\nSPLITFORGE_STEAMEMU_CONFIG=j2.json]
    LJ1 --> P2P[P2P local · UDP multicast]
    LJ2 --> P2P
    DLL --> REST[Restaurar: solo si el SHA-256 coincide\ndevuelve la original y borra lo propio]
```

«Lanzar J1 y J2» (ADR-018):
1. Calcula el panel de cada jugador con la distribución de la barra inferior, el mínimo 16:9 de 7DTD y, si está activo, el monitor completo.
2. Lanza J1 y, 1,5 s después, J2 con su identidad y `-crossplatform=None -serverplatforms=Steam -noeac -screen-fullscreen 0 -popupwindow -screen-width/-height`.
3. Si el filtro XInput está instalado, escribe la asignación por PID (J1 → mando de J1, J2 → mando de J2); el filtro la relee en vivo.
4. A ≈2 Hz, desde 12 s después, busca las ventanas «7 Days To Die» de esos PID y abre la sesión de pantalla dividida (restaurable al detener).

## Persistencia

- `%LOCALAPPDATA%\SplitForge\settings.v2.json`: distribución, modo de lectura, frecuencia de entrada, perfiles de mandos por GUID, opciones del servidor 7DTD. Escritura atómica.
- `%LOCALAPPDATA%\SplitForge\settings.json`: del prototipo Python; la 0.7 solo lo lee una vez para importar preferencias.
- `%LOCALAPPDATA%\SplitForge\splitforge-net.log`: eventos con identificador de sesión; rota a `.old` al superar 2 MB.
- `%LOCALAPPDATA%\SplitForge\7dtd\`: `serverconfig.xml`, `UserData\` (mundos y guardados del servidor local), `logs\`.
- `%LOCALAPPDATA%\SplitForge\sesion-ventanas.json`: solo mientras hay ventanas externas modificadas; se borra al detener. Si existe al arrancar (cierre inesperado), se restauran las ventanas cuyo handle sigue siendo del mismo PID.
- `%LOCALAPPDATA%\SplitForge\xinput\`: `asignacion.txt` (líneas `PID RANURA`, escrita por SplitForge) y `estado-PID-<dll>.txt` (escrito por cada copia del filtro con reemplazo atómico). Para el jugador 2 en Sandboxie, el estado queda dentro del sandbox `SplitForge_J2`.
- `%LOCALAPPDATA%\SplitForge\steamemu\`: `j1.json` y `j2.json` (identidades que se pasan por `SPLITFORGE_STEAMEMU_CONFIG`) y una carpeta de guardados por identidad (`<appid>-J1`, `<appid>-J2`, con `remote\` y `userdata\`). Registro nativo en `%LOCALAPPDATA%\SplitForge\steamemu.log`.
- Carpeta del juego (solo con el emulador instalado): `steam_api64_splitforge_original.dll` (copia de la original), `SplitForge.SteamEmu.json` y `SplitForge.SteamEmu.estado.txt` (SHA-256 de la DLL instalada).
- Carpeta del juego (solo con el filtro instalado): copias del filtro con los nombres de XInput que carga el juego y su manifiesto `splitforge-xinput.json`.
- Los juegos guardan sus partidas según su sistema; SplitForge no las toca.
- `SplitForge_Reporte_*.json`: salidas compartibles ya saneadas.
