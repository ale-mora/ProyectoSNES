; ---------------------------------------------------------------------------------
;   data.asm — Datos binarios (gráficos, paletas, música) que van dentro de la ROM.
;
;   Este archivo es ensamblador, no C. Su único trabajo es "pegar" archivos
;   binarios en la ROM con .incbin y darles un nombre (etiqueta) para que el
;   código C los encuentre con `extern`. Cada bloque va en una sección
;   "superfree": el enlazador la coloca en cualquier banco de 32 KB donde quepa,
;   así que ningún asset puede superar ese tamaño. → glosario: Banco / LoROM
; ---------------------------------------------------------------------------------
.include "hdr.asm"                 ; Mapa de memoria y cabecera generados por snes_rules.

; --- Fuente de la consola de texto (Ark Pixel 10 px, mayúsculas) -------------------
.section ".rodata_fuente" superfree

fuente_consola_til:                ; 96 tiles de 8x8 (uno por carácter ASCII 32..127).
.incbin "res/fuente/ark_consola.pic"
fuente_consola_til_end:

fuente_consola_pal:                ; Paleta: color 0 transparente, color 1 el trazo.
.incbin "res/fuente/ark_consola.pal"
fuente_consola_pal_end:

.ends
