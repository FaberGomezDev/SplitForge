# Arquitectura: situación real y diseño objetivo

> **Atención:** las figuras del bloque **objetivo** son propuestas, **no código implementado**.

## 1. Arquitectura actual v0.6.0

```mermaid
flowchart TB
    USER[Usuario / Win11] --> UI[GUI Tkinter · splitforge/gui.py]
    UI --> PAD[ControllerManager · SDL2/WinMM]
    UI --> SESS[SessionManager · session.py]
    PAD --> SESS
    SESS --> WORLD[SharedWorld · simula una vez en el padre]
    WORLD --> PIPE[WorldWriter · colas tamaño 1 / stdin JSON]
    PIPE --> V1[Visor P1 · shared_demo.py]
    PIPE --> V2[Visor P2 · shared_demo.py]
    SESS --> WIN[Win32 + layout · mover/restaurar]
    UI --> STEAM[SteamLibrary + Preflight]
    UI --> INSPECT[SteamWindows · consulta HWND/PID/EXE]
    INSPECT --> WIN
```

- **Padre (GUI):** posee los mandos SDL, orquesta procesos, ejecuta mundo local y emite instantáneas.
- **Hijos:** visores Python; reciben JSON lineal por `stdin`, dibujan un estado compartido. **No son clientes Steam**.
- **Ventanas:** se mueven con Win32 a dos regiones; se guardan/restauran ubicaciones cuando es posible.
- **Steam:** lectura de `appmanifest_*.acf`, evaluación conservadora y correlación de ventanas Win32; sin cambiar instalaciones o credenciales.
- **Almacenamiento:** preferencias y log en `%LOCALAPPDATA%\SplitForge\` (o `~/.splitforge` si falta la variable, solo para tests).

### Características técnicas verificables

- Dos procesos de visor creados con `subprocess.Popen`.
- Un solo estado autoritativo `SharedWorld` ejecutado por el proceso UI.
- Comunicación local padre→hijos basada en líneas JSON, sin sockets ni servidor público.
- Cola `WorldWriter` de capacidad 1: se reemplazan instantáneas antiguas bajo congestión (prioridad a la última).
- El `SessionManager` distingue `demo`, `shared_demo`, `external` y `attached`.
- La herramienta Steam consulta ventanas de manera conservadora; requiere 2 PID con mismo EXE para permitir *organización visual*. No demuestra multijugador.

## 2. Límites del prototipo actual

- La GUI Tkinter y el mundo corren en el mismo proceso; no hay servicio Core aislado.
- El estado viaja por texto JSON/stdio; no es una API estandarizada de plugins ni un protocolo versionado.
- No hay compositor DirectX, limitador de FPS universal, telemetría de VRAM o QoS automático.
- No hay redirección general de entrada hacia aplicaciones Steam de terceros.
- No hay proceso que duplique legítimamente sesiones de Steam; ni sistema de cuentas múltiples.
- No hay un adaptador que instale, inicie o sincronice una sesión cooperativa real.

## 3. Arquitectura objetivo modular (propuesta)

```mermaid
flowchart TB
    UI2[UI / Launcher] --> API[Orquestador de sesiones]
    API --> PROFILE[Registro y verificador de adaptadores]
    API --> INPUT[Input broker / perfiles por mando]
    API --> PROCESS[Gestor de procesos y ventanas]
    API --> NET[Coordinador LAN/servidor admitido]
    API --> PERF[Telemetría & políticas de rendimiento]
    API --> SAFETY[Guardados, rollback y políticas de seguridad]
    PROFILE --> RULES[Manifiesto de capacidades verificadas por versión]
    INPUT --> A[Cliente autorizado J1]
    INPUT --> B[Cliente autorizado J2]
    PROCESS --> A
    PROCESS --> B
    NET --> A
    NET --> B
```

### Principios

1. **Adaptadores versionados** por juego/build y plataforma: describen capacidades probadas, requisitos y bloqueos; no ejecutan código arbitrario sin permisos/validación.
2. **Separar control y dataplane:** UI orquesta; módulos críticos consumen recursos mínimos y registran estado. No migrar a C++ hasta justificarlo con perfilado.
3. **Preflight antes del lanzamiento:** licencia, launcher, cliente(s), modo red, mandos, guardados, espacio/disponibilidad y recuperación.
4. **Operaciones reversibles:** snapshot de ventanas y de archivos editados explícitamente, restauración al detener, no terminar procesos externos.
5. **Fail closed:** si falta condición crítica (dos clientes autorizados, anti-cheat, LAN), detener la integración con un motivo observable.
6. **Métricas antes de optimizar:** frametime P95/P99, FPS por instancia, memoria/VRAM, CPU y latencia de entrada.

### Elección del compositor

Primero ventanas nativas sin bordes y división Win32. Solo introducir composición DirectX 11 cuando una matriz de pruebas muestre una mejora tangible y compatibilidad legal. El compositor no puede convertir por sí solo un juego no cooperativo en coop.

### Definición de sesión comercial exitosa

**La misma partida** del juego debe mostrar a ambos jugadores; cada mando debe controlar a una sola instancia, guardados no dañados, cuentas legítimas cuando se requieran, cierre/restauración seguros y límites offline documentados. Dos ventanas organizadas sin estas condiciones **no cuentan**.
