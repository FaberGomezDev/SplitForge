@echo off
setlocal
cd /d "%~dp0"
title SplitForge v0.3 - Diagnostico PS4 alternativa Windows
if not exist ".venv\Scripts\python.exe" (
  echo [ERROR] Ejecuta Instalar_Todo.bat primero.
  pause
  exit /b 1
)
echo [INFO] Prueba alternativa PS4: SDL usa el driver de Windows, HIDAPI PS4 desactivado.
set "SDL_JOYSTICK_HIDAPI_PS4=0"
".venv\Scripts\python.exe" diagnostico_mandos.py --seconds 20
pause
