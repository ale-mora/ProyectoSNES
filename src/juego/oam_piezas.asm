; ---------------------------------------------------------------------------------
;   oam_piezas.asm — Escribe n piezas de 16x16 en el búfer de la OAM (ver oam_piezas.h).
;
;   PROPÓSITO
;     Lo mismo que n llamadas a oamSet + oamSetEx del SDK, pero en un solo
;     bucle de ensamblador: por pieza, 4 bytes en oamMemory (x, y, tile,
;     atributos) y sus 2 bits de la tabla alta a 0. Desde C costaba ~5 líneas
;     del haz por pieza (prueba de carga, 2026-10-23).
;     Convención de tcc816 (aprendizaje/unidad-15): C empuja los parámetros de
;     derecha a izquierda y llama con jsr.l; tras el php de la entrada, el
;     primero está en 5,s. Se vuelve con rtl y P como estaba; se usan A, X, Y y
;     tcc__r0..r4 (C no espera que sobrevivan).
;       5,s entrada (×4)   7,s x0   9,s y0   11,s tile_bajo   13,s atributos
;       15,s piezas (puntero de 4 bytes: dirección y banco)   19,s n   21,s espejo
;
;   SE CONECTA CON
;     Lo usan:  pantallas/prueba_carga.c (por juego/oam_piezas.h).
;     Lee:      la tabla de piezas (dx, dy) de ninja_datos.h, en la ROM (puntero largo).
;     Escribe:  oamMemory (el búfer de 544 bytes del SDK, en la WRAM).
;
;   HARDWARE QUE TOCA
;     ninguno: la NMI del SDK copia oamMemory a la OAM en el VBlank.
;
;   VARIABLES (página directa de tcc816, solo durante la llamada)
;     tcc__r0/r0h  puntero largo a la tabla de piezas
;     tcc__r1      piezas que faltan
;     tcc__r2      tile de la pieza actual (byte bajo)
;     tcc__r3      dx de la pieza actual
;     tcc__r4      número de entrada de la OAM (para la tabla alta)
;   → glosario: OAM, Ensamblador 65816
; ---------------------------------------------------------------------------------
.include "hdr.asm"
.accu 16
.index 16
.16bit

.ifdef FASTROM
.BASE $80                     ; el código, en los bancos rápidos ($80-$FF)
.endif

.SECTION ".oam_piezas_text" SUPERFREE

; Máscaras para borrar los 2 bits de una entrada en su byte de la tabla alta.
_mascaras: .db $FC, $F3, $CF, $3F

oam_piezas:
    php                         ; guarda P (los tamaños de A y X/Y del que llama)
    rep #$30                    ; A, X e Y de 16 bits
    lda 19,s                    ; n
    bne _hay
    plp
    rtl                         ; ninguna pieza: nada que hacer
_hay:
    sta.b tcc__r1               ; piezas que faltan
    lda 15,s
    sta.b tcc__r0               ; puntero a la tabla: dirección...
    lda 17,s
    sta.b tcc__r0h              ; ...y banco
    lda 11,s
    sta.b tcc__r2               ; tile de la primera pieza
    lda 5,s
    tax                         ; X = desplazamiento de la entrada en oamMemory
    lsr a
    lsr a
    sta.b tcc__r4               ; número de entrada (para la tabla alta)
    ldy.w #0                    ; Y = posición en la tabla de piezas
_bucle:
    lda [tcc__r0],y             ; A = dx | dy << 8
    and.w #$00FF
    sta.b tcc__r3               ; dx
    lda 21,s                    ; ¿mirando a la izquierda?
    beq _derecha
    lda 7,s
    sec
    sbc.b tcc__r3               ; x = x0 - dx
    bra _x
_derecha:
    lda 7,s
    clc
    adc.b tcc__r3               ; x = x0 + dx
_x:
    sep #$20                    ; A de 8 bits: los 4 bytes de la entrada
    sta.l oamMemory,x           ; x (los 8 bits bajos)
    iny
    lda [tcc__r0],y             ; dy
    clc
    adc 9,s                     ; y = y0 + dy
    sta.l oamMemory+1,x
    iny
    lda.b tcc__r2
    sta.l oamMemory+2,x         ; tile
    lda 13,s
    sta.l oamMemory+3,x         ; atributos (vhoopppN)
    lda.b tcc__r2               ; siguiente tile: +2...
    clc
    adc.b #2
    sta.b tcc__r2
    and.b #$0F
    bne _mismo_par
    lda.b tcc__r2               ; ...y tras 8 piezas (columna 16), el par siguiente
    clc
    adc.b #16
    sta.b tcc__r2
_mismo_par:
    rep #$20                    ; A de 16 bits
    phx                         ; X se usa para la tabla alta
    phy
    lda.b tcc__r4
    and.w #3
    tax                         ; X = posición dentro del byte (0..3)
    lda.b tcc__r4
    lsr a
    lsr a
    tay                         ; Y = byte de la tabla alta (entrada / 4)
    sep #$20
    lda.w oamMemory+512,y       ; (DB = $7E: oamMemory está en la WRAM)
    and.l _mascaras,x           ; x bit 8 = 0 y tamaño pequeño
    sta.w oamMemory+512,y
    rep #$20
    ply
    plx
    inc.b tcc__r4               ; entrada siguiente
    txa
    clc
    adc.w #4
    tax                         ; 4 bytes más adelante en oamMemory
    dec.b tcc__r1
    bne _bucle
    plp                         ; P como estaba
    rtl                         ; vuelta larga (C llamó con jsr.l)

.ENDS
