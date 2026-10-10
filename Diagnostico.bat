@echo off
setlocal
cd /d "%~dp0"
if not exist ".venv\Scripts\python.exe" (
  echo [ERROR] Ejecuta Instalar_Todo.bat primero.
  pause
  exit /b 1
)
".venv\Scripts\python.exe" diagnostico.py
pause
