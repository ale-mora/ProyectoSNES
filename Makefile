ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Por favor configura la variable de entorno PVSNESLIB_HOME apuntando al directorio raíz de PVSnesLib")
endif

# Nombre y título de la ROM de salida
export ROMNAME := ProyectoSNES
export ROMTITLE := TAROUH DEMO

# --- Audio (se activa en la tarea 3 del plan, cuando haya módulos .it) ---------
# AUDIOFILES y SOUNDBANK deben declararse ANTES del include de snes_rules.
# Los .it de efectos van primero en la lista.
# export AUDIOFILES := res/musica/jingle_publisher.it res/musica/titulo.it
# export SOUNDBANK  := res/musica/soundbank

# Incluir reglas estándar de compilación de PVSnesLib
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all clean

# Regla por defecto para compilar.
# `bitmaps` va ANTES de $(OBJS): data.asm hace .incbin de los .pic/.pal, así que
# los gráficos tienen que estar convertidos antes de ensamblar.
all: buildWithSummary
buildActual: bitmaps $(OBJS) $(ROMNAME).sfc

# --- Conversión de gráficos (gfx4snes) -------------------------------------------
# Fuente de consola: 96 tiles de 8x8, 2 colores en la paleta (-o 2), 4bpp (-u 16),
# sin mapa (la consola arma el suyo). Se genera el .pic, el .pal y el .inc.
res/fuente/ark_consola.pic: res/fuente/ark_consola.bmp
	@echo convert font ... $(notdir $<)
	$(GFXCONV) -s 8 -o 2 -u 16 -p -e 0 -t bmp -i $<

.PHONY: bitmaps cleanGfxRes
bitmaps: res/fuente/ark_consola.pic

# Borra las salidas de conversión que viven dentro de res/ (cleanGfx solo mira la raíz).
cleanGfxRes:
	@echo clean res graphics data
	@find res -type f \( -name "*.pic" -o -name "*.pal" -o -name "*.map" -o -name "*.inc" -o -name "*_data.as" -o -name "*.pc7" -o -name "*.mp7" \) -not -path "res/descargas/*" -delete

# Regla para limpiar los binarios y recursos
clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes
