@echo off
setlocal
cd /d "%~dp0"
echo SplitForge v0.5 - diagnostico de biblioteca Steam
if exist ".venv\Scripts\python.exe" (
    ".venv\Scripts\python.exe" steam_scan.py
) else (
    py -3 steam_scan.py
)
if errorlevel 1 echo [ERROR] No se pudo generar el reporte. Verifica la instalacion de Python.
pause
