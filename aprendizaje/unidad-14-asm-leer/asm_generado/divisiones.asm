.include "hdr.asm"
.accu 16
.index 16
.16bit
.BASE $80
.define __div_u8_local_locals 2

.SECTION ".div_u8text_0x0" SUPERFREE

div_u8:
lda.w #0
sep #$20
lda 3 + 1,s
rep #$20
sta.b tcc__r0
tax
lda.w #2
jsr.l tcc__div
lda.b tcc__r9
sta.b tcc__r0
__local_0:
rtl
.ENDS

.SECTION ".div_u8_casttext_0x1" SUPERFREE

div_u8_cast:
lda.w #0
sep #$20
lda 3 + 1,s
rep #$20
sta.b tcc__r0
lsr.b tcc__r0
__local_1:
rtl
.ENDS

.SECTION ".div_u8_localtext_0x2" SUPERFREE

div_u8_local:
.ifgr __div_u8_local_locals 0
tsa
sec
sbc #__div_u8_local_locals
tas
.endif
lda.w #0
sep #$20
lda 3 + __div_u8_local_locals + 1,s
rep #$20
sta -2 + __div_u8_local_locals + 1,s
sta.b tcc__r0
lsr.b tcc__r0
__local_2:
.ifgr __div_u8_local_locals 0
tsa
clc
adc #__div_u8_local_locals
tas
.endif
rtl
.ENDS

.SECTION ".div_u16text_0x3" SUPERFREE

div_u16:
lda 3 + 1,s
sta.b tcc__r0
lsr.b tcc__r0
__local_3:
rtl
.ENDS

.SECTION ".div_s16text_0x4" SUPERFREE

div_s16:
lda 3 + 1,s
sta.b tcc__r0
tax
lda.w #2
jsr.l tcc__div
lda.b tcc__r9
sta.b tcc__r0
__local_4:
rtl
.ENDS

.SECTION ".shr_u8text_0x5" SUPERFREE

shr_u8:
lda.w #0
sep #$20
lda 3 + 1,s
rep #$20
sta.b tcc__r0
cmp #$8000
ror.b tcc__r0
__local_5:
rtl
.ENDS

.SECTION ".div_u16_vartext_0x6" SUPERFREE

div_u16_var:
lda 3 + 1,s
sta.b tcc__r0
lda 5 + 1,s
sta.b tcc__r1
ldx.b tcc__r0
jsr.l tcc__udiv
lda.b tcc__r9
sta.b tcc__r0
__local_6:
rtl
.ENDS

.SECTION ".llenar_ctext_0x7" SUPERFREE

llenar_c:
__local_8:
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
brl __local_7
+
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
sep #$20
sta.b [tcc__r1]
rep #$20
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
jmp.w __local_8
__local_7:
rtl
.ENDS

.SECTION ".llenar_c16text_0x8" SUPERFREE

llenar_c16:
__local_11:
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
brl __local_9
+
bra __local_10
__local_12:
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
bra __local_11
__local_10:
lda 5 + 1,s
sta.b tcc__r0h
lda 3 + 1,s
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
inc.b tcc__r0
inc.b tcc__r0
lda.b tcc__r0
sta 3 + 1,s
lda.b tcc__r0h
sta 5 + 1,s
stz.b tcc__r0
lda.b tcc__r0
sta.b [tcc__r1]
bra __local_12
__local_9:
rtl
.ENDS

.SECTION ".escribir_vramtext_0x9" SUPERFREE

escribir_vram:
lda 3 + 1,s
sta.b tcc__r0
lda.w #8472
sta.b tcc__r9
stz.b tcc__r9h
lda.b tcc__r0
sta.b [tcc__r9]
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
