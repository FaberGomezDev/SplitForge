# Módulos y contratos existentes

## Proyectos .NET (0.7)

| Proyecto / espacio de nombres | Contrato | Riesgo al modificar |
| --- | --- | --- |
| `Core.Layout` · `SplitLayout.Compute(area, jugadores 2–4, orientación, separación)`, `Fit`/`ApplyFit`, `WindowFit`, `AspectRatio` | Regiones sin solape dentro del área útil; ajuste a la ventana (rellenar o mantener proporción centrada) | DPI, monitores, píxeles impares |
| `Core.Steam` · `KeyValuesParser`, `SteamLibrary`, `PreflightEvaluator`, `SteamPublicReport` | Lectura tolerante y sin escritura; informe sin rutas | Formato VDF, rutas maliciosas, privacidad |
| `Core.Windows` · `WinPath`, `GameWindowInspector` | Ventanas de procesos dentro de la carpeta del juego; `ClientHandles` (≤4, por PID); `VerifyPair` | PID reciclados, falsos positivos |
| `Core.Windows` · `FakeFocusPlan` | Mensajes y ventanas del foco simulado (ADR-020), funciones puras | Juegos que consultan el foco real |
| `Core.Input` · `PadInfo`, `PadState`, `ControllerMapping`, `PadInterpreter`, `ReadPathPolicy`, `XInputSlotDetector`, `HotkeySpec`, `InputLockFilter` | Funciones puras; Y positiva = arriba; ranura XInput pulsada; decisión del bloqueo de entrada | J1/J2, perfiles inválidos, R2 |
| `Core.Lab` · `SharedWorld` | `Step(dt, j1, j2)` → `WorldSnapshot` inmutable | Determinismo, compatibilidad con pruebas |
| `Core.Ipc` · `WorldCodec`, `LatestFrameWriter`, `FrameReader` | `SFW1` v1, límites estrictos, último fotograma gana | Cambios de formato (subir versión) |
| `Core.Sessions` · `SessionManager`, `LabSession`, `FixedRateLoop`, `IWindowSystem`, `IProcessLauncher`, `SessionRecovery` | Restaurar siempre; solo matar procesos propios; `sesion-ventanas.json` para restaurar tras un cierre inesperado si el handle sigue siendo del mismo PID | Cierre inseguro, condiciones de carrera |
| `Core.Adapters` · `IGameAdapter`, `SevenDaysToDieAdapter`, `SevenDaysServerConfig`, `SevenDaysServer` | Preflight solo lectura; servidor con config/guardados propios | Puertos, apagado, EAC, versiones |
| `Core.Settings` · `SettingsStore` | `settings.v2.json` atómico; importa sin tocar el `settings.json` Python | Datos del usuario |
| `Core.Diagnostics` · `SessionLog`, `InputModuleClassifier`, `ModuleOriginClassifier`, `SevenDaysDiagnosticReport` (v3) | Solo nombres de módulos y categoría de origen (`sistema`/`juego`/`steam`/`otro`), nunca rutas | Privacidad |
| `SplitForge.Windows` · `Win32WindowSystem`, `ProcessInspector`, `SteamLocator`, `SandboxieLocator`, `HighResolutionDelay`, `WindowsDiagnostics`, `XInputProbe`, `CursorLock`, `GlobalHotkey` | Win32 de solo consulta + mover/estilos de ventanas normales; `XInputProbe` lee las 4 ranuras XInput en un hilo de fondo; `CursorLock` (`ClipCursor`) y `GlobalHotkey` (Ctrl+Alt+L) para el bloqueo de ratón (ADR-017); `InputLock` (ganchos de bajo nivel, Ctrl+Alt+B) para el bloqueo de entrada (ADR-019); `WindowFakeFocus` (hilo propio, `PostMessageW`) para el foco simulado (ADR-020); `PrimaryBounds` y colocación «siempre encima» para tapar la barra de tareas | Permisos, pantalla completa, procesos elevados, foco |
| `SplitForge.Input` · `SdlInputService` | Único hilo que toca SDL; `Latest`, `SetMode`, `SetMapping`, `IdentifyPlayer` | Hilos, hotplug, Steam Input |
| `SplitForge.LabViewer` · `LabViewer.Run(jugador, stdin)` | Título de ventana exacto `SplitForge MUNDO COMPARTIDO - Jugador N` | `SessionManager` busca ese título |
| `SplitForge.App` | WPF + ViewModel por pestañas (01–07); `--lab-viewer N` | Bloqueos de UI (no hacer trabajo pesado en el hilo de UI) |
| `SplitForge.Diagnostico` | `steam`, `ventanas`, `7dtd`, `xinput`, `mandos` (+ `demo-frames`, `visor-lab` para pruebas) | Privacidad de informes |
| `Core.Adapters.XInputFilter` | `Install` → `XInputInstallResult` (instalado/actualizado/al día/omitido; manifiesto siempre coherente), `IsOwnFile`, `Uninstall`, asignaciones PID → ranura, `ReadStatus` por proceso y DLL (también desde la carpeta de Sandboxie) | Archivos del juego (nunca sobrescribir lo ajeno) |
| `Core.Adapters.SteamEmulator` | Emulador de Steam, componente principal (ADR-016): instalar/restaurar con copia de seguridad + SHA-256, config JSON por identidad, lanzamiento de instancias con variable de entorno | Archivos del juego, EAC, identidades |
| `native/SplitForge.XInputFilter` (C++20) | DLL con la API de XInput (8 funciones + ordinales 100–104, 108 y 109); cada proceso ve solo su ranura asignada; DLL real: `xinput1_4.dll` verificada en System32, con sus exportaciones redirigidas al filtro (ADR-021); `estado-PID-<dll>.txt` con ranura, lecturas e intercepción, actualizado en vivo | Compatibilidad por juego, foco |
| `native/SplitForge.SteamEmu` (C++20) | DLL que reimplementa el contrato ABI de `steam_api64.dll` (API plana generada + interfaces + callbacks por canal de usuario y de servidor de juego + P2P multicast local); anuncia partida y lobbies entre instancias y puede pedir al juego la unión automática (`auto_join`, ADR-022); componente principal, experimental | ABI del SDK, API plana de Steamworks.NET, NetworkingSockets sin implementar, EAC |

## Tipos públicos importantes

```csharp
public sealed record PadInfo(uint DeviceId, string Name, string Guid, PadKind Kind, int Axes, int Buttons, string GamepadType = "");
public readonly record struct PadState(uint DeviceId, float LX = 0, float LY = 0, bool Fire = false,
                                       bool Connected = true, bool Dash = false, bool Shield = false);
public sealed record WindowSnapshot(nint Handle, Region Bounds, bool Maximized, bool Minimized, long Style, long ExStyle);
public sealed record LaunchSpec(string FileName, IReadOnlyList<string> Arguments, string? WorkingDirectory = null,
                                 bool RedirectInput = false, bool NoWindow = false,
                                 IReadOnlyDictionary<string, string>? EnvironmentVariables = null);
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
