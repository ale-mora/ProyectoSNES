; ---------------------------------------------------------------------------------
;   data.asm — Los gráficos de la mini-ROM de la unidad 08.
;
;   PROPÓSITO
;     Pegar en la ROM (.incbin) la fuente y las dos tiras de sprites que convierte
;     gfx4snes, con etiquetas que el C ve como `extern char`.
;
;   SE CONECTA CON
;     Incluye:  hdr.asm (generado por snes_rules).
;     Lee:      res/fuente.*, res/barbara.*, res/lobo.* (gfx4snes).
;     Lo usa:   src/sprites.c.
;
;   HARDWARE QUE TOCA
;     ROM: tres secciones "superfree" (8 KB por tira: cada una < 32 KB).
;
;   VARIABLES (etiquetas que exporta)
;     fuente_til/_pal                 fuente 4bpp
;     barbara_til/_til_end/_pal       4 cuadros de 64x64 = 256 tiles (8 KB)
;     lobo_til/_til_end/_pal          4 cuadros de 64x64 = 256 tiles (8 KB)
;   → glosario: Banco / LoROM, Sprite
; ---------------------------------------------------------------------------------
.include "hdr.asm"

.section ".rodata_fuente" superfree
fuente_til:     .incbin "res/fuente.pic"
fuente_pal:     .incbin "res/fuente.pal"
.ends

.section ".rodata_barbara" superfree
barbara_til:    .incbin "res/barbara.pic"
barbara_til_end:
barbara_pal:    .incbin "res/barbara.pal"
.ends

.section ".rodata_lobo" superfree
lobo_til:       .incbin "res/lobo.pic"
lobo_til_end:
lobo_pal:       .incbin "res/lobo.pal"
.ends
