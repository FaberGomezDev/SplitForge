# Plan de investigación del primer adaptador real — 7 Days to Die

**Estado: EN INVESTIGACIÓN. No es un handler funcional.**

## Qué tiene que pasar para considerarlo un adaptador de SplitForge

1. **Detección**: comprobar AppID 251570, ruta real, versión del juego y su ejecutable, sin modificar Steam.
2. **Sesión soportada**: validar en un entorno de prueba si dos clientes autorizados pueden conectarse a **un mismo servidor/partida local** bajo las condiciones del juego. Distinguir LAN sin Internet de partidas que dependen de Steam u otros servicios.
3. **Identidades y guardados**: no reutilizar o falsificar sesiones. Si exige dos cuentas, avisar y solicitar que se inicien de manera legítima. No compartir simultáneamente ficheros de guardado no diseñados para concurrencia.
4. **Controles independientes**: medir si cada instancia detecta el mismo mando. Seleccionar un mecanismo permitido y estable de aislamiento de entrada **sin eludir seguridad ni antitrampas**.
5. **Pantalla dividida**: solo cuando los dos clientes y los dos mandos funcionen, colocar las ventanas con la capa de la v0.4, soportando horizontal/vertical y restauración al cerrar.
6. **Rendimiento**: comparar cada instancia y el uso total de VRAM (RTX 4060 8 GB), RAM (32 GB), CPU (Ryzen 7 7700X), frametimes y picos de memoria. No asumir 60 FPS hasta medir.
7. **Cierre y recuperación**: conservar ventanas, no terminar procesos ajenos, y verificar la integridad de partidas/sesiones tras una interrupción.

## Criterios de corte

Si no permite dos clientes/identidades en un mismo PC de forma autorizada, o si el modo offline requerido no existe en la configuración probada, el adaptador quedará **No soportado o requiere modo en línea**, con explicación. No intentar desactivar protección o falsificar licencias.

## Qué necesitamos del equipo de pruebas

El reporte de `Escanear_Steam.bat`, especialmente si aparece **7 Days to Die**. Ese archivo únicamente enumera software instalado y algunos nombres de ejecutables. No incluye rutas personales ni información de cuentas.
