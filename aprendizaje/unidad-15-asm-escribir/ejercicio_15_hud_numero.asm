;---------------------------------------------------------------------------------
;   hud_numero.asm — numero() del HUD de depuración, reescrita en ensamblador.
;   ESQUELETO del ejercicio 15: completa los TODO numerados (ver ejercicio_15.md).
;   Va en src/juego/hud_numero.asm de TU CLON del juego.
;
;   PROPÓSITO
;     Escribir un entero con signo en decimal, como palabras de tilemap, a partir
;     de un puntero, y devolver dónde sigue. Mismo algoritmo que la versión en C
;     (src/juego/hud_debug.c, numero): sin dividir; para 10000, 1000, 100 y 10 se
;     resta mientras quepa y se cuenta cuántas veces; los ceros de la izquierda no
;     se escriben; las unidades, siempre.
;     Reparto de registros que se propone:
;         A         el valor v (sin signo) mientras se resta
;         X         desplazamiento en hud_potencias (0, 2, 4, 6)
;         Y         bytes escritos desde p (cada palabra, +2)
;         tcc__r0   p, 16 bits bajos  } juntos, un puntero largo para
;         tcc__r0h  p, banco          } `sta [tcc__r0],y`
;         tcc__r1   "empezado" (0 = aún no se ha escrito ninguna cifra)
;         tcc__r2   la cifra que se está contando
;
;   SE CONECTA CON
;     La declara y la llama: src/juego/hud_debug.c (con -DHUD_NUMERO_ASM, la macro
;     NUMERO apunta aquí; sin ella, a la versión en C). Parche: andamio_hud_debug.c.patch (en la carpeta de la unidad).
;
;   HARDWARE QUE TOCA
;     ninguno: escribe en la WRAM (el mapa del HUD, banco $7E) por el puntero.
;
;   VARIABLES
;     hud_potencias  4 x 2 bytes en la ROM  10000, 1000, 100, 10
;
;   CONVENCIÓN DE LLAMADA (tcc816): u16 *hud_numero(u16 *p, s16 valor)
;     Al entrar: 4,s p (16 bits bajos), 6,s p (banco), 8,s valor.
;     Tras el php, todo +1. Devuelve el puntero en tcc__r0 / tcc__r0h.
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
    ; TODO 1: copia p a tcc__r0 (16 bits bajos, 5,s) y tcc__r0h (banco, 7,s).

    ldy.w #0                    ; Y = bytes escritos (desplazamiento desde p)
    stz.b tcc__r1               ; empezado = 0
    lda 9,s                     ; valor
    bpl _positivo               ; >= 0: tal cual
    ; TODO 2: valor negativo. Niega A (complemento a dos: eor #$FFFF e inc a),
    ;         escribe TILE_MENOS en [tcc__r0],y SIN perder v (pha / pla) y
    ;         avanza Y una palabra.

_positivo:
    ldx.w #0                    ; X = desplazamiento en hud_potencias
_potencia:
    stz.b tcc__r2               ; cifra = 0
_resta:
    ; TODO 3: mientras v >= hud_potencias[X]: v -= potencia y cifra++.
    ;         (cmp.l hud_potencias,x pone C = 1 si cabe; bcc sale del bucle;
    ;         sbc necesita sec delante.) Al salir, salta a _fin_resta.

_fin_resta:
    pha                         ; guarda v
    ; TODO 4: si cifra != 0 o empezado != 0 (lda tcc__r2 / ora tcc__r1 / beq),
    ;         escribe TILE_CERO + cifra en [tcc__r0],y, avanza Y una palabra y
    ;         marca empezado (cualquier valor distinto de 0 en tcc__r1).

_sin_cifra:
    pla                         ; recupera v
    ; TODO 5: pasa a la potencia siguiente (X += 2) y vuelve a _potencia si aún
    ;         no se han hecho las cuatro (cpx.w #8 / bne).

    ; TODO 6: las unidades (lo que queda en A, 0..9), siempre: TILE_CERO + A en
    ;         [tcc__r0],y y Y += 2.

    ; TODO 7: el valor devuelto: tcc__r0 = tcc__r0 + Y (el banco no cambia).

    plp                         ; P como estaba
    rtl                         ; vuelta larga

.ENDS
