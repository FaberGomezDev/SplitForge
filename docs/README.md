# Documentación de SplitForge

> Fuente de verdad **versionada** del repositorio. Código **0.7.0** (.NET 10 + SDL3); el prototipo Python 0.6.0 se conserva como referencia validada.

## Lectura recomendada

| Documento | Contenido |
| --- | --- |
| [`01-MEMORIA.md`](01-MEMORIA.md) | Historia real, pruebas del usuario, problemas resueltos y estado para la siguiente sesión |
| [`02-ARQUITECTURA.md`](02-ARQUITECTURA.md) | Arquitectura actual 0.7, diseño objetivo, diagramas y límites |
| [`03-MODULOS.md`](03-MODULOS.md) | Responsabilidades y contratos de cada proyecto y tipo público |
| [`04-CONTROLES.md`](04-CONTROLES.md) | PS4 por Bluetooth, SDL3 en hilo dedicado, calibración y aislamiento pendiente |
| [`05-INTEGRACION-STEAM.md`](05-INTEGRACION-STEAM.md) | Estados por juego, adaptador 7DTD (servidor oficial) y emulador de Steam |
| [`06-PLAN-DE-DESARROLLO.md`](06-PLAN-DE-DESARROLLO.md) | Fases, hitos de aceptación y prioridades |
| [`07-PRUEBAS.md`](07-PRUEBAS.md) | Pruebas automáticas (xUnit, unittest, CTest, CI) y matriz manual en Windows |
| [`08-SEGURIDAD.md`](08-SEGURIDAD.md) | Privacidad, EAC, guardados y procesos externos |
| [`09-RENDIMIENTO.md`](09-RENDIMIENTO.md) | Decisiones de rendimiento tomadas y métricas pendientes |
| [`10-DESARROLLO.md`](10-DESARROLLO.md) | Requisitos, primer arranque, estructura, convenciones |
| [`11-DIAGNOSTICO.md`](11-DIAGNOSTICO.md) | Árbol de diagnóstico de mandos, Steam, ventanas, 7DTD y emulador |
| [`12-DECISIONES.md`](12-DECISIONES.md) | Registro de decisiones arquitectónicas (ADR) |
| [`13-FLUJOS-Y-DATOS.md`](13-FLUJOS-Y-DATOS.md) | Flujos de procesos, hilos, IPC y persistencia |
| [`14-VERSIONADO-Y-RELEASE.md`](14-VERSIONADO-Y-RELEASE.md) | Versionado, CI, artefactos y releases |
| [`15-HISTORIAL.md`](15-HISTORIAL.md) | Hitos históricos y notas para próximas sesiones |
| [`16-PENDIENTES.md`](16-PENDIENTES.md) | Riesgos y preguntas abiertas sin suposiciones |
| [`plans/7-days-to-die-original.md`](plans/7-days-to-die-original.md) | Plan de investigación heredado del prototipo |
| [`templates/ADAPTADOR.md`](templates/ADAPTADOR.md) | Plantilla obligatoria para adaptadores de juegos |
| [`templates/ENTREGA.md`](templates/ENTREGA.md) | Plantilla para actualizar memoria/handoff |

## Estado actual de un vistazo

**Comprobado por el usuario (prototipo Python):** dos PS4 por Bluetooth funcionales en el laboratorio de un mundo compartido (v0.4) y detección Steam (v0.5).

**Implementado en 0.7 (.NET):**
- Entrada SDL3 en un hilo dedicado de hasta 1000 Hz, con eventos en segundo plano.
- Laboratorio con la simulación a 120 Hz en su propio hilo y visores SDL3/Direct3D, comunicados por IPC binario versionado.
- Distribución de 2 a 4 ventanas con modo sin bordes reversible y ajuste configurable: rellenar la región o mantener la proporción 16:9, 16:10, 21:9 o 4:3 centrada, con resolución sugerida por panel.
- Biblioteca Steam con preflight e inspección de clientes.
- Adaptador experimental de 7 Days to Die: servidor dedicado oficial local con configuración y guardados aislados, Steam del jugador 2 en Sandboxie-Plus, pantalla dividida y diagnóstico de las APIs de mando.
- **Interfaz de lanzador** (ADR-024): secciones JUGAR, MANDOS y AVANZADO; el botón JUGAR abre J1 y J2, les da su mando y divide la pantalla.
- **Emulador de Steam, componente principal** (ADR-016, en AVANZADO › 7 Days to Die): da a cada instancia del juego una identidad distinta sin un segundo Steam. **Con 7DTD (2026-10-10): dos clientes en el menú a la vez**, lanzados con la configuración del handler de Nucleus (ADR-018) en pantalla dividida y con su mando asignado.
- CI en Windows con paquete autónomo.

**Evidencia real de 7DTD (2026-10-10, un cliente sin EAC):** carga `xinput1_3.dll` y `xinput1_4.dll`, DirectInput, HID e InControl; los dos mandos llegan a Windows como `Xbox 360 Controller` (XInput). Ver [`05-INTEGRACION-STEAM.md`](05-INTEGRACION-STEAM.md).

**Medido en el PC del propietario (2026-10-10, solo lectura):**
- Hilo de entrada SDL3 3.5.0 a **1000 Hz**, sin mandos conectados.
- 7DTD build `25661859`.
- Servidor dedicado de 7DTD (AppID 294420) y Sandboxie-Plus **no instalados**.
- Controladores ViGEmBus y HidHide instalados.

**Verificado automáticamente:** 195 pruebas xUnit + 77 unittest, más las pruebas de humo C++ del filtro XInput y del emulador (esta última con la API plana de Steamworks.NET). La CI en Windows comprueba además que la API plana generada está al día. En Linux, el hilo SDL3 mide 1000 Hz y el visor dibuja correctamente bajo pantalla virtual.

**Experimental, sin validar en hardware:**
- Filtro XInput nativo en C++ (un mando por cliente dentro del juego, ADR-015), con estado por DLL y contador de lecturas, detección de la ranura XInput de cada mando y diagnóstico v3 con el origen de cada módulo.
- Bloqueo de ratón por ventana (`ClipCursor` + atajo global Ctrl+Alt+L, ADR-017) para pantalla dividida.
- Bloqueo de entrada «solo mandos» (Ctrl+Alt+B, ADR-019), con aviso en pantalla al bloquear y al desbloquear, y pantalla dividida que tapa la barra de tareas (comprobado con 7DTD).
- Un puntero propio por cliente (ADR-025): implementado y probado sin el juego.
- Teclas que no se quedan pegadas al perder el foco (ADR-026): tras Alt+Tab, LB/RB dejaban de cambiar de objeto en la ventana sin foco. **Reproducido y corregido con el juego real** en un ensayo sin manos (`tools/ensayo_7dtd.ps1`); falta la confirmación del propietario con los mandos.
- Los juegos se mantienen por encima de la barra de tareas durante la sesión (comprobado en el mismo ensayo).
- Aislamiento de mandos dentro de 7DTD: el filtro intercepta también la `xinput1_4.dll` real (ADR-021) y cada cliente enumera un solo mando (J1 → mando 1, J2 → mando 2). **Validado por el propietario con los mandos (W16).**
- Misma partida en 7DTD: J2 entra sola en la partida de J1 (W14, ADR-022), comprobado con el juego real. Falta una sesión de juego de los dos jugadores.

**No implementado / no verificado:**
- La 0.7 con mandos en el hardware del propietario.
- Simulación de foco para juegos que la exijan.
- Dos jugadores reales en 7DTD.
- Métricas de FPS/VRAM.

> `docs/` no sustituye las pruebas: documenta contratos y conocimiento acumulado. Para agentes, leer también [`../CLAUDE.md`](../CLAUDE.md).
