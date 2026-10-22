.include "hdr.asm"
.accu 16
.index 16
.16bit
.BASE $80

.SECTION ".nivel_columna_vramtext_0x0" SUPERFREE

nivel_columna_vram:
lda 3 + 1,s
and.w #32
sta.b tcc__r0
ldy.w #5
-
asl a
dey
bne -
+
sta.b tcc__r0
lda 3 + 1,s
and.w #31
sta.b tcc__r1
ora.b tcc__r0
sta.b tcc__r0
__local_0:
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
