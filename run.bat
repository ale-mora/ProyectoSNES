@echo off
echo ===================================================
echo   Compilando y Ejecutando Proyecto SNES
echo ===================================================

:: Variables de entorno locales a la sesion
set "PVSNESLIB_HOME=/e/ProyectoSNES/tools/pvsneslib"
set "DEVKITSNES=/e/ProyectoSNES/tools/pvsneslib/devkitsnes"
set "PATH=E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;%PATH%"

echo [1/3] Limpiando compilaciones anteriores...
call make clean

echo [2/3] Compilando el codigo fuente...
call make
if errorlevel 1 (
    echo.
    echo [ERROR] Error durante la compilacion.
    pause
    exit /b %errorlevel%
)

if not exist ProyectoSNES.sfc (
    echo.
    echo [ERROR] No se genero el archivo ROM: ProyectoSNES.sfc
    pause
    exit /b 1
)

echo [3/3] Lanzando emulador Mesen con la ROM...
start "" "E:\ProyectoSNES\tools\mesen\Mesen.exe" "%~dp0ProyectoSNES.sfc"

echo Proceso completado con exito.
