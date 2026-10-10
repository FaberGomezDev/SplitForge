@echo off
setlocal
cd /d "%~dp0"
if not exist "publish\SplitForge\SplitForge.exe" (
  echo [INFO] Primera vez: compilando la version .NET...
  call "Compilar_SplitForge.bat"
  if not exist "publish\SplitForge\SplitForge.exe" exit /b 1
)
start "" "publish\SplitForge\SplitForge.exe"
exit /b 0
