; ---------------------------------------------------------------------------------
;   camara_anotado.asm — Solución del ejercicio 03: el ensamblador que genera tcc816
;   para src/juego/camara.c, anotado línea a línea.
;
;   PROPÓSITO
;     Es una COPIA de estudio (no se compila): el camara.asm que dejó
;     `make src/juego/camara.asm` el 2026-10-22 (FastROM, commit 9a7234c), con un
;     comentario en cada línea que dice a qué sentencia de C corresponde.
;
;   SE CONECTA CON
;     Viene de: src/juego/camara.c (la función camara_para) y src/juego/nivel.h
;               (ANCHO_CELDA 96, ANCHO_PANTALLA 256, ANCHO_NIVEL 2048).
;     Lo usan:  nadie (lectura). El original sin notas está en ../camara.asm.
;
;   HARDWARE QUE TOCA
;     ninguno: solo CPU (registros A, X, Y, la pila y la página directa).
;
;   VARIABLES
;     camara (local, s16, 2 bytes en la pila)   el resultado antes de recortarlo
;     tcc__r0 (página directa, 16 bits)         "registro" donde tcc816 devuelve el valor
;     tcc__r5 (página directa, 16 bits)         guarda el resultado de una comparación
;
;   → glosario: Banco / LoROM, FastROM
;
;   El C de partida (src/juego/camara.c):
;
;     u16 camara_para(u16 x_pix)
;     {
;         s16 camara = (s16)x_pix + (ANCHO_CELDA / 2) - (ANCHO_PANTALLA / 2);
;         if (camara < 0)          { camara = 0; }
;         if (camara > CAMARA_MAX) { camara = CAMARA_MAX; }    // CAMARA_MAX = 2048 - 256 = 1792
;         return (u16)camara;
;     }
;
;   Notación: "1,s" = la dirección S+1 (S es el puntero de pila; apunta al primer
;   byte LIBRE, por eso el último byte guardado está en S+1). "#48" = el número 48
;   (inmediato). "tcc__r0" = una variable de 16 bits en la página directa que tcc816
;   usa como registro de trabajo y para devolver resultados.
; ---------------------------------------------------------------------------------

.include "hdr.asm"            ; Mapa de memoria y cabecera: todo .asm del proyecto lo incluye.
.accu 16                      ; Avisa a WLA: el acumulador A trabaja con 16 bits...
.index 16                     ; ...y los índices X e Y también (tcc816 vive en 16 bits).
.16bit                        ; Las direcciones sin banco se escriben con 16 bits.
.BASE $80                     ; FastROM: las etiquetas de código caen en los bancos $80-$FF.
.define __camara_para_locals 2  ; Bytes de variables locales: 2, la s16 `camara`.

.SECTION ".camara_paratext_0x0" SUPERFREE   ; Sección de CÓDIGO de esta función; SUPERFREE:
                                            ; el enlazador la pone en el banco que quiera.

camara_para:                  ; La etiqueta = el nombre de la función de C. jsl salta aquí.
.ifgr __camara_para_locals 0  ; Si hay locales (las hay: 2 bytes), reservarlas en la pila:
tsa                           ;   A = S (copiar el puntero de pila a A).
sec                           ;   Preparar la resta (acarreo a 1 = "sin préstamo").
sbc #__camara_para_locals     ;   A = S - 2.
tas                           ;   S = A: la pila crece 2 bytes; ahí vive `camara` (1,s).
.endif                        ; Fin del bloque condicional del ensamblador.

; --- s16 camara = (s16)x_pix + 48 - 128; -----------------------------------------
lda 3 + __camara_para_locals + 1,s   ; CARGA DEL PARÁMETRO: A = x_pix, que está en 6,s:
                                     ;   +1 (S apunta a lo libre) +2 (las locales)
                                     ;   +3 (la dirección de vuelta que guardó jsl:
                                     ;   banco + 16 bits). El llamador lo empujó antes.
clc                           ; Acarreo a 0 antes de sumar (el 65816 siempre suma con acarreo).
adc.w #48                     ; LA MEDIA CELDA: A = x_pix + 48 (ANCHO_CELDA / 2, calculado
                              ;   por el compilador: en la ROM no hay ninguna división).
sec                           ; Acarreo a 1 antes de restar.
sbc.w #128                    ; LA MEDIA PANTALLA: A = A - 128 (ANCHO_PANTALLA / 2).
sta -2 + __camara_para_locals + 1,s  ; camara = A  (la local, en 1,s).
sta.b tcc__r0                 ; Copia también en tcc__r0 (tcc816 lo hace por costumbre).

; --- if (camara < 0) camara = 0; ---------------------------------------------------
sec                           ; Comparar con 0 CON SIGNO: se hace restando...
sbc.w #0                      ;   A - 0: no cambia A, pero fija las banderas N y V.
bvc +                         ; Si no hubo desbordamiento (V = 0), N ya es el signo correcto.
eor #$8000                    ; Si lo hubo, invertir el bit de signo (truco de comparación
+                             ;   con signo del 65816; aquí, restando 0, nunca ocurre).
bmi +                         ; RECORTE POR ABAJO: si es negativo (N = 1), saltar al "+" de abajo.
brl __local_0                 ; Si no, saltar el cuerpo del if (brl: salto relativo largo).
+                             ; Cuerpo del if:
stz.b tcc__r0                 ;   tcc__r0 = 0 (stz: guardar un cero).
lda.b tcc__r0                 ;   A = 0...
sta -2 + __camara_para_locals + 1,s  ;   ...y camara = 0.
__local_0:                    ; Fin del primer if.

; --- if (camara > 1792) camara = 1792; ---------------------------------------------
lda -2 + __camara_para_locals + 1,s  ; A = camara (se vuelve a leer de la pila).
sta.b tcc__r0                 ; Copia en tcc__r0.
ldx #1                        ; X = 1: de momento, "la condición es cierta".
sec                           ; Comparar con signo: restar...
sbc.w #1792                   ; RECORTE POR ARRIBA: A = camara - 1792 (CAMARA_MAX).
tay                           ; Y = A (tay también pone Z a 1 si el resultado es 0).
beq ++                        ; Si camara == 1792 NO es mayor: ir a "dex" (falso).
bvc +                         ; Mismo truco de signo que arriba: sin desbordamiento, N vale.
eor #$8000                    ; Con desbordamiento, invertir el signo.
+                             ;
bpl +++                       ; Si camara - 1792 es positivo, es mayor: X sigue en 1 (cierto).
++                            ;
dex                           ; Si no, X = 0 (falso).
+++                           ;
stx.b tcc__r5                 ; Guardar el resultado de la comparación (0 o 1)...
txa                           ; ...y mirarlo: A = X.
bne +                         ; Distinto de 0 (cierto): entrar al cuerpo del if.
brl __local_1                 ; Falso: saltarlo.
+                             ; Cuerpo del if:
lda.w #1792                   ;   A = 1792...
sta.b tcc__r0                 ;   ...en tcc__r0...
sta -2 + __camara_para_locals + 1,s  ;   ...y camara = 1792.
__local_1:                    ; Fin del segundo if.

; --- return (u16)camara; -----------------------------------------------------------
lda -2 + __camara_para_locals + 1,s  ; A = camara.
sta.b tcc__r0                 ; EL VALOR DEVUELTO va en tcc__r0 (así lo recoge el llamador).
__local_2:                    ; Etiqueta del final de la función (los return saltarían aquí).
.ifgr __camara_para_locals 0  ; Liberar las locales:
tsa                           ;   A = S.
clc                           ;   Acarreo a 0.
adc #__camara_para_locals     ;   A = S + 2.
tas                           ;   S = A: la pila vuelve a como estaba al entrar.
.endif                        ;
rtl                           ; EL RETORNO: rtl = "return long", saca 3 bytes (banco y
                              ;   dirección). Las funciones de tcc816 son "largas": se
                              ;   llaman con jsl (jsr.l) porque pueden estar en otro banco.
.ENDS                         ; Fin de la sección de código.

; --- Lo que sigue lo emite tcc816 en TODOS los .asm, aunque estén vacías ------------
.BASE $00                     ; Las RAMSECTION no llevan la base $80 de FastROM.
.RAMSECTION "ram{WLA_FILENAME}.data" APPENDTO "globram.data"   ; Variables inicializadas
                                                               ; (copia en RAM): ninguna.
.ENDS

.SECTION "{WLA_FILENAME}.data" APPENDTO "glob.data"   ; Sus valores iniciales en la ROM: ninguno.

.ENDS

.SECTION ".rodata" SUPERFREE  ; Constantes de solo lectura: ninguna.

.ENDS

.BASE $00                     ;
.RAMSECTION ".bss" BANK $7e SLOT 2   ; Variables sin valor inicial, en la WRAM ($7E): ninguna.
.ENDS
