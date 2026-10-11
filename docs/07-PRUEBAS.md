# Estrategia de pruebas y criterios de evidencia

## Suites automáticas

```sh
dotnet test SplitForge.slnx -c Release          # 166 pruebas xUnit (Core; las [Theory] cuentan por caso)
cmake -S native/SplitForge.XInputFilter -B native/build -A x64 && cmake --build native/build --config Release
ctest --test-dir native/build -C Release        # prueba de humo del filtro XInput (Windows)
cmake -S native/SplitForge.SteamEmu -B native/build-steamemu -A x64 && cmake --build native/build-steamemu --config Release
ctest --test-dir native/build-steamemu -C Release   # prueba de humo del emulador de Steam (Windows)
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
- Atajos globales (Ctrl+Alt+L del ratón y Ctrl+Alt+B de la entrada; modificadores que coinciden con los MOD_* de Win32).
- Bloqueo de entrada «solo mandos» (`InputLockFilter`): descarta pulsaciones, deja pasar las teclas soltadas y se suelta solo con Ctrl+Alt+B exacto (ni con B sola, ni con Ctrl+B, ni con Shift añadido).
- Distribución por juego: proporción mínima 16:9 de 7DTD (solo estrecha los paneles más estrechos; se conserva al reaplicar), monitor completo y ventanas encima de la barra de tareas con «Tapar barra de tareas», y su persistencia.
- Lanzamiento de clientes emulados: argumentos de 7DTD sin juego cruzado y con el tamaño del panel; `InstanceLaunch` con argumentos.
- Detección de la ranura XInput pulsada (flancos de botones, gatillos que cruzan el umbral, mandos recién conectados ignorados).
- Emulador de Steam (.NET):
  - instala con copia de seguridad, configuración y estado SHA-256, también con la estructura de plugins de Unity;
  - se niega a instalar dos veces o sin DLL original;
  - restaura sin dejar rastro y no pisa cambios ajenos;
  - genera un JSON de identidad válido y valida la identidad;
  - el lanzamiento lleva `SPLITFORGE_STEAMEMU_CONFIG`.
- Filtro XInput (.NET): nunca sobrescribe DLL del juego; actualiza solo copias propias sin modificar; manifiesto coherente aunque falle una copia; nombres no permitidos rechazados antes de copiar; retira solo lo propio sin modificar; asignaciones válidas y únicas; estado por proceso vivo y por DLL (formato antiguo y nuevo, con lecturas); carpeta de estado de Sandboxie-Plus con nombres seguros.

### Filtro XInput nativo (C++)

`smoke_test.cpp` (CTest en la CI): carga el filtro como `xinput1_4.dll` **y** como `xinput1_3.dll` en el mismo proceso, igual que hace 7DTD, y asigna la ranura 2. Verifica:

- que cada copia carga la DLL real de System32 y no la otra copia;
- que los índices 1–3 aparecen desconectados;
- que cada copia escribe su `estado-PID-<dll>.txt` con ranura y lecturas;
- que quien pide la `xinput1_4.dll` real de System32 por su cuenta (como InControlNative en 7DTD) recibe las funciones del filtro: índices 1–3 ocultos y lecturas contadas como «interceptadas» (ADR-021);
- que la segunda copia queda como «delegada» y no cambia el índice otra vez;
- que, al cambiar la asignación con el proceso abierto, el estado pasa a la ranura nueva. Compila sin avisos con MSVC `/W4 /WX` (CI) y MinGW `-Wall -Wextra -Werror` (Linux).

### Emulador de Steam nativo (C++)

`steamemu_humo` (CTest en la CI) lanza dos procesos que cargan la DLL con identidades distintas y verifica:
- `SteamAPI_InitSafe`, SteamID, persona, AppID y propiedad de apps;
- que `SteamInternal_ContextInit` resuelve la cadena de versiones;
- el callback de stats y su persistencia, la nube y un intercambio P2P local;
- lo que usa 7DTD para encontrarse (ADR-022): la respuesta del ticket llega solo por el canal del servidor de juego y con el ABI del SDK (20 bytes, dueño en el desplazamiento 12); `LobbyCreated_t` llega como callback antes que `LobbyEnter_t`; el lobby público del otro proceso aparece en `RequestLobbyList` con sus datos y su filtro; `JoinLobby` entrega `LobbyEnter_t`; y con `auto_join` llega `GameServerChangeRequested_t` con `127.0.0.1:26900`.

Superada en el PC del propietario (2026-10-10) y en la CI de Windows (runs #14 y #16). **No cubre** los métodos planos `SteamAPI_ISteam*` que usa Steamworks.NET. Por eso no detectó el fallo con 7DTD real (ver `05-INTEGRACION-STEAM.md`).

### CI (`.github/workflows/build.yml`, windows-latest)

Se ejecuta en cada push a `main`, en cada PR y a mano. Pasos:
- Módulos nativos C++, filtro XInput y emulador de Steam (MSVC `/W4 /WX` + CTest).
- Pruebas .NET y Python, y documentación.
- Publicación autónoma win-x64 con las dos DLL nativas.
- **Prueba de humo** del ejecutable de diagnóstico (escaneo Steam + SDL3).
- **Prueba de arranque de la interfaz:** abre `SplitForge.exe`, selecciona con UI Automation las **3 secciones** (JUGAR, MANDOS, AVANZADO) y las **6 pestañas de AVANZADO** (falla si no son esas), guarda una captura de cada una (artefacto `captura-interfaz`, 7 días) y exige un cierre ordenado.

Historial:
- Primera ejecución (commit 3018ffa): verde; SDL3 3.5.0 inicializó en Windows (hilo a 646 Hz).
- Run #2 (d1614bb): **1000 Hz** tras usar plazos absolutos.
- Run #4: la prueba de arranque detectó un fallo real de cierre, que se corrigió.
- Run #16 (`783da1d`, `main` actual): verde.

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
| W16 | Filtro XInput | Con dos clientes sin EAC y el filtro asignado, cada mando controla solo su cliente; el estado de cada cliente muestra su ranura, «XInput real interceptada» y lecturas que suben; anotar si la ventana sin foco responde |
| W17 | Ranuras XInput | «Detectar» (o `SplitForge.Diagnostico.exe xinput`) identifica la ranura de cada mando al pulsar un botón, también con otra ventana enfocada |
| W18 | Bloqueo de ratón | Con pantalla dividida activa y el bloqueo (botón o Ctrl+Alt+L), el cursor no cruza a la otra ventana; al soltar o detener la sesión, vuelve a ser libre |
| W19 | Ajuste a ventana | Con «Mantener proporción» la ventana conserva su forma sin deformarse; la resolución sugerida por panel coincide con lo que encaja en el juego en modo ventana |
| W20 | Emulador de Steam | Instalar desde la pestaña 07; «Lanzar J1 y J2» abre dos instancias de 7DTD con identidades distintas que llegan al menú, en pantalla dividida y con su mando asignado; «Restaurar DLL original» deja el juego como estaba |
| W21 | Bloqueo de entrada | Con Ctrl+Alt+B (o el botón), el teclado y el ratón no llegan a los juegos ni al escritorio y los mandos siguen funcionando; Ctrl+Alt+B lo suelta sin teclas pegadas; detener la sesión o cerrar SplitForge también lo suelta |
| W22 | Tapar barra de tareas | Con la casilla activa, las ventanas ocupan el monitor completo y la barra de tareas no se ve; al detener, vuelven a su sitio y a su orden |

**Resultado físico conocido:**
- Prototipo: laboratorio v0.4 y Steam v0.5 confirmados.
- 0.7, PC del propietario (2026-10-10):
  - W13 con un cliente (informe real).
  - Parte de W02: hilo SDL3 a 1000 Hz, sin mandos.
  - **W20 superado (tarde):** tras generar la API plana y lanzar sin juego cruzado, J1 y J2 llegan al menú a la vez con perfiles distintos, colocados arriba/abajo, con el filtro cargado en ambos (J1 → mando 1, J2 → mando 2, lecturas subiendo). Por la mañana fallaba: 7DTD se cerraba al iniciar Steam.
  - **W16, primer intento fallido (propietario):** cada stick movía las dos ventanas. Corregido con ADR-021.
  - **W16 parcial (noche):** con dos clientes reales y los dos mandos conectados, cada cliente enumera un solo mando (InControl) y el filtro intercepta la XInput real. 
  - **W16 superado (propietario, noche):** con el paquete nuevo y «Lanzar J1 y J2», cada mando controla solo su ventana.
  - **W22 superado:** con los dos clientes arriba/abajo sobre el monitor completo, la franja de la barra de tareas (esquinas incluidas) es de la ventana de J2.
  - **W21:** pendiente de que el propietario lo pruebe (no se puede simular sin tocar su teclado y ratón).
  - **W14 superado (noche, sin servidor dedicado):** J1 aloja su partida y J2 entra sola en ella (ADR-022). En el registro del anfitrión: autenticación `k_EAuthSessionResponseOK`, «Player 'SplitForgeJ2' joined the game» y `PlayerSpawnedInWorld (reason: JoinMultiplayer)`. Falta una sesión de juego real de los dos jugadores.
- El resto, sin confirmar. W11 y W12 requieren instalar antes el servidor dedicado (no instalado).

## Benchmark cuando exista el primer adaptador validado

- Misma build, resolución, presets, escena, mandos, red y número de jugadores.
- Intentos alternos SplitForge ↔ referencia para minimizar deriva.
- FPS medio, 1 % low, frametime P50/P95/P99, RAM, VRAM, CPU por proceso, fallos.
- Sobrecarga del lanzador separada de la del juego.

## Evidencias por cambio

Cada entrega adjunta: pruebas ejecutadas y conteo, sistema, escenario real o simulado, archivos cambiados y estado de validación de hardware. Se registran en `01-MEMORIA.md` y `15-HISTORIAL.md`.
