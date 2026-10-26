;---------------------------------------------------------------------------------
;   dma_ejecutar.asm — Ejecuta de un tirón una lista de copias a la VRAM por DMA.
;
;   PROPÓSITO
;     En el VBlank hay 37 líneas (unos 2,3 ms) y en ellas hay que copiar a la
;     VRAM los tiles del cuadro nuevo de Tarou (3 KB, en 8 a 16 trozos), las
;     columnas del fondo que entran por los bordes y el HUD. El DMA copia a
;     2,7 MB/s, pero PREPARAR cada copia en C con tcc816 cuesta unos 100-200
;     ciclos (cargar, calcular, escribir ocho registros): con 16 copias, más de
;     10 líneas solo de preparación, y la copia se salía del VBlank (los píxeles
;     de Tarou se perdían: fallo del 2026-10-20). Esta rutina lee una lista que
;     C ya dejó preparada DURANTE el frame (dma_lista.c) y solo escribe los
;     registros y dispara cada copia: unos 60 ciclos por copia, un tercio de
;     línea.
;
;     Se llama dma_ejecutar.asm y no dma_lista.asm a propósito: snes_rules
;     compila dma_lista.c pasando por un dma_lista.asm intermedio, y un .asm
;     nuestro con ese nombre quedaría SOBRESCRITO por el compilador.
;
;   SE CONECTA CON
;     La llama:  src/juego/dma_lista.h (dma_lista_ejecutar) desde
;                pantallas/pantalla_nivel1.c (_vblank) y, con la pantalla
;                apagada, desde _iniciar, reaparecer y fondo_nivel_rellenar.
;     Lee:       dma_lista_n y dma_lista_items, que rellena dma_lista.c.
;
;   HARDWARE QUE TOCA
;     $2115 VMAIN   cómo avanza la dirección de VRAM tras cada palabra: $80
;                   (+1, tiles) o $81 (+32: bajar una fila del mapa, columnas).
;                   Al acabar lo deja en $80, que es lo que espera el SDK.
;     $2116 VMADD   dirección de VRAM (en palabras) donde empieza cada copia.
;     $4300 DMAP0   modo 1: los bytes van alternando a $2118 y $2119.
;     $4301 BBAD0   $18: el registro de destino es VMDATA ($2118).
;     $4302-$4304   origen: dirección de 16 bits dentro del banco y el banco.
;     $4305 DAS0    cuántos bytes.
;     $420B MDMAEN  bit 0: arranca el canal 0. La CPU se para hasta que acaba.
;
;   VARIABLES (en la WRAM, banco $7E; las declara dma_lista.h para C)
;     dma_lista_n        1 byte     cuántas copias hay en la lista (0..32)
;     dma_lista_libre    1 byte     relleno, para que la lista empiece en par
;     dma_lista_items    32 x 8 B   por copia: origen (2), vram (2), bytes (2),
;                                   banco (1), vmain (1)
;
;   CONVENCIÓN DE LLAMADA (tcc816): C llama con `jsr.l` (dirección de 24 bits),
;     así que se vuelve con `rtl`. No recibe parámetros; no devuelve nada; puede
;     usar A, X e Y a su antojo. Se guarda y restaura P (php/plp) para devolver
;     los tamaños de registro tal como estaban.
;   → glosario: DMA / HDMA, VRAM, VBlank, Streaming de columnas, Ensamblador
;---------------------------------------------------------------------------------
.include "hdr.asm"
.accu 16
.index 16
.16bit

; OJO con .BASE: con FastROM, hdr.asm deja .BASE $80 (los bancos de la ROM se ven en
; $80-$FF) y WLA-DX se lo suma TAMBIÉN al banco de una RAMSECTION: "BANK $7E" acabaría
; en $FE, que es ROM. tcc816 hace lo mismo que aquí: .BASE $00 antes de la RAM y
; vuelta a $80 antes del código. (Fallo del 2026-10-21: el nivel en negro con FastROM.)
.BASE $00
.RAMSECTION ".dma_lista_ram" BANK $7E SLOT 2
dma_lista_n:      db          ; cuántas copias hay
dma_lista_libre:  db          ; relleno: la lista empieza en dirección par
dma_lista_items:  dsb 256     ; 32 copias de 8 bytes
.ENDS
.ifdef FASTROM
.BASE $80                     ; el código, en los bancos rápidos ($80-$FF)
.endif

.SECTION ".dma_ejecutar_text" SUPERFREE

dma_lista_ejecutar:
    php                         ; guarda P: los tamaños de A y X/Y del que llama
    rep #$30                    ; A, X e Y de 16 bits
    lda.w #0                    ; el byte alto de A a 0 (la carga de abajo es de 8 bits)
    sep #$20                    ; A de 8 bits
    lda.l dma_lista_n           ; cuántas copias hay (pone Z si son 0)
    rep #$20                    ; A de 16 bits otra vez (rep no toca Z)
    beq _fin                    ; ninguna: nada que hacer
    tay                         ; Y = copias que faltan
    ldx.w #0                    ; X = desplazamiento de la copia actual en la lista
    sep #$20                    ; A de 8 bits para los registros de 1 byte
    lda.b #$01
    sta.l $4300                 ; DMAP0: modo 1, dos registros seguidos ($2118, $2119)
    lda.b #$18
    sta.l $4301                 ; BBAD0: destino $2118 (VMDATA)
_bucle:
    lda.l dma_lista_items+7,x   ; vmain de esta copia ($80 tiles, $81 columnas)
    sta.l $2115                 ; VMAIN
    lda.l dma_lista_items+6,x   ; banco del origen
    sta.l $4304                 ; A1B0
    rep #$20                    ; A de 16 bits para los registros de 2 bytes
    lda.l dma_lista_items+2,x   ; dirección de VRAM, en palabras
    sta.l $2116                 ; VMADD
    lda.l dma_lista_items+0,x   ; dirección del origen dentro del banco
    sta.l $4302                 ; A1T0
    lda.l dma_lista_items+4,x   ; bytes
    sta.l $4305                 ; DAS0
    txa
    clc
    adc.w #8                    ; X apunta a la copia siguiente
    tax
    sep #$20                    ; A de 8 bits
    lda.b #$01
    sta.l $420B                 ; MDMAEN: ¡ya! (la CPU espera a que termine)
    dey                         ; una menos
    bne _bucle
    lda.b #$80
    sta.l $2115                 ; VMAIN normal (+1): lo que esperan las funciones del SDK
_fin:
    plp                         ; P como estaba
    rtl                         ; vuelta larga (C llamó con jsr.l)

.ENDS
