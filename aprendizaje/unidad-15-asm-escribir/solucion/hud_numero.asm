;---------------------------------------------------------------------------------
;   hud_numero.asm — numero() del HUD de depuración, reescrita en ensamblador.
;   (Solución del ejercicio 15. Va en src/juego/hud_numero.asm del juego.)
;
;   PROPÓSITO
;     Escribir un entero con signo en decimal, como palabras de tilemap, a partir
;     de un puntero, y devolver dónde sigue. Mismo algoritmo que la versión en C
;     (src/juego/hud_debug.c, numero): sin dividir; para 10000, 1000, 100 y 10 se
;     resta mientras quepa y se cuenta cuántas veces; los ceros de la izquierda
;     no se escriben; las unidades, siempre. Lo que cambia es CÓMO: el valor vive
;     en A todo el rato, el puntero no se mueve (Y es el desplazamiento) y la
;     cifra y la bandera "empezado" van en dos registros de tcc816 (tcc__r1,
;     tcc__r2) en vez de en la pila.
;
;     Se llama hud_numero.asm (y la etiqueta hud_numero): no puede llamarse como
;     ningún .c (hud_debug.c pasa por un hud_debug.asm intermedio).
;
;   SE CONECTA CON
;     La declara y la llama: src/juego/hud_debug.c (con -DHUD_NUMERO_ASM, la macro
;     NUMERO apunta aquí; sin ella, a la versión en C).
;
;   HARDWARE QUE TOCA
;     ninguno: escribe en la WRAM (el mapa del HUD, banco $7E) por el puntero.
;
;   VARIABLES
;     hud_potencias  4 x 2 bytes en la ROM  10000, 1000, 100, 10
;
;   CONVENCIÓN DE LLAMADA (tcc816): u16 *hud_numero(u16 *p, s16 valor)
;     Al entrar (jsr.l: 3 bytes de vuelta en 1,s..3,s):
;         4,s  p, 16 bits bajos (dirección dentro del banco)
;         6,s  p, banco (los punteros de tcc816 ocupan 4 bytes: 2 + 2)
;         8,s  valor (s16)
;     Tras el php, todo +1 (5,s 7,s 9,s). Devuelve el puntero en tcc__r0 (16 bits
;     bajos) y tcc__r0h (banco), como cualquier función de tcc816 que devuelve un
;     puntero. Usa A, X, Y, tcc__r0, tcc__r0h, tcc__r1 y tcc__r2 (registros de
;     trabajo que tcc816 no conserva entre llamadas: el que llama guarda lo suyo
;     en la pila; comprobado en hud_debug.asm alrededor de cada jsr.l ..._numero).
;   → glosario: HUD de depuración, Tilemap
;---------------------------------------------------------------------------------
.include "hdr.asm"
.accu 16
.index 16
.16bit

.define TILE_MENOS  $200D       ; ('-' - 32) | $2000: tile del signo, con prioridad
.define TILE_CERO   $2010       ; ('0' - 32) | $2000: el tile de '0'; '0'+n es +n

.SECTION ".hud_numero_text" SUPERFREE

hud_potencias:
    .dw 10000, 1000, 100, 10    ; las potencias que se restan (en la ROM)

;--- u16 *hud_numero(u16 *p, s16 valor) ------------------------------------------
hud_numero:
    php                         ; guarda P del que llama
    rep #$30                    ; A, X e Y de 16 bits
    lda 5,s                     ; p: 16 bits bajos
    sta.b tcc__r0
    lda 7,s                     ; p: banco
    sta.b tcc__r0h              ; tcc__r0/tcc__r0h = puntero largo para [tcc__r0],y
    ldy.w #0                    ; Y = bytes escritos (desplazamiento desde p)
    stz.b tcc__r1               ; tcc__r1 = "empezado": 0 = aún no hay cifras
    lda 9,s                     ; valor
    bpl _positivo               ; >= 0: tal cual
    eor.w #$FFFF                ; negativo: complemento a dos...
    inc a                       ; ...-valor (con -32768 da $8000 = 32768, como en C)
    pha                         ; guarda v
    lda.w #TILE_MENOS
    sta [tcc__r0],y             ; escribe '-'
    iny
    iny                         ; una palabra más
    pla                         ; recupera v
_positivo:
    ldx.w #0                    ; X = desplazamiento en hud_potencias (0, 2, 4, 6)
_potencia:
    stz.b tcc__r2               ; tcc__r2 = cifra = 0
_resta:
    cmp.l hud_potencias,x       ; ¿v >= potencia? (sin signo: C = 1 si cabe)
    bcc _fin_resta              ; no cabe: esta cifra está lista
    sec
    sbc.l hud_potencias,x       ; v -= potencia
    inc.b tcc__r2               ; cifra++
    bra _resta
_fin_resta:
    pha                         ; guarda v
    lda.b tcc__r2
    ora.b tcc__r1               ; cifra != 0 || empezado
    beq _sin_cifra              ; cero a la izquierda: no se escribe
    lda.b tcc__r2
    clc
    adc.w #TILE_CERO            ; tile de la cifra
    sta [tcc__r0],y             ; escribe la cifra
    iny
    iny
    sta.b tcc__r1               ; empezado = algo distinto de 0
_sin_cifra:
    pla                         ; recupera v
    inx
    inx                         ; la potencia siguiente
    cpx.w #8                    ; ¿las cuatro?
    bne _potencia
    clc
    adc.w #TILE_CERO            ; las unidades (0..9), siempre
    sta [tcc__r0],y
    iny
    iny
    tya                         ; bytes escritos
    clc
    adc.b tcc__r0               ; p + bytes: dónde sigue (el banco no cambia)
    sta.b tcc__r0               ; el valor devuelto, en tcc__r0 / tcc__r0h
    plp                         ; P como estaba
    rtl                         ; vuelta larga

.ENDS
