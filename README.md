# SplitForge

**Pantalla dividida local para juegos de Steam compatibles en una sola PC con Windows**: cada jugador con su mando y su parte de la pantalla, en la misma partida cuando el juego lo permite.

> **Versión 0.7.0 · experimental.** Reconstruida en **.NET 10 + SDL3 + WPF** para eficiencia y fluidez. El prototipo Python 0.6 (validado con dos PS4) sigue en el repositorio como referencia.
> **Todavía no hay ningún juego comercial validado con dos jugadores.** 7 Days to Die tiene un adaptador experimental.

## Descargar y usar (sin instalar nada)

1. En GitHub abre **Actions** → la última ejecución verde de **Compilar y probar** → descarga el artefacto **SplitForge-win-x64** (o la página **Releases** cuando haya una versión etiquetada).
2. Descomprime y ejecuta **`SplitForge.exe`**. Incluye todo (runtime de .NET y SDL3).

| Pestaña | Para qué sirve |
| --- | --- |
| 01 MANDOS | Mandos en vivo (SDL3, hilo de hasta 1000 Hz, funciona con el juego en primer plano), perfiles, aprender botones, identificar cada mando con su luz |
| 02 MUNDO COMPARTIDO | Laboratorio de dos jugadores en un mismo mundo (simulación a 120 Hz, visores con GPU) |
| 03 VENTANAS ABIERTAS | Distribuir 2–4 ventanas cualesquiera, con o sin bordes, y restaurarlas |
| 04 STEAM | Biblioteca instalada y diagnóstico de solo lectura |
| 05 VENTANAS STEAM | Distribuir clientes de un mismo juego (procesos distintos del mismo .exe) |
| 06 7 DAYS TO DIE | Preflight, servidor dedicado local aislado, Steam del jugador 2 en Sandboxie-Plus, pantalla dividida, filtro XInput y diagnóstico |
| 07 EMULADOR STEAM | Emulador de `steam_api64.dll`: dos instancias del juego con identidades distintas sin un segundo Steam (instalar/restaurar, identidades J1/J2, lanzar) |

Barra inferior (común a todas las pestañas): distribución lado a lado o arriba/abajo, separación, sin bordes, **tapar la barra de tareas**, «Mantener proporción» (16:9, 16:10, 21:9, 4:3), reaplicar, restaurar, bloquear el ratón en la ventana (Ctrl+Alt+L) y **bloquear la entrada para jugar solo con mandos (Ctrl+Alt+B)**.

Diagnóstico por consola (solo lectura): `SplitForge.Diagnostico.exe 7dtd | xinput | mandos | steam | ventanas`.

## 7 Days to Die con dos jugadores: qué hace falta

1. **El servidor dedicado gratuito** «7 Days to Die Dedicated Server» (Steam → Biblioteca → Herramientas). SplitForge lo arranca con configuración y guardados propios.
2. **Clientes sin EAC** con la opción oficial: Steam → Propiedades → General → *Preguntar al iniciar* → **Launch game without EAC**.
3. Cada jugador se une a `127.0.0.1:26900`; SplitForge divide la pantalla.
4. **Un mando por ventana (experimental):** el filtro XInput nativo en C++ hace que cada cliente vea solo su mando (pestaña 06 → *Instalar filtro* → *Detectar* → *Asignar mandos*). Requiere los PS4 como mandos XInput (Steam Input o DS4Windows). Ya está comprobado que 7DTD carga XInput (informe del 2026-10-10); el aislamiento con dos clientes sigue sin validar.
5. **Sin segundo Steam (experimental):** «Lanzar J1 y J2» en la pestaña 07 abre los dos clientes con identidades distintas (emulador de Steam), sin juego cruzado, en pantalla dividida y con su mando asignado. Comprobado el 2026-10-10: los dos llegan al menú a la vez. Después: J1 crea o carga una partida y J2 se une a la IP 127.0.0.1. Para volver a jugar con tu cuenta de Steam, *Restaurar DLL original*.

## Desarrollo

```powershell
dotnet test SplitForge.slnx -c Release     # pruebas .NET
Compilar_SplitForge.bat                    # pruebas + publish\SplitForge autónomo
```

Requiere el SDK de .NET 10. Detalles en [`docs/10-DESARROLLO.md`](docs/10-DESARROLLO.md).

**Empezar a leer:** [`CLAUDE.md`](CLAUDE.md) · [`docs/README.md`](docs/README.md) · [`docs/01-MEMORIA.md`](docs/01-MEMORIA.md) · [`docs/02-ARQUITECTURA.md`](docs/02-ARQUITECTURA.md).

## Límites y precauciones

- Una ventana de un juego **no es** una instancia cooperativa; los clientes deben estar en la misma partida.
- El modo sin conexión de Steam no crea LAN ni autenticación para dos personas.
- No se sobrescriben archivos ni guardados de los juegos y no se cierran juegos ajenos.
- Pantalla completa exclusiva y procesos elevados pueden impedir mover ventanas: usa modo ventana o sin bordes del propio juego.
- Configuración en `%LOCALAPPDATA%\SplitForge\settings.v2.json`; el `settings.json` del prototipo no se toca.

## Prototipo Python 0.6 (referencia)

`Iniciar_SplitForge.bat`, `Instalar_Todo.bat`, `run_tests.bat` y los diagnósticos `.bat` siguen funcionando con su `.venv`. Ver [`docs/15-HISTORIAL.md`](docs/15-HISTORIAL.md).
