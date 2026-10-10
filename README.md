# SplitForge

**Pantalla dividida local para juegos de Steam compatibles en una sola PC con Windows**: cada jugador con su mando y su parte de la pantalla, en la misma partida cuando el juego lo permite de forma legítima.

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
| 06 7 DAYS TO DIE | Preflight, servidor dedicado local aislado, Steam del jugador 2 en Sandboxie-Plus, pantalla dividida y diagnóstico |

Diagnóstico por consola (solo lectura): `SplitForge.Diagnostico.exe 7dtd | mandos | steam | ventanas`.

## 7 Days to Die con dos jugadores: qué hace falta

1. **Una cuenta de Steam con el juego por jugador**, cada una con su propia sesión.
2. **El servidor dedicado gratuito** «7 Days to Die Dedicated Server» (Steam → Biblioteca → Herramientas). SplitForge lo arranca con configuración y guardados propios.
3. **Clientes sin EAC** con la opción oficial: Steam → Propiedades → General → *Preguntar al iniciar* → **Launch game without EAC**.
4. Cada jugador se une a `127.0.0.1:26900`; SplitForge divide la pantalla.
5. **Un mando por ventana (experimental):** el filtro XInput nativo en C++ hace que cada cliente vea solo su mando (pestaña 06 → *Instalar filtro* → *Asignar mandos*). Requiere los PS4 como mandos XInput (Steam Input o DS4Windows). Sin validar todavía en hardware: confirma con `SplitForge.Diagnostico.exe 7dtd` que el juego carga XInput.

## Desarrollo

```powershell
dotnet test SplitForge.slnx -c Release     # pruebas .NET
Compilar_SplitForge.bat                    # pruebas + publish\SplitForge autónomo
```

Requiere el SDK de .NET 10. Detalles en [`docs/10-DESARROLLO.md`](docs/10-DESARROLLO.md).

**Empezar a leer:** [`CLAUDE.md`](CLAUDE.md) · [`docs/README.md`](docs/README.md) · [`docs/01-MEMORIA.md`](docs/01-MEMORIA.md) · [`docs/02-ARQUITECTURA.md`](docs/02-ARQUITECTURA.md).

## Límites y precauciones

- Una ventana de un juego **no es** una instancia cooperativa; dos clientes necesitan identidades autorizadas y la misma partida.
- El modo sin conexión de Steam no crea LAN ni autenticación para dos personas.
- No se modifican ejecutables, DRM, antitrampas, cuentas ni guardados de los juegos. No se cierran juegos ajenos.
- Pantalla completa exclusiva y procesos elevados pueden impedir mover ventanas: usa modo ventana o sin bordes del propio juego.
- Configuración en `%LOCALAPPDATA%\SplitForge\settings.v2.json`; el `settings.json` del prototipo no se toca.

## Prototipo Python 0.6 (referencia)

`Iniciar_SplitForge.bat`, `Instalar_Todo.bat`, `run_tests.bat` y los diagnósticos `.bat` siguen funcionando con su `.venv`. Ver [`docs/15-HISTORIAL.md`](docs/15-HISTORIAL.md).
