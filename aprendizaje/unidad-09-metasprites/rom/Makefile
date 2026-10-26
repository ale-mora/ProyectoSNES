# ---------------------------------------------------------------------------------
#   Makefile — Mini-ROM de la unidad 09: Tarou como metasprite de 96x64 con el
#   motor de sprites dinámicos de PVSnesLib.
#
#   PROPÓSITO
#     Partir del ejemplo del SDK graphics/Sprites/DynamicEngineMetaSprite (mismo
#     esquema: snes_rules, una regla gfx4snes por imagen, data.asm) con dos
#     tiras de Tarou (reposo y caminar) convertidas EXACTAMENTE como en el
#     Makefile del juego: piezas de 32x32 (-s 32), tabla metasprite (-T) por
#     celda de 96x64 (-X 96 -Y 64) y prioridad 3 (-P 3).
#
#   SE CONECTA CON
#     Lee:      res/tarou_reposo.png, res/tarou_caminar.png (copias de
#               res/personajes/tarou/ del juego), res/fuente.png (la fuente del
#               ejemplo del SDK).
#     Genera:   res/*.pic, *.pal, *_meta.inc (ignorados por git) y u09_metasprite.sfc.
#     Lo usan:  src/main.c (incluye los _meta.inc), data.asm (.incbin de .pic/.pal).
#
#   HARDWARE QUE TOCA
#     ninguno (es la receta de compilación).
#
#   Compilar (PowerShell, dentro de esta carpeta):
#     $env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make
# ---------------------------------------------------------------------------------
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Falta la variable de entorno PVSNESLIB_HOME (ver el README de la unidad)")
endif

# Nombre del .sfc y título de la cabecera (21 caracteres como mucho).
export ROMNAME := u09_metasprite
export ROMTITLE := U09 METASPRITE

# FastROM (como el Makefile del juego): el código en los bancos $80-$FF corre a
# 3,58 MHz en vez de 2,68. Aquí importa porque el motor dinámico prepara cada
# DMA con instrucciones: más rápido = más margen en las 37 líneas del VBlank.
# Debe ir ANTES del include de snes_rules (y tras cambiarlo, make clean).
export FASTROM := 1

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

# Los _meta.inc que genera gfx4snes quedan en res/: main.c los incluye por esta ruta.
CFLAGS += -Ires

.PHONY: bitmaps all cleanLogs

# Como el Makefile del juego y hello_world del SDK: la compilación pasa por
# buildWithSummary, que guarda el detalle en el .log e imprime un resumen
# ("No warnings or errors found." si todo fue bien).
all: buildWithSummary
buildActual: bitmaps $(OBJS) $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanLogs
	rm -f res/*.pic res/*.pal res/*.inc res/*_data.as

# Las tiras de Tarou: misma conversión que la regla patrón del Makefile del juego.
#   -s 32         piezas (bloques) de 32x32
#   -o 16 -u 16   16 colores de salida, paleta de 16
#   -p            escribe también la paleta (.pal)
#   -T            escribe la tabla metasprite (<nombre>_meta.inc)
#   -X 96 -Y 64   tamaño de la celda de un cuadro: 3x2 piezas
#   -P 3          prioridad 3 en los atributos de cada pieza
res/%.pic: res/%.png
	@echo convertir metasprite ... $(notdir $<)
	$(GFXCONV) -s 32 -o 16 -u 16 -p -T -X 96 -Y 64 -P 3 -i $<

# La fuente de la consola de texto (como en el ejemplo del SDK).
res/fuente.pic: res/fuente.png
	@echo convertir fuente ... $(notdir $<)
	$(GFXCONV) -s 8 -o 2 -u 16 -e 1 -i $<

bitmaps: res/tarou_reposo.pic res/tarou_caminar.pic res/fuente.pic

# data.asm incrusta los .pic con .incbin: que se reensamble si cambian.
data.obj: res/tarou_reposo.pic res/tarou_caminar.pic res/fuente.pic
