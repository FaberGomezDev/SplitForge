# Documentación de SplitForge

> Fuente de verdad **versionada** del repositorio. Código **0.7.0** (.NET 10 + SDL3); el prototipo Python 0.6.0 se conserva como referencia validada.

## Lectura recomendada

| Documento | Contenido |
| --- | --- |
| [`01-MEMORIA.md`](01-MEMORIA.md) | Historia real, pruebas del usuario, problemas resueltos y estado para la siguiente sesión |
| [`02-ARQUITECTURA.md`](02-ARQUITECTURA.md) | Arquitectura actual 0.7, diseño objetivo, diagramas y límites |
| [`03-MODULOS.md`](03-MODULOS.md) | Responsabilidades y contratos de cada proyecto y tipo público |
| [`04-CONTROLES.md`](04-CONTROLES.md) | PS4 por Bluetooth, SDL3 en hilo dedicado, calibración y aislamiento pendiente |
| [`05-INTEGRACION-STEAM.md`](05-INTEGRACION-STEAM.md) | Estados por juego y adaptador 7DTD (servidor oficial) |
| [`06-PLAN-DE-DESARROLLO.md`](06-PLAN-DE-DESARROLLO.md) | Fases, hitos de aceptación y prioridades |
| [`07-PRUEBAS.md`](07-PRUEBAS.md) | Pruebas automáticas (xUnit, unittest, CI) y matriz manual en Windows |
| [`08-SEGURIDAD.md`](08-SEGURIDAD.md) | Privacidad, EAC, guardados y procesos externos |
| [`09-RENDIMIENTO.md`](09-RENDIMIENTO.md) | Decisiones de rendimiento tomadas y métricas pendientes |
| [`10-DESARROLLO.md`](10-DESARROLLO.md) | Requisitos, primer arranque, estructura, convenciones |
| [`11-DIAGNOSTICO.md`](11-DIAGNOSTICO.md) | Árbol de diagnóstico de mandos, Steam, ventanas y 7DTD |
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

**Implementado en 0.7 (.NET):** entrada SDL3 en hilo dedicado de hasta 1000 Hz con eventos en segundo plano; laboratorio con simulación a 120 Hz en su hilo y visores SDL3/Direct3D; IPC binario versionado; distribución de 2 a 4 ventanas con modo sin bordes reversible y ajuste configurable (rellenar la región o mantener proporción 16:9/16:10/21:9/4:3 centrada), con resolución sugerida por panel; biblioteca Steam y preflight; inspección de clientes; adaptador experimental de 7 Days to Die (servidor dedicado oficial local con configuración y guardados aislados, Steam del jugador 2 en Sandboxie-Plus, pantalla dividida); diagnóstico de APIs de mando de 7DTD; CI en Windows con paquete autónomo.

**Evidencia real de 7DTD (2026-10-10, un cliente sin EAC):** carga `xinput1_3.dll` y `xinput1_4.dll`, DirectInput, HID e InControl; los dos mandos llegan a Windows como `Xbox 360 Controller` (XInput). Ver [`05-INTEGRACION-STEAM.md`](05-INTEGRACION-STEAM.md).

**Verificado automáticamente:** 126 pruebas xUnit + 77 unittest; CI en Windows verde (SDL3 3.5.0 inicializa); en Linux el hilo SDL3 mide 1000 Hz y el visor dibuja correctamente bajo pantalla virtual.

**Experimental, sin validar en hardware:** filtro XInput nativo en C++ (un mando por cliente dentro del juego, ADR-015), con estado por DLL y contador de lecturas, detección de la ranura XInput de cada mando y diagnóstico v3 con el origen de cada módulo. Bloqueo de ratón por ventana (`ClipCursor` + atajo global Ctrl+Alt+L) para pantalla dividida.

**No implementado / no verificado:** la 0.7 en el hardware del propietario; simulación de foco para juegos que la exijan; dos jugadores reales en 7DTD; métricas de FPS/VRAM.

> `docs/` no sustituye las pruebas: documenta contratos y conocimiento acumulado. Para agentes, leer también [`../CLAUDE.md`](../CLAUDE.md).
