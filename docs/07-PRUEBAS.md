# Estrategia de pruebas y criterios de evidencia

## Suite automática actual

Pruebas estándar sin instalaciones nuevas (pueden ejecutarse en SO diferente para lógica pura):

```sh
python -m unittest discover -s tests -v
python tools/check_docs.py
```

En Windows, utilizar preferiblemente `\.venv\Scripts\python.exe` y `run_tests.bat`.

La v0.6 original registró **72 pruebas automáticas**; eso verifica lógica aislada y mocks, **no** dos procesos Steam reales ni DOS PS4 en Windows. Esta edición de documentación añade comprobaciones de docs. Anotar conteo real de la versión empaquetada, nunca copiar el número sin ejecutar.

### Cobertura existente

- Lectura/normalización/configuración de gamepads.
- Aprendizaje de botones y gatillo analógico.
- Serialización/lectura de input; transmisión al mundo compartido.
- Geometría de pantalla, ciclo de sesiones y seguridad de cierre.
- Steam library/preflight con rutas y manifests simulados.
- Steam windows: dos HWND/1 PID rechazado; dos PID/mismo exe admitidos para ordenar ventanas; revalidación; errores de permiso; JSON privado.
- Instalación de dependencias (sin ejecutar descargas reales en test).

## Matriz de pruebas manuales Windows requerida

| ID | Escenario | Criterio |
| --- | --- | --- |
| W01 | Arranque limpio | GUI abre con Python + pygame-ce adecuados |
| W02 | 2 PS4 Bluetooth | Detecta ambos como mandos distintos y el movimiento es visible |
| W03 | P1/P2 | Mando de P1 no mueve actor P2 y viceversa |
| W04 | Acciones | Disparo/impulso/escudo son independientes y visibles |
| W05 | Reinicio + reconexión | Identificadores pueden cambiar; reassign explícito reestablece entrada |
| W06 | Mundo compartido | Ambas ventanas observan los dos actores y el mismo objeto compartido |
| W07 | Dividir pantalla | Vertical/horizontal sin ventanas perdidas; restaurar al cerrar |
| W08 | Biblioteca Steam | Juegos instalados se detectan en bibliotecas secundarias |
| W09 | Steam 1 cliente | Detecta una ventana/PID pero NO permite pareja falsa |
| W10 | Steam 2 clientes legítimos | Solo ordena si hay 2 PID del mismo ejecutable, revalida antes de mover |
| W11 | Error/pérdida de ventana | Restauración best effort; no mata procesos ajenos |
| W12 | Informes | JSON sin rutas personales, cuentas, ventanas ni identificadores privados |

**Resultado físico conocido:** usuario confirmó laboratorio v0.4 y detección Steam v0.5; faltan resultados de varias filas en v0.6.

## Benchmark cuando exista primer adaptador

- Misma build, resolución, presets, escena, mandos, red/servidor, número de jugadores y herramientas.
- Varios intentos de orden alterno SplitForge ↔ baseline para minimizar calentamiento/deriva.
- Registrar: FPS promedio, 1% low, frametime P50/P95/P99, memoria RAM, VRAM/presupuesto, CPU por proceso, cierre y fallos.
- Registrar sobrecarga del **launcher** separada de la del propio juego.
- Informar mínimos, medianas, dispersión, configuración y fallas; no confundir mejora de pantalla con FPS más altos.
- Umbrales de éxito futuros se establecen después del benchmark base; objetivo sugerido: cero controles cruzados, sin pérdida de guardados, menos fallas, sobrecarga marginal medible.

## Evidencias por cambio

Cada PR/entrega debe adjuntar: prueba ejecutada y conteo, sistema donde corrió, escenario real o mock, incidencia/caso regresivo, archivos cambiados y estado de validación de hardware. Se registran en `01-MEMORIA.md` y `15-HISTORIAL.md`.
