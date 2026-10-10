# Estrategia de pruebas y criterios de evidencia

## Suites automáticas

```sh
dotnet test SplitForge.slnx -c Release          # 126 pruebas xUnit (Core)
cmake -S native/SplitForge.XInputFilter -B native/build -A x64 && cmake --build native/build --config Release
ctest --test-dir native/build -C Release        # prueba de humo del filtro XInput (Windows)
python -m unittest discover -s tests -v         # 77 pruebas del prototipo Python
python tools/check_docs.py                      # enlaces y documentos obligatorios
```

Estas pruebas verifican lógica aislada y dobles de prueba; **no** validan dos procesos Steam reales ni dos PS4 en Windows. Anotar el conteo real al entregar, nunca copiarlo sin ejecutar.

### Cobertura .NET (tests/SplitForge.Tests)

- Distribución 2–4 jugadores, área útil desplazada, sin solapes, validaciones.
- Ajuste a la ventana: letterbox/pillarbox centrado por proporción, proporción inválida ignorada, «Rellenar» no toca las regiones, «Mantener proporción» encaja dentro de cada región.
- Parser KeyValues (anidación, escapes, comentarios, condicionales, profundidad acotada).
- Biblioteca Steam (manifiestos sin modificar, bibliotecas modernas/antiguas, índice en `config/`).
- Preflight (sin declarar cooperativo, rutas fuera de `common`, recorrido `..`, enlaces simbólicos, AppID desconocido, URI).
- Rutas Windows y ventanas de juego (2 ventanas/1 PID rechazado, 2–4 PID mismo exe, revalidación, privacidad del informe).
- Mandos (zona muerta, flancos, perfiles, R2 analógico, Y arriba, política PS4, dos PS4 iguales independientes).
- Mundo compartido (mismo snapshot, disparo por flanco, desconexión, dt acotado, escudo).
- IPC binario (ida y vuelta, corrupción, truncado UTF-8, último fotograma gana, pipe real de extremo a extremo).
- Sesiones (colocar/restaurar, sin bordes, 4 ventanas, rollback, relayout, laboratorio con títulos exactos, cierre y gracia, fallo a medio lanzar, recuperación tras cierre inesperado validando el PID).
- Adaptador 7DTD (preflight, configuración del servidor, escape XML, opciones inválidas, apagado ordenado con telnet falso, salida inesperada).
- Configuración (importación de la versión Python sin tocarla, ida y vuelta incluido modo de ajuste y proporción, archivo dañado, saneado con proporción acotada).
- Diagnóstico de módulos de entrada (incluido el informe real de 7DTD del propietario), origen de cada módulo como categoría sin ruta, informe v3 sin rutas.
- Tipo legible de mando (PS4/PS5/Xbox/estándar/genérico) y etiqueta de batería (porcentaje, cargando, cable, aviso de batería baja).
- Atajo global de bloqueo (etiqueta Ctrl+Alt+L; modificadores que coinciden con los MOD_* de Win32).
- Detección de la ranura XInput pulsada (flancos de botones, gatillos que cruzan el umbral, mandos recién conectados ignorados).
- Filtro XInput (.NET): nunca sobrescribe DLL del juego; actualiza solo copias propias sin modificar; manifiesto coherente aunque falle una copia; nombres no permitidos rechazados antes de copiar; retira solo lo propio sin modificar; asignaciones válidas y únicas; estado por proceso vivo y por DLL (formato antiguo y nuevo, con lecturas); carpeta de estado de Sandboxie-Plus con nombres seguros.

### Filtro XInput nativo (C++)

`smoke_test.cpp` (CTest en la CI): carga el filtro como `xinput1_4.dll` **y** como `xinput1_3.dll` en el mismo proceso, igual que hace 7DTD, y asigna la ranura 2. Verifica:

- que cada copia carga la DLL real de System32 y no la otra copia;
- que los índices 1–3 aparecen desconectados;
- que cada copia escribe su `estado-PID-<dll>.txt` con ranura y lecturas;
- que, al cambiar la asignación con el proceso abierto, el estado pasa a la ranura nueva. Compila sin avisos con MSVC `/W4 /WX` (CI) y MinGW `-Wall -Wextra -Werror` (Linux).

### CI (`.github/workflows/build.yml`, windows-latest)

Módulo nativo C++ (MSVC `/W4 /WX` + CTest), pruebas .NET y Python, documentación, publicación autónoma win-x64, **prueba de humo** del ejecutable de diagnóstico (escaneo Steam + SDL3) y **prueba de arranque de la interfaz**: abre `SplitForge.exe`, selecciona las 6 pestañas con UI Automation, guarda una captura de cada una (artefacto `captura-interfaz`) y exige un cierre ordenado. Esta prueba detectó y permitió corregir un fallo real de cierre (run #4). Primera ejecución (commit 3018ffa): verde; SDL3 3.5.0 inicializó en Windows (hilo a 646 Hz). Run #2 (d1614bb): **1000 Hz** tras usar plazos absolutos.

### Pruebas de humo en Linux (desarrollo)

- `SplitForge.Diagnostico mandos`: SDL3 3.5.0, hilo a 1000 Hz tras la corrección de plazos absolutos.
- `demo-frames | visor-lab 1` bajo `xvfb-run`: el visor dibuja correctamente ambos jugadores, orbe, disparos y escudo (captura revisada).

## Matriz de pruebas manuales en Windows (0.7)

| ID | Escenario | Criterio |
| --- | --- | --- |
| W01 | Arranque limpio del paquete | `SplitForge.exe` abre sin instalar nada |
| W02 | 2 PS4 Bluetooth | Ambos aparecen en 01 MANDOS con tipo (PS4), carga de batería en vivo y estado en vivo; hilo ≈1000 Hz |
| W03 | Segundo plano | `SplitForge.Diagnostico.exe mandos` responde con otra ventana enfocada |
| W04 | Identificar | Las luces de los PS4 cambian al color de cada jugador |
| W05 | Laboratorio | Cada mando mueve solo su nave; acciones independientes; dos visores colocados |
| W06 | Reconexión | Reasignación en vivo restablece la entrada |
| W07 | Distribución | 2–4 ventanas lado a lado/arriba-abajo, sin bordes, restauración exacta |
| W08 | Biblioteca Steam | Juegos en bibliotecas secundarias detectados |
| W09 | Steam 1 cliente | Detecta una ventana/PID pero NO permite pareja falsa |
| W10 | Steam 2 clientes | Solo distribuye procesos del mismo exe, revalida antes de mover |
| W11 | Servidor 7DTD | Arranca, llega a LISTO, guardados en `%LOCALAPPDATA%\SplitForge\7dtd`, apagado ordenado |
| W12 | Cliente 7DTD | Un cliente sin EAC se une a `127.0.0.1:26900` |
| W13 | Diagnóstico 7DTD | Informe con APIs de mando; sin rutas ni títulos. **Hecho con 1 cliente (2026-10-10, esquema v2)**; repetir con v3 y dos clientes |
| W14 | Dos clientes 7DTD | Dos clientes en la misma partida |
| W15 | Cierre | Cerrar SplitForge restaura ventanas y apaga el servidor propio |
| W16 | Filtro XInput | Con dos clientes sin EAC y el filtro asignado, cada mando controla solo su cliente; el estado de cada cliente muestra su ranura y sus lecturas suben; anotar si la ventana sin foco responde |
| W17 | Ranuras XInput | «Detectar» (o `SplitForge.Diagnostico.exe xinput`) identifica la ranura de cada mando al pulsar un botón, también con otra ventana enfocada |
| W18 | Bloqueo de ratón | Con pantalla dividida activa y el bloqueo (botón o Ctrl+Alt+L), el cursor no cruza a la otra ventana; al soltar o detener la sesión, vuelve a ser libre |
| W19 | Ajuste a ventana | Con «Mantener proporción» la ventana conserva su forma sin deformarse; la resolución sugerida por panel coincide con lo que encaja en el juego en modo ventana |

**Resultado físico conocido:** laboratorio v0.4 y Steam v0.5 confirmados en el prototipo. En la 0.7, W13 con un cliente (informe real del 2026-10-10); el resto sin confirmar.

## Benchmark cuando exista el primer adaptador validado

- Misma build, resolución, presets, escena, mandos, red y número de jugadores.
- Intentos alternos SplitForge ↔ referencia para minimizar deriva.
- FPS medio, 1 % low, frametime P50/P95/P99, RAM, VRAM, CPU por proceso, fallos.
- Sobrecarga del lanzador separada de la del juego.

## Evidencias por cambio

Cada entrega adjunta: pruebas ejecutadas y conteo, sistema, escenario real o simulado, archivos cambiados y estado de validación de hardware. Se registran en `01-MEMORIA.md` y `15-HISTORIAL.md`.
