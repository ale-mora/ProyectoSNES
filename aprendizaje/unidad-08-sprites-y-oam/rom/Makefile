# ---------------------------------------------------------------------------------
#   Makefile de la mini-ROM de la unidad 08 (sprites estáticos y la OAM).
#
#   Base: tools/pvsneslib/snes-examples/graphics/Sprites/SimpleSprite (receta de
#   aprendizaje/PLAN.md §6.3), con las reglas gfx4snes del Makefile del proyecto
#   (las de Bárbara y el lobo samurái) y el resumen de compilación.
#
#   Compilar (PowerShell, en esta carpeta):
#     $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
# ---------------------------------------------------------------------------------
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Configura PVSNESLIB_HOME (ver aprendizaje/PLAN.md §6.3)")
endif

export ROMNAME := u08_sprites
export ROMTITLE := U08 SPRITES Y OAM

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all bitmaps clean cleanGfxRes

all: buildWithSummary
buildActual: bitmaps $(OBJS) $(ROMNAME).sfc

# Fuente de consola (4bpp, 2 colores en la paleta), para el texto del fondo 0.
res/fuente.pic: res/fuente.bmp
	@echo convert font ... $(notdir $<)
	$(GFXCONV) -s 8 -o 2 -u 16 -p -e 0 -t bmp -i $<

# Sprites del pack: tiras de 4 cuadros de 64x64 (256x64), 16 colores. -s 64 =
# bloques de 64x64, que gfx4snes empaqueta en filas de 128 px (dos por fila): la
# misma regla que res/personajes/barbara_samurai.pic en el Makefile del juego.
res/barbara.pic: res/barbara.png
	@echo convert sprite ... $(notdir $<)
	$(GFXCONV) -s 64 -o 16 -u 16 -p -i $<

res/lobo.pic: res/lobo.png
	@echo convert sprite ... $(notdir $<)
	$(GFXCONV) -s 64 -o 16 -u 16 -p -i $<

ASSETS := res/fuente.pic res/barbara.pic res/lobo.pic
bitmaps: $(ASSETS)
data.obj: $(ASSETS)

cleanGfxRes:
	@rm -f res/*.pic res/*.pal res/*.map res/*.inc res/*_data.as

clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes
