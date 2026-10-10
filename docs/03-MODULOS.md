# Módulos y contratos existentes

## Proyectos .NET (0.7)

| Proyecto / espacio de nombres | Contrato | Riesgo al modificar |
| --- | --- | --- |
| `Core.Layout` · `SplitLayout.Compute(area, jugadores 2–4, orientación, separación)` | Regiones sin solape dentro del área útil | DPI, monitores, píxeles impares |
| `Core.Steam` · `KeyValuesParser`, `SteamLibrary`, `PreflightEvaluator`, `SteamPublicReport` | Lectura tolerante y sin escritura; informe sin rutas | Formato VDF, rutas maliciosas, privacidad |
| `Core.Windows` · `WinPath`, `GameWindowInspector` | Ventanas de procesos dentro de la carpeta del juego; `ClientHandles` (≤4, por PID); `VerifyPair` | PID reciclados, falsos positivos |
| `Core.Input` · `PadInfo`, `PadState`, `ControllerMapping`, `PadInterpreter`, `ReadPathPolicy` | Funciones puras; Y positiva = arriba | J1/J2, perfiles inválidos, R2 |
| `Core.Lab` · `SharedWorld` | `Step(dt, j1, j2)` → `WorldSnapshot` inmutable | Determinismo, compatibilidad con pruebas |
| `Core.Ipc` · `WorldCodec`, `LatestFrameWriter`, `FrameReader` | `SFW1` v1, límites estrictos, último fotograma gana | Cambios de formato (subir versión) |
| `Core.Sessions` · `SessionManager`, `LabSession`, `FixedRateLoop`, `IWindowSystem`, `IProcessLauncher` | Restaurar siempre; solo matar procesos propios | Cierre inseguro, condiciones de carrera |
| `Core.Adapters` · `IGameAdapter`, `SevenDaysToDieAdapter`, `SevenDaysServerConfig`, `SevenDaysServer` | Preflight solo lectura; servidor con config/guardados propios | Puertos, apagado, EAC, versiones |
| `Core.Settings` · `SettingsStore` | `settings.v2.json` atómico; importa sin tocar el `settings.json` Python | Datos del usuario |
| `Core.Diagnostics` · `SessionLog`, `InputModuleClassifier`, `SevenDaysDiagnosticReport` | Solo nombres de módulos, nunca rutas | Privacidad |
| `SplitForge.Windows` · `Win32WindowSystem`, `ProcessInspector`, `SteamLocator`, `SandboxieLocator`, `HighResolutionDelay`, `WindowsDiagnostics` | Win32 de solo consulta + mover/estilos de ventanas normales | Permisos, pantalla completa, procesos elevados |
| `SplitForge.Input` · `SdlInputService` | Único hilo que toca SDL; `Latest`, `SetMode`, `SetMapping`, `IdentifyPlayer` | Hilos, hotplug, Steam Input |
| `SplitForge.LabViewer` · `LabViewer.Run(jugador, stdin)` | Título de ventana exacto `SplitForge MUNDO COMPARTIDO - Jugador N` | `SessionManager` busca ese título |
| `SplitForge.App` | WPF + ViewModel por pestañas; `--lab-viewer N` | Bloqueos de UI (no hacer trabajo pesado en el hilo de UI) |
| `SplitForge.Diagnostico` | `steam`, `ventanas`, `7dtd`, `mandos` (+ `demo-frames`, `visor-lab` para pruebas) | Privacidad de informes |

## Tipos públicos importantes

```csharp
public sealed record PadInfo(uint DeviceId, string Name, string Guid, PadKind Kind, int Axes, int Buttons, string GamepadType = "");
public readonly record struct PadState(uint DeviceId, float LX = 0, float LY = 0, bool Fire = false,
                                       bool Connected = true, bool Dash = false, bool Shield = false);
public sealed record WindowSnapshot(nint Handle, Region Bounds, bool Maximized, bool Minimized, long Style, long ExStyle);
public sealed record LaunchSpec(string FileName, IReadOnlyList<string> Arguments, string? WorkingDirectory = null,
                                bool RedirectInput = false, bool NoWindow = false);
```

- `SessionManager.StartLab(viewerSpec, j1, j2, layout)` exige dos mandos distintos; `RouteLab(states, dt)` se llama desde el hilo de simulación.
- `SessionManager.Attach(handles 2–4, layout)` captura TODAS las posiciones antes de mover; si una falla, restaura.
- `SessionManager.Stop()` restaura ventanas y estilos; cierra solo visores propios (WM_CLOSE y, tras 2 s, terminación).
- `SevenDaysServer.StopAsync()` envía `shutdown` por telnet local, espera hasta 45 s y solo entonces termina su propio proceso.

## Prototipo Python 0.6 (referencia)

`launcher.py`, `splitforge/*.py`, `diagnostico*.py`, `steam_*scan.py`, `setup_deps.py` y los `.bat` antiguos siguen funcionando como antes. No se amplían salvo diagnósticos; sus pruebas siguen en la CI.

## Reglas para cambios en interfaces

- `PadState`: actualizar `PadInterpreter`, `SharedWorld`, la UI de mandos, pruebas y docs.
- Instantánea del mundo: actualizar `WorldCodec` (subir `Version`), `LabViewer`, pruebas.
- Ciclo de sesiones: cubrir cierre normal, error a medio iniciar, ventana desaparecida e ID cambiantes.
- Steam/adaptadores: preflight de solo lectura, informes sin secretos, estado máximo automático `INSPECCIONADO`.
