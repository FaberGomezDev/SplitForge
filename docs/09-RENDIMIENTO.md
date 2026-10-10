# Rendimiento y optimización: decisiones vs mediciones

## Equipo de referencia

Ryzen 7 7700X (8C/16T), RTX 4060 8 GB, 32 GB RAM, dos PS4, Windows 11. No se ha medido todavía el rendimiento de **dos instancias de un juego** en este equipo.

## Decisiones de rendimiento ya tomadas en 0.7 (con su porqué)

| Decisión | Motivo | Evidencia |
| --- | --- | --- |
| .NET 10 en lugar de Python/Tkinter | JIT/AOT, hilos reales sin GIL, interop Win32 directa | ADR-010 |
| Entrada SDL3 en hilo propio a ≤1000 Hz | Latencia de entrada independiente de la interfaz (antes 60 Hz con fluctuaciones del temporizador Tk) | 1000 Hz medidos en Windows (CI) y Linux; 646 Hz antes de corregir la acumulación de retrasos |
| Publicación de instantáneas solo con cambios | Sin basura a 1 kHz cuando no se toca el mando | Diseño |
| Esperas con temporizador de alta resolución (`SDL_DelayNS`, `CREATE_WAITABLE_TIMER_HIGH_RESOLUTION`) | Precisión de ~1 ms sin girar la CPU (no robar núcleos al juego) | Diseño; medir en el PC del propietario |
| Simulación del laboratorio a 120 Hz en su hilo | Estable aunque la UI se bloquee (diálogos) | Diseño |
| IPC binario de ~100 bytes por fotograma, último gana | Sin análisis de texto, sin latencia acumulada | Pruebas de IPC |
| Visores con renderizador SDL3 (Direct3D) y vsync | GPU en lugar de lienzo de software | Captura bajo Xvfb |
| Ventanas nativas sin bordes, sin compositor | Cero copias de fotogramas | ADR (compositor solo con evidencia) |
| Prioridad de hilos AboveNormal solo en entrada/simulación, sin afinidad fija | Evitar empeorar el planificador | Pendiente de A/B |

## Qué medir (pendiente)

| Métrica | Técnica | Riesgo |
| --- | --- | --- |
| FPS/frametime por instancia | PresentMon / ETW | Promedios esconden tirones |
| 1 % low, P95/P99 | Distribución de frametimes | Variabilidad por escena |
| VRAM y presupuesto | DXGI / contadores del sistema | Dos instancias duplican recursos (8 GB) |
| RAM por proceso | Working Set / Commit | — |
| CPU por proceso e hilo | Contadores / ETW | — |
| Entrada → efecto | Marcas de tiempo en `InputSnapshot` y en el juego | — |
| Frecuencia real del hilo de entrada | Mostrada en 01 MANDOS y en `mandos` | Temporizadores del sistema |

## Estrategia de optimización

1. Perfilado sobre escena y build repetibles; fijar referencia.
2. Reducir primero la sobrecarga del lanzador (ya en hilos dedicados y sin sondeos innecesarios).
3. Para 8 GB de VRAM: presupuestos preventivos y degradación explícita mediante opciones del propio juego.
4. Sin afinidad ni prioridad agresiva sin A/B.
5. Componentes C++ solo donde el perfilado o la funcionalidad (aislamiento de entrada) lo justifiquen.

## Aceptación de mejoras

Cada optimización: hipótesis, métrica, referencia, ensayo A/B, variabilidad, regresiones y vuelta atrás. «Superior a Nucleus» solo con datos repetibles del mismo juego y hardware.
