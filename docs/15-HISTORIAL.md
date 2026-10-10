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
EVIDENCIA AUTOMÁTICA: 112 xUnit + 77 unittest; CI windows-latest verde (pruebas, publicación,
  humo: SDL3 3.5.0 inicializa en Windows); Linux: hilo de entrada 1000 Hz, visor dibujado bajo Xvfb.
EVIDENCIA EN WINDOWS (VM de la CI): interfaz WPF arranca, 6 pestañas capturadas, cierre ordenado,
  EnumWindows real, SDL3 3.5.0 a 1000 Hz, filtro XInput C++ cargando la DLL real de System32.
EVIDENCIA EN WINDOWS REAL (propietario, con mandos): pendiente.
NUEVOS RIESGOS: perfiles de botones a recalibrar (GUID SDL3); Sandboxie y Steam; firewall al abrir el servidor.
LIMITACIONES EXPLÍCITAS: sin aislamiento de mandos validado dentro del juego.
  Retirados a propósito: lanzamiento doble genérico de .exe y demo de dos mundos separados (siguen en el prototipo).
PRÓXIMO PASO: validar W01–W07; SplitForge_Reporte_7DTD.json; validar el filtro XInput (W16).
AMPLIACIÓN (mismo día): recuperación de ventanas tras cierre inesperado; filtro XInput nativo C++20
  (native/SplitForge.XInputFilter, ADR-015) con instalación reversible desde la pestaña 06 y prueba de humo en la CI.
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

## 2026-10-10 · Informe real de 7DTD e instrumentación del filtro XInput

```text
FECHA: 2026-10-10
RAMA: 7dtd-diagnostico-xinput (el propietario la integra en main)
OBJETIVO: aprovechar el primer SplitForge_Reporte_7DTD.json real para preparar la prueba W16.
EVIDENCIA RECIBIDA: 1 cliente 7DaysToDie.exe sin EAC (módulos legibles, sin módulos de EAC);
  XInput: xinput1_3.dll + xinput1_4.dll; DirectInput (dinput8), HID, InControl (incontrolnative),
  steam_api64, superposición de Steam, EOS SDK; SDL3 ve 2 × "Xbox 360 Controller (xbox360)".
CAMBIOS EFECTIVOS:
  - Filtro nativo: estado-PID-<dll>.txt por copia (antes las dos copias escribían el mismo archivo
    y solo una vez, así que tras asignar seguía apareciendo "sin asignar"); se reescribe al cambiar
    la asignación y cada 2 s con el contador de lecturas; reemplazo atómico; DLL real cargada por ruta
    absoluta y verificada en System32 (la alternativa xinput1_4 ya no puede resolver a otra copia).
  - XInputFilter.Install: valida nombres antes de copiar, omite archivos ajenos sin abortar,
    actualiza solo copias propias sin modificar y guarda el manifiesto aunque falle una copia.
  - Detección de la ranura XInput de cada mando («Detectar» en la pestaña 06 y comando xinput).
  - Informe 7DTD v3: origen de cada módulo de mando (sistema/juego/steam/otro, sin rutas),
    filtro_xinput_cargado (SHA-256), motor, ranuras XInput conectadas.
  - Estado del filtro y de las ranuras refrescado en segundo plano en la interfaz.
  - CI también en las ramas de trabajo (paquete para probar antes de integrar).
EVIDENCIA AUTOMÁTICA: 126 xUnit + 77 unittest; filtro y prueba de humo compilados sin avisos con
  MinGW (-Wall -Wextra -Werror); la prueba de humo (dos copias, reasignación en vivo) corre en la CI de Windows.
EVIDENCIA EN WINDOWS REAL: el informe del propietario; nada de lo nuevo probado aún en su PC.
PRÓXIMO PASO: W16 con dos clientes (lecturas, un mando por ventana, ventana sin foco) e informe v3.
```

