; ---------------------------------------------------------------------------------
;   sprites_oam.asm — sprites_dibujar(): la tabla de entidades, a la OAM (ver sprites.h).
;
;   PROPÓSITO
;     Recorre sprites_orden[0..sprites_n) y, por cada entidad, sus n piezas de
;     16x16: x = x0 + dx, y = y0 + dy (mirando a la izquierda la tabla ya viene
;     espejada), y escribe cada entrada de oamMemory (el búfer del SDK) con DOS
;     escrituras de 16 bits, sin cambiar de tamaño de A:
;       - palabra 0: x (byte bajo) | y << 8;
;       - palabra 1: tile + atrib << 8. Como el bit 0 de los atributos es el bit 8
;         del tile (N) y atrib lo trae a 0, sumar el tile de 9 bits ya pone N: al
;         pasar del tile 255 al 256 el acarreo lo enciende solo.
;     El tile avanza 2 por pieza y, tras 8, salta al par de filas siguiente (+16).
;     CAMINO RÁPIDO: si la celda entera está en la pantalla (borde = 0) y caben sus
;     piezas en la OAM, la palabra 0 es una sola suma: (dx | dy << 8) + (x0 | y0 << 8)
;     (sin acarreo entre bytes porque x0 + dx < 256). Si no, el camino lento
;     comprueba cada pieza.
;     La tabla alta (2 bits por entrada: bit 8 de x y tamaño) se pone entera a 0
;     al empezar (todas las piezas son pequeñas) y solo una pieza con x negativa
;     pone su bit 8. Una pieza fuera de la pantalla (x de 256 en adelante o por
;     debajo de -256; y de 224 en adelante o por encima de -16) se esconde con
;     y = 224 (nunca con x = -256: la PPU contaría sus 8 px por línea). Al final
;     esconde (y = 224) las entradas que el frame anterior usó y este no.
;     Devuelve en tcc__r0 cuántas entradas usó.
;     Una sola llamada por frame: lo caro en la prueba de carga era llamar desde C.
;     Convención de tcc816 (aprendizaje/unidad-15): sin parámetros, vuelve con rtl
;     y P como estaba; usa A, X, Y y tcc__r0..r5, r9, r10 (C no espera que
;     sobrevivan). D = $0000 y DB = $7E (las variables de C están en la WRAM).
;
;   SE CONECTA CON
;     Lee:      sprites_n, sprites_orden, sprites_ent, sprites_oam_antes (sprites.c) y
;               las tablas (dx, dy) de las piezas, en la ROM (puntero largo).
;     Escribe:  oamMemory (544 bytes del SDK) y sprites_oam_antes.
;     Lo usan:  juego/sprites.h (sprites_dibujar).
;
;   HARDWARE QUE TOCA
;     ninguno: la NMI del SDK copia oamMemory a la OAM en el VBlank.
;
;   VARIABLES (página directa de tcc816, solo durante la llamada)
;     r0/r0h  puntero largo a las piezas     r1  piezas que faltan
;     r1h  tile + atrib << 8 de la pieza actual
;     r2  x0       r2h  y0                   r3  x0 | y0 << 8 (camino rápido)
;     r4  desplazamiento en oamMemory        r4h  número de entrada de la OAM
;     r5  posición en sprites_orden          r9  dx / temporal    r9h  x    r10  y
;   → glosario: OAM, Ensamblador 65816, Orden por profundidad
; ---------------------------------------------------------------------------------
.include "hdr.asm"
.accu 16
.index 16
.16bit

.ifdef FASTROM
.BASE $80                     ; el código, en los bancos rápidos ($80-$FF)
.endif

.SECTION ".sprites_oam_text" SUPERFREE

; Su bit 8 de x, por posición de la entrada dentro de su byte de la tabla alta (0..3).
_bit_x:    .db $01, $04, $10, $40

sprites_dibujar:
    php
    rep #$30                    ; A, X e Y de 16 bits (y así se queda)
    ldx.w #30                   ; la tabla alta entera a 0 (32 bytes)
_tabla_alta:
    stz.w oamMemory+512,x       ; (DB = $7E: oamMemory está en la WRAM)
    dex
    dex
    bpl _tabla_alta
    stz.b tcc__r4               ; desplazamiento en oamMemory
    stz.b tcc__r4h              ; entrada de la OAM
    stz.b tcc__r5               ; posición en el orden

_entidad:
    lda.l sprites_n
    and.w #$00FF
    cmp.b tcc__r5
    beq _sin_mas                ; ya no quedan entidades
    bcs _hay_entidad
_sin_mas:
    brl _sobrantes
_hay_entidad:
    ldx.b tcc__r5
    inc.b tcc__r5               ; la entidad siguiente, después
    lda.l sprites_orden,x
    and.w #$00FF
    asl a
    asl a
    asl a
    asl a                       ; × 16: la entidad en sprites_ent
    tax
    lda.l sprites_ent+6,x       ; banco (bajo) y n (alto)
    xba
    and.w #$00FF
    beq _entidad                ; sin piezas: nada que dibujar
    sta.b tcc__r1               ; n
    lda.l sprites_ent+6,x
    and.w #$00FF
    sta.b tcc__r0h              ; banco de la tabla de piezas
    lda.l sprites_ent+4,x
    sta.b tcc__r0               ; dirección de la tabla
    lda.l sprites_ent+0,x
    sta.b tcc__r2               ; x0
    lda.l sprites_ent+2,x
    sta.b tcc__r2h              ; y0
    lda.b tcc__r2h
    xba
    and.w #$FF00
    sta.b tcc__r3
    lda.b tcc__r2
    and.w #$00FF
    ora.b tcc__r3
    sta.b tcc__r3               ; x0 | y0 << 8, para el camino rápido
    lda.l sprites_ent+10,x
    and.w #$FF00
    sta.b tcc__r9               ; borde (alto)
    lda.l sprites_ent+10,x
    and.w #$00FF
    xba                         ; atrib << 8
    clc
    adc.l sprites_ent+8,x       ; + tile (9 bits): la palabra 1 de la primera pieza
    sta.b tcc__r1h
    ldy.w #0                    ; Y = posición en la tabla de piezas (2 bytes por pieza)
    lda.b tcc__r9
    bne _lenta                  ; puede salirse de la pantalla: pieza a pieza
    lda.b tcc__r4h
    clc
    adc.b tcc__r1
    cmp.w #129
    bcs _lenta                  ; no caben todas en la OAM: el lento para en 128
    ldx.b tcc__r4               ; X = desplazamiento en oamMemory

_rapida:
    lda [tcc__r0],y             ; dx | dy << 8
    clc
    adc.b tcc__r3               ; + x0 | y0 << 8
    sta.l oamMemory,x           ; palabra 0
    lda.b tcc__r1h
    sta.l oamMemory+2,x         ; palabra 1: tile | atributos (con N)
    clc
    adc.w #2                    ; tile siguiente
    sta.b tcc__r1h
    iny
    iny
    tya
    and.w #$000F                ; tras 8 piezas, el par siguiente
    bne _r_mismo_par
    lda.b tcc__r1h
    clc
    adc.w #16
    sta.b tcc__r1h
_r_mismo_par:
    txa
    clc
    adc.w #4
    tax
    inc.b tcc__r4h
    dec.b tcc__r1
    bne _rapida
    stx.b tcc__r4
    brl _entidad

_lenta:
    ldx.b tcc__r4               ; X = desplazamiento en oamMemory

_pieza:
    lda.b tcc__r4h
    cmp.w #128
    bcc _cabe
    brl _llena                  ; la OAM está llena: lo que falta no se dibuja
_cabe:
    lda [tcc__r0],y             ; dx | dy << 8
    and.w #$00FF
    clc
    adc.b tcc__r2               ; x = x0 + dx
    sta.b tcc__r9h
    iny
    lda [tcc__r0],y
    and.w #$00FF                ; dy
    clc
    adc.b tcc__r2h
    sta.b tcc__r10              ; y = y0 + dy
    iny

    cmp.w #224                  ; ¿se ve en vertical? (0..223, o -16..-1 asomando arriba)
    bcc _y_bien
    cmp.w #$FFF0
    bcc _esconder
_y_bien:
    lda.b tcc__r9h
    cmp.w #256                  ; ¿x en 0..255? (lo normal)
    bcc _escribir
    cmp.w #$FF00
    bcc _esconder               ; 256..$FEFF: fuera de la pantalla
    jsr _bit8_x                 ; -256..-1: su bit 8 de x en la tabla alta
_escribir:
    lda.b tcc__r10
    xba
    and.w #$FF00                ; y << 8
    sta.b tcc__r9
    lda.b tcc__r9h
    and.w #$00FF                ; x (bajo)
    ora.b tcc__r9
    sta.l oamMemory,x           ; palabra 0: x | y << 8
    bra _palabra1
_esconder:
    lda.w #224 << 8
    sta.l oamMemory,x           ; x = 0, y = 224: debajo de la pantalla
_palabra1:
    lda.b tcc__r1h
    sta.l oamMemory+2,x         ; palabra 1: tile | atributos (con N)

    clc
    adc.w #2                    ; tile siguiente: +2 ...
    sta.b tcc__r1h
    tya
    and.w #$000F                ; ... y tras 8 piezas (Y = 16, 32...), el par siguiente
    bne _mismo_par
    lda.b tcc__r1h
    clc
    adc.w #16
    sta.b tcc__r1h
_mismo_par:
    txa
    clc
    adc.w #4
    tax                         ; entrada siguiente en oamMemory
    inc.b tcc__r4h
    dec.b tcc__r1
    beq _fin_entidad
    brl _pieza                  ; (bne no llega: más de 128 bytes atrás)
_fin_entidad:
    stx.b tcc__r4
    brl _entidad

; Pone el bit 8 de x de la entrada actual (r4h) en la tabla alta. Conserva X e Y.
_bit8_x:
    phx
    phy
    lda.b tcc__r4h
    and.w #3
    tax                         ; X = posición dentro del byte
    lda.b tcc__r4h
    lsr a
    lsr a
    tay                         ; Y = byte de la tabla alta
    sep #$20
    lda.w oamMemory+512,y
    ora.l _bit_x,x
    sta.w oamMemory+512,y
    rep #$20
    ply
    plx
    rts

_llena:
    stx.b tcc__r4
_sobrantes:
    ; Esconder (y = 224) las entradas que usó el frame anterior y este no.
    lda.b tcc__r4h
    sta.b tcc__r5               ; las usadas este frame (lo que se devuelve)
    ldx.b tcc__r4
    sep #$20
_esconder_resto:
    lda.b tcc__r4h
    cmp.l sprites_oam_antes
    bcs _hecho
    lda.b #224
    sta.l oamMemory+1,x         ; y = 224
    inx
    inx
    inx
    inx
    inc.b tcc__r4h
    bra _esconder_resto
_hecho:
    lda.b tcc__r5
    sta.l sprites_oam_antes     ; para el frame siguiente
    rep #$20
    lda.b tcc__r5
    and.w #$00FF
    sta.b tcc__r0               ; lo que devuelve la función (u8)
    plp
    rtl

.ENDS
