# ---------------------------------------------------------------------------------
#   Makefile de la mini-ROM de la unidad 07 (scroll, cámara y parallax).
#
#   Base: tools/pvsneslib/snes-examples/graphics/Backgrounds/Mode1Scroll y
#   graphics/Effects/ParallaxScrolling (receta de aprendizaje/PLAN.md §6.3), con
#   las reglas gfx4snes del Makefile del proyecto y el resumen de compilación.
#
#   Compilar (PowerShell, en esta carpeta):
#     $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
# ---------------------------------------------------------------------------------
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Configura PVSNESLIB_HOME (ver aprendizaje/PLAN.md §6.3)")
endif

export ROMNAME := u07_parallax
export ROMTITLE := U07 SCROLL Y CAMARA

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all bitmaps clean cleanGfxRes

all: buildWithSummary
buildActual: bitmaps $(OBJS) $(ROMNAME).sfc

# Fuente de 4 colores (2bpp) para el texto en el fondo 2 (BG3), como el título.
res/fuente2bpp.pic: res/fuente2bpp.bmp
	@echo convert font 2bpp ... $(notdir $<)
	$(GFXCONV) -s 8 -o 2 -u 4 -p -e 0 -t bmp -i $<

# Capa cercana: los arbustos del título (res/titulo/fondo_cerca.png), 512x256.
# -y = mapa en páginas de 32x32, el formato que la PPU espera para SC_64x32.
res/cerca.pic: res/cerca.png
	@echo convert near layer ... $(notdir $<)
	$(GFXCONV) -y -s 8 -o 16 -u 16 -e 1 -p -m -i $<

# Capa lejana: las montañas del nivel 1 (res/nivel1/fondo_lejos.png), 512x256,
# original + espejo (scripts/preparar_fondo_nivel.py). Paleta 2.
res/lejos.pic: res/lejos.png
	@echo convert far layer ... $(notdir $<)
	$(GFXCONV) -y -s 8 -o 16 -u 16 -e 2 -p -m -i $<

ASSETS := res/fuente2bpp.pic res/cerca.pic res/lejos.pic
bitmaps: $(ASSETS)
data.obj: $(ASSETS)

cleanGfxRes:
	@rm -f res/*.pic res/*.pal res/*.map res/*.inc res/*_data.as

clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes
