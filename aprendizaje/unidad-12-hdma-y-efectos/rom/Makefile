# ---------------------------------------------------------------------------------
#   Makefile de la mini-ROM de la unidad 12 (HDMA y efectos de la PPU).
#
#   Base: tools/pvsneslib/snes-examples/graphics/Effects/GradientColors (receta de
#   aprendizaje/PLAN.md §6.3), con las reglas gfx4snes escritas como en el
#   Makefile del proyecto y el resumen de compilación (buildWithSummary).
#   La tabla HDMA del cielo NO sale de gfx4snes -n (como la del título): se
#   escribe a mano en src/cielos.c para ver su formato byte a byte.
#
#   Compilar (PowerShell, en esta carpeta):
#     $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
# ---------------------------------------------------------------------------------
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Configura PVSNESLIB_HOME (ver aprendizaje/PLAN.md §6.3)")
endif

# Nombre del archivo .sfc y título de la cabecera de la ROM (21 letras como mucho).
export ROMNAME := u12_cielo
export ROMTITLE := U12 HDMA Y EFECTOS

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all bitmaps clean cleanGfxRes

# Igual que el proyecto: compila y resume avisos y errores al final.
all: buildWithSummary
buildActual: bitmaps $(OBJS) $(ROMNAME).sfc

# --- Conversión de gráficos (gfx4snes) ---------------------------------------------
# Fuente de consola del juego (res/fuente/ark_consola.bmp): 96 tiles 4bpp, 2 colores.
res/fuente.pic: res/fuente.bmp
	@echo convert font ... $(notdir $<)
	$(GFXCONV) -s 8 -o 2 -u 16 -p -e 0 -t bmp -i $<

# Las montañas del título (res/titulo/fondo_lejos.png): 16 colores, mapa con la
# paleta 2. Su índice 0 (el cielo) es transparente: ahí se ve el color 0 de la
# CGRAM, que es lo que pinta el HDMA línea a línea.
res/montanas.pic: res/montanas.png
	@echo convert background ... $(notdir $<)
	$(GFXCONV) -s 8 -o 16 -u 16 -e 2 -p -m -i $<

ASSETS := res/fuente.pic res/montanas.pic
bitmaps: $(ASSETS)
data.obj: $(ASSETS)

# Las salidas de gfx4snes viven en res/ (cleanGfx del SDK solo mira esta carpeta).
cleanGfxRes:
	@rm -f res/*.pic res/*.pal res/*.map res/*.inc res/*_data.as

clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes
