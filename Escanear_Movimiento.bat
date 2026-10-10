@echo off
setlocal
cd /d "%~dp0"
title SplitForge - Lectura real de controles PS4
if not exist ".venv\Scripts\python.exe" (
  echo ERROR: No existe el entorno Python de la carpeta actual.
  echo Abre esta misma carpeta y ejecuta Instalar_Todo.bat si no tienes instaladas las dependencias.
  pause
  exit /b 1
)
".venv\Scripts\python.exe" escanear_movimiento.py
set "CODE=%ERRORLEVEL%"
echo.
echo El archivo Reporte_Movimiento.txt se guardo en esta carpeta.
echo Enviame su contenido si los mandos siguen sin mover la nave.
pause
exit /b %CODE%
