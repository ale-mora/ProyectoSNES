/*---------------------------------------------------------------------------------
    relleno_c.c — Las versiones en C que se comparan con relleno.asm y con memset.

    PROPÓSITO
      relleno_vram_c hace exactamente lo mismo que relleno_vram (relleno.asm),
      pero escrita en C con las macros REG_xxx de la librería. Sirve para dos
      comparaciones de la unidad 15:
        - leer su ensamblador generado (`make src/relleno_c.asm`) junto al
          escrito a mano y contar instrucciones por vuelta;
        - medir los ciclos de las dos en el profiler de Mesen.
      limpiar_c es el bucle `*p++ = 0` que el juego dejó de usar en el HUD
      (src/juego/hud_debug.c, empezar_fila) en favor de memset.

    SE CONECTA CON
      Incluye:  <snes.h> (REG_INIDISP, REG_VMAIN, REG_VMADDLH, REG_VMDATALH,
                mirrorINIDISP), relleno.h.
      Lo usa:   principal.c.

    HARDWARE QUE TOCA
      INIDISP $2100, VMAIN $2115, VMADD $2116, VMDATA $2118 (como relleno.asm).

    VARIABLES: ninguna.
    → glosario: VRAM, Forced blank
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "relleno.h"

void relleno_vram_c(u16 vram, u16 palabra, u16 cuantas)
{
    REG_INIDISP = 0x80;                 /* Forced blank: la VRAM se puede escribir ya.   */
    REG_VMAIN = 0x80;                   /* +1 palabra tras escribir el byte alto.         */
    REG_VMADDLH = vram;                 /* Dónde empieza la escritura (en palabras).      */
    for (; cuantas != 0; cuantas--)     /* Cuenta atrás, como el dex/bne del .asm.        */
    {
        REG_VMDATALH = palabra;         /* Una palabra a $2118/$2119.                     */
    }
    REG_INIDISP = mirrorINIDISP;        /* La pantalla, como la tenía el SDK.             */
}

void limpiar_c(u8 *p, u16 n)
{
    for (; n != 0; n--)                 /* n vueltas...                                   */
    {
        *p++ = 0;                       /* ...cada una pone un byte a 0 y avanza.         */
    }
}
