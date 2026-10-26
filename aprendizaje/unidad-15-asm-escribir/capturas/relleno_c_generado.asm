.include "hdr.asm"
.accu 16
.index 16
.16bit
.BASE $80

.SECTION ".relleno_vram_ctext_0x0" SUPERFREE

relleno_vram_c:
lda.w #128
sep #$20
sta.l 8448
rep #$20
lda.w #128
sep #$20
sta.l 8469
rep #$20
lda 3 + 1,s
sta.b tcc__r0
lda.w #8470
sta.b tcc__r9
stz.b tcc__r9h
lda.b tcc__r0
sta.b [tcc__r9]
__local_2:
lda 7 + 1,s
sta.b tcc__r0
ldx #1
sec
sbc #0
tay
bne +
dex
+
stx.b tcc__r5
txa
bne +
brl __local_0
+
bra __local_1
__local_3:
lda 7 + 1,s
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
clc
lda.b tcc__r0
adc.w #65535
sta.b tcc__r0
sta 7 + 1,s
bra __local_2
__local_1:
lda 5 + 1,s
sta.b tcc__r0
lda.w #8472
sta.b tcc__r9
stz.b tcc__r9h
lda.b tcc__r0
sta.b [tcc__r9]
bra __local_3
__local_0:
lda.w #0
sep #$20
lda.l mirrorINIDISP + 0
rep #$20
sta.b tcc__r0
sep #$20
sta.l 8448
rep #$20
rtl
.ENDS

.SECTION ".limpiar_ctext_0x1" SUPERFREE

limpiar_c:
__local_6:
lda 7 + 1,s
sta.b tcc__r0
ldx #1
sec
sbc #0
tay
bne +
dex
+
stx.b tcc__r5
txa
bne +
brl __local_4
+
bra __local_5
__local_7:
lda 7 + 1,s
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
clc
lda.b tcc__r0
adc.w #65535
sta.b tcc__r0
sta 7 + 1,s
bra __local_6
__local_5:
lda 5 + 1,s
sta.b tcc__r0h
lda 3 + 1,s
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
lda.b tcc__r0
sta 3 + 1,s
lda.b tcc__r0h
sta 5 + 1,s
lda.w #0
sta.b tcc__r0
sep #$20
sta.b [tcc__r1]
rep #$20
bra __local_7
__local_4:
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
