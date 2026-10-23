; ---------------------------------------------------------------------------------
;   dma_copias.asm — dma_lista_copias(): pasa n copias de una subida a la lista de DMA.
;
;   PROPÓSITO
;     Lo mismo que n llamadas a dma_lista_anadir() desde C, en un bucle de
;     ensamblador. La prueba de carga midió que, con un ninja que sube su cuadro
;     (hasta 6 copias) y Tarou (3), las llamadas desde C con 5 parámetros cada
;     una costaban decenas de líneas por frame. Una Copia (juego/subida.h) y una
;     entrada de la lista (juego/dma_lista.h) empiezan igual: origen, vram, bytes
;     (6 bytes); aquí se copian esos 6 y se completan el banco y VMAIN = $80 (tiles).
;     Si la lista se llena (32), las que no caben se pierden (como en dma_lista_anadir).
;     Convención de tcc816 (aprendizaje/unidad-15): C empuja de derecha a izquierda
;     (u8 = 1 byte, puntero = 4) y llama con jsr.l; tras el php de la entrada:
;       5,s copias (puntero: dirección y banco)   9,s n (u8)   10,s banco (u8)
;     Vuelve con rtl y P como estaba; usa A, X, Y, tcc__r0/r0h y tcc__r1.
;
;   SE CONECTA CON
;     Lo usan:  juego/dma_lista.h (dma_lista_copias), pantallas/prueba_carga.c.
;     Escribe:  dma_lista_n y dma_lista_items (dma_ejecutar.asm).
;
;   HARDWARE QUE TOCA
;     ninguno: solo prepara la lista (la dispara dma_lista_ejecutar en el VBlank).
;
;   VARIABLES: ninguna propia (tcc__r0/r0h: puntero a la copia; tcc__r1: las que faltan).
;   → glosario: DMA / HDMA, Ensamblador 65816
; ---------------------------------------------------------------------------------
.include "hdr.asm"
.accu 16
.index 16
.16bit

.ifdef FASTROM
.BASE $80                     ; el código, en los bancos rápidos ($80-$FF)
.endif

.SECTION ".dma_copias_text" SUPERFREE

dma_lista_copias:
    php
    rep #$30                    ; A, X e Y de 16 bits
    lda 5,s
    sta.b tcc__r0               ; puntero a la primera copia: dirección...
    lda 7,s
    sta.b tcc__r0h              ; ...y banco
    lda 9,s
    and.w #$00FF
    beq _fin                    ; ninguna copia
    sta.b tcc__r1               ; las que faltan
_copia:
    lda.l dma_lista_n
    and.w #$00FF
    cmp.w #32
    bcs _fin                    ; la lista está llena
    asl a
    asl a
    asl a
    tax                         ; X = 8 × n: la entrada libre
    ldy.w #0
    lda [tcc__r0],y
    sta.l dma_lista_items,x     ; origen
    ldy.w #2
    lda [tcc__r0],y
    sta.l dma_lista_items+2,x   ; vram
    ldy.w #4
    lda [tcc__r0],y
    sta.l dma_lista_items+4,x   ; bytes
    lda 10,s
    and.w #$00FF
    ora.w #$8000                ; banco (bajo) | VMAIN $80 (alto): tiles, +1 por palabra
    sta.l dma_lista_items+6,x
    sep #$20
    lda.l dma_lista_n
    inc a
    sta.l dma_lista_n           ; una más en la lista
    rep #$20
    lda.b tcc__r0
    clc
    adc.w #8
    sta.b tcc__r0               ; la copia siguiente (8 bytes cada una)
    dec.b tcc__r1
    bne _copia
_fin:
    plp
    rtl

.ENDS
