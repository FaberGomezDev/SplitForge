# Desarrollo y ejecución

## Usar SplitForge sin compilar (recomendado)

1. GitHub → pestaña **Actions** → última ejecución verde de «Compilar y probar» → artefacto **SplitForge-win-x64** (o la página **Releases** si hay una versión etiquetada).
2. Descomprimir en una carpeta nueva y ejecutar `SplitForge.exe`. No necesita Python, .NET ni SDL instalados.

## Requisitos para desarrollar

- Windows 10/11 x64 (para ejecutar la app); Linux o Windows para compilar y probar Core.
- **SDK de .NET 10** (`global.json` admite 10.0.1xx o superior de la misma versión principal). Visual Studio 2026/2022 17.14+ o VS Code con C# Dev Kit.
- Python 3.11+ solo para el prototipo y `tools/check_docs.py`.

## Compilar y ejecutar

```powershell
dotnet test SplitForge.slnx -c Release
dotnet run --project src/SplitForge.App
dotnet run --project src/SplitForge.Diagnostico -- mandos
Compilar_SplitForge.bat        # pruebas + publish\SplitForge (autónomo)
Iniciar_SplitForge_NET.bat     # compila si hace falta y abre SplitForge.exe
```

En Linux: `dotnet build` compila también la app WPF (`EnableWindowsTargeting`). SDL3 trae `libSDL3.so`, así que el hilo de entrada y el visor se pueden probar (`demo-frames | visor-lab 1` con `xvfb-run`). No valida Win32 ni hardware.

## Estructura

```
SplitForge.slnx            solución .NET
src/SplitForge.Core        lógica pura (sin dependencias de Windows)
src/SplitForge.Windows     interop Win32
src/SplitForge.Input       SDL3 (hilo de entrada)
src/SplitForge.LabViewer   visor SDL3
src/SplitForge.App         WPF → SplitForge.exe
src/SplitForge.Diagnostico consola → SplitForge.Diagnostico.exe
tests/SplitForge.Tests     xUnit
docs/                      memoria y arquitectura
launcher.py, splitforge/   prototipo Python 0.6 (referencia)
```

## Convenciones

- C# moderno: espacios de nombres por archivo, `record` para datos, `nullable` activado, **advertencias como errores**.
- Interop con `LibraryImport` (generado en compilación) y `[SupportedOSPlatform("windows")]`.
- Paquetes con versiones centralizadas en `Directory.Packages.props`; versiones fijas y con al menos dos semanas de antigüedad.
- Lógica en `Core` con pruebas; Win32/SDL3 como capas finas.
- Ningún trabajo pesado en el hilo de la interfaz.
- Textos de UI, docs y commits en español; identificadores de código en inglés.
- Antes de una migración profunda: ADR con coste, alternativa y pruebas.

## Datos locales

- `%LOCALAPPDATA%\SplitForge\settings.v2.json` (0.7) — `settings.json` es del prototipo y no se toca.
- `%LOCALAPPDATA%\SplitForge\splitforge-net.log` (rota a `.old` al superar 2 MB).
- `%LOCALAPPDATA%\SplitForge\7dtd\` — configuración, guardados y registros del servidor local.
- Los reportes se guardan en el Escritorio; no commitearlos (`.gitignore`).

## Archivos importantes

- `CLAUDE.md`: reglas para agentes. `README.md`: manual breve. `docs/`: memoria. `tools/check_docs.py`: valida enlaces.
