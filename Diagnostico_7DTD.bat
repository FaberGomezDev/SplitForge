@echo off
setlocal
cd /d "%~dp0"
echo === SplitForge - Diagnostico de 7 Days to Die (solo lectura) ===
echo Abre antes 7 Days to Die SIN EAC y deja el juego en el menu principal.
rem Prefiere la version .NET si esta compilada; si no, usa el prototipo Python.
if exist "publish\SplitForge\SplitForge.Diagnostico.exe" (
    "publish\SplitForge\SplitForge.Diagnostico.exe" 7dtd
    goto :done
)
if not exist ".venv\Scripts\python.exe" (
    echo [ERROR] Ejecuta Compilar_SplitForge.bat o Instalar_Todo.bat primero.
    pause
    exit /b 1
)
".venv\Scripts\python.exe" diagnostico_7dtd.py
:done
if errorlevel 1 (
    echo [ERROR] No pudo completarse el diagnostico.
    pause
    exit /b 1
)
echo No se han modificado ni cerrado juegos.
pause
