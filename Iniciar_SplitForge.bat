@echo off
setlocal
cd /d "%~dp0"
title SplitForge v0.6.0 - Windows 11
if not exist ".venv\Scripts\python.exe" (
  echo [INFO] Primera ejecucion: preparando dependencias para mandos PS4...
  call "Instalar_Todo.bat"
  if errorlevel 1 echo [AVISO] SDL pendiente: se intentara abrir con soporte nativo limitado.
)
if not exist ".venv\Scripts\python.exe" goto :error
".venv\Scripts\python.exe" -c "import sys, tkinter; assert sys.version_info >= (3, 11)" >nul 2>&1
if errorlevel 1 goto :error
".venv\Scripts\python.exe" setup_deps.py --check
if errorlevel 1 (
 echo [AVISO] SDL aun no esta disponible en este entorno.
 echo [AVISO] Ejecuta Instalar_Todo.bat para reparar la instalacion.
)
echo [INFO] Abriendo interfaz...
".venv\Scripts\python.exe" launcher.py
if errorlevel 1 goto :error
exit /b 0
:error
echo [ERROR] Fallo al iniciar. Ejecuta Diagnostico_Mandos.bat.
pause
exit /b 1
