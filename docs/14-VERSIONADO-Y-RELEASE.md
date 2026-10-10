# Versionado, compilación y entregas

## Versión y naturaleza del paquete

Esta entrega es **SplitForge 0.6.0 (código original) + repositorio documental**. No modifica el número mostrado por la GUI porque no altera su funcionalidad. El paquete es **fuente completo**, no binario `.exe` autocontenido.

## Política propuesta

- `MAJOR.MINOR.PATCH` desde el primer adaptador estable. Hasta entonces `0.x` experimental.
- `0.6.0+docs` es designación de distribución documental, no un nuevo parche funcional.
- Mantener `CLAUDE.md`, README, memoria y changelog sincronizados con el código.

## Checklist para release Windows

1. Ejecutar `python -m unittest discover -s tests -v`; registrar cantidad y resultado.
2. Ejecutar `python tools/check_docs.py` y resolver enlaces rotos.
3. En una PC Windows 11, probar arranque, mandos PS4, mundo compartido, cierre y restauración.
4. Validar los reportes Steam con datos de prueba, verificar que no filtran rutas/cuentas.
5. Probar instalación limpia de `.venv` con Python compatible y wheel `pygame-ce` si se requiere.
6. Empaquetar **sin** `.venv`, `__pycache__`, `.git`, `.pyc`, settings, logs, reportes ni secretos.
7. Generar SHA-256 del ZIP y documentar cambios y limitaciones.
8. No declarar un adaptador como validado antes de probarlo en build real y hardware real.

## Cómo instalar paquete completo sobre una instalación probada

**No sobrescribir a ciegas** la carpeta que ya tiene `.venv` y settings. Descomprimir en carpeta nueva y, si se desea aprovechar Python ya instalado, crear un nuevo entorno o copiar la carpeta `.venv` del mismo intérprete únicamente tras comprobación de compatibilidad. La configuración `%LOCALAPPDATA%` se conserva por separado.

## Cómo actualizar la memoria de próxima sesión

Aplicar [`templates/ENTREGA.md`](templates/ENTREGA.md): qué cambió, por qué, qué pruebas, restricciones nuevas, cuál es el siguiente paso. No permitir que documentación prometa funcionalidades futuras como actuales.
