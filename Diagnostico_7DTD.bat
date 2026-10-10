@echo off
setlocal
cd /d "%~dp0"
echo === SplitForge - Diagnostico de 7 Days to Die (solo lectura) ===
echo Abre antes 7 Days to Die SIN EAC y deja el juego en el menu principal.
if not exist ".venv\Scripts\python.exe" (
    echo [ERROR] Ejecuta primero Instalar_Todo.bat
    pause
    exit /b 1
)
".venv\Scripts\python.exe" diagnostico_7dtd.py
if errorlevel 1 (
    echo [ERROR] No pudo completarse el diagnostico.
    pause
    exit /b 1
)
echo No se han modificado ni cerrado juegos.
pause
