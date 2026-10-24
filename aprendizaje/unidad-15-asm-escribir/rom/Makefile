# Mini-ROM de la unidad 15: una rutina en ensamblador (src/relleno.asm) llamada
# desde C, junto a la misma rutina escrita en C (src/relleno_c.c) para comparar.
# Parte de tools/pvsneslib/snes-examples/hello_world/Makefile (receta de
# aprendizaje/PLAN.md §6.3). Diferencias con el ejemplo:
#   - ROMNAME y ROMTITLE propios;
#   - FASTROM 1, como el juego: así la trampa de .BASE $00 / .BASE $80 alrededor
#     de la RAMSECTION de relleno.asm es real (sin FastROM no se notaría).
# No hace falta nombrar relleno.asm en ningún sitio: snes_rules compila todo
# src/*.asm (y src/*/*.asm) además de los .c. Por eso el .asm NO puede llamarse
# como ningún .c: principal.c y relleno_c.c pasan por principal.asm y
# relleno_c.asm intermedios, y un relleno_c.asm nuestro quedaría sobrescrito.

ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Configura PVSNESLIB_HOME (formato MSYS: /e/ProyectoSNES/tools/pvsneslib)")
endif

# Nombre de los archivos de salida (unidad15.sfc, .sym, .log).
export ROMNAME := unidad15
# Nombre dentro de la cabecera de la ROM (21 caracteres como máximo).
export ROMTITLE := UNIDAD 15 ASM
# Bancos en $80-$FF y CPU a 3,58 MHz, como el juego. Debe ir ANTES del include.
export FASTROM := 1

# Las reglas genéricas del SDK: %.ps, %.asm, %.obj, %.sfc, hdr.asm, buildWithSummary.
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all cleanLogs

# `make` a secas: compila volcándolo todo a unidad15.log e imprime el resumen.
all: buildWithSummary
# Lo que compila de verdad buildWithSummary: los objetos y la ROM.
buildActual: $(OBJS) $(ROMNAME).sfc

# Limpieza total: objetos, ROM, gráficos y el .log.
clean: cleanBuildRes cleanRom cleanGfx cleanLogs
