@echo off
setlocal
cd /d "%~dp0"
title SplitForge v0.3 - Dependencias SDL y PS4
if not exist ".venv\Scripts\python.exe" (
  echo [1/3] Creando entorno Python aislado con acceso a bibliotecas ya instaladas...
  where py >nul 2>&1
  if not errorlevel 1 (
     py -3.13 -m venv --system-site-packages .venv >nul 2>&1
     if errorlevel 1 py -3 -m venv --system-site-packages .venv
  ) else (
     python -m venv --system-site-packages .venv
  )
  if errorlevel 1 goto :error
)
echo [2/3] Verificando pygame-ce y SDL en el mismo Python que usara SplitForge...
".venv\Scripts\python.exe" setup_deps.py
if errorlevel 1 (
  echo.
  echo [AVISO] No fue posible completar SDL. SplitForge puede abrir con WinMM.
  echo Revisa el enlace en pantalla y ejecuta Instalar_Todo.bat cuando tengas el .whl.
  pause
  exit /b 1
)
echo [3/3] Comprobacion correcta. Puedes ejecutar Iniciar_SplitForge.bat
echo.
pause
exit /b 0
:error
echo [ERROR] No se pudo crear el entorno Python. Instala Python 3.11+ x64 con venv y Tcl/Tk.
pause
exit /b 1
