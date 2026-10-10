# Guía de diagnóstico basada en evidencias

## A. Los mandos PS4 no aparecen

1. Abrir `Win + R` → `joy.cpl`. Comprobar que ambos figuren y pulsen botones en *Probar*.
2. Desde el `.venv` de SplitForge ejecutar:

   ```powershell
   .\.venv\Scripts\python.exe -c "import pygame; pygame.init(); pygame.joystick.init(); print('SDL', pygame.get_sdl_version(), 'MANDOS', pygame.joystick.get_count()); [print(i, pygame.joystick.Joystick(i).get_name()) for i in range(pygame.joystick.get_count())]"
   ```
3. Si SDL = 0 y Windows = 2, investigar enumeración/Steam Input/conexión Bluetooth, **no reinstalar pygame-ce automáticamente**.
4. Si SDL = 2 pero GUI = 0, ejecutar diagnóstico del motor:

   ```powershell
   .\.venv\Scripts\python.exe -c "from splitforge.gamepads import ControllerManager; m=ControllerManager(); print(m.backend_name, m.warning, [(p.instance_id,p.name) for p in m.devices()]); m.close()"
   ```
5. Si GUI lista ambos pero no registra X/Y: alternar `Lector SDL` → `joystick`, revisar `Escanear_Movimiento.bat` y archivos del usuario; no cambiar los drivers del sistema sin diagnóstico.

## B. Ambos aparecen, uno deja de moverse

- Verificar ID asignado a J1/J2. Tras reconexión los IDs pueden cambiar.
- Probar movimiento en pestaña Controles por separado.
- En Mundo Compartido mirar línea de estado `P1`/`P2`, `X`/`Y` y `SIN LECTURA`.
- Usar «Reasignar mandos» si hubo nueva enumeración; observar si segundo mando mueve actor equivocado.
- Si el segundo mando funciona en UI pero no en mundo, instrumentar `SessionManager.route_shared_inputs()` y pruebas con estados sintéticos **antes** de cambiar SDL.

## C. Disparo / gatillo no responden

- Usar función «Aprender» con botón Cruz o R2 en el mando afectado.
- Evaluar lectura como botón digital o eje analógico (reposo, dirección, umbral).
- Comprobar estado `a_pressed` y resultado en mundo compartido.

## D. Steam aparece pero el juego no se divide

- Detectar biblioteca no significa que el juego esté abierto.
- Una ventana o un PID no es dos clientes.
- Dos ventanas del mismo proceso tampoco son dos clientes distintos.
- Dos PID del mismo ejecutable **solo permiten organización visual**; aún faltan mandos aislados y partida común.
- Fullscreen exclusivo o procesos elevados pueden no permitir reubicación/inspección; usar opciones normales de ventana del propio juego si existen.
- Si requiere cuentas diferentes o modo online, no forzar una sesión offline inexistente.

## E. Reportes seguros

Usar `Escanear_Steam.bat` y `Analizar_Ventanas_Steam.bat`. Compartir solo archivos JSON generados por esas utilidades; revisar contenido si han sido editadas por terceros. No compartir `settings.json`, logs completos, capturas con nombres de usuario ni rutas personales.
