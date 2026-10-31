@echo off
REM @project AuvyronEngine
REM @author  D4NTE / Auvyron Games
REM @file    Run_Editor.bat
REM @license MIT
setlocal
cd /d "%~dp0"
if not exist "build\Release\AuvyronEngine.exe" (
  echo [AuvyronEngine] No editor executable. Run Build_Clean_Windows.bat first.
  pause
  exit /b 1
)
echo [AuvyronEngine v0.4.2] Launching:
echo %CD%\build\Release\AuvyronEngine.exe
"build\Release\AuvyronEngine.exe" "%~dp0examples\StarterWorld"
if errorlevel 1 (
  echo [ERROR] Editor exited with code %ERRORLEVEL%.
  pause
)
