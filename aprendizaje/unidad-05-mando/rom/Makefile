# Mini-ROM de la unidad 05: qué botones del mando están pulsados.
# Parte de tools/pvsneslib/snes-examples/input/controller/Makefile y hello_world (receta de
# aprendizaje/PLAN.md §6.3). Diferencias con el ejemplo:
#   - ROMNAME y ROMTITLE propios;
#   - FASTROM 1, como el juego: los bancos se ven en $80-$FF y tcc816 compila
#     con -F (compárese el .sym de esta ROM con el de ProyectoSNES).
# No hay gráficos propios: consoleInitDefaultText usa la fuente que trae la
# librería, así que no hace falta data.asm (el ejemplo del SDK trae su propia
# fuente en BMP; aquí no hace falta).
# Variante que se juega sola: $env:CFLAGS="-DGUION_MANDO"; make clean; make (README).

ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Configura PVSNESLIB_HOME (formato MSYS: /e/ProyectoSNES/tools/pvsneslib)")
endif

# Nombre de los archivos de salida (botones_mando.sfc, .sym, .log).
export ROMNAME := botones_mando
# Nombre dentro de la cabecera de la ROM (21 caracteres como máximo).
export ROMTITLE := UNIDAD 05 MANDO
# Bancos en $80-$FF y CPU a 3,58 MHz, como el juego. Debe ir ANTES del include.
export FASTROM := 1

# Las reglas genéricas del SDK: %.ps, %.asm, %.obj, %.sfc, hdr.asm, buildWithSummary.
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all cleanLogs

# `make` a secas: compila volcándolo todo al .log e imprime el resumen.
all: buildWithSummary
# Lo que compila de verdad buildWithSummary: los objetos y la ROM.
buildActual: $(OBJS) $(ROMNAME).sfc

# Limpieza total: objetos, ROM, gráficos y el .log.
clean: cleanBuildRes cleanRom cleanGfx cleanLogs
