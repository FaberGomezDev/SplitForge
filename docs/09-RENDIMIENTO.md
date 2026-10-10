# Rendimiento y optimización: hipótesis vs mediciones

## Equipo de referencia

- CPU AMD Ryzen 7 7700X (8 núcleos / 16 hilos).
- GPU NVIDIA RTX 4060, 8 GB VRAM en la variante de referencia.
- 32 GB RAM, dos controles PS4, Windows 11.

No se ha medido todavía rendimiento de **dos instancias Steam** sobre este equipo. Los objetivos siguientes son decisiones de diseño, no resultados.

## Políticas propuestas

- **Equilibrado (predeterminado):** primero estabilidad y frametime sostenido; resolución del juego reducida por región y calidad intermedia cuando sea admisible.
- **Rendimiento:** límite de FPS y textura/contenido gráfico más conservador, según opciones expuestas por el juego.
- **Calidad:** más calidad si no produce saturación de RAM/VRAM ni tirones.

No cambiar valores de un juego mediante manipulación de memoria, DLLs o flags no documentados. Priorizar opciones accesibles por ajustes normales y perfiles permitidos.

## Qué medir

| Métrica | Técnica de medición futura | Riesgo |
| --- | --- | --- |
| FPS/frametime por instancia | PresentMon u otra herramienta compatible / trazas disponibles | Promedios esconden tirones |
| 1% low y P95/P99 | Distribución de frametimes, no solo FPS medios | Variabilidad por escena/carga |
| Uso VRAM y presupuesto | DXGI/telemetría sistema cuando esté soportado | 2 instancias duplican recursos |
| RAM por proceso | Contadores de proceso/Working Set/Commit | 32 GB no garantizan suficiencia universal |
| CPU/GPU | Contadores y timeline por instancia | Afinidad fija puede empeorar planificación |
| Input → efecto | Prueba fotograma y estados con timestamps | El IPC extra añade trabajo |
| Inicio/cierre y fallos | Logs de sesión estructurados | Hotspots de launcher, pérdida de guardados |

## Estrategia de optimización

1. Perfilado sobre escena y build repetibles; fijar baseline.
2. Reducir primero **sobrecarga del launcher** (event loop, polling, colas, ventanas).
3. Monitorizar VRAM: para 8 GB preferir presupuestos preventivos, degradación explícita en vez de falta de memoria.
4. No fijar CPU affinity ni prioridad agresiva sin evidencia A/B.
5. Usar ventanas nativas/Win32 antes de construir un compositor que duplique copias o readbacks.
6. Solo después considerar componentes C++ nativos y medir la ganancia real.

## Aceptación de mejoras

Cada optimización tendrá: hipótesis, métrica, baseline, ensayo A/B, variabilidad, regresiones de control/estabilidad y rollback. Una mejora solo es "superior a Nucleus" donde haya datos repetibles y evidencia del mismo juego y hardware.
