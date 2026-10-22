;---------------------------------------------------------------------------------
;   nivel_anotado.asm — Solución del TODO 2 del ejercicio 14: el .asm que genera
;   tcc816 para src/juego/nivel.c (nivel_columna_vram), anotado. NO se ensambla.
;
;   PROPÓSITO
;     Identificar en el ensamblador cada pieza de
;         return (u16)(((col & 32) << 5) | (col & 31));
;     El original sin anotar está en asm_generado/nivel.asm (43 líneas).
;
;   SE CONECTA CON
;     src/juego/nivel.c, ejercicio_14.md (TODO 2), solucion_14.md.
;
;   HARDWARE QUE TOCA: ninguno.   VARIABLES: ninguna.
;   → glosario: Streaming de columnas (mapa-anillo)
;---------------------------------------------------------------------------------
.include "hdr.asm"      ; cabecera común (mapa de memoria, FASTROM)
.accu 16                ; el ensamblador supone A de 16 bits...
.index 16               ; ...y X/Y de 16 bits
.16bit
.BASE $80               ; FastROM: código en los bancos $80-$FF

; Sin `.define __nivel_columna_vram_locals`: la función NO tiene variables locales,
; así que tampoco hay `tsa / sbc / tas` al principio ni al final.

.SECTION ".nivel_columna_vramtext_0x0" SUPERFREE

nivel_columna_vram:     ; u16 nivel_columna_vram(u16 col)
lda 3 + 1,s             ; (1) A = col. Pila: 1,s..3,s = vuelta del jsr.l (3 bytes);
                        ;     4,s = col. Se escribe "3 + 1" porque tcc816 genera
                        ;     siempre "3 + locales + 1" y aquí locales = 0.
and.w #32               ; (2) A = col & 32 (0 o 32: ¿segunda página del mapa de 64?)
sta.b tcc__r0           ;     copia en tcc__r0 (no hace falta, pero tcc816 lo hace)
ldy.w #5                ; (3) << 5 como BUCLE: Y = 5 vueltas
-                       ;     etiqueta anónima "hacia atrás"
asl a                   ;     A <<= 1 (desplaza un bit a la izquierda)
dey                     ;     una vuelta menos (pone Z al llegar a 0)
bne -                   ;     quedan: vuelve al `-`
+                       ;     (etiqueta que tcc816 deja siempre, no se usa)
sta.b tcc__r0           ;     tcc__r0 = (col & 32) << 5  (0 o $400)
lda 3 + 1,s             ; (4) A = col otra vez (se vuelve a leer de la pila)
and.w #31               ;     A = col & 31 (columna dentro de la página)
sta.b tcc__r1           ;     tcc__r1 = col & 31
ora.b tcc__r0           ;     A = (col & 31) | ((col & 32) << 5)
sta.b tcc__r0           ; (5) EL VALOR DEVUELTO, en tcc__r0
__local_0:
rtl                     ;     vuelta larga

; CUENTA: 3 + 1 + 15 (5 vueltas de asl/dey/bne) + 6 + rtl = 26 instrucciones.
; ¿Por qué un bucle? Visto en los .asm del juego: tcc816 escribe seguidos los
; desplazamientos de hasta 4 bits (los `lsr a` x4 de `x >> 4` en colision.asm, los
; `asl a` x3 de `<< 3`) y a partir de 5 bits genera este bucle de `ldy #n` (también
; `* 64` en personajes.asm sale como `ldy.w #6`). Con las tablas del
; 65816: 5 x (asl 2 + dey 2 + bne 3) - 1 = 34 ciclos, frente a 10 de cinco `asl`
; seguidos. Por eso las funciones de colisión del juego escriben `(y & 0xFFF0) << 3`
; en vez de `(y >> 4) << 7`: 3 desplazamientos en vez de 11 (ver colision.c).
.ENDS
