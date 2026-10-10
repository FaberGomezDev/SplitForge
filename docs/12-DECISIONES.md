# Registro de decisiones (ADR)

Formato: **ID · estado · decisión · por qué · consecuencias**. Revisar este archivo antes de modificar límites arquitectónicos.

## ADR-001 · Aceptada · Prototipo Python/Tkinter + SDL

Se conserva Python 3.11+, Tkinter y pygame-ce/SDL2 en fase prototipo porque está comprobado en el entorno del usuario. Evitar migrar por gusto a C++/C#; esa migración podría considerarse con benchmarks sobre módulos que lo requieran.

## ADR-002 · Aceptada · SDL Joystick directo por defecto para PS4

La ruta SDL GameController podía enumerar pero no entregar correctamente ejes en el prototipo anterior. PS4/DualShock/Wireless Controller usa joystick directo en modo `auto`; botones aprendidos por dispositivo, no índices asumidos. Consecuencia: mantener test de detección, movimiento y botones.

## ADR-003 · Aceptada · Estado autoritativo único del laboratorio

Un padre calcula `SharedWorld` y difunde instantáneas por stdin a dos visores. Evita dos universos simulados inconexos y no requiere servicio de red. Consecuencia: demo no reproduce todavía networking/latencia de clientes Steam.

## ADR-004 · Aceptada · Inspección Steam sin modificación

Manifestos Steam, carpetas y procesos se leen; no se cambia contenido del juego. El lanzamiento por URI es de una sola instancia. Consecuencia: la detección no concede estado `COMPATIBLE`.

## ADR-005 · Aceptada · Organizar ventanas solo con dos PID del mismo EXE

Rechazar dos HWND de un PID y procesos de ejecutables distintos para no presentar split-screen falso. Revalidar HWND/PID/EXE justo antes de mover. No cerrar procesos externos. Consecuencia: los juegos con arquitectura launcher/subprocesos distinta requieren adaptador específico.

## ADR-006 · Aceptada · Ninguna evasión DRM / antitrampas

No forzar sesiones protegidas ni duplicar licencias/cuentas. Cualquier compatibilidad debe apoyarse en modos admitidos y clientes autorizados. Consecuencia: algunos títulos nunca serán soportados sin cambios oficiales.

## ADR-007 · Propuesta · Adaptadores versionados por juego

Antes de desarrollar un adaptador, separar preflight, start, monitor, stop y restore con flags por build y evidencia de pruebas. Pendiente de implementación y aprobación. Riesgo: complejidad excesiva para el primer título; empezar por un contrato mínimo.

## ADR-008 · Propuesta · Telemetría reproducible antes de optimización

Instrumentar tiempos de fotograma, procesos y VRAM antes de construir compositor GPU, afinidades o servicios nativos. Pendiente de mediciones. Evita decisiones basadas en conjeturas.

## ADR-009 · Propuesta · Empaquetado `.exe` independiente

Estudiar PyInstaller o equivalente solo después de que funcione el primer adaptador en Windows; fijar versiones, hashes y validar licencias de dependencias. No está implementado.
