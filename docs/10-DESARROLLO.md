# Desarrollo y ejecución

## Requisitos

- Windows 10/11 (Windows 11 es el equipo de pruebas disponible).
- Python >=3.11 con Tkinter y venv (en el equipo de referencia se usó Python 3.13).
- Para PS4 genérico por Bluetooth: `pygame-ce==2.5.8` que ya incorpora SDL en el wheel Windows.
- Conectar dos mandos; Steam solo es necesario para la evaluación de biblioteca/juegos reales.

## Primer arranque desde ZIP completo

1. Descomprimir en una **carpeta nueva**, evitando mezclar con archivos de otra release.
2. Ejecutar `Instalar_Todo.bat` **solo** si no tienes `.venv` con pygame-ce funcionando en ese directorio; si se migran archivos desde instalación anterior, conservarla sin publicar el ZIP con el entorno binario.
3. Ejecutar `Iniciar_SplitForge.bat`.
4. En **01 CONTROLES**, confirmar X/Y y asignar mandos distintos. Probar **02 MUNDO COMPARTIDO**.
5. Para Steam usar pestañas **05 STEAM** y **06 VENTANAS STEAM**. No simular compatibilidad por ver una carpeta/ventana.

> Esta es una **versión completa de código fuente**, no un `.exe` autónomo. Python y pygame-ce se instalan localmente si no están disponibles. No ejecutar `Instalar_SDL_Opcional.bat` si el entorno ya reconoce los mandos.

## Ejecutar pruebas

```powershell
.\.venv\Scripts\python.exe -m unittest discover -s tests -v
.\.venv\Scripts\python.exe tools\check_docs.py
```

En entorno de desarrollo distinto de Windows se pueden ejecutar tests Python puros pero **no** validar interfaz Win32 o hardware.

## Convenciones

- Python 3.11+ y biblioteca estándar donde sea posible; imports explícitos, type hints, `dataclasses` para DTOs.
- Módulos gui/steam/windows/gamepads desacoplados, no poner políticas de un juego en GUI.
- Tests nuevos para cada corrección; mockear Win32, Steam y GPIO/HID cuando no haya acceso al hardware.
- Errores con mensajes legibles para usuario, log estructurado sin datos sensibles.
- README y docs en español. Comentarios y nombres de APIs del estándar se mantienen en inglés cuando mejore trazabilidad.
- Antes de una migración profunda crear ADR documentando coste, alternativa y pruebas.

## Entorno Python y settings

- `.venv/` es local, ignorado por Git y no incluido en el ZIP.
- Configuración y log: `%LOCALAPPDATA%\SplitForge\settings.json`, `splitforge.log`.
- Los reportes se generan en el Escritorio cuando es posible; no commitearlos.

## Archivos importantes

- `CLAUDE.md`: reglas para agente.
- `README.md`: manual breve de v0.6.
- `docs/`: memoria permanente y arquitectura.
- `tests/`: suite regresión.
- `tools/check_docs.py`: valida referencias Markdown dentro del repo.
