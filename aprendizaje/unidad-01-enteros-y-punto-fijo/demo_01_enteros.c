/*---------------------------------------------------------------------------------
    demo_01_enteros.c — Demo 1 de la unidad 01: lo que pasa cuando un entero de
    8 o 16 bits se queda sin sitio.

    PROPÓSITO
      Enseña, en cinco pasos numerados en la salida: el desbordamiento de los
      enteros sin signo (255 + 1 en un u8 da 0), cómo se guardan los negativos
      en un s16 (complemento a dos: -1 es 0xFFFF), el desbordamiento con signo
      (32767 + 1 da -32768), por qué el punto fijo 12.4 del juego no llega a
      2048 px y por qué 255 * 256 no cabe en el int de 16 bits de tcc816.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), sistema/puntofijo.h (PF, PF_ENTERO; trae
                sistema/tipos.h: u8, u16, s16).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/sistema/puntofijo.h, la constante X_MAX_FP de
                src/juego/tarou.c y la macro RAMPA de src/escena/escena.h.
      Compilar: gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I..\..\src -o demo_01_enteros.exe demo_01_enteros.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      c          u8    0..255          contador de 8 bits que da la vuelta
      n          u16   0..65535        contador de 16 bits que da la vuelta
      v          s16   -32768..32767   valores con signo
      bruto      s16   -256 / 32640    productos calculados "a 16 bits"

    → conceptos (README de la unidad 01): desbordamiento, complemento a dos,
      punto fijo, int de 16 bits
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include "sistema/puntofijo.h"       /* El mismo archivo que usa la ROM: PF y PF_ENTERO.  */

/* En gcc un int mide 32 bits; en tcc816 (el compilador de la SNES), 16. Para ver
   en el PC lo que pasaría en la consola, esta función multiplica y recorta el
   resultado a 16 bits, que es lo que hace tcc816 con cualquier producto. */
static s16 por_16_bits(s16 a, s16 b)
{
    return (s16)(a * b);             /* El cast tira los bits que no caben en 16.         */
}

int main(void)                       /* El programa empieza aquí.                         */
{
    u8 c;                            /* Un entero sin signo de 8 bits.                    */
    u16 n;                           /* Uno de 16 bits.                                   */
    s16 v;                           /* Uno de 16 bits con signo.                         */
    s16 bruto;                       /* Un producto calculado a 16 bits.                  */

    /* 1. Sin signo: al pasarse del máximo vuelve a 0, y al bajar de 0 va al máximo. */
    c = 255;                         /* El mayor valor de un u8.                          */
    c = (u8)(c + 1);                 /* 256 no cabe en 8 bits: se queda el byte bajo.     */
    printf("1. u8:  255 + 1 = %u\n", (unsigned)c);
    c = 0;                           /* El menor valor de un u8.                          */
    c = (u8)(c - 1);                 /* -1 tampoco cabe: da la vuelta por arriba.         */
    printf("1. u8:  0 - 1 = %u\n", (unsigned)c);
    n = 65535;                       /* El mayor valor de un u16.                         */
    n = (u16)(n + 1);                /* Vuelve a 0, igual que el u8.                      */
    printf("1. u16: 65535 + 1 = %u\n", (unsigned)n);

    /* 2. Complemento a dos: los mismos 16 bits leídos con signo (s16) y sin signo (u16). */
    v = 1;
    printf("2. s16 %6d se guarda como 0x%04X (leido sin signo: %u)\n", v, (unsigned)(u16)v, (unsigned)(u16)v);
    v = -1;
    printf("2. s16 %6d se guarda como 0x%04X (leido sin signo: %u)\n", v, (unsigned)(u16)v, (unsigned)(u16)v);
    v = -96;                         /* VEL_SALTO de tarou.h.                             */
    printf("2. s16 %6d se guarda como 0x%04X (leido sin signo: %u)\n", v, (unsigned)(u16)v, (unsigned)(u16)v);
    v = -32768;                      /* El menor valor de un s16.                         */
    printf("2. s16 %6d se guarda como 0x%04X (leido sin signo: %u)\n", v, (unsigned)(u16)v, (unsigned)(u16)v);
    /* La receta para negar: invertir todos los bits (~) y sumar 1. */
    printf("2. ~96 + 1 = 0x%04X, el mismo patron que -96\n", (unsigned)(u16)(~96 + 1));

    /* 3. Con signo: pasarse de 32767 da el negativo más grande. */
    v = 32767;                       /* El mayor valor de un s16.                         */
    v = (s16)(v + 1);                /* 0x7FFF + 1 = 0x8000, que con signo es -32768.     */
    printf("3. s16: 32767 + 1 = %d\n", v);

    /* 4. El punto fijo 12.4 vive en un s16: llega hasta 32767 / 16 = 2047,9 px. */
    printf("4. PF(1952) = %d (X_MAX_FP de tarou.c: cabe)\n", PF(1952));
    printf("4. PF(2047) = %d (el ultimo pixel entero que cabe)\n", PF(2047));
    printf("4. PF(2048) = %d (el ancho del nivel ya NO cabe)\n", PF(2048));

    /* 5. El int de tcc816 es de 16 bits: 255 * 256 = 65280 no cabe (el máximo es 32767). */
    bruto = por_16_bits(255, 256);   /* Lo que daría la SNES si lo calculara en el frame. */
    printf("5. 255 * 256 a 16 bits = %d (deberia ser 65280)\n", bruto);
    printf("5. ... y dividido entre 32 = %d (deberia ser 2040)\n", bruto / 32);
    bruto = por_16_bits(255, 128);   /* La mitad sí cabe: 32640 < 32767.                  */
    printf("5. (255 * 128 / 32) * 2 a 16 bits = %d (correcto)\n", (s16)((bruto / 32) * 2));

    return 0;                        /* Terminó bien.                                     */
}
