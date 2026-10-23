/*---------------------------------------------------------------------------------
    p16.c — Leer el índice de una tira .p16 (ver p16.h).

    PROPÓSITO
      El registro del cuadro k está en tira + k·8 (un desplazamiento: k << 3):
      u16 desplazamiento de las piezas, u16 desplazamiento de los tiles, u8 n,
      u8 relleno, u16 desplazamiento de las piezas espejadas.

    SE CONECTA CON
      Incluye:  juego/p16.h.
      Lo usan:  pantallas/prueba_carga.c y tests/prueba_p16.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Pieza de 16×16
---------------------------------------------------------------------------------*/
#include "juego/p16.h"

void p16_cuadro(const u8 *tira, u8 cuadro, P16Cuadro *c)
{
    const u8 *r = tira + ((u16)cuadro << 3);         /* Su registro: 8 bytes por cuadro.    */

    c->piezas = (u16)(r[0] | ((u16)r[1] << 8));      /* Little endian, byte a byte.         */
    c->tiles = (u16)(r[2] | ((u16)r[3] << 8));
    c->n = r[4];
    c->espejo = (u16)(r[6] | ((u16)r[7] << 8));
}
