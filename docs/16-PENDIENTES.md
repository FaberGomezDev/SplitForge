# Riesgos y preguntas abiertas

Resolver con evidencia concreta antes de programar sobre suposiciones.

## 7 Days to Die (prioritario)

- ¿Qué APIs de mando carga 7DTD en el PC del propietario? → `SplitForge.Diagnostico.exe 7dtd` (con uno y con dos clientes).
- ¿La ventana **sin foco** sigue simulando el juego y leyendo mandos? (Unity puede pausar o ignorar la entrada en segundo plano).
- ¿Abre 7DTD una segunda instancia en la misma sesión de Windows (con otra cuenta en Sandboxie-Plus u otro usuario)?
- ¿Steam Input captura los PS4 en ambos Steam? ¿Conviene desactivarlo para 7DTD y exponer los PS4 como XInput (DS4Windows) para el filtro por proceso?
- ¿Coinciden versión de cliente y servidor tras las actualizaciones de Steam?
- Segunda cuenta de Steam con el juego: **pendiente del propietario**.

## Diseño técnico

- Método del módulo nativo: filtro XInput por proceso, simulación de foco, o ambos (según el diagnóstico).
- Identificador estable tras reconexión Bluetooth sin guardar seriales en reportes (hoy: reasignación explícita).
- Fullscreen exclusivo, DPI mixto y varios monitores (hoy: área útil del monitor principal).
- ~~Cierre de golpe con clientes sin bordes~~ → resuelto: `SessionRecovery` guarda posiciones/estilos y los restaura al siguiente arranque si el handle sigue siendo del mismo proceso.
- Medir el rendimiento con dos instancias en 8 GB de VRAM antes de proponer perfiles.
- ¿Ruta Gamepad de SDL3 para PS4 como valor por defecto? (probar en hardware).

## Prioridad

P0: validar la 0.7 en el PC del propietario. P1: diagnóstico 7DTD y segunda cuenta. P2: módulo nativo de un mando por ventana. P3: métricas. P4: firma y distribución.
