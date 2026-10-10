# Memoria persistente del repositorio

**Propósito:** que otro desarrollador o una futura sesión continúe SplitForge sin volver a investigar los mismos fallos. Distingue lo observado por el usuario de lo probado en desarrollo.

## Objetivo no negociable

Una PC Windows con juegos de Steam compatibles, 2–4 jugadores, cada uno con mando y perspectiva propia, compartiendo **la misma sesión de juego**, incluso sin Internet **solo donde el juego soporte LAN/local y autenticación adecuada**. No basta con colocar ventanas ni abrir copias aisladas.

## Preferencias del propietario (vinculantes)

- **Mejores tecnologías para eficiencia, optimización y fluidez** → .NET 10 + SDL3 + C++ nativo donde haga falta (ADR-010). Tkinter descartado.
- Avanzar todo lo posible en cada entrega, no por etapas pequeñas.
- Git: cada cambio en una **rama de trabajo con nombre descriptivo** (nunca `claude/*`), que el propietario integra en `main`; **sin firmas de asistente** en commits.
- Objetivo inmediato: **7 Days to Die con dos mandos y dos jugadores en la misma partida**.

## Equipo de referencia y entorno conocido

- Ryzen 7 7700X, RTX 4060 de 8 GB VRAM, 32 GB RAM, Windows 11 (`10.0.26300.0`).
- Prototipo: Python 3.13.13, `pygame-ce 2.5.8`, SDL 2.32.10.
- Dos mandos Bluetooth `Wireless Controller` en Windows, `PS4 Controller` en SDL.
- 7 Days to Die instalado. El propietario consiguió iniciarlo **sin EAC** con la opción oficial de Steam ("Preguntar al iniciar" → "Launch game without EAC").
- 2026-10-10 (informe 7DTD): SDL3 ve los dos mandos como `Xbox 360 Controller` (XInput); antes aparecían como `PS4 Controller`. Probablemente se traducen con una herramienta tipo DS4Windows (supuesto).

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
| **0.7.0** | Reconstrucción en .NET 10 + SDL3 + WPF; adaptador experimental 7DTD; filtro XInput C++; CI Windows. | 112 xUnit; en la VM Windows de la CI: interfaz arranca y se cierra, 6 pestañas capturadas, enumeración Win32 real, SDL3 a 1000 Hz, filtro C++ probado; **sin prueba en el PC del propietario ni con mandos** |
| 2026-10-10 · informe 7DTD | Primer `SplitForge_Reporte_7DTD.json` real: 1 cliente **sin EAC**; carga `xinput1_3` + `xinput1_4`, DirectInput, HID e InControl (Unity); mandos como `Xbox 360 Controller`. En consecuencia: estado del filtro por DLL y en vivo con lecturas, DLL real comprobada en System32, instalación que actualiza solo copias propias y no se detiene a medias, detección de ranura XInput, informe v3 con origen de módulos y comando `xinput`. | Informe real del propietario; 126 xUnit + 77 unittest; filtro compilado sin avisos con MinGW; prueba de humo de dos copias en la CI de Windows |

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
- Un mando por cliente dentro del juego: **filtro XInput nativo implementado (experimental)**. 7DTD carga XInput (informe real); el aislamiento con dos clientes está sin validar.
- Dos clientes de 7DTD en la misma partida con dos mandos aislados: **NO** (falta validar el aislamiento de mandos).
- FPS/frametimes/VRAM medidos: **NO**.

## Decisiones tomadas con el propietario (2026-10-10)

- **Técnicas de Nucleus Co-op que adopta SplitForge:** filtrado de mandos por instancia, simulación de foco y colocación de ventanas (ADR-013, ADR-015).
- **Remote Play Together:** solo funciona con juegos con multijugador local en PC; 7DTD solo tiene pantalla dividida en consola.
- **Retirado en 0.7 a propósito:** el lanzamiento doble genérico de cualquier `.exe` (pestaña «03 EJECUTABLES» del prototipo; sin aislamiento de guardados, riesgo para partidas) y la demo antigua de dos mundos separados (sustituida por el mundo compartido). Siguen disponibles en el prototipo Python. Los juegos se integran mediante adaptadores con preflight.

## Estado para la siguiente sesión

**P0:** que el propietario descargue el artefacto `SplitForge-win-x64` de la CI (o ejecute `Compilar_SplitForge.bat`) y valide: mandos en «01 MANDOS» (incluida la frecuencia del hilo), laboratorio con dos PS4, distribución sin bordes y restauración.
**P1:** hecho con un cliente (2026-10-10): 7DTD sin EAC carga `xinput1_3` y `xinput1_4` (ver `05-INTEGRACION-STEAM.md`). Falta el informe con dos clientes.
**W16 (siguiente prueba física):**
1. Con los clientes cerrados, pulsa «Instalar filtro» en la pestaña 06.
2. Abre los dos clientes sin EAC.
3. Pulsa «Detectar» para J1 y J2 y luego «Asignar mandos a los clientes».
4. Comprueba que las lecturas de cada cliente suben y que cada mando mueve solo su ventana.
5. Anota si la ventana sin foco responde.
6. Guarda un informe v3 (`7dtd`) con ambos clientes abiertos.
**P2:** prueba real de dos jugadores en la misma partida.

## Política de actualización de memoria

Después de un cambio: **fecha**, hash/versión, archivos afectados, comandos y salida de pruebas, hechos confirmados vs hipótesis, evidencias de hardware si las hay y riesgo pendiente. Usar [`templates/ENTREGA.md`](templates/ENTREGA.md). Nunca insertar secretos, IDs de Steam o información personal.
