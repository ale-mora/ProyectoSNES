ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Por favor configura la variable de entorno PVSNESLIB_HOME apuntando al directorio raíz de PVSnesLib")
endif

# Nombre y título de la ROM de salida
export ROMNAME := ProyectoSNES
export ROMTITLE := TAROU DEMO

# --- Audio (se activa en la tarea 3 del plan, cuando haya módulos .it) ---------
# AUDIOFILES y SOUNDBANK deben declararse ANTES del include de snes_rules.
# Los .it de efectos van primero en la lista.
# export AUDIOFILES := res/musica/jingle_publisher.it res/musica/titulo.it
# export SOUNDBANK  := res/musica/soundbank

# --- Tamaño de la ROM: 16 bancos de 32 KB = 512 KB ---------------------------
# Decisión del 2026-10-16 (docs/diseno-demo-jugable.md §2, decisión 4): los ~55
# cuadros de 64x64 de Tarou ocupan ~110 KB y con 8 bancos (256 KB) quedaban
# solo ~115 KB libres. ROMSIZE es el byte de tamaño de la cabecera de la SNES
# (08 = 256 KB, 09 = 512 KB) y ROMBANKS cuántas secciones de 32 KB reparte el
# enlazador. Deben ir ANTES del include de snes_rules.
export ROMSIZE  := 09
export ROMBANKS := 16

# Incluir reglas estándar de compilación de PVSnesLib
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

# Los fuentes viven en subcarpetas de src/ (sistema/, pantallas/, juego/...) y se
# incluyen con ruta desde src/: #include "sistema/comun.h". snes_rules ya pone
# -I$(CURDIR); añadimos -Isrc DESPUÉS del include porque las recetas expanden
# CFLAGS al ejecutarse, no al definirse. Decisión 13 de docs/diseno-demo-jugable.md.
CFLAGS += -Isrc
# tarou_anim.h (cuántos cuadros tiene cada animación) lo genera el pipeline en res/.
CFLAGS += -Ires/personajes/tarou

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
res/personajes/lobo_samurai.pic: res/personajes/lobo_samurai.png
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

# Capa lejana del nivel 1: la del título sin logotipo y duplicada en espejo hasta
# 512 px (scripts/preparar_fondo_nivel.py). -y = mapa en páginas de 32x32 (SC_64x32).
res/nivel1/fondo_lejos.pic: res/nivel1/fondo_lejos.png
	$(GFXCONV) -y -s 8 -o 16 -u 16 -e 2 -p -m -i $<

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
# Animaciones de Tarou (Revisión 1, docs/diseno-demo-jugable.md §7 y §10): cada
# tira de cuadros de 64x64 se convierte en piezas de 32x32 (-s 32) con una tabla
# de metasprite (-T) de 64x64 (-X 64 -Y 64) y prioridad 3 (-P 3). Por cada
# tarou_<anim>.png salen tarou_<anim>.pic (tiles), .pal (paleta, igual en todas)
# y tarou_<anim>_meta.inc (tablas t_metasprite que incluye src/juego/tarou_sprite.c).
TAROU_ANIMS := reposo caminar correr saltar caer agachado agachado_caminar desenvainar envainar atacar echarse dormido despertar
TAROU_PICS := $(foreach a,$(TAROU_ANIMS),res/personajes/tarou/tarou_$(a).pic)
res/personajes/tarou/%.pic: res/personajes/tarou/%.png
	@echo convert metasprite ... $(notdir $<)
	$(GFXCONV) -s 32 -o 16 -u 16 -p -T -X 64 -Y 64 -P 3 -i $<

ASSETS := res/fuente/ark_consola.pic res/fuente/ark_consola_2bpp.pic res/personajes/lobo_samurai.pic res/personajes/barbara_samurai.pic res/nintendo/logo_nintendo.pic res/publisher/logo_publisher.pic res/devs/logo_devs.pic res/intro/escena_1.pic res/intro/escena_2.pic res/intro/escena_3.pic res/titulo/fondo_lejos.pic res/titulo/fondo_cerca.pic res/titulo/cielo_grad_data.as res/nivel1/fondo_lejos.pic $(TAROU_PICS)

.PHONY: bitmaps cleanGfxRes
bitmaps: $(ASSETS)
data.obj: $(ASSETS)

# Borra las salidas de conversión que viven dentro de res/ (cleanGfx solo mira la raíz).
cleanGfxRes:
	@echo clean res graphics data
	@find res -type f \( -name "*.pic" -o -name "*.pal" -o -name "*.map" -o -name "*.inc" -o -name "*_data.as" -o -name "*.pc7" -o -name "*.mp7" -o -name "*_grad_data.as" \) -not -path "res/descargas/*" -delete

# Regla para limpiar los binarios y recursos
clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes
