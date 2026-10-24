# ---------------------------------------------------------------------------------
#   Makefile de la mini-ROM de la unidad 06 (tiles, paletas, tilemaps y fondos).
#
#   Base: tools/pvsneslib/snes-examples/graphics/Backgrounds/Mode1 (receta de
#   aprendizaje/PLAN.md §6.3), con las reglas gfx4snes escritas como en el
#   Makefile del proyecto y el resumen de compilación (buildWithSummary).
#
#   Compilar (PowerShell, en esta carpeta):
#     $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
# ---------------------------------------------------------------------------------
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Configura PVSNESLIB_HOME (ver aprendizaje/PLAN.md §6.3)")
endif

# Nombre del archivo .sfc y título de la cabecera de la ROM (21 letras como mucho).
export ROMNAME := u06_fondo
export ROMTITLE := U06 TILES Y FONDOS

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all bitmaps clean cleanGfxRes

# Igual que el proyecto: compila y resume avisos y errores al final.
all: buildWithSummary
buildActual: bitmaps $(OBJS) $(ROMNAME).sfc

# --- Conversión de gráficos (gfx4snes) ---------------------------------------------
# Fuente de consola (la del juego): 96 tiles de 8x8, 4bpp (-u 16), solo 2 colores en
# la paleta (-o 2), sin mapa (la consola arma el suyo). Misma regla que el Makefile
# del proyecto (res/fuente/ark_consola.pic).
res/fuente.pic: res/fuente.bmp
	@echo convert font ... $(notdir $<)
	$(GFXCONV) -s 8 -o 2 -u 16 -p -e 0 -t bmp -i $<

# Las montañas del título (res/titulo/fondo_lejos.png): 256x224, 16 colores (-u 16),
# tiles de 8x8 (-s 8), paleta de 16 (-o 16) a la salida (-p), con mapa (-m) cuyas
# entradas llevan la paleta 2 (-e 2), como en el juego.
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
