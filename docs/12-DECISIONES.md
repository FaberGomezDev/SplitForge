# Registro de decisiones (ADR)

Formato: **ID · estado · decisión · por qué · consecuencias**. Revisar antes de modificar límites arquitectónicos.

## ADR-001 · Sustituida por ADR-010 · Prototipo Python/Tkinter + SDL

Se usó Python 3.11+, Tkinter y pygame-ce/SDL2 en la fase de prototipo porque funcionó en el entorno del usuario. Sustituida el 2026-10-10: el propietario exigió las mejores tecnologías para eficiencia y fluidez. El prototipo se conserva como referencia validada.

## ADR-002 · Aceptada · Joystick directo por defecto para PS4

La ruta GameController del prototipo enumeraba sin entregar ejes. PS4/DualShock/Wireless Controller usa joystick directo en modo Auto; botones aprendidos. Se mantiene en la 0.7 con SDL3 hasta probar la ruta Gamepad en el hardware del propietario.

## ADR-003 · Aceptada · Estado autoritativo único del laboratorio

Un anfitrión calcula `SharedWorld` y difunde instantáneas a dos visores. Evita dos universos inconexos y no requiere red.

## ADR-004 · Aceptada · Inspección Steam sin modificación

Manifiestos, carpetas y procesos se leen; no se cambia contenido del juego. La detección no concede `COMPATIBLE`.

## ADR-005 · Aceptada (ampliada) · Organizar ventanas solo de procesos distintos del mismo EXE

Se rechazan dos HWND de un PID y ejecutables distintos. 0.7: hasta 4 clientes, ordenados por PID, revalidados justo antes de mover. No se cierran procesos externos.

## ADR-006 · Aceptada · Ninguna evasión de DRM / antitrampas

No forzar sesiones protegidas ni duplicar licencias/cuentas. Solo modos admitidos y clientes autorizados. Algunos títulos nunca serán soportados sin cambios oficiales.

## ADR-007 · Aceptada (mínima) · Adaptadores versionados por juego

Implementado el contrato mínimo `IGameAdapter` (manifiesto + preflight de solo lectura) y el primer adaptador (7DTD). Estado máximo automático `INSPECCIONADO`; los superiores requieren evidencia física registrada.

## ADR-008 · Propuesta · Telemetría reproducible antes de optimizar

Instrumentar frametimes, procesos y VRAM antes de compositor, afinidades o servicios nativos de rendimiento.

## ADR-009 · Aceptada (parcial) · Paquete ejecutable

Implementado como publicación .NET autónoma win-x64 en la CI (sin instalar nada). Pendiente: firma de código y actualizador.

## ADR-010 · Aceptada · .NET 10 + SDL3 + WPF, con C++ para piezas nativas

**Decisión:** reconstruir SplitForge en C# / .NET 10 (LTS) con WPF (renderizado por GPU) y SDL3; C++20 solo para componentes nativos justificados (aislamiento de entrada dentro del juego, captura de frametimes). **Por qué:** el propietario pidió las mejores tecnologías para eficiencia y fluidez; Tkinter acoplaba la entrada al temporizador de la interfaz (~60 Hz) y dibujaba por software; Python no sirve para hooks nativos. .NET da hilos reales, interop Win32 directa (`LibraryImport`), publicación autónoma y es el ecosistema habitual de herramientas de este tipo en Windows. **Consecuencias:** nueva solución `SplitForge.slnx`; el prototipo Python queda como referencia con sus pruebas en la CI; configuración propia `settings.v2.json`.

## ADR-011 · Aceptada · Entrada SDL3 en un hilo dedicado

Un único hilo llama a SDL3 (hasta 1000 Hz, plazos absolutos, eventos en segundo plano) y publica instantáneas inmutables. La UI y la simulación solo leen. **Consecuencia:** una ventana bloqueada no afecta a la entrada; cualquier llamada nueva a SDL de mandos debe ir por la cola de comandos del servicio.

## ADR-012 · Aceptada · IPC binario versionado "último fotograma gana"

`[u32 longitud][SFW1 v1 …]` con validación estricta; el escritor descarta fotogramas viejos. Sustituye a las líneas JSON. **Consecuencia:** cualquier cambio de formato sube `WorldCodec.Version`.

## ADR-013 · Aceptada · 7 Days to Die mediante servidor dedicado oficial y cuentas separadas

**Decisión:** el adaptador lanza el servidor dedicado oficial con configuración y guardados propios; cada jugador usa su propia cuenta de Steam (segunda sesión de Steam en otro usuario de Windows o Sandboxie-Plus); EAC solo se desactiva con la opción oficial. **Rechazado:** el emulador Goldberg (lo que usa Nucleus Co-op para «una sola copia»), el modo sin conexión con una sola cuenta y Remote Play Together (no aplica a 7DTD en PC). **Consecuencia:** dos jugadores reales requieren dos licencias; SplitForge lo indica antes de empezar.

## ADR-014 · Aceptada · CI en Windows con artefacto autónomo

Cada push a `main` ejecuta pruebas .NET y Python, valida docs, publica win-x64 autónomo, ejecuta una prueba de humo y sube el artefacto `SplitForge-win-x64`. Las etiquetas `v*` crean una release con ZIP y SHA-256.

## ADR-015 · Aceptada (experimental) · Filtro XInput por proceso como primer módulo nativo

**Decisión:** primer componente C++ (`native/SplitForge.XInputFilter`): una DLL que se copia en la carpeta del juego con el nombre de la DLL de XInput que este carga y reenvía todo a la DLL real de System32, pero hace que cada proceso vea **un único mando**: la ranura física que SplitForge le asigna por PID en `%LOCALAPPDATA%\SplitForge\xinput\asignacion.txt`. Sin asignación es transparente. **Por qué:** XInput es la vía de mandos más común en juegos de PC y, a diferencia de Windows.Gaming.Input, funciona sin foco; es la técnica de aislamiento más simple y reversible (no inyecta en otros procesos, no escribe memoria ajena). **Límites:** solo con EAC desactivado mediante la opción oficial; los PS4 deben llegar como XInput (Steam Input o DS4Windows); si el juego exige foco hará falta además la simulación de foco. **Garantías:** nunca sobrescribe archivos del juego, retira solo lo que instaló (SHA-256), prueba de humo en la CI de Windows. **Pendiente:** validar con el diagnóstico real de 7DTD.

