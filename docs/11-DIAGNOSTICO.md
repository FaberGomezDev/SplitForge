# Guía de diagnóstico basada en evidencias

Herramienta principal (0.7): `SplitForge.Diagnostico.exe` en la carpeta del paquete (o `dotnet run --project src/SplitForge.Diagnostico -- <comando>`). Todos los comandos son de **solo lectura**.

## A. Los mandos no aparecen

1. `Win + R` → `joy.cpl`: ambos deben figurar y responder en *Probar*.
2. `SplitForge.Diagnostico.exe mandos --segundos 20`: muestra el motor (`SDL3 x.y.z`), cada mando, su ruta de lectura y los cambios en vivo.
3. Si Windows ve 2 y SDL3 ve 0: revisar Bluetooth/Steam Input (Steam puede capturar los mandos si está abierto con «PlayStation Configuration Support»). **No reinstalar drivers** sin evidencia.
4. Si aparece «No se encontró SDL3 nativo»: el paquete está incompleto; vuelve a descomprimirlo (debe existir `SDL3.dll` junto al `.exe`).
5. Prototipo Python: los pasos de `Diagnostico_Mandos.bat` siguen siendo válidos.

## B. Ambos aparecen, uno deja de moverse

- Pestaña 01: ¿su estado en vivo cambia? Si dice `SIN LECTURA`, el perfil es inválido para ese modelo → recalibrar.
- Tras reconectar, el ID cambia: reasignar en 02 («Reasignar mandos en vivo»).
- Probar «Lector: Gamepad» y «Joystick» para comparar.
- Si la UI lo ve pero el laboratorio no: revisar la línea `P1/P2` (señal que llega a la simulación).

## C. Disparo / gatillo no responden

- «Aprender» con Cruz o R2 en el mando afectado; R2 se calibra como eje (reposo, dirección, umbral).
- En modo Gamepad el disparo es el botón Sur (Cruz) fijo.

## D. Steam aparece pero el juego no se divide

- Detectar la biblioteca no significa que el juego esté abierto.
- Una ventana o un PID no es dos clientes; dos ventanas del mismo proceso tampoco.
- 2–4 PID del mismo exe **solo** permiten organizar ventanas; faltan mandos aislados y partida común.
- Pantalla completa exclusiva o procesos elevados pueden impedir mover la ventana: usar modo ventana/sin bordes del propio juego.

## E. 7 Days to Die

1. Pestaña 06 → «Comprobar»: revisa cada línea del preflight.
2. Servidor que no llega a «LISTO»: ver el registro en la propia pestaña (también en `%LOCALAPPDATA%\SplitForge\7dtd\logs`). La primera vez puede tardar varios minutos generando el mundo. Comprobar que el puerto (26900 por defecto) no esté en uso.
3. Cliente que no conecta: ¿EAC coincide con el servidor? (servidor sin EAC ⇒ cliente con «Launch game without EAC»). ¿Misma versión de juego y servidor? Actualiza ambos en Steam.
4. `SplitForge.Diagnostico.exe 7dtd` con el juego abierto guarda `SplitForge_Reporte_7DTD.json` (esquema v3). Si dice «Windows denegó la lectura de módulos», EAC está activo. Cómo leerlo:
   - `familias`: APIs de mando cargadas (`xinput`, `directinput`, `hid`, `incontrol`, `windows_gaming_input`...).
   - `origenes`: de dónde viene cada DLL de mando: `sistema`, `juego`, `steam` u `otro`. Una `xinput1_4.dll` con origen `juego` es el filtro instalado.
   - `filtro_xinput_cargado`: copias del filtro de SplitForge cargadas, verificadas por SHA-256.
   - `ranuras_xinput`: ranuras con mando según Windows.
   - `motor`: `unity` si carga `UnityPlayer.dll`.
   - Informe real del 2026-10-10 (v2, un cliente): `xinput1_3` + `xinput1_4`, `dinput8`, `hid`, `incontrolnative`; ver `05-INTEGRACION-STEAM.md`.
5. `SplitForge.Diagnostico.exe xinput` muestra las ranuras XInput con mando, la ranura de cada pulsación en vivo y el estado del filtro en cada cliente. Ese estado sale de `%LOCALAPPDATA%\SplitForge\xinput\estado-PID-<dll>.txt` y, para el jugador 2, de la carpeta por defecto del sandbox `SplitForge_J2`. Si un cliente asignado tiene las «lecturas» en 0 mientras el juego responde al mando, el juego no lee por esa DLL. El estado indica además si la XInput real está interceptada y cuántas lecturas llegaron por ella; «ya estaba cargada antes del filtro» significa que algún módulo pudo resolverla antes y leer sin filtrar (ADR-021).
6. **La ventana sin foco no responde a todos los botones, o su puntero parpadea o salta** (ADR-025):
   - El estado de **los dos** clientes debe decir «foco simulado · puntero propio». Si no, el filtro instalado es antiguo: JUGAR lo pone al día al lanzar.
   - En `estado-PID-<dll>.txt`, `puntero_propio=N` debe subir en el cliente que no tiene el foco: son las veces que el juego pidió, movió o encerró el puntero sin tocar el de Windows.
   - Si sigue fallando, el juego tiene su propio registro de entrada: lanzado con el argumento `-debuginput=verbose` anota en `Player.log` los cambios de foco («Focus:»), de cursor («CursorEnabled:») y de conjuntos de acciones. Anotar qué botones fallan y con qué pantalla abierta.
   - Un botón que la herramienta de los mandos convierte en tecla o ratón solo llega a la ventana activa: no pasa por XInput.
6. Anotar si la ventana **sin foco** sigue procesando el juego y los mandos: es la clave para el siguiente paso del módulo nativo.

## F. Emulador de Steam (pestaña 07)

1. La línea de estado de la pestaña dice si está instalado, con qué identidad y si la original está respaldada (`steam_api64_splitforge_original.dll`, junto a la DLL del juego; en 7DTD, `7DaysToDie_Data\Plugins\x86_64`).
2. `%LOCALAPPDATA%\SplitForge\steamemu.log`: una línea `SteamAPI_Init` y otra «iniciado: app=… steamid=… persona=…» por instancia. Si no aparecen, el juego no cargó la DLL emulada.
3. Registro de Unity del juego: `%USERPROFILE%\AppData\LocalLow\The Fun Pimps\7 Days To Die\Player.log` (y `Player-prev.log`). Si termina en `Crash!!!` con `steam_api64` arriba en la traza, el fallo está en el emulador.
   - Caso conocido y resuelto (2026-10-10): `SteamAPI_ISteamClient_GetISteamUser` ← `Steamworks.CSteamAPIContext:Init` (API plana sin `self`). Si vuelve a ocurrir con otro juego, comprobar con `steamemu.log` qué entradas llama («API plana: … [sin implementar]») y regenerar la API con `tools/generar_api_plana.py`.
4. Diálogo «Inicio de sesión de servicios en línea de Epic» con «Jugar sin conexión» desactivado: el cliente se abrió con juego cruzado (EOS) y Epic rechaza la identidad local. Ábrelo desde «Lanzar J1 y J2», que añade `-crossplatform=None -serverplatforms=Steam`.
5. «La DLL instalada no coincide» al instalar: algo cambió la DLL del juego (p. ej. «Verificar archivos» de Steam); SplitForge no la pisa. Si la DLL sigue siendo la que registró SplitForge, «Instalar / actualizar» la sustituye por la nueva.
6. «La DLL instalada no coincide con la registrada» al restaurar: lo mismo; SplitForge no restaura para no pisar cambios ajenos; verifica los archivos del juego en Steam.
7. Mientras el emulador esté instalado, el juego abierto desde Steam también usa la DLL emulada. Para jugar con tu cuenta de Steam, «Restaurar DLL original».
8. Ventana con el menú cortado por un lado: el panel es más estrecho que 16:9. «Lanzar J1 y J2» y «Dividir pantalla» ya aplican el mínimo 16:9 de 7DTD; arriba/abajo da paneles más anchos.

## G. Reportes seguros

Compartir solo los JSON generados (`SplitForge_Reporte_*.json`); revisarlos si alguien los editó. No compartir `settings*.json`, registros completos, capturas con nombres de usuario ni rutas personales.
