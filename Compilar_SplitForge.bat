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
rem Modulos nativos C++ (opcional: requieren CMake y Visual Studio Build Tools con C++).
where cmake >nul 2>&1
if errorlevel 1 (
  echo [AVISO] Sin CMake: los modulos nativos no se compilan. Descargalos del paquete de GitHub Actions si los necesitas.
) else (
  cmake -S native\SplitForge.XInputFilter -B native\build -A x64 >nul && cmake --build native\build --config Release >nul
  if exist "native\build\Release\SplitForge.XInputFilter.dll" (
    if not exist "publish\SplitForge\native" mkdir "publish\SplitForge\native"
    copy /y native\build\Release\SplitForge.XInputFilter.dll publish\SplitForge\native\ >nul
    echo Filtro XInput nativo compilado.
  ) else (
    echo [AVISO] No se pudo compilar el filtro XInput nativo.
  )
  cmake -S native\SplitForge.SteamEmu -B native\build-steamemu -A x64 >nul && cmake --build native\build-steamemu --config Release >nul
  if exist "native\build-steamemu\Release\steam_api64.dll" (
    if not exist "publish\SplitForge\native\SplitForge.SteamEmu" mkdir "publish\SplitForge\native\SplitForge.SteamEmu"
    copy /y native\build-steamemu\Release\steam_api64.dll publish\SplitForge\native\SplitForge.SteamEmu\ >nul
    echo Emulador de Steam educativo compilado.
  ) else (
    echo [AVISO] No se pudo compilar el emulador de Steam educativo.
  )
)
echo.
echo Listo: publish\SplitForge\SplitForge.exe
pause
exit /b 0
:error
echo [ERROR] La compilacion o las pruebas fallaron. Revisa los mensajes anteriores.
pause
exit /b 1
