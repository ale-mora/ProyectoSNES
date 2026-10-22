.include "hdr.asm"
.accu 16
.index 16
.16bit
.BASE $80
.define __tccs_{WLA_FILENAME}_tile_del_bloque32_locals 0
.define __tccs_{WLA_FILENAME}_tile_del_bloque64_locals 0
.define __tccs_{WLA_FILENAME}_esconder_locals 0
.define __personajes_cargar_locals 4
.define __tccs_{WLA_FILENAME}_dibujar_locals 16

.SECTION ".tccs_{WLA_FILENAME}_tile_del_bloque32text_0x0" SUPERFREE

tccs_{WLA_FILENAME}_tile_del_bloque32:
.ifgr __tccs_{WLA_FILENAME}_tile_del_bloque32_locals 0
tsa
sec
sbc #__tccs_{WLA_FILENAME}_tile_del_bloque32_locals
tas
.endif
lda 3 + __tccs_{WLA_FILENAME}_tile_del_bloque32_locals + 1,s
sta.b tcc__r0
lsr.b tcc__r0
lsr.b tcc__r0
lda.b tcc__r0
ldy.w #6
-
asl a
dey
bne -
+
sta.b tcc__r0
lda 3 + __tccs_{WLA_FILENAME}_tile_del_bloque32_locals + 1,s
and #3
asl a
asl a
sta.b tcc__r1
clc
adc.b tcc__r0
sta.b tcc__r0
__local_0:
.ifgr __tccs_{WLA_FILENAME}_tile_del_bloque32_locals 0
tsa
clc
adc #__tccs_{WLA_FILENAME}_tile_del_bloque32_locals
tas
.endif
rtl
.ENDS

.SECTION ".tccs_{WLA_FILENAME}_tile_del_bloque64text_0x1" SUPERFREE

tccs_{WLA_FILENAME}_tile_del_bloque64:
.ifgr __tccs_{WLA_FILENAME}_tile_del_bloque64_locals 0
tsa
sec
sbc #__tccs_{WLA_FILENAME}_tile_del_bloque64_locals
tas
.endif
lda.w #0
sep #$20
lda 3 + __tccs_{WLA_FILENAME}_tile_del_bloque64_locals + 1,s
rep #$20
sta.b tcc__r0
tax
lda.w #2
jsr.l tcc__div
lda.b tcc__r9
sta.b tcc__r0
ldy.w #7
-
asl a
dey
bne -
+
clc
adc.w #256
sta.b tcc__r0
lda.w #0
sep #$20
lda 3 + __tccs_{WLA_FILENAME}_tile_del_bloque64_locals + 1,s
rep #$20
sta.b tcc__r1
tax
lda.w #2
jsr.l tcc__div
txa
asl a
asl a
asl a
sta.b tcc__r1
clc
adc.b tcc__r0
sta.b tcc__r0
__local_1:
.ifgr __tccs_{WLA_FILENAME}_tile_del_bloque64_locals 0
tsa
clc
adc #__tccs_{WLA_FILENAME}_tile_del_bloque64_locals
tas
.endif
rtl
.ENDS

.SECTION ".tccs_{WLA_FILENAME}_escondertext_0x2" SUPERFREE

tccs_{WLA_FILENAME}_esconder:
.ifgr __tccs_{WLA_FILENAME}_esconder_locals 0
tsa
sec
sbc #__tccs_{WLA_FILENAME}_esconder_locals
tas
.endif
lda.w #0
sep #$20
lda 3 + __tccs_{WLA_FILENAME}_esconder_locals + 1,s
rep #$20
asl a
asl a
sta.b tcc__r0
pea.w (1 * 256 + 0)
sep #$20
rep #$20
pei (tcc__r0)
jsr.l oamSetEx
tsa
clc
adc #4
tas
lda.w #0
sep #$20
lda 3 + __tccs_{WLA_FILENAME}_esconder_locals + 1,s
rep #$20
asl a
asl a
sta.b tcc__r0
pea.w 224
pea.w 256
pei (tcc__r0)
jsr.l oamSetXY
tsa
clc
adc #6
tas
.ifgr __tccs_{WLA_FILENAME}_esconder_locals 0
tsa
clc
adc #__tccs_{WLA_FILENAME}_esconder_locals
tas
.endif
rtl
.ENDS

.SECTION ".personajes_cargartext_0x3" SUPERFREE

personajes_cargar:
.ifgr __personajes_cargar_locals 0
tsa
sec
sbc #__personajes_cargar_locals
tas
.endif
lda.w #:tarou_reposo_estatico_til_end
lda.w #tarou_reposo_estatico_til_end + 0
sta.b tcc__r0
lda.w #:tarou_reposo_estatico_til
lda.w #tarou_reposo_estatico_til + 0
sta.b tcc__r1
sec
lda.b tcc__r0
sbc.b tcc__r1
sta.b tcc__r0
sta -2 + __personajes_cargar_locals + 1,s
lda.w #:barbara_samurai_til_end
sta.b tcc__r0h
lda.w #barbara_samurai_til_end + 0
sta.b tcc__r0
lda.w #:barbara_samurai_til
sta.b tcc__r1h
lda.w #barbara_samurai_til + 0
sta.b tcc__r1
sec
lda.b tcc__r0
sbc.b tcc__r1
sta -4 + __personajes_cargar_locals + 1,s
lda -2 + __personajes_cargar_locals + 1,s
sta.b tcc__r0
ldx #1
sec
sbc.w #6144
tay
beq +
bcs ++
+ dex
++
stx.b tcc__r5
txa
bne +
brl __local_2
+
lda.w #6144
sta.b tcc__r0
sta -2 + __personajes_cargar_locals + 1,s
__local_2:
jsr.l oamInit
sep #$20
lda #160
pha
rep #$20
pea.w 0
jsr.l oamInitGfxAttr
tsa
clc
adc #3
tas
lda -2 + __personajes_cargar_locals + 1,s
pha
pea.w 0
pea.w :tarou_reposo_estatico_til
pea.w tarou_reposo_estatico_til + 0
jsr.l dmaCopyVram
tsa
clc
adc #8
tas
pea.w 32
pea.w 128
pea.w :tarou_pal
pea.w tarou_pal + 0
jsr.l dmaCopyCGram
tsa
clc
adc #8
tas
lda -4 + __personajes_cargar_locals + 1,s
pha
pea.w 4096
pea.w :barbara_samurai_til
pea.w barbara_samurai_til + 0
jsr.l dmaCopyVram
tsa
clc
adc #8
tas
pea.w 32
pea.w 144
pea.w :barbara_samurai_pal
pea.w barbara_samurai_pal + 0
jsr.l dmaCopyCGram
tsa
clc
adc #8
tas
jsr.l personajes_ocultar
.ifgr __personajes_cargar_locals 0
tsa
clc
adc #__personajes_cargar_locals
tas
.endif
rtl
.ENDS

.SECTION ".tccs_{WLA_FILENAME}_dibujartext_0x4" SUPERFREE

tccs_{WLA_FILENAME}_dibujar:
.ifgr __tccs_{WLA_FILENAME}_dibujar_locals 0
tsa
sec
sbc #__tccs_{WLA_FILENAME}_dibujar_locals
tas
.endif
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_cuadro_tarou + 0
rep #$20
asl a
asl a
sta.b tcc__r0
lda.w #:tarou_reposo_estatico_metasprites
sta.b tcc__r1h
lda.w #tarou_reposo_estatico_metasprites + 0
clc
adc.b tcc__r0
sta.b tcc__r1
ldy #0
lda.b [tcc__r1],y
sta.b tcc__r0
iny
iny
lda.b [tcc__r1],y
sta.b tcc__r0h
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_tabla + 0
lda.b tcc__r0h
sta.w tccs_{WLA_FILENAME}_tabla + 0 + 2
lda.w tccs_{WLA_FILENAME}_tabla + 0
sta.b tcc__r0
lda.w tccs_{WLA_FILENAME}_tabla + 0 + 2
pha
pei (tcc__r0)
jsr.l metasprite_piezas
tsa
clc
adc #4
tas
sep #$20
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_piezas + 0
rep #$20
lda.w #0
sta.b tcc__r0
sep #$20
sta.w tccs_{WLA_FILENAME}_i + 0
rep #$20
__local_5:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
sta.b tcc__r0
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_piezas + 0
rep #$20
sta.b tcc__r1
lda.b tcc__r0
sec
sbc.b tcc__r1
bvc +
eor #$8000
+
bmi +
brl __local_3
+
bra __local_4
__local_9:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
sep #$20
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_i + 0
rep #$20
jmp.w __local_5
__local_4:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_tarou_izquierda + 0
rep #$20
sta.b tcc__r0
lda.b tcc__r0 ; DON'T OPTIMIZE
bne +
brl __local_6
+
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
asl a
asl a
asl a
sta.b tcc__r0
lda.w tccs_{WLA_FILENAME}_tabla + 0
sta.b tcc__r1
lda.w tccs_{WLA_FILENAME}_tabla + 0 + 2
sta.b tcc__r1h
clc
lda.b tcc__r1
adc.b tcc__r0
sta.b tcc__r1
lda.w #64
sta.b tcc__r0
lda.b [tcc__r1]
sta.b tcc__r2
sec
lda.b tcc__r0
sbc.b tcc__r2
sta.b tcc__r0
bra __local_7
__local_6:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
asl a
asl a
asl a
sta.b tcc__r0
lda.w tccs_{WLA_FILENAME}_tabla + 0
sta.b tcc__r1
lda.w tccs_{WLA_FILENAME}_tabla + 0 + 2
sta.b tcc__r1h
clc
lda.b tcc__r1
adc.b tcc__r0
sta.b tcc__r1
lda.b [tcc__r1]
sta.b tcc__r0
__local_7:
__local_8:
lda.b tcc__r0
sta -2 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
asl a
asl a
sta.b tcc__r0
lda.w tccs_{WLA_FILENAME}_x_tarou + 0
sta.b tcc__r1
lda -2 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
clc
adc.b tcc__r1
sta.b tcc__r1
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
asl a
asl a
asl a
sta.b tcc__r2
lda.w tccs_{WLA_FILENAME}_tabla + 0
sta.b tcc__r3
lda.w tccs_{WLA_FILENAME}_tabla + 0 + 2
sta.b tcc__r3h
clc
lda.b tcc__r3
adc.b tcc__r2
inc a
inc a
sta.b tcc__r3
lda.w tccs_{WLA_FILENAME}_y_tarou + 0
sta.b tcc__r2
lda.b [tcc__r3]
clc
adc.b tcc__r2
sta.b tcc__r2
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
asl a
asl a
asl a
sta.b tcc__r3
lda.w tccs_{WLA_FILENAME}_tabla + 0
sta.b tcc__r4
lda.w tccs_{WLA_FILENAME}_tabla + 0 + 2
sta.b tcc__r4h
clc
lda.b tcc__r4
adc.b tcc__r3
clc
adc.w #4
sta.b tcc__r4
lda.b [tcc__r4]
pha
lda.b tcc__r0
sta -6 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
lda.b tcc__r0h
sta -4 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
lda.b tcc__r1
sta -10 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
lda.b tcc__r1h
sta -8 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
lda.b tcc__r2
sta -14 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
lda.b tcc__r2h
sta -12 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
jsr.l tccs_{WLA_FILENAME}_tile_del_bloque32
pla
sep #$20
lda #0
pha
rep #$20
pei (tcc__r0)
sep #$20
lda #0
pha
rep #$20
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_tarou_izquierda + 0
pha
lda #3
pha
rep #$20
lda -10 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
pha
lda -4 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
pha
lda 2 + __tccs_{WLA_FILENAME}_dibujar_locals + 1,s
pha
jsr.l oamSet
tsa
clc
adc #12
tas
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
asl a
asl a
sta.b tcc__r0
pea.w (0 * 256 + 0)
sep #$20
rep #$20
pei (tcc__r0)
jsr.l oamSetEx
tsa
clc
adc #4
tas
jmp.w __local_9
__local_3:
__local_12:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
sta.b tcc__r0
sec
sbc.w #6
bvc +
eor #$8000
+
bmi +
brl __local_10
+
bra __local_11
__local_13:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
sep #$20
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_i + 0
rep #$20
bra __local_12
__local_11:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
and.w #255
sep #$20
pha
rep #$20
jsr.l tccs_{WLA_FILENAME}_esconder
tsa
clc
adc #1
tas
bra __local_13
__local_10:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_cuadro_barbara + 0
pha
rep #$20
jsr.l tccs_{WLA_FILENAME}_tile_del_bloque64
tsa
clc
adc #1
tas
sep #$20
lda #1
pha
rep #$20
pei (tcc__r0)
pea.w (0 * 256 + 0)
sep #$20
lda #3
pha
rep #$20
lda.w tccs_{WLA_FILENAME}_y_barbara + 0
pha
lda.w tccs_{WLA_FILENAME}_x_barbara + 0
pha
pea.w 24
jsr.l oamSet
tsa
clc
adc #12
tas
pea.w (0 * 256 + 1)
sep #$20
rep #$20
pea.w 24
jsr.l oamSetEx
tsa
clc
adc #4
tas
.ifgr __tccs_{WLA_FILENAME}_dibujar_locals 0
tsa
clc
adc #__tccs_{WLA_FILENAME}_dibujar_locals
tas
.endif
rtl
.ENDS

.SECTION ".personajes_mostrartext_0x5" SUPERFREE

personajes_mostrar:
lda 3 + 1,s
sta.w tccs_{WLA_FILENAME}_x_tarou + 0
lda 5 + 1,s
sta.w tccs_{WLA_FILENAME}_y_tarou + 0
lda.w #0
sep #$20
lda 7 + 1,s
sta.w tccs_{WLA_FILENAME}_tarou_izquierda + 0
rep #$20
lda 8 + 1,s
sta.w tccs_{WLA_FILENAME}_x_barbara + 0
lda 10 + 1,s
sta.w tccs_{WLA_FILENAME}_y_barbara + 0
lda.w #0
sep #$20
sta.w tccs_{WLA_FILENAME}_cuadro_tarou + 0
rep #$20
lda.w #0
sep #$20
sta.w tccs_{WLA_FILENAME}_cuadro_barbara + 0
rep #$20
lda.w #0
sep #$20
sta.w tccs_{WLA_FILENAME}_frames_cuadro + 0
rep #$20
lda.w #1
sep #$20
sta.w tccs_{WLA_FILENAME}_visibles + 0
rep #$20
jsr.l tccs_{WLA_FILENAME}_dibujar
rtl
.ENDS

.SECTION ".personajes_animartext_0x6" SUPERFREE

personajes_animar:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_visibles + 0
rep #$20
sta.b tcc__r0
lda.b tcc__r0 ; DON'T OPTIMIZE
bne +
brl __local_14
+
bra __local_15
__local_14:
jmp.w __local_16
__local_15:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_frames_cuadro + 0
rep #$20
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
sep #$20
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_frames_cuadro + 0
rep #$20
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_frames_cuadro + 0
rep #$20
sta.b tcc__r0
sec
sbc.w #8
bvc +
eor #$8000
+
bmi +
brl __local_17
+
jmp.w __local_18
__local_17:
lda.w #0
sep #$20
sta.w tccs_{WLA_FILENAME}_frames_cuadro + 0
rep #$20
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_cuadro_tarou + 0
rep #$20
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
sep #$20
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_cuadro_tarou + 0
rep #$20
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_cuadro_tarou + 0
rep #$20
sta.b tcc__r0
ldx #1
sec
sbc.w #2
tay
bvc +
eor #$8000
+
bpl +++
++
dex
+++
stx.b tcc__r5
txa
bne +
brl __local_19
+
lda.w #0
sta.b tcc__r0
sep #$20
sta.w tccs_{WLA_FILENAME}_cuadro_tarou + 0
rep #$20
__local_19:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_cuadro_barbara + 0
rep #$20
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
sep #$20
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_cuadro_barbara + 0
rep #$20
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_cuadro_barbara + 0
rep #$20
sta.b tcc__r0
ldx #1
sec
sbc.w #4
tay
bvc +
eor #$8000
+
bpl +++
++
dex
+++
stx.b tcc__r5
txa
bne +
brl __local_20
+
lda.w #0
sta.b tcc__r0
sep #$20
sta.w tccs_{WLA_FILENAME}_cuadro_barbara + 0
rep #$20
__local_20:
jsr.l tccs_{WLA_FILENAME}_dibujar
__local_16:
__local_18:
rtl
.ENDS

.SECTION ".personajes_ocultartext_0x7" SUPERFREE

personajes_ocultar:
lda.w #0
sta.b tcc__r0
sep #$20
sta.w tccs_{WLA_FILENAME}_i + 0
rep #$20
__local_23:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
sta.b tcc__r0
sec
sbc.w #6
bvc +
eor #$8000
+
bmi +
brl __local_21
+
bra __local_22
__local_24:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
sep #$20
lda.b tcc__r0
sta.w tccs_{WLA_FILENAME}_i + 0
rep #$20
bra __local_23
__local_22:
lda.w #0
sep #$20
lda.w tccs_{WLA_FILENAME}_i + 0
rep #$20
and.w #255
sep #$20
pha
rep #$20
jsr.l tccs_{WLA_FILENAME}_esconder
tsa
clc
adc #1
tas
bra __local_24
__local_21:
sep #$20
lda #6
pha
rep #$20
jsr.l tccs_{WLA_FILENAME}_esconder
tsa
clc
adc #1
tas
lda.w #0
sta.b tcc__r0
sep #$20
sta.w tccs_{WLA_FILENAME}_visibles + 0
rep #$20
rtl
.ENDS
.BASE $00
.RAMSECTION "ram{WLA_FILENAME}.data" APPENDTO "globram.data"

tarou_reposo_estatico_metasprites dsb 8

.ENDS

.SECTION "{WLA_FILENAME}.data" APPENDTO "glob.data"

.dw tarou_reposo_estatico_metasprite0 + 0, :tarou_reposo_estatico_metasprite0
.dw tarou_reposo_estatico_metasprite1 + 0
.dw :tarou_reposo_estatico_metasprite1
.ENDS

.SECTION ".rodata" SUPERFREE

tarou_reposo_estatico_metasprite0: .db $0,$0,$0,$0,$0,$0,$30,$0,$20,$0,$0,$0,$1,$0,$30,$0,$40,$0,$0,$0,$2,$0,$30,$0,$0,$0,$20,$0,$6,$0,$30,$0,$20,$0,$20,$0,$7,$0,$30,$0,$40,$0,$20,$0,$8,$0,$30,$0,$80,$ff,$0,$0,$0,$0,$0,$0
tarou_reposo_estatico_metasprite1: .db $0,$0,$0,$0,$3,$0,$30,$0,$20,$0,$0,$0,$4,$0,$30,$0,$40,$0,$0,$0,$5,$0,$30,$0,$0,$0,$20,$0,$9,$0,$30,$0,$20,$0,$20,$0,$a,$0,$30,$0,$40,$0,$20,$0,$b,$0,$30,$0,$80,$ff,$0,$0,$0,$0,$0,$0
.ENDS

.BASE $00
.RAMSECTION ".bss" BANK $7e SLOT 2
tccs_{WLA_FILENAME}_x_tarou dsb 2
tccs_{WLA_FILENAME}_y_tarou dsb 2
tccs_{WLA_FILENAME}_x_barbara dsb 2
tccs_{WLA_FILENAME}_y_barbara dsb 2
tccs_{WLA_FILENAME}_tarou_izquierda dsb 1
tccs_{WLA_FILENAME}_cuadro_tarou dsb 1
tccs_{WLA_FILENAME}_cuadro_barbara dsb 1
tccs_{WLA_FILENAME}_frames_cuadro dsb 1
tccs_{WLA_FILENAME}_visibles dsb 1
tccs_{WLA_FILENAME}_tabla dsb 4
tccs_{WLA_FILENAME}_piezas dsb 1
tccs_{WLA_FILENAME}_i dsb 1
.ENDS
