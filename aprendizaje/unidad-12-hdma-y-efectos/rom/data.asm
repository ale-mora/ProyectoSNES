; ---------------------------------------------------------------------------------
;   data.asm — Los gráficos de la mini-ROM de la unidad 12, pegados en la ROM.
;
;   PROPÓSITO
;     Como el data.asm del proyecto: .incbin pega en la ROM los archivos de
;     gfx4snes y una etiqueta les da nombre para que el C los vea con
;     `extern char nombre;`. Las etiquetas _end marcan el final.
;
;   SE CONECTA CON
;     Incluye:  hdr.asm (lo genera snes_rules: cabecera de la ROM).
;     Lee:      res/fuente.pic/.pal y res/montanas.pic/.pal/.map (gfx4snes).
;     Lo usa:   src/efectos12.c.
;
;   HARDWARE QUE TOCA
;     ROM: dos secciones "superfree" (el enlazador elige el banco; cada una < 32 KB).
;
;   VARIABLES (etiquetas que exporta)
;     fuente_til/_til_end/_pal                      fuente 4bpp, 96 tiles
;     montanas_til/_til_end/_pal/_map/_map_end      las montañas del título y su mapa
;   → glosario: Banco / LoROM, Tile, Tilemap, Paleta
; ---------------------------------------------------------------------------------
.include "hdr.asm"

.section ".rodata_fuente" superfree
fuente_til:     .incbin "res/fuente.pic"        ; 96 tiles x 32 bytes
fuente_til_end:
fuente_pal:     .incbin "res/fuente.pal"        ; 2 colores x 2 bytes
.ends

.section ".rodata_montanas" superfree
montanas_til:   .incbin "res/montanas.pic"      ; los tiles distintos de la imagen
montanas_til_end:
montanas_pal:   .incbin "res/montanas.pal"      ; 16 colores x 2 bytes
montanas_map:   .incbin "res/montanas.map"      ; 32x28 celdas x 2 bytes
montanas_map_end:
.ends
