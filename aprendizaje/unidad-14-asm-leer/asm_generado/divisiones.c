/*---------------------------------------------------------------------------------
    divisiones.c — Sonda de la unidad 14: qué hace tcc816 con `/`, `>>` y bucles
    de relleno según el TIPO de los operandos.

    PROPÓSITO
      No es parte de ninguna ROM: se compiló en un clon del juego (como
      src/prueba/divs.c) con `make src/prueba/divs.asm`, y su ensamblador está en
      divisiones.asm, al lado. Cada función hace UNA cosa para ver su traducción
      aislada. Lo que sale (2026-10-22, tcc816 de PVSnesLib 4.6.0, -F):
        div_u8        u8 / 2              jsr.l tcc__div  (u8 se promueve a int, CON signo)
        div_u8_cast   (u16)u8 / 2         lsr             (sin signo: desplazar basta)
        div_u8_local  u16 m = u8; m / 2   lsr
        div_u16       u16 / 2             lsr
        div_s16       s16 / 2             jsr.l tcc__div  (con signo: -3/2 debe dar -1)
        shr_u8        u8 >> 1             cmp #$8000 + ror (desplazamiento aritmético)
        div_u16_var   u16 / u16           jsr.l tcc__udiv (divisor desconocido)
        llenar_c      *p++ = 0 (u8)       37 instrucciones por vuelta
        llenar_c16    *p++ = 0 (u16)      39 instrucciones por vuelta
        escribir_vram REG_VMDATALH = v    puntero largo a $002118 y sta [tcc__r9]

    SE CONECTA CON
      Incluye:  <snes.h> (u8, u16, s16, REG_VMDATALH).
      Explicado en: README.md de la unidad 14, «División, multiplicación y tipos».

    HARDWARE QUE TOCA
      escribir_vram: VMDATA $2118 (solo para ver cómo se compila).

    VARIABLES: ninguna.
    → glosario: u8, u16, s16
---------------------------------------------------------------------------------*/
#include <snes.h>

u16 div_u8(u8 n) { return (u16)(n / 2); }                       /* u8 / 2: tcc__div.    */
u16 div_u8_cast(u8 n) { return (u16)((u16)n / 2); }             /* (u16)u8 / 2: lsr.    */
u16 div_u8_local(u8 n) { u16 m = n; return (u16)(m / 2); }      /* vía un u16: lsr.     */
u16 div_u16(u16 n) { return (u16)(n / 2); }                     /* u16 / 2: lsr.        */
u16 div_s16(s16 n) { return (u16)(n / 2); }                     /* s16 / 2: tcc__div.   */
u16 shr_u8(u8 n) { return (u16)(n >> 1); }                      /* >> de un int: ror.   */
u16 div_u16_var(u16 n, u16 d) { return (u16)(n / d); }          /* tcc__udiv.           */
void llenar_c(u8 *p, u16 n) { while (n != 0) { *p++ = 0; n--; } }        /* Bytes.      */
void llenar_c16(u16 *p, u16 n) { for (; n != 0; n--) { *p++ = 0; } }     /* Palabras.   */
void escribir_vram(u16 v) { REG_VMDATALH = v; }                 /* Un registro de 16 b. */
