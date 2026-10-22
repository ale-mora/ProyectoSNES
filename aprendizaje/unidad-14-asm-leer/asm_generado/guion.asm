.include "hdr.asm"
.accu 16
.index 16
.16bit
.BASE $80
.define __tccs_{WLA_FILENAME}_mantenidos_locals 1
.define __guion_mando_locals 3

.SECTION ".tccs_{WLA_FILENAME}_mantenidostext_0x0" SUPERFREE

tccs_{WLA_FILENAME}_mantenidos:
.ifgr __tccs_{WLA_FILENAME}_mantenidos_locals 0
tsa
sec
sbc #__tccs_{WLA_FILENAME}_mantenidos_locals
tas
.endif
lda.w #0
sta.b tcc__r0
sep #$20
sta -1 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
rep #$20
__local_2:
lda.w #0
sep #$20
lda 7 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
rep #$20
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
__local_5:
lda.w #0
sep #$20
lda 7 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
rep #$20
sta.b tcc__r0
lda.b tcc__r0h
dec.b tcc__r0
sep #$20
lda.b tcc__r0
sta 7 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
rep #$20
lda 5 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r0h
lda 3 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r0
sta.b tcc__r1
lda.b tcc__r0h
sta.b tcc__r1h
clc
lda.b tcc__r0
adc.w #6
sta.b tcc__r0
sta 3 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
lda.b tcc__r0h
sta 5 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
jmp.w __local_2
__local_1:
lda 3 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r0
lda 5 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r0h
lda 8 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r1
lda.b [tcc__r0]
sta.b tcc__r2
ldx #1
lda.b tcc__r1
sec
sbc.b tcc__r2
tay
bcs ++
+ dex
++
stx.b tcc__r5
txa
bne +
brl __local_3
+
lda 5 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r0h
lda 3 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
inc a
inc a
sta.b tcc__r0
lda 8 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r1
lda.b [tcc__r0]
sta.b tcc__r2
ldx #1
lda.b tcc__r1
sec
sbc.b tcc__r2
tay
bcc ++
+ dex
++
stx.b tcc__r5
txa
bne +
__local_3:
brl __local_4
+
lda 3 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r0
lda 5 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #4
sta.b tcc__r0
lda.w #0
sep #$20
lda -1 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
rep #$20
sta.b tcc__r1
lda.w #0
sep #$20
lda.b [tcc__r0]
rep #$20
sta.b tcc__r2
ora.b tcc__r1
sta.b tcc__r1
sep #$20
sta -1 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
rep #$20
__local_4:
jmp.w __local_5
__local_0:
lda.w #0
sep #$20
lda -1 + __tccs_{WLA_FILENAME}_mantenidos_locals + 1,s
rep #$20
sta.b tcc__r0
__local_6:
.ifgr __tccs_{WLA_FILENAME}_mantenidos_locals 0
tsa
clc
adc #__tccs_{WLA_FILENAME}_mantenidos_locals
tas
.endif
rtl
.ENDS

.SECTION ".guion_mandotext_0x1" SUPERFREE

guion_mando:
.ifgr __guion_mando_locals 0
tsa
sec
sbc #__guion_mando_locals
tas
.endif
lda 8 + __guion_mando_locals + 1,s
pha
lda.w #0
sep #$20
lda 9 + __guion_mando_locals + 1,s
pha
rep #$20
lda 6 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 8 + __guion_mando_locals + 1,s
pha
pei (tcc__r0)
jsr.l tccs_{WLA_FILENAME}_mantenidos
tsa
clc
adc #7
tas
sep #$20
lda.b tcc__r0
sta -1 + __guion_mando_locals + 1,s
rep #$20
lda 8 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda.b tcc__r0 ; DON'T OPTIMIZE
bne +
brl __local_7
+
lda 8 + __guion_mando_locals + 1,s
sta.b tcc__r0
dec.b tcc__r0
pei (tcc__r0)
lda.w #0
sep #$20
lda 9 + __guion_mando_locals + 1,s
pha
rep #$20
lda 6 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 8 + __guion_mando_locals + 1,s
pha
pei (tcc__r0)
jsr.l tccs_{WLA_FILENAME}_mantenidos
tsa
clc
adc #7
tas
bra __local_8
__local_7:
lda.w #0
sta.b tcc__r0
__local_8:
__local_9:
sep #$20
lda.b tcc__r0
sta -2 + __guion_mando_locals + 1,s
rep #$20
lda.w #0
sep #$20
lda -2 + __guion_mando_locals + 1,s
rep #$20
eor.w #65535
sta.b tcc__r0
lda.w #0
sep #$20
lda -1 + __guion_mando_locals + 1,s
rep #$20
and.b tcc__r0
and.w #255
sep #$20
sta -3 + __guion_mando_locals + 1,s
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
lda.w #0
sep #$20
lda -1 + __guion_mando_locals + 1,s
rep #$20
and.w #1
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
lda 10 + __guion_mando_locals + 1,s
inc a
sta.b tcc__r0
lda.w #0
sep #$20
lda -1 + __guion_mando_locals + 1,s
rep #$20
and.w #2
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
lda 10 + __guion_mando_locals + 1,s
inc a
inc a
sta.b tcc__r0
lda.w #0
sep #$20
lda -1 + __guion_mando_locals + 1,s
rep #$20
and.w #4
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #3
sta.b tcc__r0
lda.w #0
sep #$20
lda -1 + __guion_mando_locals + 1,s
rep #$20
and.w #8
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #4
sta.b tcc__r0
lda.w #0
sep #$20
lda -1 + __guion_mando_locals + 1,s
rep #$20
and.w #16
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #5
sta.b tcc__r0
lda.w #0
sep #$20
lda -3 + __guion_mando_locals + 1,s
rep #$20
and.w #16
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #6
sta.b tcc__r0
lda.w #0
sep #$20
lda -3 + __guion_mando_locals + 1,s
rep #$20
and.w #32
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #7
sta.b tcc__r0
lda.w #0
sep #$20
lda -3 + __guion_mando_locals + 1,s
rep #$20
and.w #64
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #8
sta.b tcc__r0
lda.w #0
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #9
sta.b tcc__r0
lda.w #0
sep #$20
sta.b [tcc__r0]
rep #$20
lda 10 + __guion_mando_locals + 1,s
sta.b tcc__r0
lda 12 + __guion_mando_locals + 1,s
sta.b tcc__r0h
clc
lda.b tcc__r0
adc.w #10
sta.b tcc__r0
lda.w #0
sep #$20
lda -1 + __guion_mando_locals + 1,s
rep #$20
sta.b tcc__r1
ldx #1
sec
sbc #0
tay
bne +
dex
+
txa
and.w #255
sta.b tcc__r5
sep #$20
sta.b [tcc__r0]
rep #$20
.ifgr __guion_mando_locals 0
tsa
clc
adc #__guion_mando_locals
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
