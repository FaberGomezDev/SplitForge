# Roadmap técnico con puertas de aceptación

> Código **0.7.0**. Las fases no están terminadas salvo mención explícita. Las versiones futuras son tentativas.

## P0 — Base técnica eficiente y laboratorio estable

- [x] Dos PS4 Bluetooth leídos con SDL en el equipo del usuario (prototipo Python).
- [x] Mundo simulado una sola vez, dos visores y estados separados.
- [x] Reconstrucción en .NET 10 + SDL3 + WPF (ADR-010): entrada en hilo dedicado, simulación a 120 Hz, IPC binario, visores con GPU.
- [x] Pruebas automáticas (158 xUnit + 77 unittest + pruebas de humo C++) y CI en Windows con paquete autónomo.
- [x] Registro local con identificador de sesión, sin datos privados.
- [x] Hilo de entrada SDL3 3.5.0 a 1000 Hz medido en el PC del propietario (2026-10-10, sin mandos).
- [ ] **Validar la 0.7 en el PC del propietario con los dos PS4** (W01–W07 de `07-PRUEBAS.md`).
- [ ] Automatizar pruebas Windows end-to-end (captura de ventanas, reconexión).

**Criterio de salida:** dos jugadores mueven y disparan por separado en la 0.7 durante una sesión sostenida, repetible tras reinicio, sin cambiar mandos ni perder ventanas.

## P1 — Juego real elegido: 7 Days to Die

- [x] Juego instalado en el PC de pruebas; arranca sin EAC con la opción oficial.
- [x] Escenario definido: servidor dedicado oficial + un cliente por jugador (ADR-013).
- [x] Preflight, servidor local aislado, Sandboxie-Plus para el Steam del jugador 2, pantalla dividida (implementado, sin validar).
- [x] `SplitForge_Reporte_7DTD.json` con **un** cliente sin EAC (2026-10-10): `xinput1_3` + `xinput1_4`, DirectInput, HID, InControl; mandos como `Xbox 360 Controller`.
- [x] Emulador de Steam (componente principal, pestaña 07) para abrir J1 y J2 con identidades distintas sin un segundo Steam.
- [x] Primera prueba real del emulador con 7DTD (2026-10-10): carga J1/J2, pero el juego se cierra en `SteamAPI_ISteamClient_GetISteamUser`.
- [ ] Emulador: API plana con `self` y métodos `SteamAPI_ISteam*` que 7DTD usa al arrancar; prueba de humo por la API plana; repetir W20.
- [ ] Instalar el servidor dedicado (AppID 294420) en el PC de pruebas (no instalado el 2026-10-10).
- [ ] Informe v3 con **dos** clientes abiertos y anotación de si la ventana sin foco sigue funcionando.

**Criterio de salida:** dos clientes conectados al servidor local de SplitForge.

## P2 — Un mando por ventana dentro del juego (módulo nativo C++)

- [x] Primer método implementado: filtro XInput por proceso en C++20 (`native/SplitForge.XInputFilter`, ADR-015), con prueba de humo en la CI de Windows.
- [x] Reversible: instalar/retirar con consentimiento, sin sobrescribir archivos del juego (manifiesto con SHA-256).
- [x] Confirmado con el diagnóstico real que 7DTD carga XInput (`xinput1_3` y `xinput1_4`).
- [x] Instrumentación para la prueba real: estado por DLL con contador de lecturas, DLL real comprobada en System32, detección de la ranura XInput de cada mando, actualización segura del filtro, informe v3 y comando `xinput`.
- [ ] Comprobar con dos clientes (W16) si las lecturas suben en ambos y si la ventana sin foco lee su mando; decidir el siguiente paso con esa evidencia.
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

- Lanzar dos ejecutables Steam a la fuerza como sustituto del multijugador.
- Un mando virtual sin comprobar que no mueve ambos clientes.
- Detectar dos ventanas y anunciar compatibilidad.
- Probar solo con pruebas simuladas y declarar funcional Windows/Steam.

## Orden concreto del próximo sprint

1. ~~Propietario: enviar `SplitForge_Reporte_7DTD.json`~~ → recibido (2026-10-10, un cliente sin EAC).
2. Desarrollo: corregir la API plana del emulador (`self` + métodos `SteamAPI_ISteam*` de arranque), con prueba de humo por la API plana.
3. Propietario: descargar el `SplitForge-win-x64` de `main` (o `Compilar_SplitForge.bat`).
   - Con los clientes cerrados, «Instalar filtro».
   - Abrir dos clientes sin EAC: pestaña 07 con el emulador corregido, o un segundo Steam en Sandboxie-Plus.
   - Pulsar «Detectar» y «Asignar».
   - Comprobar las lecturas y el control de cada ventana (W16) y guardar un informe v3.
4. Con esa evidencia: registrar `INPUT_AISLADO_VALIDADO` o decidir el siguiente método (ADR) en `05-INTEGRACION-STEAM.md`.
