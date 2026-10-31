@echo off
REM @project AuvyronEngine
REM @author  D4NTE / Auvyron Games
REM @file    Build_Windows.bat
REM @license MIT
setlocal
cd /d "%~dp0"
where cmake >nul 2>nul
if errorlevel 1 (
  echo [ERROR] CMake not found. Install Visual Studio 2022 with C++ and CMake.
  pause
  exit /b 1
)
where git >nul 2>nul
if errorlevel 1 (
  echo [ERROR] Git not found. Install Git for Windows and add it to PATH.
  pause
  exit /b 1
)
echo [AuvyronEngine] Configuring Visual Studio 2022 x64...
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto fail
echo [AuvyronEngine] Compiling...
cmake --build build --config Release --parallel 4
if errorlevel 1 goto fail
echo [AuvyronEngine] Running automated tests...
ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 goto fail
echo [OK] Build ready. Run Run_Editor.bat
pause
exit /b 0
:fail
echo [ERROR] Build failed. Read BUILD_NOTES.md.
pause
exit /b 1
