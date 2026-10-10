# Roadmap técnico con puertas de aceptación

> Código **0.7.0**. Las fases no están terminadas salvo mención explícita. Las versiones futuras son tentativas.

## P0 — Base técnica eficiente y laboratorio estable

- [x] Dos PS4 Bluetooth leídos con SDL en el equipo del usuario (prototipo Python).
- [x] Mundo simulado una sola vez, dos visores y estados separados.
- [x] Reconstrucción en .NET 10 + SDL3 + WPF (ADR-010): entrada en hilo dedicado, simulación a 120 Hz, IPC binario, visores con GPU.
- [x] Pruebas automáticas (106 xUnit + 77 unittest) y CI en Windows con paquete autónomo.
- [x] Registro local con identificador de sesión, sin datos privados.
- [ ] **Validar la 0.7 en el PC del propietario** (W01–W07 de `07-PRUEBAS.md`).
- [ ] Automatizar pruebas Windows end-to-end (captura de ventanas, reconexión).

**Criterio de salida:** dos jugadores mueven y disparan por separado en la 0.7 durante una sesión sostenida, repetible tras reinicio, sin cambiar mandos ni perder ventanas.

## P1 — Juego real elegido: 7 Days to Die

- [x] Juego instalado en el PC de pruebas; arranca sin EAC con la opción oficial.
- [x] Escenario legítimo definido: servidor dedicado oficial + un cliente por cuenta (ADR-013).
- [x] Preflight, servidor local aislado, Sandboxie-Plus para el Steam del jugador 2, pantalla dividida (implementado, sin validar).
- [ ] `SplitForge_Reporte_7DTD.json` con uno y con dos clientes abiertos (APIs de mando, ¿la ventana sin foco sigue funcionando?).
- [ ] **Segunda cuenta de Steam con 7DTD** (bloqueante para dos jugadores reales).

**Criterio de salida:** dos clientes con cuentas distintas conectados al servidor local de SplitForge.

## P2 — Un mando por ventana dentro del juego (módulo nativo C++)

- [ ] Elegir método según el diagnóstico (filtro XInput por proceso; simulación de foco si el juego exige foco).
- [ ] Implementar `native/` en C++20 con ADR, pruebas y activación solo con EAC desactivado por la opción oficial.
- [ ] Reversible: instalar/retirar con consentimiento, sin modificar archivos existentes del juego.
- [ ] Verificar en Windows 11 con dos PS4 reales: cada mando controla solo su cliente.

**Criterio de salida:** una partida de 7DTD con J1 y J2 visibles entre sí, entrada aislada y cierre seguro → estado `INPUT_AISLADO_VALIDADO`.

## P3 — Instrumentación y optimización

- [ ] Frametimes por instancia (PresentMon/ETW), CPU, RAM, VRAM y latencia de entrada.
- [ ] Perfiles Rendimiento / Equilibrado / Calidad solo con opciones que el juego soporte.
- [ ] Comparar con Nucleus Co-op en las mismas condiciones; publicar datos y variabilidad.

**Criterio de salida:** métricas respaldadas por evidencia, sin declarar FPS hipotéticos.

## P4 — Plataforma extensible y distribución

- [x] Paquete autónomo win-x64 en cada push (artefacto) y releases con etiqueta `v*`.
- [ ] Firma de código del ejecutable; actualizador seguro.
- [ ] Más adaptadores con matriz de compatibilidad por build; 3–4 jugadores donde el juego y los recursos lo permitan.

## Evitar estos anti-patrones

- Emuladores de Steam o «una sola copia para varios jugadores».
- Lanzar dos ejecutables Steam a la fuerza como sustituto del multijugador.
- Un mando virtual sin comprobar que no mueve ambos clientes.
- Detectar dos ventanas y anunciar compatibilidad.
- Probar solo con pruebas simuladas y declarar funcional Windows/Steam.

## Orden concreto del próximo sprint

1. Propietario: descargar `SplitForge-win-x64`, validar mandos y laboratorio, enviar `SplitForge_Reporte_7DTD.json`.
2. Con el informe: ADR del método de aislamiento y primera versión del módulo nativo.
3. Con la segunda cuenta: prueba real de dos jugadores y registro de evidencias en `05-INTEGRACION-STEAM.md`.
