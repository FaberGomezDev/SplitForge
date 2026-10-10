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
4. `SplitForge.Diagnostico.exe 7dtd` con el juego abierto: guarda `SplitForge_Reporte_7DTD.json` con las APIs de mando cargadas por cada cliente. Si dice «Windows denegó la lectura de módulos», EAC está activo.
5. Anotar si la ventana **sin foco** sigue procesando el juego y los mandos: es la clave para el módulo nativo.

## F. Reportes seguros

Compartir solo los JSON generados (`SplitForge_Reporte_*.json`); revisarlos si alguien los editó. No compartir `settings*.json`, registros completos, capturas con nombres de usuario ni rutas personales.
