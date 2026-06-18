@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0.."

echo ===================================================
echo        Compilando Breakout
echo ===================================================
echo.

set "PATH=C:\mingw64\bin;%PATH%"

where gcc >nul 2>&1
if !ERRORLEVEL! NEQ 0 (
    echo [ERROR] No se encontro gcc.
    pause
    exit /b 1
)

where mingw32-make >nul 2>&1
if !ERRORLEVEL! NEQ 0 (
    echo [ERROR] No se encontro mingw32-make.
    pause
    exit /b 1
)

echo [INFO] Compilando...
mingw32-make -f breakout\Makefile

if !ERRORLEVEL! NEQ 0 (
    echo [ERROR] La compilacion fallo.
    pause
    exit /b 1
)

echo [OK] Compilacion exitosa.

if "%1"=="run" (
    if not exist "allegro_monolith-5.2.dll" (
        if exist "C:\allegro-5.2.9.1-mingw-14.1.0\bin\allegro_monolith-5.2.dll" (
            copy "C:\allegro-5.2.9.1-mingw-14.1.0\bin\allegro_monolith-5.2.dll" . >nul
        )
    )
    echo [INFO] Ejecutando Breakout...
    breakout\breakout.exe
) else (
    echo Para ejecutar: build.bat run
)
pause
