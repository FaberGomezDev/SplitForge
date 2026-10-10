# Memoria persistente del repositorio

**Propósito:** que otro desarrollador o una futura sesión continúe SplitForge sin volver a investigar los mismos fallos. Distingue lo observado por el usuario de lo probado en desarrollo.

## Objetivo no negociable

Una PC Windows con juegos de Steam compatibles, 2–4 jugadores, cada uno con mando y perspectiva propia, compartiendo **la misma sesión de juego**, incluso sin Internet **solo donde el juego soporte LAN/local y autenticación adecuada**. No basta con colocar ventanas ni abrir copias aisladas.

## Preferencias del propietario (vinculantes)

- **Mejores tecnologías para eficiencia, optimización y fluidez** → .NET 10 + SDL3 + C++ nativo donde haga falta (ADR-010). Tkinter descartado.
- Avanzar todo lo posible en cada entrega, no por etapas pequeñas.
- Git: trabajar en `main`, **sin ramas `claude/*` ni firmas de asistente** en commits.
- Objetivo inmediato: **7 Days to Die con dos mandos y dos jugadores en la misma partida**.

## Equipo de referencia y entorno conocido

- Ryzen 7 7700X, RTX 4060 de 8 GB VRAM, 32 GB RAM, Windows 11 (`10.0.26300.0`).
- Prototipo: Python 3.13.13, `pygame-ce 2.5.8`, SDL 2.32.10.
- Dos mandos Bluetooth `Wireless Controller` en Windows, `PS4 Controller` en SDL.
- 7 Days to Die instalado. El propietario consiguió iniciarlo **sin EAC** con la opción oficial de Steam ("Preguntar al iniciar" → "Launch game without EAC").
- El propietario **no tiene una segunda cuenta de Steam con 7DTD** (2026-10-10).

## Línea temporal de aprendizajes

| Etapa | Hecho observado o desarrollado | Estado de evidencia |
| --- | --- | --- |
| v0.1 | Laboratorio simple y organización de ventanas; mandos solo XInput. | Prototipo; problema con PS4 |
| v0.2–0.2.2 | SDL vía `pygame-ce`, alternativa WinMM, instaladores. | Falla original en descubrimiento PS4 |
| v0.3 | Windows mostró dos `Wireless Controller`; SDL registraba `0` al principio. | Reporte del usuario |
| Diagnóstico | `joy.cpl` los mostró; `pygame.joystick.get_count()` devolvió **2**. | Salida del usuario |
| v0.3.1 | Lectura `pygame.joystick` para PS4 en lugar de GameController. | Usuario confirmó movimiento independiente |
| v0.3.2 | Aprendizaje de botones y R2 analógico. | Regresión intermitente del 2.º mando antes de v0.4 |
| v0.4 | Un proceso simula `SharedWorld`, dos visores por stdin. | Usuario: «ya listo si funciona» |
| v0.5 | Biblioteca Steam, evaluación solo lectura, apertura de **una** instancia. | Usuario: «sí funcionó, siguiente» |
| v0.6 | Asociación HWND/PID/exe; dos procesos del mismo exe antes de distribuir. | 72 pruebas; sin aceptación física de dos clientes |
| 0.6.0+docs | `CLAUDE.md`, `docs/`, chequeo de documentación. | 75 pruebas |
| 2026-10-10 | Repositorio en GitHub; diagnóstico Python `diagnostico_7dtd.py` (APIs de mando de 7DTD). | 77 pruebas Python |
| **0.7.0** | Reconstrucción en .NET 10 + SDL3 + WPF; adaptador experimental 7DTD; CI Windows. | 106 xUnit; humo SDL3 en Linux; **sin prueba en el PC del propietario** |

## Incidente PS4: causa, corrección y reglas (siguen vigentes en 0.7)

1. **Enumeración no es movimiento**: un mando puede aparecer sin que la ruta de lectura aporte X/Y.
2. **No instalar SDL aparte**: el prototipo usa el SDL del wheel de `pygame-ce`; la 0.7 trae `SDL3.dll` en su paquete.
3. **PS4**: en modo Auto se lee como joystick directo (ADR-002). En SDL3 la ruta Gamepad es mucho más sólida que el módulo experimental de pygame; probarla con «Lector: Gamepad» antes de cambiar el valor por defecto.
4. **Botones**: no asumir Cruz = 0; aprender en la interfaz y permitir R2 analógico. Con SDL3/HIDAPI el orden suele ser el estándar (0 Cruz, 1 Círculo, 2 Cuadrado), pero se mantiene el aprendizaje.
5. **Aislamiento del laboratorio**: los mandos se leen en el anfitrión y cada jugador recibe su estado; los visores no abren mandos.
6. **Reconexión**: los ID cambian; reasignación explícita, nunca intercambio silencioso.

## Qué ya se logró frente al objetivo final

- Laboratorio en una sola PC: **SÍ** (prototipo validado; 0.7 pendiente de validar).
- Dos perspectivas del mismo mundo simulado: **SÍ**.
- Steam detectado/diagnosticado: **SÍ, solo lectura**.
- Servidor dedicado 7DTD local con guardados aislados: **implementado**, no probado en el PC del propietario.
- Dos/cuatro ventanas de clientes distribuidas sin bordes: **implementado**, eficacia por juego no comprobada.
- Dos clientes de 7DTD en la misma partida con dos mandos aislados: **NO** (falta segunda cuenta y módulo nativo de entrada).
- FPS/frametimes/VRAM medidos: **NO**.

## Decisiones tomadas con el propietario (2026-10-10)

- **Cómo lo hace Nucleus Co-op:** combina (1) emulador de Steam Goldberg para que una copia actúe como varias cuentas, (2) hooks de foco y filtrado de mandos por instancia, (3) colocación de ventanas. SplitForge adopta (2) y (3); **rechaza (1)** porque elude la licencia de Steam (ADR-013).
- **Modo sin conexión de Steam con una sola cuenta:** no sirve; la identidad es la misma y la partida vería al mismo jugador dos veces.
- **Remote Play Together:** solo funciona con juegos con multijugador local en PC; 7DTD solo tiene pantalla dividida en consola.

## Estado para la siguiente sesión

**P0:** que el propietario descargue el artefacto `SplitForge-win-x64` de la CI (o ejecute `Compilar_SplitForge.bat`) y valide: mandos en «01 MANDOS» (incluida la frecuencia del hilo), laboratorio con dos PS4, distribución sin bordes y restauración.
**P1:** con 7DTD abierto sin EAC (idealmente dos instancias), `SplitForge.Diagnostico.exe 7dtd` → `SplitForge_Reporte_7DTD.json`, y anotar si la ventana sin foco sigue funcionando. Con eso se elige el método del módulo nativo C++ (filtro XInput por proceso, foco simulado, etc.).
**P2:** segunda cuenta de Steam con 7DTD para la prueba real de dos jugadores.

## Política de actualización de memoria

Después de un cambio: **fecha**, hash/versión, archivos afectados, comandos y salida de pruebas, hechos confirmados vs hipótesis, evidencias de hardware si las hay y riesgo pendiente. Usar [`templates/ENTREGA.md`](templates/ENTREGA.md). Nunca insertar secretos, IDs de Steam o información personal.
