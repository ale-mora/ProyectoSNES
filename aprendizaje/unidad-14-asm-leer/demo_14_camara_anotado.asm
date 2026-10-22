;---------------------------------------------------------------------------------
;   demo_14_camara_anotado.asm — El ensamblador que genera tcc816 para
;   src/juego/camara.c (camara_para), anotado línea a línea. NO se ensambla:
;   es para leer (la copia sin tocar está en asm_generado/camara.asm, 85 líneas).
;
;   PROPÓSITO
;     En la unidad 03 (solucion/camara_anotado.asm) este mismo archivo se anotó
;     sentencia a sentencia de C. Aquí se lee como lo ve la CPU: qué REGISTRO y qué
;     BANDERA cambia cada instrucción, y al final hay una TRAZA con dos valores de
;     x_pix (8, el de la entrada al nivel, y 1900, cerca del final).
;     La demo de la unidad 14: ver, con una función de 10 líneas de C que ya se
;     conoce (unidad 07), todas las piezas que salen en cualquier .asm de tcc816:
;     cabecera, sección, reserva de variables locales en la pila, lectura del
;     parámetro con `n,s`, aritmética de 16 bits, comparación CON signo (el truco
;     de `bvc`/`eor #$8000`), comparación que calcula un booleano en X, el valor
;     devuelto en tcc__r0 y el `rtl`.
;
;   EL C (src/juego/camara.c), para tenerlo delante:
;       u16 camara_para(u16 x_pix)
;       {
;           s16 camara = (s16)x_pix + (ANCHO_CELDA / 2) - (ANCHO_PANTALLA / 2);
;           if (camara < 0)          camara = 0;
;           if (camara > CAMARA_MAX) camara = CAMARA_MAX;      /* 2048 - 256 = 1792 */
;           return (u16)camara;
;       }
;
;   CÓMO SE GENERÓ (receta PLAN §6.4, en un clon, nunca en E:\ProyectoSNES\src):
;       make src/juego/camara.asm     (desde PowerShell, con el entorno del SDK)
;
;   SE CONECTA CON
;     asm_generado/camara.asm (el original), src/juego/camara.c, README.md §«Leer
;     una función entera».
;
;   HARDWARE QUE TOCA: ninguno.   VARIABLES: ninguna.
;   → glosario: Cámara, s16, Banco / LoROM, FastROM
;---------------------------------------------------------------------------------

.include "hdr.asm"      ; la cabecera de la ROM (mapa de memoria, vectores, FASTROM)
.accu 16                ; para el ENSAMBLADOR: suponer A de 16 bits (inmediatos de 2 bytes)
.index 16               ; ídem para X e Y. tcc816 trabaja SIEMPRE en 16 bits y solo baja
.16bit                  ;   a 8 (sep #$20) un momento para leer o escribir un byte
.BASE $80               ; FastROM: el código se ve en los bancos $80-$FF (3,58 MHz)
.define __camara_para_locals 2   ; bytes de variables locales en la pila: la s16 `camara`

.SECTION ".camara_paratext_0x0" SUPERFREE   ; una sección por función; SUPERFREE: el
                                            ; enlazador la pone en el banco que quiera
camara_para:            ; la ETIQUETA: el nombre de la función, igual que en C
.ifgr __camara_para_locals 0    ; si hay locales (2 > 0, sí)...
tsa                     ; A = S (el puntero de pila)
sec                     ; acarreo a 1 antes de restar (sbc resta también el "no acarreo")
sbc #__camara_para_locals       ; A = S - 2
tas                     ; S = A: la pila crece hacia abajo 2 bytes; ahí vive `camara`
.endif

; ---- s16 camara = (s16)x_pix + 48 - 128 -----------------------------------------
; La pila ahora (S apunta al primer hueco LIBRE, por eso todo empieza en 1,s):
;     1,s 2,s     `camara` (los 2 bytes reservados)        = -2 + locals + 1
;     3,s 4,s 5,s dirección de vuelta (la puso jsr.l)
;     6,s 7,s     x_pix (el parámetro, lo empujó quien llama) = 3 + locals + 1
lda 3 + __camara_para_locals + 1,s  ; A = x_pix        (modo "relativo a la pila": 6,s)
clc                     ; acarreo a 0 antes de sumar
adc.w #48               ; A += 48  (ANCHO_CELDA / 2: el compilador ya hizo la cuenta)
sec
sbc.w #128              ; A -= 128 (ANCHO_PANTALLA / 2)
sta -2 + __camara_para_locals + 1,s ; camara = A        (1,s)
sta.b tcc__r0           ; y una copia en tcc__r0 (dirección $00 de la página directa)

; ---- if (camara < 0) ---------------------------------------------------------
; Comparar CON signo en el 65816: se resta (A - 0) y se mira N (el bit 15). Pero si la
; resta se desbordó (V = 1), N miente; `eor #$8000` le da la vuelta. Este trío
; (sbc / bvc / eor #$8000) es la firma de una comparación de s16 en tcc816.
sec
sbc.w #0                ; A - 0 (pone N, Z, V y C)
bvc +                   ; sin desbordamiento: N es fiable, salta al `+` siguiente
eor #$8000              ; con desbordamiento: corrige el signo
+
bmi +                   ; N = 1: camara < 0 -> salta al cuerpo del if
brl __local_0           ; si no, se lo salta (brl: salto relativo LARGO, 16 bits)
+
stz.b tcc__r0           ; tcc__r0 = 0
lda.b tcc__r0           ; A = 0 (tcc816 no se da cuenta de que podía hacer `lda #0`)
sta -2 + __camara_para_locals + 1,s ; camara = 0
__local_0:              ; etiqueta local: el final del primer if

; ---- if (camara > CAMARA_MAX) -----------------------------------------------
; Otra manera de comparar: calcula el BOOLEANO (1 o 0) en X y luego mira X.
lda -2 + __camara_para_locals + 1,s ; A = camara
sta.b tcc__r0
ldx #1                  ; X = 1 ("es cierto", hasta que se demuestre lo contrario)
sec
sbc.w #1792             ; A = camara - 1792 (2048 - 256: el compilador sustituyó la macro)
tay                     ; (guarda el resultado; no se vuelve a usar)
beq ++                  ; igual a 1792: NO es mayor -> a `++` (X pasa a 0)
bvc +                   ; mismo truco de signo que arriba
eor #$8000
+
bpl +++                 ; positivo: camara > 1792 -> a `+++` con X = 1
++
dex                     ; X = 0: falso
+++
stx.b tcc__r5           ; el booleano, en tcc__r5 (tampoco se usa después)
txa                     ; A = X: pone Z
bne +                   ; cierto: al cuerpo del if
brl __local_1           ; falso: se lo salta
+
lda.w #1792
sta.b tcc__r0
sta -2 + __camara_para_locals + 1,s ; camara = 1792
__local_1:

; ---- return (u16)camara -----------------------------------------------------------
lda -2 + __camara_para_locals + 1,s ; A = camara
sta.b tcc__r0           ; EL VALOR DEVUELTO va en tcc__r0 (quien llama lo lee de ahí)
__local_2:
.ifgr __camara_para_locals 0
tsa
clc
adc #__camara_para_locals       ; S + 2: libera los locales
tas
.endif
rtl                     ; vuelta LARGA: saca 3 bytes (banco + dirección) de la pila
.ENDS

; ---- El resto del archivo: secciones vacías que tcc816 escribe siempre ----------
.BASE $00               ; las RAMSECTION, con base 0 (si no, banco $7E + $80 = $FE: ROM)
.RAMSECTION "ram{WLA_FILENAME}.data" APPENDTO "globram.data"   ; variables inicializadas
.ENDS
.SECTION "{WLA_FILENAME}.data" APPENDTO "glob.data"            ; sus valores iniciales
.ENDS
.SECTION ".rodata" SUPERFREE                                   ; constantes (vacía aquí)
.ENDS
.BASE $00
.RAMSECTION ".bss" BANK $7e SLOT 2                             ; variables sin valor inicial
.ENDS

; =================================================================================
; TRAZA (hecha a mano con las reglas del 65816; compruébala en Mesen, ejercicio TODO 6)
; N = bit 15 del resultado, Z = resultado 0, C = acarreo (en una resta: 1 = NO hubo
; préstamo), V = desbordamiento con signo.
;
; x_pix = 8 (pantalla_nivel1_iniciar: camara_para(X_INICIAL), X_INICIAL = 56 - 48)
;   lda 6,s          A = $0008
;   adc.w #48        A = $0038 (56)        N=0 V=0 C=0
;   sbc.w #128       A = $FFB8 (-72)       N=1 V=0 C=0 (hubo préstamo)
;   sbc.w #0         A = $FFB8             N=1 V=0 C=1
;   bvc +            V=0: salta (no hace falta corregir N)
;   bmi +            N=1: salta al cuerpo del if -> camara = 0
;   sbc.w #1792      A = $0000 - $0700 = $F900   N=1 V=0 C=0 Z=0
;   beq ++           Z=0: no salta
;   bvc +            salta
;   bpl +++          N=1: no salta -> cae en ++ : dex -> X = 0
;   txa / bne +      Z=1: no salta -> brl __local_1 (no recorta)
;   salida           tcc__r0 = $0000
;
; x_pix = 1900
;   adc / sbc        A = 1900 + 48 - 128 = 1820 = $071C   N=0
;   bmi +            N=0: no salta -> brl __local_0 (no recorta por la izquierda)
;   sbc.w #1792      A = $001C (28)        N=0 V=0 Z=0
;   bpl +++          salta con X = 1
;   txa / bne +      Z=0: salta -> camara = 1792
;   salida           tcc__r0 = $0700 (1792)
; =================================================================================
