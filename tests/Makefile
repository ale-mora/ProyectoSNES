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
#     Incluye: -I. (tarou_anim.h de pruebas, minitest.h), -I../src (sistema/, juego/) y
#              -I../res/nivel1 (nivel1_datos.h: el nivel REAL, con su rejilla para el PC).
#
#   HARDWARE QUE TOCA
#     ninguno.
# ---------------------------------------------------------------------------------
CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -DPRUEBAS_PC -I. -I../src -I../res/nivel1
PRUEBAS = pruebas.c $(wildcard prueba_*.c)
# Lógica pura del juego: solo los archivos que ya existen (las tareas los van añadiendo).
PURO    = $(wildcard ../src/juego/nivel.c ../src/juego/camara.c ../src/juego/animaciones.c ../src/juego/tarou.c ../src/juego/metasprite.c ../src/juego/colision.c ../src/juego/partida.c ../src/juego/guion.c ../src/juego/guion_tutorial.c ../src/juego/profundidad.c ../src/juego/subida.c ../src/juego/p16.c ../src/juego/doble_toque.c ../src/juego/mundo.c ../src/juego/combate.c ../src/juego/luchador.c ../src/enemigos/ninja.c ../src/juego/marcador.c ../src/juego/cambio_protagonista.c ../src/juego/apoyos.c ../src/juego/fase1.c ../src/juego/oleadas.c ../src/juego/fase.c ../src/juego/dialogos.c)
# El motor de escenas (src/escena/): el secuenciador y las tablas; efectos.c toca la PPU y no entra.
PURO   += ../src/escena/escena.c ../src/escena/escena_nintendo.c ../src/escena/escena_dialogo.c
CABECERAS = $(wildcard *.h ../src/sistema/*.h ../src/juego/*.h ../src/escena/*.h ../res/nivel1/nivel1_datos.h)

pruebas.exe: $(PRUEBAS) $(PURO) $(CABECERAS)
	$(CC) $(CFLAGS) -o $@ $(PRUEBAS) $(PURO)

.PHONY: limpiar
limpiar:
	rm -f pruebas.exe
