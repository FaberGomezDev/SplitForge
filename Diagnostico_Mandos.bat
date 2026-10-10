@echo off
setlocal
cd /d "%~dp0"
title SplitForge v0.3 - Prueba de mandos Bluetooth / USB
if not exist ".venv\Scripts\python.exe" (
  echo [ERROR] Ejecuta primero Instalar_Todo.bat
  pause
  exit /b 1
)
echo [INFO] Conecta los dos Wireless Controller. Mueve sticks y pulsa X / Cruz.
echo [INFO] La prueba dura 20 segundos. Se guardara un reporte en esta carpeta.
".venv\Scripts\python.exe" diagnostico_mandos.py --seconds 20
pause
