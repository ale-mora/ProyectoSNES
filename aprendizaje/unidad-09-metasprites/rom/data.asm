;---------------------------------------------------------------------------------
;   data.asm — Los gráficos de la mini-ROM de la unidad 09, incrustados en la ROM.
;
;   PROPÓSITO
;     Cada tira de Tarou va en su propia sección "superfree" (el enlazador elige
;     el banco), igual que las .rodata_tarou_* de data.asm del juego: una
;     sección no puede cruzar un banco de 32 KB, y 9 cuadros de 96x64 son
;     9 x 3 KB = 27 KB. Las etiquetas se ven desde C como `extern char nombre;`.
;
;   SE CONECTA CON
;     Lee:     res/tarou_reposo.pic/.pal, res/tarou_caminar.pic, res/fuente.pic/.pal
;              (los genera gfx4snes desde los PNG, ver Makefile).
;     Lo usan: src/main.c (tarou_reposo_til, tarou_caminar_til, tarou_pal,
;              fuente_til, fuente_pal).
;
;   HARDWARE QUE TOCA
;     ninguno: son datos en la ROM.
;
;   VARIABLES (etiquetas)
;     tarou_reposo_til / _fin    tiles de los 9 cuadros de reposo (27 KB)
;     tarou_caminar_til / _fin   tiles de los 8 cuadros de caminar (24 KB)
;     tarou_pal                  la paleta de 16 colores (igual en todas las tiras)
;     fuente_til / fuente_pal    la fuente de la consola de texto
;---------------------------------------------------------------------------------
.include "hdr.asm"

.section ".rodata_fuente" superfree
fuente_til:          .incbin "res/fuente.pic"
fuente_til_fin:
fuente_pal:          .incbin "res/fuente.pal"
.ends

.section ".rodata_tarou_reposo" superfree
tarou_reposo_til:    .incbin "res/tarou_reposo.pic"
tarou_reposo_til_fin:
tarou_pal:           .incbin "res/tarou_reposo.pal"
.ends

.section ".rodata_tarou_caminar" superfree
tarou_caminar_til:   .incbin "res/tarou_caminar.pic"
tarou_caminar_til_fin:
.ends
