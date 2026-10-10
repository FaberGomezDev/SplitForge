@echo off
setlocal
cd /d "%~dp0"
title SplitForge 0.7 - Compilar version .NET
where dotnet >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Falta el SDK de .NET 10: https://dotnet.microsoft.com/download/dotnet/10.0
  echo Alternativa sin instalar nada: descarga el paquete SplitForge-win-x64 desde GitHub ^(pestana Actions o Releases^).
  pause
  exit /b 1
)
echo [1/3] Pruebas automaticas...
dotnet test SplitForge.slnx -c Release --nologo
if errorlevel 1 goto :error
echo [2/3] Publicando SplitForge.exe autonomo en publish\SplitForge ...
dotnet publish src\SplitForge.App -c Release -r win-x64 --self-contained -o publish\SplitForge --nologo
if errorlevel 1 goto :error
echo [3/3] Publicando SplitForge.Diagnostico.exe ...
dotnet publish src\SplitForge.Diagnostico -c Release -r win-x64 --self-contained -o publish\SplitForge --nologo
if errorlevel 1 goto :error
copy /y LEEME_DESCARGA.txt publish\SplitForge\LEEME.txt >nul
echo.
echo Listo: publish\SplitForge\SplitForge.exe
pause
exit /b 0
:error
echo [ERROR] La compilacion o las pruebas fallaron. Revisa los mensajes anteriores.
pause
exit /b 1
