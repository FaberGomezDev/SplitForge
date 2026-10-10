@echo off
setlocal
cd /d "%~dp0"
echo === SplitForge - Analisis de ventanas Steam (solo lectura) ===
if not exist ".venv\Scripts\python.exe" (
    echo [ERROR] Instala dependencias con Instalar_Todo.bat o copia v0.6 encima de v0.5.
    pause
    exit /b 1
)
".venv\Scripts\python.exe" steam_windows_scan.py --appid 251570
if errorlevel 1 (
    echo [ERROR] No pudo completarse el analisis.
    pause
    exit /b 1
)
echo No se han lanzado, modificado ni cerrado juegos.
pause
