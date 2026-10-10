# Riesgos y preguntas abiertas

Estas preguntas deben resolverse con evidencia concreta antes de programar una integración comercial, no con suposiciones.

## Datos de juego de referencia

- ¿Cuál es el primer juego **instalado** en el PC de pruebas? Obtener `SplitForge_Reporte_Steam.json`.
- ¿Qué versión/build exacta, ejecutables y launchers presenta? Obtener informe de ventanas y verificar visualmente.
- ¿Permite dos instancias/clientes **autorizados**, posiblemente con cuentas diferentes, en la misma computadora?
- ¿Permite conexión a la misma sesión LAN sin Internet, o necesita plataformas/autenticación online?
- ¿Qué ocurre con el guardado local al abrir dos clientes? No probar sin respaldo.
- ¿Existe una opción oficial (si procede) para jugar en modo privado sin protecciones online activadas? Nunca bypassear.

## Diseño técnico

- ¿Cómo se separa físicamente la visibilidad de cada mando para el cliente seleccionado sin interferir con Steam Input o seguridad del juego?
- ¿Qué identificador estable se usa tras reconexión Bluetooth sin almacenar seriales en reportes?
- ¿Cómo negociar fullscreen/borderless, DPI y múltiple monitor sin perder ventana?
- ¿Qué hacer si la GUI muere y quedan dos clientes abiertos?
- ¿Cómo soportar rendimiento variable de un mismo juego con 8 GB VRAM sin conjeturas?
- ¿Necesitamos compositor GPU? Decidir tras pruebas, no antes.

## Prioridad

P0: mantener los dos mandos y mundo compartido funcionales. P1: seleccionar un juego y establecer legalidad y escenario. P2: dos clientes y mundo real. P3: optimización. P4: plataforma de adaptadores y distribución instalable.
