#---------------------------------------------------------------------------------
#   Makefile de la mini-ROM de la unidad 17 (audio desde C con PVSnesLib).
#
#   Base: tools/pvsneslib/snes-examples/audio/effectsandmusic (soundbank con smconv)
#   y systems/games/likemario (efecto BRR con snesbrr). Compilar desde PowerShell,
#   en esta carpeta:
#     $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
#---------------------------------------------------------------------------------
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Configura PVSNESLIB_HOME (ver aprendizaje/PLAN.md §6.3)")
endif

# --- Audio: ANTES del include de snes_rules ---------------------------------------
# AUDIOFILES: los módulos .it que smconv mete en el soundbank, en orden (el primero
# es el número 0, MOD_OVERWORLD en soundbank.h). Si hubiera un .it de efectos, iría
# el primero de la lista.
AUDIOFILES := res/overworld.it
# SOUNDBANK: base del nombre de lo que genera smconv (res/soundbank.asm, .bnk, .h).
export SOUNDBANK := res/soundbank

# Nombre de los archivos de salida y título de la cabecera de la ROM.
export ROMNAME := audio17
export ROMTITLE := UNIDAD 17 AUDIO

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps musics sonidos all

#---------------------------------------------------------------------------------
# smconv: -s = modo soundbank, -o = base de salida, -V = cuenta cuánta RAM del
# SPC700 usa cada módulo (sale en audio17.log), -b 5 = el soundbank empieza en el
# banco 5 de la ROM (necesita bancos para él solo).
SMCONVFLAGS := -s -o $(SOUNDBANK) -V -b 5
musics: $(SOUNDBANK).obj

# El efecto: snes_rules ya trae la regla %.brr: %.wav (snesbrr -e). Basta pedirlo.
sonidos: res/mariojump.brr

# make a secas = buildWithSummary: compila y resume avisos y errores.
all: buildWithSummary
buildActual: musics sonidos bitmaps $(ROMNAME).sfc

# data.asm hace .incbin de la fuente y del .brr: make no lo ve, se declara aquí.
data.obj: res/pvsneslibfont.pic res/mariojump.brr

clean: cleanBuildRes cleanRom cleanGfx cleanAudio
	@rm -f res/*.pic res/*.pal res/*.inc res/*_data.as res/*.brr

#---------------------------------------------------------------------------------
# Fuente de la consola de texto: 2 colores (-o 2), 4bpp (-u 16), paleta 1 (-e 1).
res/pvsneslibfont.pic: res/pvsneslibfont.bmp
	@echo convert font ... $(notdir $@)
	$(GFXCONV) -s 8 -o 2 -u 16 -p -e 1 -t bmp -i $<

bitmaps: res/pvsneslibfont.pic
