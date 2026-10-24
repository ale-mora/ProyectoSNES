/*---------------------------------------------------------------------------------
    prueba_p16.c — Pruebas de juego/p16.c.

    PROPÓSITO
      Leer el índice de una tira .p16 hecha a mano con el formato de
      scripts/piezas16.py: 8 bytes por cuadro (u16 piezas, u16 tiles, u8 n, relleno).

    SE CONECTA CON
      Incluye:  minitest.h, juego/p16.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_p16).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Pieza de 16×16
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/p16.h"

void pruebas_p16(void)
{
    /* Dos cuadros: el 0 con 3 piezas (tabla en 16, espejada en 22, tiles en 0x0124),
       el 1 con 21 (tabla en 28, espejada en 70, tiles en 0x02A4). */
    static const u8 tira[16] = {
        16, 0, 0x24, 0x01, 3, 0, 22, 0,
        28, 0, 0xA4, 0x02, 21, 0, 70, 0,
    };
    P16Cuadro c;

    p16_cuadro(tira, 0, &c);
    COMPROBAR(c.piezas == 16 && c.tiles == 0x0124 && c.n == 3 && c.espejo == 22);
    p16_cuadro(tira, 1, &c);
    COMPROBAR(c.piezas == 28 && c.tiles == 0x02A4 && c.n == 21 && c.espejo == 70);
}
