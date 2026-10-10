# Riesgos y preguntas abiertas

Resolver con evidencia concreta antes de programar sobre suposiciones.

## 7 Days to Die (prioritario)

- ~~¿Qué APIs de mando carga 7DTD en el PC del propietario?~~ → **Resuelto con un cliente (2026-10-10):** `xinput1_3` + `xinput1_4`, DirectInput, HID, InControl. Falta repetir con dos clientes (informe v3).
- ¿Por qué DLL lee 7DTD los mandos: XInput, DirectInput o HID? → «lecturas» de cada copia del filtro en W16.
- Los mandos llegan como `Xbox 360 Controller`: ¿quedan ocultos los PS4 originales para el juego (sin duplicados por HID/DirectInput)?
- ¿La ventana **sin foco** sigue simulando el juego y leyendo mandos? (Unity puede pausar o ignorar la entrada en segundo plano).
- ¿Abre 7DTD una segunda instancia en la misma sesión de Windows (en Sandboxie-Plus u otro usuario)?
- ¿Steam Input o la superposición de Steam (`gameoverlayrenderer64.dll`, cargada en 7DTD) interfieren con el filtro? Si las lecturas no suben, probar con Steam Input desactivado para 7DTD.
- ¿Coinciden versión de cliente y servidor tras las actualizaciones de Steam?

## Diseño técnico

- Filtro XInput implementado (experimental): ¿lo carga 7DTD? → el informe v3 lo dice (`filtro_xinput_cargado`). ¿Basta para aislar los mandos? → W16.
- Clientes en Sandboxie: el estado del filtro queda dentro del sandbox (escritura virtualizada); SplitForge lo lee de la carpeta por defecto de `SplitForge_J2`. Si el sandbox usa otra ruta raíz, no se verá (la asignación sí se lee).
- Identificador estable tras reconexión Bluetooth sin guardar seriales en reportes (hoy: reasignación explícita).
- Fullscreen exclusivo, DPI mixto y varios monitores (hoy: área útil del monitor principal).
- ~~Cierre de golpe con clientes sin bordes~~ → resuelto: `SessionRecovery` guarda posiciones/estilos y los restaura al siguiente arranque si el handle sigue siendo del mismo proceso.
- Medir el rendimiento con dos instancias en 8 GB de VRAM antes de proponer perfiles.
- ¿Ruta Gamepad de SDL3 para PS4 como valor por defecto? (probar en hardware).

## Prioridad

P0: validar la 0.7 en el PC del propietario. P1: diagnóstico 7DTD. P2: módulo nativo de un mando por ventana. P3: métricas. P4: firma y distribución.
