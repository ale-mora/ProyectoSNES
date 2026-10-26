;---------------------------------------------------------------------------------
;   relleno.asm — Rellena una zona de la VRAM con la misma palabra. En ensamblador,
;   llamada desde C (principal.c) como `relleno_vram(vram, palabra, cuantas)`.
;
;   PROPÓSITO
;     Enseñar lo mínimo para escribir una rutina propia en ensamblador 65816 y
;     llamarla desde C con la convención de tcc816 (unidad 15):
;       - leer los parámetros de la pila con el modo `n,s`;
;       - cambiar el tamaño de A con rep/sep y devolver P como estaba (php/plp);
;       - un bucle con un contador en X (`dex` / `bne`);
;       - escribir registros de la PPU a mano (`sta.l $2100`, `$2115`, `$2116`,
;         `$2118`);
;       - leer una variable del SDK (mirrorINIDISP) y tener una variable propia
;         en la WRAM (relleno_llamadas) que C ve con `extern`.
;     Es la versión pequeña de src/juego/dma_ejecutar.asm, la rutina en
;     ensamblador del juego.
;
;     El archivo se llama relleno.asm y NO relleno_c.asm: snes_rules compila
;     relleno_c.c pasando por un relleno_c.asm intermedio, y un .asm nuestro con
;     ese nombre quedaría sobrescrito por el compilador.
;
;   SE CONECTA CON
;     La declara: src/relleno.h (para C es una función más).
;     La llama:   src/principal.c.
;     Lee:        mirrorINIDISP (variable del SDK: el valor de INIDISP que el SDK
;                 cree que hay en pantalla; lo escriben setScreenOn/setBrightness).
;
;   HARDWARE QUE TOCA
;     $2100 INIDISP  bit 7 = forced blank (pantalla apagada: la VRAM se puede
;                    escribir en cualquier momento); bits 0-3 = brillo.
;     $2115 VMAIN    $80: la dirección de VRAM avanza 1 tras escribir $2119.
;     $2116 VMADD    dirección de VRAM (en palabras) donde empieza a escribir.
;     $2118 VMDATA   la palabra que se escribe ($2118 el byte bajo, $2119 el alto).
;
;   VARIABLES (en la WRAM, banco $7E)
;     relleno_llamadas  2 bytes  veces que se ha llamado a relleno_vram
;
;   CONVENCIÓN DE LLAMADA (tcc816)
;     C empuja los parámetros en la pila de DERECHA a IZQUIERDA (el último
;     primero), cada u16 son 2 bytes, y llama con `jsr.l` (dirección de 24 bits,
;     3 bytes de vuelta en la pila). Al entrar, con S apuntando al primer hueco
;     libre:
;         1,s 2,s 3,s   dirección de vuelta (la pone jsr.l)
;         4,s           vram      (primer parámetro)
;         6,s           palabra
;         8,s           cuantas
;     Tras nuestro `php` todo se desplaza un byte (5,s 7,s 9,s). Se vuelve con
;     `rtl` y es C quien saca los parámetros de la pila. Si devolviera algo, iría
;     en tcc__r0 (no devuelve nada). A, X e Y se pueden usar libremente.
;   → glosario: VRAM, Forced blank, Banco / LoROM, FastROM
;---------------------------------------------------------------------------------
.include "hdr.asm"
.accu 16
.index 16
.16bit

; OJO con .BASE: con FastROM, hdr.asm deja .BASE $80 (los bancos de la ROM se ven
; en $80-$FF) y WLA-DX se lo suma TAMBIÉN al banco de una RAMSECTION: "BANK $7E"
; acabaría en $FE, que es ROM, y la variable "viviría" donde no se puede escribir.
; Por eso: .BASE $00 antes de la RAM y vuelta a $80 antes del código.
.BASE $00
.RAMSECTION ".relleno_ram" BANK $7E SLOT 2
relleno_llamadas:  dw          ; 2 bytes: cuántas veces se ha llamado a la rutina
.ENDS
.ifdef FASTROM
.BASE $80                      ; el código, en los bancos rápidos ($80-$FF)
.endif

.SECTION ".relleno_text" SUPERFREE

;--- void relleno_vram(u16 vram, u16 palabra, u16 cuantas) -----------------------
relleno_vram:
    php                         ; guarda P (tamaños de A y X/Y del que llama)
    rep #$30                    ; A, X e Y de 16 bits
    sep #$20                    ; A de 8 bits: los dos registros que siguen son de 1 byte
    lda.b #$80                  ; bit 7 = forced blank, brillo 0
    sta.l $2100                 ; INIDISP: pantalla apagada, la VRAM se puede escribir ya
    sta.l $2115                 ; VMAIN = $80 (el mismo valor): +1 palabra tras cada $2119
    rep #$20                    ; A de 16 bits otra vez
    lda 5,s                     ; vram (4,s al entrar, 5,s tras el php)
    sta.l $2116                 ; VMADD: dónde empieza la escritura
    lda 9,s                     ; cuantas
    beq _fin                    ; 0 palabras: nada que escribir
    tax                         ; X = palabras que faltan
    lda 7,s                     ; A = palabra (se queda en A todo el bucle)
_bucle:
    sta.l $2118                 ; VMDATA: A de 16 bits escribe $2118 y luego $2119
    dex                         ; una menos (pone Z cuando llega a 0)
    bne _bucle                  ; quedan: otra vuelta
_fin:
    sep #$20                    ; A de 8 bits
    lda.l mirrorINIDISP         ; el brillo que el SDK cree que hay (p. ej. $0F)
    sta.l $2100                 ; INIDISP: la pantalla, como estaba
    rep #$20                    ; A de 16 bits
    lda.l relleno_llamadas      ; el contador (variable nuestra, en la WRAM)
    inc a                       ; +1
    sta.l relleno_llamadas
    plp                         ; P como estaba
    rtl                         ; vuelta larga (C llamó con jsr.l)

.ENDS
