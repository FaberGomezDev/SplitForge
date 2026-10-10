# Roadmap técnico con puertas de aceptación

> La versión de código continúa siendo **0.6.0**. Las fases no están terminadas salvo mención explícita. El número de versión futuro es **tentativo**, no una promesa.

## P0 — Preservar el laboratorio estable

- [x] Dos PS4 Bluetooth leídos con SDL en equipo del usuario.
- [x] Mundo simulado una sola vez, dos visores y estados separados.
- [x] Pruebas unitarias de regresión y diagnóstico de Steam.
- [ ] Formalizar y automatizar pruebas Windows end-to-end repetibles (captura P1/P2, reinicio, reconexión, restauración de ventanas).
- [ ] Agregar logging estructurado con correlación por sesión, sin datos privados.

**Criterio de salida:** dos jugadores mueven y disparan por separado durante sesión sostenida, repetible tras reinicio, sin cambiar mandos ni perder ventanas.

## P1 — Elegir juego Steam realmente instalado

- [ ] Obtener reportes Steam de la PC de pruebas.
- [ ] Elegir título y build exacta, revisar EULA/requisitos/servidor y dos identidades legítimas si procede.
- [ ] Verificar si modo offline/LAN existe realmente en ese build con dos clientes.
- [ ] Registrar un manifiesto de investigación: requisitos, bloqueos y alternativa legal.

**Criterio de salida:** existe un escenario realista, documentado y permitido para la prueba de dos jugadores; si no, cambiar de juego.

## P2 — Primer adaptador comercial (candidato 7DTD)

- [ ] Implementar `GameAdapter` con preflight / start / health / stop / restore, manteniendo modo solo lectura hasta preflight satisfactorio.
- [ ] Probar dos clientes autorizados que compartan **la misma partida**.
- [ ] Determinar y validar mecanismo por juego de separación de entrada.
- [ ] Ubicar ventanas, restaurar posiciones, proteger guardados y detectar fallos.
- [ ] Verificar en Windows 11 con dos PS4 reales.

**Criterio de salida:** una partida de juego comercial con P1 y P2 visibles entre sí, entrada aislada y cierre seguro. Si depende de Internet, rotular claramente `ONLINE REQUIRED`.

## P3 — Instrumentación y optimización

- [ ] Crear medición repetible de frametimes, tiempo CPU por proceso, RAM, presupuesto/uso VRAM y pérdidas de input.
- [ ] Perfiles `Rendimiento / Equilibrado / Calidad` únicamente con parámetros que el juego soporte.
- [ ] Comparar contra Nucleus con mismas condiciones; publicar datos y variabilidad.
- [ ] Estudiar composición GPU opcional **solo si** reduce problemas medidos.

**Criterio de salida:** métricas respaldadas por evidencia, sin declarar FPS hipotéticos.

## P4 — Plataforma extensible y distribución

- [ ] Manifiestos versionados, validación de contratos, plugin sandbox/permissions según riesgo.
- [ ] Actualizador seguro/firmado, diagnósticos de usuario, empaquetado ejecutable.
- [ ] Nuevos juegos con matriz de compatibilidad por build, pruebas de regresión automatizadas.
- [ ] 3–4 jugadores solo si recursos y juegos permiten esa cantidad.

## Evitar estos anti-patrones

- Lanzar dos ejecutables Steam “a la fuerza” como sustituto de coop.
- Usar un controlador virtual sin comprobar que no mueve ambos clientes.
- Reescribir en C++ porque “es más rápido” sin perf profiling.
- Detectar dos ventanas y anunciar compatibilidad verificada.
- Probar solo con pruebas simuladas y declarar funcional Windows/Steam.

## Orden concreto del próximo sprint

1. Analizar reportes locales Steam y escoger juego real.
2. Especificar preflight verificable y estados `BLOQUEADO` / `EXPERIMENTAL` / `VALIDADO`.
3. Agregar tests sobre casos de error / replay de reportes sin datos privados.
4. **Solo entonces** implementar adapter piloto, no duplicar aún sesiones de Steam de forma general.
