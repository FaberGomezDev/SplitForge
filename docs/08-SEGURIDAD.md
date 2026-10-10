# Seguridad, compatibilidad y límites éticos

## Principios obligatorios

- No manipular DRM, licencias, tokens, sesiones de cuentas, ni antitrampas. No afirmar que el modo sin conexión de Steam equivale a soporte LAN del juego.
- No modificar binarios Steam ni instalar drivers de kernel por defecto.
- Usar opciones autorizadas y oficiales del juego cuando existan. Si un juego impide varios clientes válidos, marcarlo como `BLOQUEADO`.
- No cerrar a la fuerza un juego ajeno. El SessionManager solo fuerza terminación de procesos de prueba propios y tras intento de cierre normal.
- No alterar guardados/configuración de juegos sin backup explícito, verificación de cambios, restauración y consentimiento del usuario.
- No recopilar ni exportar rutas personales, identificadores Steam, GUID persistentes, MAC o títulos de ventana en reportes compartibles.
- No iniciar procesos externos elevados ni pedir privilegios admin sin motivo documentado. Evitar `shell=True`, rutas inseguras y argumentos concatenados de datos externos sin validación.

## Superficies de riesgo existentes

| Superficie | Salvaguarda actual | Riesgo / mejora futura |
| --- | --- | --- |
| Lectura Steam ACF | `steam_library.py` solo lee manifestos | VDF podría cambiar; no confiar en rutas de manifest sin validar |
| Preflight | `steam_preflight.py` valida carpeta dentro de `steamapps/common` | Archivos vinculados/enlaces, builds y ejecutables en subcarpetas |
| Informes | `public_report()` y `public_dict()` eliminan paths/títulos | Revisar cada nuevo campo antes de compartir |
| Organización Steam | `steam_windows.py` exige 2 PID mismo EXE, revalida | PID reciclados, acceso denegado, ventana protegida |
| Ejecutable genérico experimental | `SessionManager.start_external()` permite doble arranque de exe elegido manualmente | No usar para games Steam protegidos, no asumir guardados independientes; futuro: restricciones adicionales y advertencias fuertes |
| Ventanas Win32 | Guardar/restaurar HWND | Restauración best effort, falla con fullscreen/procesos elevados |
| Mandos | SDL/WinMM local, archivos settings | IDs cambiantes, conflicto Steam Input; no aisla juegos externos |
| IPC local | stdin texto JSON desde padre a hijos propios | Validar tamaños y schema antes de extensiones futuras |

## Reglas para una integración nueva

1. Realizar preflight de derechos/modos de juego y no lanzar si falla.
2. Solicitar privilegios mínimos, separar proceso de control de cuentas/juego.
3. Calcular plan de restauración antes de modificar ventanas/configuración.
4. No inyectar DLL en juegos protegidos ni desactivar protecciones; si es necesaria una modalidad oficialmente permitida, documentar cómo activarla desde el juego.
5. No persistir credenciales ni secretos.
6. Mantener un modo diagnóstico solo lectura y una salida de bloqueo explicativa.

## Comunicación clara al usuario

Si el programa solo detecta dos ventanas: decir "dos procesos detectados y colocados", no "juego compatible". Si los dos mandos no están aislados: la operación es incompleta. Si el juego requiere Internet, no etiquetarlo como offline.
