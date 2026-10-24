<#
---------------------------------------------------------------------------------
    pruebas_con_arte_real.ps1 — Unidad 11: ejecuta las pruebas de PC dos veces,
    con la copia fija de los cuadros y con los cuadros del arte real.

    PROPÓSITO
      Las pruebas de tests/ compilan animaciones.c con tests/tarou_anim.h, una
      COPIA FIJA (reposo 4, desenvainar 4...). La ROM usa el tarou_anim.h que
      genera el pipeline (res/personajes/tarou/: reposo 9, desenvainar 9,
      despertar 10). Un cambio de ritmo puede pasar todas las pruebas y aun así
      romper la ROM que se juega sola. Este script:
        1. make -C tests y tests\pruebas.exe (lo de siempre);
        2. compila las mismas pruebas con -I../res/personajes/tarou DELANTE de
           -I., para que animaciones.c tome los cuadros reales, y enseña el
           resultado. Ahí fallan siempre las cuentas que dependen de la copia
           fija (prueba_animacion y algunas de prueba_tarou: 16 hoy); lo que
           importa es si aparece prueba_recorrido.

    SE CONECTA CON
      Usa: tests/Makefile, tests/*.c, src/juego/*.c, res/personajes/tarou/tarou_anim.h.
      Lo usa: ejercicio_11.md (parte C).

    HARDWARE QUE TOCA
      ninguno.

    USO (PowerShell, en cualquier carpeta)
      .\pruebas_con_arte_real.ps1 -Clon E:\ProyectoSNES_trabajo\mio11
---------------------------------------------------------------------------------
#>
param([string]$Clon = (Get-Location).Path)       # Raíz de la copia del proyecto.
$env:PATH = "C:\msys64\usr\bin;" + $env:PATH    # make de MSYS2.
Set-Location "$Clon\tests"
make 2>&1 | Out-Null                             # Las pruebas de siempre...
"--- con la copia fija tests/tarou_anim.h (make -C tests) ---"
.\pruebas.exe
$src = @((Get-ChildItem prueba_*.c).Name) + "pruebas.c"   # ...y otra vez con el arte real.
$puro = "../src/juego/nivel.c ../src/juego/camara.c ../src/juego/animaciones.c ../src/juego/tarou.c ../src/juego/metasprite.c ../src/juego/colision.c ../src/juego/partida.c ../src/juego/guion.c ../src/juego/guion_tutorial.c ../src/escena/escena.c ../src/escena/escena_nintendo.c".Split(" ")
gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../res/personajes/tarou -I. -I../src -I../res/nivel1 -o pruebas_arte_real.exe $src $puro
"--- con el tarou_anim.h REAL (res/personajes/tarou): solo importa prueba_recorrido ---"
.\pruebas_arte_real.exe | Select-String "recorrido|pruebas,"
Remove-Item pruebas_arte_real.exe                # No dejar el ejecutable en tests/.
