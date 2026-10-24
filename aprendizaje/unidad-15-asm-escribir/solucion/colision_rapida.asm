;---------------------------------------------------------------------------------
;   colision_rapida.asm — colision_apoyan() en ensamblador.
;   (Solución de la ampliación del ejercicio 15. Va en src/juego/colision_rapida.asm
;   del juego, con el parche colision.c.patch.)
;
;   PROPÓSITO
;     ¿Sostiene algo a alguno de los dos pies? Devuelve 1 si la celda de
;     (xa, y) o la de (xb, y) es sólida o plataforma. Es la consulta que
;     juego/tarou.c (apoyado) hace en CADA frame que Tarou está en el suelo y en
;     cada borde de celda que cruza al caer: la más repetida de la física hoy
;     (colision_en, la que nombraba el plan, ya no la llama el juego: la
;     sustituyeron las consultas de columna y fila de colision.c).
;     Mismo algoritmo que la versión en C (que se queda para las pruebas del PC):
;         fuera de la pantalla (y < 0 o y >= 224) o sin rejilla  -> 0
;         base = (y & $FFF0) << 3                 (fila * 128)
;         celda = rejilla[base | (x >> 4)]        (si x < 2048)
;     El .asm que genera tcc816 para la versión en C son 247 líneas: compara con banderas
;     calculadas en X, guarda la celda en la pila y la vuelve a cargar.
;
;     Se llama colision_rapida.asm: no puede llamarse como ningún .c
;     (colision.c pasa por un colision.asm intermedio).
;
;   SE CONECTA CON
;     La declara: src/juego/colision.h (para C es una función más).
;     La llama:   src/juego/tarou.c (apoyado, desde mover_y).
;     Lee:        colision_rejilla (colision.c, sin static: puntero de 4 bytes,
;                 16 bits bajos + banco).
;
;   HARDWARE QUE TOCA
;     ninguno: lee la rejilla de la ROM con un puntero largo.
;
;   VARIABLES: ninguna propia.
;
;   CONVENCIÓN DE LLAMADA (tcc816): u8 colision_apoyan(u16 xa, u16 xb, s16 y)
;     Al entrar: 4,s xa   6,s xb   8,s y   (tras el php: 5,s 7,s 9,s).
;     Devuelve 0 o 1 en tcc__r0 (el que llama lee su byte bajo).
;     Usa A, Y, tcc__r0, tcc__r0h y tcc__r1.
;   → glosario: Colisión por celdas (rejilla de colisión)
;---------------------------------------------------------------------------------
.include "hdr.asm"
.accu 16
.index 16
.16bit

.define COL_SOLIDO      1       ; colision.h
.define COL_PLATAFORMA  2
.define ALTO_PX         224     ; 14 filas de 16 px
.define ANCHO_PX        2048    ; 128 columnas de 16 px

.SECTION ".colision_rapida_text" SUPERFREE

;--- u8 colision_apoyan(u16 xa, u16 xb, s16 y) -----------------------------------
colision_apoyan:
    php                         ; guarda P del que llama
    rep #$30                    ; A, X e Y de 16 bits
    lda 9,s                     ; y
    cmp.w #ALTO_PX              ; SIN signo: una y negativa es >= $8000, también fuera
    bcs _no                     ; fuera de la pantalla: nada sostiene
    and.w #$FFF0                ; la fila, en múltiplos de 16...
    asl a
    asl a
    asl a                       ; ...por 8: fila * 128 (primera celda de la fila)
    sta.b tcc__r1               ; tcc__r1 = base
    lda.l colision_rejilla      ; puntero a la rejilla: 16 bits bajos...
    sta.b tcc__r0
    lda.l colision_rejilla+2    ; ...y banco
    sta.b tcc__r0h              ; tcc__r0/tcc__r0h: puntero largo para [tcc__r0],y
    ora.b tcc__r0               ; ¿puntero entero a 0?
    beq _no                     ; sin nivel: nada sostiene
    lda 5,s                     ; xa
    jsr _pie                    ; ¿sostiene? (C = 1 si sí)
    bcs _si
    lda 7,s                     ; xb (ojo: dentro de _pie la pila tiene 2 bytes más,
                                ; pero aquí ya hemos vuelto: 7,s es xb otra vez)
    jsr _pie
    bcs _si
_no:
    lda.w #0                    ; 0: no sostiene
    sta.b tcc__r0               ; valor devuelto
    plp
    rtl
_si:
    lda.w #1                    ; 1: sostiene
    sta.b tcc__r0
    plp
    rtl

;--- _pie: A = x del pie, tcc__r1 = base, tcc__r0 = rejilla -> C = 1 si sostiene ---
;    Subrutina local CORTA (jsr/rts, dentro del mismo banco): no la ve C.
_pie:
    cmp.w #ANCHO_PX             ; ¿fuera del nivel por los lados?
    bcs _pie_no                 ; sí: este pie no cuenta (como en C)
    lsr a
    lsr a
    lsr a
    lsr a                       ; columna = x >> 4
    ora.b tcc__r1               ; índice = base | columna
    tay                         ; Y = índice (16 bits)
    sep #$20                    ; A de 8 bits: la celda es un byte
    lda [tcc__r0],y             ; celda = rejilla[índice]
    cmp.b #COL_SOLIDO
    beq _pie_si
    cmp.b #COL_PLATAFORMA
    beq _pie_si
    rep #$20                    ; A de 16 bits otra vez
_pie_no:
    clc                         ; C = 0: no sostiene
    rts
_pie_si:
    rep #$20                    ; A de 16 bits otra vez
    sec                         ; C = 1: sostiene
    rts

.ENDS
