;---------------------------------------------------------------------------------
;   data.asm — Assets de la mini-ROM de la unidad 17: la fuente y el efecto BRR.
;
;   PROPÓSITO
;     Incrustar en la ROM la fuente de la consola de texto y la muestra BRR del
;     salto. La música NO va aquí: smconv genera su propio res/soundbank.asm
;     (banco 5 entero para él) y snes_rules lo ensambla solo.
;
;   SE CONECTA CON
;     Incluye:  hdr.asm (cabecera generada por snes_rules).
;     Lee:      res/pvsneslibfont.pic/.pal (gfx4snes), res/mariojump.brr (snesbrr).
;     Lo usan:  src/audio17.c (extern char snesfont, snespal, saltobrr, saltobrr_fin).
;
;   HARDWARE QUE TOCA
;     ninguno: son datos de la ROM. La muestra la copia el driver al SPC700.
;
;   VARIABLES (etiquetas)
;     snesfont / snespal         tiles y paleta de la fuente
;     saltobrr / saltobrr_fin    principio y fin de la muestra: su resta es la longitud
;
;   → glosario: Banco / LoROM, Soundbank / módulo .it
;---------------------------------------------------------------------------------
.include "hdr.asm"

.section ".rodata1" superfree          ; Una sección libre: el enlazador elige el banco.

snesfont:                              ; Tiles de la fuente (4bpp).
.incbin "res/pvsneslibfont.pic"

snespal:                               ; Su paleta.
.incbin "res/pvsneslibfont.pal"

saltobrr:                              ; La muestra BRR del salto (3771 bytes).
.incbin "res/mariojump.brr"
saltobrr_fin:                          ; Fin: saltobrr_fin - saltobrr = longitud.

.ends
