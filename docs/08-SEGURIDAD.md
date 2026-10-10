# Seguridad, compatibilidad y límites éticos

## Principios obligatorios

- No manipular DRM, licencias, tokens, sesiones de cuentas ni antitrampas. **Prohibidos los emuladores de Steam** (Goldberg u otros) y cualquier sustitución de `steam_api*.dll`.
- No duplicar la identidad de una cuenta (p. ej. dos clientes con la misma cuenta en modo sin conexión).
- EAC solo se desactiva con la **opción oficial** del juego («Launch game without EAC»); nunca por otros medios.
- No modificar binarios de Steam ni instalar drivers de kernel por defecto.
- No cerrar a la fuerza un juego ajeno. Solo se terminan procesos **creados por SplitForge** (visores, servidor 7DTD) tras un cierre/apagado normal fallido.
- No alterar guardados/configuración de juegos. El servidor 7DTD usa configuración y `UserDataFolder` **propios** de SplitForge.
- No recopilar ni exportar rutas personales, identificadores Steam, GUID persistentes, MAC ni títulos de ventana en reportes compartibles.
- No pedir privilegios de administrador (`asInvoker` en el manifiesto). Sin `shell=True`/`UseShellExecute` con datos externos; argumentos siempre por `ArgumentList`.

## Superficies de riesgo (0.7)

| Superficie | Salvaguarda actual | Riesgo / mejora futura |
| --- | --- | --- |
| Lectura Steam (VDF/ACF) | Parser propio tolerante, límites de tamaño y profundidad, sin escritura | Cambios de formato de Valve |
| Preflight | Carpeta confinada a `steamapps/common`, resolución de enlaces simbólicos | Uniones creadas después del escaneo |
| Informes | Tipos públicos dedicados (`ToPublic`, `SteamPublicReport`, `SevenDaysDiagnosticReport`) sin rutas ni títulos | Revisar cada campo nuevo |
| Ventanas | Revalidación PID+exe+HWND antes de mover; captura de todas las posiciones y estilos antes de cambiar nada; restauración | Pantalla completa exclusiva, procesos elevados |
| Modo sin bordes | Estilos originales guardados y restaurados (también al cerrar SplitForge); copia en disco `sesion-ventanas.json` para recuperar tras un cierre inesperado, validando que el handle sigue siendo del mismo PID | Juegos que reescriben su propio estilo |
| Servidor 7DTD | Configuración propia; `ServerVisibility=0`; telnet sin contraseña ⇒ solo loopback; apagado ordenado antes de terminar | Firewall de Windows puede pedir permiso: aceptar solo redes privadas |
| Sandboxie-Plus | SplitForge solo lanza Steam en un sandbox; el usuario inicia sesión con su otra cuenta; no se ven ni guardan credenciales | Políticas de Steam/antitrampas sobre sandboxes |
| Visores del laboratorio | IPC binario con validación estricta; procesos propios | — |
| Configuración | `settings.v2.json` atómico; `settings.json` del prototipo solo lectura | — |
| Diagnóstico de módulos | `PROCESS_QUERY_INFORMATION | PROCESS_VM_READ` solo para enumerar nombres de módulos; con EAC activo Windows lo deniega y se informa | — |
| Filtro XInput nativo (ADR-015) | Se instala con consentimiento solo si ningún cliente tiene EAC cargado; nunca sobrescribe DLL del juego; retira solo lo propio (SHA-256); carga la DLL real por ruta absoluta de System32; sin E/S en `DllMain`; compilado con `/guard:cf`, ASLR y DEP | Validar en hardware; simulación de foco aún no implementada |

## Reglas para una integración nueva

1. Preflight de derechos/modos de juego; no lanzar si falla.
2. Privilegios mínimos; separar control de cuentas/juego.
3. Plan de restauración antes de modificar ventanas o configuración propia.
4. Nada de inyección en juegos protegidos; si una modalidad oficial lo permite, documentar cómo activarla desde el juego.
5. No persistir credenciales ni secretos.
6. Mantener un modo diagnóstico de solo lectura y una salida de bloqueo explicativa.

## Comunicación clara al usuario

Si solo se detectan dos ventanas: «dos procesos detectados y colocados», no «juego compatible». Si los mandos no están aislados: la operación es incompleta. Si el juego requiere Internet, no etiquetarlo como offline. Si hacen falta dos cuentas, decirlo antes de empezar.
