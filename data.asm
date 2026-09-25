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

; --- Sprites de los personajes (solo para la demo) --------------------------------
; Cada uno en su propia sección: así ninguno cruza un banco de 32 KB.
.section ".rodata_tarouh" superfree

tarouh_lobo_til:                   ; 4 cuadros de 64x64 (bloques 0..3), 8 KB.
.incbin "res/personajes/tarouh_lobo.pic"
tarouh_lobo_til_end:

tarouh_lobo_pal:                   ; 16 colores, el 0 transparente.
.incbin "res/personajes/tarouh_lobo.pal"
tarouh_lobo_pal_end:

.ends

.section ".rodata_barbara" superfree

barbara_samurai_til:               ; 4 cuadros de 64x64 (bloques 4..7), 8 KB.
.incbin "res/personajes/barbara_samurai.pic"
barbara_samurai_til_end:

barbara_samurai_pal:
.incbin "res/personajes/barbara_samurai.pal"
barbara_samurai_pal_end:

.ends
