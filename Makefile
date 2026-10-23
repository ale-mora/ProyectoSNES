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

# --- Tamaño de la ROM: 128 bancos de 32 KB = 4 MB (32 Mbit), LoROM + FastROM --
# Decisión del 2026-10-21: el máximo de los mapas estándar de la SNES y el mayor
# que sabe generar la cabecera de PVSnesLib (los cartuchos comerciales de 6 MB
# usaban ExHiROM, que el SDK no admite). Antes: 16 bancos (512 KB) y solo ~45 KB
# libres. ROMSIZE es el byte de tamaño de la cabecera (08 = 256 KB, 09 = 512 KB,
# 0A = 1 MB, 0B = 2 MB, 0C = 4 MB: 2^n KB) y ROMBANKS cuántas secciones de 32 KB
# reparte el enlazador.
#
# Por qué FastROM: en LoROM los bancos de la ROM se ven en $00-$7D y $80-$FF,
# pero $7E y $7F son la WRAM. Con SlowROM el enlazador numera los bancos desde
# $00, así que los bancos 126 y 127 caerían ENCIMA de la WRAM. Con FastROM los
# numera desde $80 (.BASE $80 en hdr.asm): los 128 caben en $80-$FF. Y de
# regalo, el código que corre en $80-$FF va a 3,58 MHz en vez de 2,68 MHz
# (snes_rules compila con -F y enlaza la librería LoROM_FastROM).
# Las tres variables deben ir ANTES del include de snes_rules.
export ROMSIZE  := 0C
export ROMBANKS := 128
export FASTROM  := 1

# Incluir reglas estándar de compilación de PVSnesLib
include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

# Los fuentes viven en subcarpetas de src/ (sistema/, pantallas/, juego/...) y se
# incluyen con ruta desde src/: #include "sistema/comun.h". snes_rules ya pone
# -I$(CURDIR); añadimos -Isrc DESPUÉS del include porque las recetas expanden
# CFLAGS al ejecutarse, no al definirse. Decisión 13 de docs/diseno-demo-jugable.md.
CFLAGS += -Isrc
# tarou_anim.h (cuántos cuadros tiene cada animación) lo genera el pipeline en res/.
CFLAGS += -Ires/personajes/tarou -Ires/nivel1

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

# Sprites del pack (Bárbara; el lobo queda para los enemigos de la Etapa 3): tiras
# de cuadros de 64x64 (-s 64), 16 colores, PNG. gfx4snes empaqueta los bloques en
# filas de 128 px; ver tile_del_bloque64 en src/pantallas/personajes.c.
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

# Capa cercana del nivel 1 (el tutorial): la lógica dibujada en Tiled vestida con el
# tileset Sidescroller de Pixellab, el seto y los carteles (scripts/preparar_nivel.py).
# 2048 px más 32 del «parche» del tronco (2080): SIN -y, así que el mapa sale por filas
# de 260 tiles; paleta 1 (-e 1).
# El juego no carga ese mapa de golpe (no cabe en un mapa de 64x32 de la VRAM): copia
# columnas sueltas según avanza la cámara, y para eso mapa_a_columnas.py lo reordena
# columna a columna (src/juego/fondo_nivel.c).
# -F reutiliza también los tiles volteados (el mapa guarda el volteo en cada celda):
# 281 → 260 tiles (medido en la unidad 06 de aprendizaje/, 2026-10-22).
res/nivel1/fondo_cerca.pic: res/nivel1/fondo_cerca.png
	$(GFXCONV) -s 8 -o 16 -u 16 -e 1 -p -m -F -i $<

res/nivel1/fondo_cerca.col: res/nivel1/fondo_cerca.pic
	@echo map to columns ... $(notdir $@)
	python scripts/mapa_a_columnas.py res/nivel1/fondo_cerca.map $@ --ancho 260

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
# tira de cuadros de 96x64 se convierte en piezas de 32x32 (-s 32) con una tabla
# de metasprite (-T) de 96x64 (-X 96 -Y 64: 3x2 piezas) y prioridad 3 (-P 3). Por cada
# tarou_<anim>.png salen tarou_<anim>.pic (tiles), .pal (paleta, igual en todas)
# y tarou_<anim>_meta.inc (tablas t_metasprite que incluye src/juego/tarou_sprite.c).
TAROU_ANIMS := reposo caminar correr saltar caer agachado agachado_caminar desenvainar envainar atacar echarse dormido despertar guardia
TAROU_PICS := $(foreach a,$(TAROU_ANIMS),res/personajes/tarou/tarou_$(a).pic)
# Más la tira ESTÁTICA de reposo (siempre 2 cuadros) que intro y título cargan de
# una vez en VRAM (src/pantallas/personajes.c); misma conversión que las demás.
TAROU_PICS += res/personajes/tarou/tarou_reposo_estatico.pic
res/personajes/tarou/%.pic: res/personajes/tarou/%.png
	@echo convert metasprite ... $(notdir $<)
	$(GFXCONV) -s 32 -o 16 -u 16 -p -T -X 96 -Y 64 -P 3 -i $<

# Las mismas tiras de Tarou en piezas de 16x16 sin las vacías (formato .p16, para la capa
# de sprites del beat 'em up: docs/diseno-beat-em-up.md §8 v3). Las genera
# scripts/piezas16.py desde los PNG versionados; data.asm las incluye con .incbin.
TAROU_P16 := $(foreach a,$(TAROU_ANIMS),res/personajes/tarou/tarou_$(a).p16)
res/personajes/tarou/%.p16: res/personajes/tarou/%.png scripts/piezas16.py
	@echo convert pieces16 ... $(notdir $<)
	python scripts/piezas16.py $< $@ --celda 96x64

ASSETS := res/fuente/ark_consola.pic res/fuente/ark_consola_2bpp.pic res/personajes/lobo_samurai.pic res/personajes/barbara_samurai.pic res/nintendo/logo_nintendo.pic res/publisher/logo_publisher.pic res/devs/logo_devs.pic res/intro/escena_1.pic res/intro/escena_2.pic res/intro/escena_3.pic res/titulo/fondo_lejos.pic res/titulo/fondo_cerca.pic res/titulo/cielo_grad_data.as res/nivel1/fondo_lejos.pic res/nivel1/fondo_cerca.pic res/nivel1/fondo_cerca.col res/nivel1/colision.bin $(TAROU_PICS) $(TAROU_P16)

.PHONY: bitmaps cleanGfxRes
bitmaps: $(ASSETS)
data.obj: $(ASSETS)

# Los .c que incluyen archivos GENERADOS (las tablas _meta.inc de gfx4snes -T y
# tarou_anim.h del pipeline): snes_rules no ve esos #include, así que se declara
# aquí la dependencia sobre el .obj (igual que data.obj). Sin estas líneas, al
# regenerar el arte de Tarou el código seguía con las tablas y los cuadros viejos
# hasta un make clean.
src/juego/tarou_sprite.obj: $(TAROU_PICS)
src/pantallas/personajes.obj: res/personajes/tarou/tarou_reposo_estatico.pic
src/juego/animaciones.obj: res/personajes/tarou/tarou_anim.h
src/pantallas/pantalla_nivel1.obj: res/nivel1/nivel1_datos.h

# ROM de prueba de carga (docs/diseno-beat-em-up.md §9; solo con CFLAGS=-DPRUEBA_CARGA):
# los ninjas provisionales que genera scripts/preparar_prueba_carga.py. Son .bin
# versionados (el pack de origen no se versiona), así que make no los regenera.
PRUEBA_BINS := $(wildcard res/prueba_carga/*.bin)
data.obj: $(PRUEBA_BINS)

# Borra las salidas de conversión que viven dentro de res/ (cleanGfx solo mira la raíz).
cleanGfxRes:
	@echo clean res graphics data
	@find res -type f \( -name "*.pic" -o -name "*.pal" -o -name "*.map" -o -name "*.col" -o -name "*.inc" -o -name "*_data.as" -o -name "*.pc7" -o -name "*.mp7" -o -name "*.p16" -o -name "*_grad_data.as" \) -not -path "res/descargas/*" -delete

# Regla para limpiar los binarios y recursos
clean: cleanBuildRes cleanRom cleanGfx cleanGfxRes
