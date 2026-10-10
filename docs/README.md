# Documentación de SplitForge

> Fuente de verdad **versionada** del repositorio. Base de código 0.6.0; edición documental añadida sin modificar la lógica probada.

## Lectura recomendada

| Documento | Contenido |
| --- | --- |
| [`01-MEMORIA.md`](01-MEMORIA.md) | Historia real, pruebas del usuario, problemas resueltos y pendientes |
| [`02-ARQUITECTURA.md`](02-ARQUITECTURA.md) | Arquitectura actual, diseño objetivo, diagramas y límites |
| [`03-MODULOS.md`](03-MODULOS.md) | Responsabilidades y contratos de cada módulo existente |
| [`04-CONTROLES.md`](04-CONTROLES.md) | PS4 por Bluetooth, SDL/WinMM, calibración y aislamiento real pendiente |
| [`05-INTEGRACION-STEAM.md`](05-INTEGRACION-STEAM.md) | Capacidades y puerta de aceptación por juego, matriz preliminar |
| [`06-PLAN-DE-DESARROLLO.md`](06-PLAN-DE-DESARROLLO.md) | Fases futuras, hitos de aceptación y prioridades |
| [`07-PRUEBAS.md`](07-PRUEBAS.md) | Pruebas automáticas/manuales, métricas y regresiones |
| [`08-SEGURIDAD.md`](08-SEGURIDAD.md) | Privacidad, DRM, antitrampas, guardados y procesos externos |
| [`09-RENDIMIENTO.md`](09-RENDIMIENTO.md) | Métricas, perfiles y optimización para hardware de referencia |
| [`10-DESARROLLO.md`](10-DESARROLLO.md) | Primer arranque, estructura, estilo, contribuciones |
| [`11-DIAGNOSTICO.md`](11-DIAGNOSTICO.md) | Árbol de diagnóstico de mandos, Steam y ventanas |
| [`12-DECISIONES.md`](12-DECISIONES.md) | Registro de decisiones arquitectónicas (ADR) |
| [`13-FLUJOS-Y-DATOS.md`](13-FLUJOS-Y-DATOS.md) | Flujo de procesos, datos y comunicación local |
| [`14-VERSIONADO-Y-RELEASE.md`](14-VERSIONADO-Y-RELEASE.md) | Cómo empaquetar, probar y publicar releases |
| [`15-HISTORIAL.md`](15-HISTORIAL.md) | Hitos históricos y notas para próximas sesiones |
| [`16-PENDIENTES.md`](16-PENDIENTES.md) | Riesgos y preguntas abiertas sin suposiciones |
| [`plans/7-days-to-die-original.md`](plans/7-days-to-die-original.md) | Plan existente heredado del prototipo |
| [`templates/ADAPTADOR.md`](templates/ADAPTADOR.md) | Plantilla obligatoria para perfiles/handlers de juegos |
| [`templates/ENTREGA.md`](templates/ENTREGA.md) | Plantilla para actualizar memoria/handoff |

## Estado actual de un vistazo

**Comprobado por el usuario:** dos mandos PS4 detectados y funcionales dentro del laboratorio de un único mundo local (v0.4). También indicó que v0.5 funcionó. La v0.6 tiene pruebas automáticas y requiere contraste contra juegos Steam reales en el equipo.

**Implementado en el código:** lectura de mandos, selección P1/P2, demostración compartida con IPC por stdin, biblioteca Steam pasiva, evaluación de instalaciones, identificación conservadora de dos procesos y disposición/restauración de ventanas.

**No implementado/verificado:** coop local de 7 Days to Die u otro título comercial, ejecución múltiple Steam autorizada, redirección de mandos dentro de clientes comerciales, emparejamiento LAN automático, presupuesto de VRAM dinámico, aplicación nativa C++/C#.

> Una carpeta `docs/` no sustituye las pruebas: documenta los contratos y el conocimiento acumulado. Para agentes, leer también [`../CLAUDE.md`](../CLAUDE.md).
