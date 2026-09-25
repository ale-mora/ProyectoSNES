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

# Sprites de los personajes: tiras de cuadros de 64x64 (-s 64), 16 colores, PNG.
# gfx4snes empaqueta los bloques en filas de 128 px; ver src/personajes.c.
res/personajes/tarouh_lobo.pic: res/personajes/tarouh_lobo.png
	@echo convert sprites ... $(notdir $<)
	$(GFXCONV) -s 64 -o 16 -u 16 -p -i $<

res/personajes/barbara_samurai.pic: res/personajes/barbara_samurai.png
	@echo convert sprites ... $(notdir $<)
	$(GFXCONV) -s 64 -o 16 -u 16 -p -i $<

# Fuente de 4 colores (2bpp) para la capa BG3 del título: misma imagen, -u 4.
res/fuente/ark_consola_2bpp.pic: res/fuente/ark_consola_2bpp.bmp
	@echo convert font 2bpp ... $(notdir $<)
	$(GFXCONV) -s 8 -o 2 -u 4 -p -e 0 -t bmp -i $<

# Fondos de 16 colores (Mode 1). -e N = número de paleta que llevan los tiles del mapa.
res/nintendo/logo_nintendo.pic: res/nintendo/logo_nintendo.png
	$(GFXCONV) -s 8 -o 16 -u 16 -e 1 -p -m -i $<
res/publisher/logo_publisher.pic: res/publisher/logo_publisher.png
	$(GFXCONV) -s 8 -o 16 -u 16 -e 1 -p -m -i $<
res/intro/escena_3.pic: res/intro/escena_3.png
	$(GFXCONV) -s 8 -o 16 -u 16 -e 1 -p -m -i $<
res/titulo/fondo_lejos.pic: res/titulo/fondo_lejos.png
	$(GFXCONV) -s 8 -o 16 -u 16 -e 2 -p -m -i $<
# Capa cercana de 512 px: -y genera el mapa en páginas de 32x32 (formato SC_64x32).
res/titulo/fondo_cerca.pic: res/titulo/fondo_cerca.png
	$(GFXCONV) -y -s 8 -o 16 -u 16 -e 1 -p -m -i $<

# Escenas de 256 colores (Mode 3).
res/intro/escena_1.pic: res/intro/escena_1.png
	$(GFXCONV) -s 8 -o 256 -u 256 -e 0 -p -m -i $<
res/intro/escena_2.pic: res/intro/escena_2.png
	$(GFXCONV) -s 8 -o 256 -u 256 -e 0 -p -m -i $<

res/devs/logo_devs.pic: res/devs/logo_devs.png
	$(GFXCONV) -s 8 -o 16 -u 16 -e 1 -p -m -i $<

# Cielo del título: tabla HDMA con un color por línea (-n), sale como .as con datos.
res/titulo/cielo_grad_data.as: res/titulo/cielo.bmp
	$(GFXCONV) -n -t bmp -i $<

# Todos los assets convertidos. data.asm los incrusta con .incbin, y make no
# ve esa dependencia: declaramos que data.obj depende de ellos para que se
# reensamble cuando cambie cualquier imagen.
ASSETS := res/fuente/ark_consola.pic res/fuente/ark_consola_2bpp.pic res/personajes/tarouh_lobo.pic res/personajes/barbara_samurai.pic res/nintendo/logo_nintendo.pic res/publisher/logo_publisher.pic res/devs/logo_devs.pic res/intro/escena_1.pic res/intro/escena_2.pic res/intro/escena_3.pic res/titulo/fondo_lejos.pic res/titulo/fondo_cerca.pic res/titulo/cielo_grad_data.as

.PHONY: bitmaps cleanGfxRes
bitmaps: $(ASSETS)
data.obj: $(ASSETS)

# Borra las salidas de conversión que viven dentro de res/ (cleanGfx solo mira la raíz).
cleanGfxRes:
	@echo clean res graphics data
	@find res -type f \( -name "*.pic" -o -name "*.pal" -o -name "*.map" -o -name "*.inc" -o -name "*_data.as" -o -name "*.pc7" -o -name "*.mp7" -o -name "*_grad_data.as" \) -not -path "res/descargas/*" -delete

# Regla para limpiar los binarios y recursos
clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes
