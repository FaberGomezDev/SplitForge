# Historial del proyecto y de la documentación

Los hitos provienen de los paquetes del prototipo, del repositorio Git y de las verificaciones comunicadas por el usuario. No son benchmarks de juegos comerciales.

## Hitos

- **0.1–0.2:** primer laboratorio; de XInput exclusivo a SDL/WinMM; instaladores.
- **0.3:** diagnóstico reproducible SDL/Windows; enumeración separada de lectura.
- **0.3.1:** lectura directa joystick de PS4.
- **0.3.2:** aprendizaje de botones; disparo, escudo, impulso.
- **0.4:** laboratorio de mundo compartido. **Usuario confirmó que funcionó.**
- **0.5:** biblioteca Steam e informes sin rutas personales. **Usuario confirmó funcionamiento.**
- **0.6:** inspección HWND/PID/EXE y distribución conservadora; 72 pruebas.
- **0.6.0+docs:** `CLAUDE.md`, `docs/`, plantillas, validación de enlaces; 75 pruebas.

## 2026-10-10 · Repositorio y diagnóstico 7DTD

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 180218f
OBJETIVO: subir el código a GitHub y obtener evidencia de cómo lee 7DTD los mandos.
CAMBIOS EFECTIVOS: diagnostico_7dtd.py + Diagnostico_7DTD.bat (módulos de entrada por proceso, solo lectura);
  README sin la línea UTF-16 corrupta; reglas de Git del propietario en CLAUDE.md.
EVIDENCIA AUTOMÁTICA: 77 pruebas Python OK.
EVIDENCIA EN WINDOWS REAL: el propietario inicia 7DTD sin EAC con la opción oficial.
PRÓXIMO PASO: ejecutar el diagnóstico con el juego abierto.
```

## 2026-10-10 · 0.7.0 · Reconstrucción en .NET 10 + SDL3

```text
FECHA: 2026-10-10
RELEASE/COMMIT: 3018ffa (+ correcciones posteriores del mismo día)
OBJETIVO: tecnologías más eficientes (decisión del propietario) y camino hacia 7DTD con dos jugadores.
CAMBIOS EFECTIVOS: solución SplitForge.slnx (Core, Windows, Input, LabViewer, App WPF, Diagnostico, Tests);
  entrada SDL3 en hilo dedicado ≤1000 Hz; simulación 120 Hz; IPC binario; ventanas 2–4 sin bordes;
  adaptador 7DTD (servidor dedicado oficial aislado, Sandboxie-Plus, pantalla dividida, diagnóstico);
  CI Windows con paquete autónomo y releases por etiqueta; documentación actualizada (ADR-010…014).
EVIDENCIA AUTOMÁTICA: 106 xUnit + 77 unittest; CI windows-latest verde (pruebas, publicación,
  humo: SDL3 3.5.0 inicializa en Windows); Linux: hilo de entrada 1000 Hz, visor dibujado bajo Xvfb.
EVIDENCIA EN WINDOWS REAL (propietario): pendiente.
NUEVOS RIESGOS: perfiles de botones a recalibrar (GUID SDL3); Sandboxie y Steam; firewall al abrir el servidor.
LIMITACIONES EXPLÍCITAS: sin aislamiento de mandos dentro del juego; dos jugadores requieren dos cuentas.
PRÓXIMO PASO: validar W01–W07; SplitForge_Reporte_7DTD.json; módulo nativo C++.
```

## Siguiente entrada (plantilla)

```text
FECHA:
RELEASE/COMMIT:
OBJETIVO:
CAMBIOS EFECTIVOS:
EVIDENCIA AUTOMÁTICA:
EVIDENCIA EN WINDOWS REAL:
NUEVOS RIESGOS:
LIMITACIONES EXPLÍCITAS:
PRÓXIMO PASO:
```
