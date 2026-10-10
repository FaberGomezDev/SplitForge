@echo off
setlocal
cd /d "%~dp0"
call "Instalar_Todo.bat"
exit /b %errorlevel%
