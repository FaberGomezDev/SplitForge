# Memoria persistente del repositorio

**Propósito:** que otro desarrollador o una futura sesión de Claude Code continúe SplitForge sin volver a investigar los mismos fallos. Distingue experiencias observadas por el usuario de pruebas simuladas en desarrollo.

## Objetivo no negociable

Una PC Windows con juegos de Steam compatibles, 2–4 jugadores potenciales, cada jugador con mando y perspectiva propia, compartiendo **la misma sesión de juego**, incluso sin Internet **solo donde el juego soporte modos LAN/locales y autenticación adecuada**. No basta con colocar ventanas ni abrir copias aisladas.

## Equipo de referencia y entorno conocido

- Ryzen 7 7700X, RTX 4060 de 8 GB VRAM, 32 GB RAM.
- Windows 11; versión del sistema reportada durante diagnóstico: `10.0.26300.0` (identificador interno Windows NT).
- Python 3.13.13 y `pygame-ce 2.5.8`, SDL 2.32.10 en el reporte de diagnóstico.
- Dos dispositivos Bluetooth etiquetados `Wireless Controller` en Windows, reconocidos en SDL como `PS4 Controller`.
- Usuario confirmó uso de ambos controles en el laboratorio; no se deben registrar MAC, serie ni rutas de su perfil.

## Línea temporal de aprendizajes

| Etapa | Hecho observado o desarrollado | Estado de evidencia |
| --- | --- | --- |
| v0.1 | Laboratorio simple y organización de ventanas; mandos limitados a XInput. | Prototipo anterior; problema con PS4 |
| v0.2–0.2.2 | Introducción de SDL vía `pygame-ce`, alternativa WinMM y mejora de instaladores. | Falla original en descubrimiento PS4; varias iteraciones |
| v0.3 | Windows mostró dos `Wireless Controller`, pero el diagnóstico SDL registraba `0` inicialmente. | Reporte del usuario |
| Diagnóstico posterior | Al activar/probar mandos en Windows, `joy.cpl` los mostró; comando `pygame.joystick.get_count()` devolvió **2**, nombres `PS4 Controller`. | Salida proporcionada por el usuario |
| Investigación del motor | `ControllerManager` devolvió `SDL2 (pygame-ce)`, sin advertencias, e IDs 0 y 1. | Salida proporcionada por el usuario |
| v0.3.1 | Se prefirió lectura `pygame.joystick` para PS4 sobre la ruta estandarizada GameController. | Usuario confirmó movimiento independiente de dos joysticks inicialmente |
| v0.3.2 | Aprendizaje de botones/trigger analógico y acciones del laboratorio. | El usuario informó de una regresión intermitente del segundo mando antes de v0.4 |
| v0.4 | Un proceso simula `SharedWorld`, dos visores reciben misma instantánea por stdin; estado visible de P1/P2. | Usuario confirmó: «ya listo si funciona» |
| v0.5 | Biblioteca Steam, evaluación solo lectura y apertura normal de **una** instancia Steam. | Usuario confirmó: «sí funcionó, siguiente» |
| v0.6 | Asociación HWND/PID/ejecutable a juego instalado; comprobar dos procesos del mismo EXE antes de distribuir. | 72 pruebas automatizadas previas; **no** aceptación física de dos clientes Steam |
| Edición documental | Se agregan `CLAUDE.md`, carpeta `docs/`, `.gitignore` y chequeo de documentación. | No cambia motor de juego ni el estado de compatibilidad |

## Incidente PS4: causa, corrección y reglas

1. **Enumeración no es movimiento**: un joystick puede aparecer en Windows y SDL sin que el camino de lectura activo aporte X/Y.
2. **No instalar SDL aparte**: el wheel Windows de `pygame-ce` ya incluye SDL; los fallos de descargas de 9.7 MB no eran prueba de error de hardware.
3. **PS4**: `choose_read_path(..., mode='auto')` elige `joystick` para PS4/DualShock/Wireless Controller; `pygame.joystick` mueve los ejes.
4. **Botones**: no asumir que Cruz = índice físico 0; usar aprendizaje en GUI y permitir ejes de gatillo analógico.
5. **Aislamiento del laboratorio**: los mandos se leen en el padre y cada jugador recibe su estado específico; los visores no vuelven a abrir los mandos.
6. **Precaución**: USB/Bluetooth, reconexiones e IDs cambiantes deben revalidarse; conservar pruebas manuales P1/P2.

## Qué ya se logró frente al objetivo final

- Laboratorio en una sola PC: **SÍ**.
- Dos perspectivas del mismo mundo simulado: **SÍ**.
- Steam instalado detectado/diagnosticado: **SÍ, solo lectura**.
- Dos ventanas comerciales existentes distribuidas cuando son de procesos distintos: **función implementada, eficacia real por juego no comprobada**.
- Dos clientes de 7 Days to Die cooperando offline y dirigidos por dos mandos: **NO**.
- FPS/frametimes/VRAM medidos contra Nucleus: **NO**.

## Estado para la siguiente sesión

**Prioridad P0:** pedir/reportar `SplitForge_Reporte_Steam.json` y, para juego abierto, `SplitForge_Reporte_Ventanas_Steam.json`. Escoger título **instalado** y escenario legal/reproducible; no afirmar que dos cuentas o modo offline funcionan sin prueba. Antes de diseñar inyección de entrada, verificar qué ofrece el juego y cómo permite dos clientes. Mantener laboratorio v0.4 como prueba de regresión.

## Política de actualización de memoria

Después de un cambio: registrar **fecha**, hash/versión, archivos afectados, comandos y salida de pruebas, hechos confirmados vs hipótesis, evidencias de hardware si las hay y riesgo pendiente. Usar [`templates/ENTREGA.md`](templates/ENTREGA.md). Nunca insertar secretos, IDs de Steam o información personal del usuario.
