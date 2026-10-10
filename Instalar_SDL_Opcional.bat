@echo off
setlocal
cd /d "%~dp0"
echo pygame-ce INCLUYE SDL2. No se necesita otro instalador de SDL.
call "Instalar_Todo.bat"
exit /b %errorlevel%
