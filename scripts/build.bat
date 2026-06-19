@echo off
setlocal enabledelayedexpansion
REM Script de compilacion para Breakout
REM Usa el Makefile del proyecto y rutas manuales
REM Autor: RGiskard7

cd /d "%~dp0\.."

echo ===================================================
echo        Compilando Breakout
echo ===================================================
echo.

REM Asegurar que MinGW esta en el PATH
set "PATH=C:\mingw64\bin;%PATH%"

REM Verificar herramientas
where gcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] No se encontro gcc.
    echo Ejecuta scripts\install-deps.bat primero.
    pause
    exit /b 1
)

where mingw32-make >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] No se encontro mingw32-make.
    echo Ejecuta scripts\install-deps.bat primero.
    pause
    exit /b 1
)

echo [INFO] Ejecutando mingw32-make -f Makefile...
mingw32-make -f Makefile

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] La compilacion fallo.
    pause
    exit /b 1
)

echo.
echo [OK] Compilacion exitosa.
echo.

if "%1"=="run" (
    echo Iniciando juego...
    REM El ejecutable necesita la DLL de Allegro junto a el (en la raiz)
    if not exist "allegro_monolith-5.2.dll" (
        if exist "C:\allegro-5.2.9.1-mingw-14.1.0\bin\allegro_monolith-5.2.dll" (
            copy "C:\allegro-5.2.9.1-mingw-14.1.0\bin\allegro_monolith-5.2.dll" . >nul
            echo [INFO] DLL copiada.
        )
    )

    breakout.exe
) else (
    echo Para ejecutar: scripts\build.bat run
    echo Para empaquetar: scripts\dist.bat
)

pause
