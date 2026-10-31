@echo off
REM @project AuvyronEngine
REM @author  D4NTE / Auvyron Games
REM @file    Build_Clean_Windows.bat
REM @license MIT
setlocal
cd /d "%~dp0"
echo.
echo [AuvyronEngine v0.4.2] Full clean rebuild - fixes stale Lua API binaries.
echo Close AuvyronEngine.exe and AuvyronPlayer.exe before continuing.
echo.
tasklist /FI "IMAGENAME eq AuvyronEngine.exe" 2>nul | find /I "AuvyronEngine.exe" >nul
if not errorlevel 1 (
  echo [ERROR] AuvyronEngine.exe is still running. Close it first.
  pause
  exit /b 1
)
tasklist /FI "IMAGENAME eq AuvyronPlayer.exe" 2>nul | find /I "AuvyronPlayer.exe" >nul
if not errorlevel 1 (
  echo [ERROR] AuvyronPlayer.exe is still running. Close it first.
  pause
  exit /b 1
)
where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] cmake missing. Install VS 2022 C++ with CMake.
  pause
  exit /b 1
)
where git >nul 2>nul
if errorlevel 1 (
  echo [ERROR] git missing. Install Git for Windows.
  pause
  exit /b 1
)
if exist "build" (
  echo [INFO] Removing ONLY the old build directory...
  rmdir /S /Q "build"
  if exist "build" (
    echo [ERROR] Unable to remove build directory. Check locks / OneDrive sync.
    pause
    exit /b 1
  )
)
echo [1/3] Configuring CMake...
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto fail
echo [2/3] Building AuvyronEngine, AuvyronPlayer and tests...
cmake --build build --config Release --parallel 4
if errorlevel 1 goto fail
echo [3/3] Checking Lua bindings via CTest...
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 goto fail
if not exist "build\Release\AuvyronEngine.exe" goto fail
if not exist "build\Release\AuvyronPlayer.exe" goto fail
echo.
echo [SUCCESS] AuvyronEngine v0.4.2 ALPHA build complete (Game Package + Lua UI).
echo Launch Run_Editor.bat. Window title should show AuvyronEngine and project name.
pause
exit /b 0
:fail
echo [ERROR] Build failed. Please send the COMPLETE log from this window.
pause
exit /b 1
