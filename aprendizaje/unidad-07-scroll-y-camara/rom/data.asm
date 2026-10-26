; ---------------------------------------------------------------------------------
;   data.asm — Los gráficos de la mini-ROM de la unidad 07.
;
;   PROPÓSITO
;     Pegar en la ROM (.incbin) la fuente de 2bpp y las dos capas de 512 px que
;     convierte gfx4snes, con etiquetas que el C ve como `extern char`.
;
;   SE CONECTA CON
;     Incluye:  hdr.asm (generado por snes_rules).
;     Lee:      res/fuente2bpp.*, res/cerca.*, res/lejos.* (gfx4snes).
;     Lo usa:   src/parallax.c.
;
;   HARDWARE QUE TOCA
;     ROM: tres secciones "superfree" (cada una < 32 KB).
;
;   VARIABLES (etiquetas que exporta)
;     fuente_til/_pal                           fuente de 4 colores
;     cerca_til/_til_end/_pal/_map/_map_end     arbustos, mapa 64x32 (4 KB)
;     lejos_til/_til_end/_pal/_map/_map_end     montañas, mapa 64x32 (4 KB)
;   → glosario: Banco / LoROM, Tilemap
; ---------------------------------------------------------------------------------
.include "hdr.asm"

.section ".rodata_fuente" superfree
fuente_til:     .incbin "res/fuente2bpp.pic"    ; 96 tiles x 16 bytes
fuente_pal:     .incbin "res/fuente2bpp.pal"
.ends

.section ".rodata_cerca" superfree
cerca_til:      .incbin "res/cerca.pic"
cerca_til_end:
cerca_pal:      .incbin "res/cerca.pal"
cerca_map:      .incbin "res/cerca.map"         ; 64x32 entradas = 4096 bytes
cerca_map_end:
.ends

.section ".rodata_lejos" superfree
lejos_til:      .incbin "res/lejos.pic"
lejos_til_end:
lejos_pal:      .incbin "res/lejos.pal"
lejos_map:      .incbin "res/lejos.map"         ; 64x32 entradas = 4096 bytes
lejos_map_end:
.ends
