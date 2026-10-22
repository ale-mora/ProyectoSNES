.include "hdr.asm"
.accu 16
.index 16
.16bit
.BASE $80
.define __camara_para_locals 2

.SECTION ".camara_paratext_0x0" SUPERFREE

camara_para:
.ifgr __camara_para_locals 0
tsa
sec
sbc #__camara_para_locals
tas
.endif
lda 3 + __camara_para_locals + 1,s
clc
adc.w #48
sec
sbc.w #128
sta -2 + __camara_para_locals + 1,s
sta.b tcc__r0
sec
sbc.w #0
bvc +
eor #$8000
+
bmi +
brl __local_0
+
stz.b tcc__r0
lda.b tcc__r0
sta -2 + __camara_para_locals + 1,s
__local_0:
lda -2 + __camara_para_locals + 1,s
sta.b tcc__r0
ldx #1
sec
sbc.w #1792
tay
beq ++
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
brl __local_1
+
lda.w #1792
sta.b tcc__r0
sta -2 + __camara_para_locals + 1,s
__local_1:
lda -2 + __camara_para_locals + 1,s
sta.b tcc__r0
__local_2:
.ifgr __camara_para_locals 0
tsa
clc
adc #__camara_para_locals
tas
.endif
rtl
.ENDS
.BASE $00
.RAMSECTION "ram{WLA_FILENAME}.data" APPENDTO "globram.data"

.ENDS

.SECTION "{WLA_FILENAME}.data" APPENDTO "glob.data"

.ENDS

.SECTION ".rodata" SUPERFREE

.ENDS

.BASE $00
.RAMSECTION ".bss" BANK $7e SLOT 2
.ENDS
