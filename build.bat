```bat
@echo off
setlocal
cd /d "%~dp0"

echo.
echo === Configurando o projeto ===
cmake -S . -B build -G "Visual Studio 18 2026" -A Win32

if errorlevel 1 (
    echo.
    echo [ERRO] Falha ao configurar o projeto.
    pause
    exit /b 1
)

echo.
echo === Compilando CameraMod ===
cmake --build build --config Release

if errorlevel 1 (
    echo.
    echo [ERRO] Falha na compilacao.
    pause
    exit /b 1
)

echo.
echo [OK] Compilacao concluida com sucesso!
pause
exit /b 0
```