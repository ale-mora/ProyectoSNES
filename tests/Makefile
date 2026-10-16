# ---------------------------------------------------------------------------------
#   tests/Makefile — Compila y enlaza las pruebas de PC con gcc.
#
#   PROPÓSITO
#     La lógica pura de src/juego/ (sin snes.h) se compila aquí con gcc y
#     -DPRUEBAS_PC, que hace que sistema/tipos.h tome u8/u16/s16 de stdint.h en
#     vez de snes.h. Uso, desde PowerShell en la raíz del repo:
#         make -C tests ; .\tests\pruebas.exe
#     (make es el de MSYS2, C:\msys64\usr\bin; gcc es el de WinLibs/MinGW).
#
#   SE CONECTA CON
#     Compila: pruebas.c, prueba_*.c y los .c puros de ../src/juego/ que existan.
#     Incluye: -I. (tarou_anim.h de pruebas, minitest.h) y -I../src (sistema/, juego/).
#
#   HARDWARE QUE TOCA
#     ninguno.
# ---------------------------------------------------------------------------------
CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -DPRUEBAS_PC -I. -I../src
PRUEBAS = pruebas.c $(wildcard prueba_*.c)
# Lógica pura del juego: solo los archivos que ya existen (las tareas los van añadiendo).
PURO    = $(wildcard ../src/juego/nivel.c ../src/juego/camara.c ../src/juego/animaciones.c ../src/juego/tarou.c)
CABECERAS = $(wildcard *.h ../src/sistema/*.h ../src/juego/*.h)

pruebas.exe: $(PRUEBAS) $(PURO) $(CABECERAS)
	$(CC) $(CFLAGS) -o $@ $(PRUEBAS) $(PURO)

.PHONY: limpiar
limpiar:
	rm -f pruebas.exe
