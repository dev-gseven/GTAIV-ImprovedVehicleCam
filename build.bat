```bat
@echo off
setlocal
cd /d "%~dp0"

echo.
echo === Configuring the project ===
cmake -S . -B build -G "Visual Studio 18 2026" -A Win32

if errorlevel 1 (
    echo.
    echo [ERRO] Error while configuring the project.
    pause
    exit /b 1
)

echo.
echo === Compiling ImprovedVehicleCam ===
cmake --build build --config Release

if errorlevel 1 (
    echo.
    echo [ERRO] Error on compilation.
    pause
    exit /b 1
)

echo.
echo [OK] Compilation completed successfully!
pause
exit /b 0
```