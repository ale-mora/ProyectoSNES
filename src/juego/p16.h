/*---------------------------------------------------------------------------------
    p16.h — Leer el índice de una tira en formato .p16 (piezas de 16x16).

    PROPÓSITO
      Una tira .p16 (scripts/piezas16.py) empieza con un registro de 8 bytes por
      cuadro: dónde está su tabla de piezas (dx, dy), dónde está la misma tabla
      espejada (para mirar a la izquierda), dónde están sus tiles (en orden de
      subida) y cuántas piezas tiene. p16_cuadro() lee ese registro y
      devuelve los DESPLAZAMIENTOS desde el principio de la tira: el que llama les
      suma la dirección de la tira dentro de su banco (en la SNES, los 16 bits
      bajos del puntero, como hace tarou_sprite.c con las tiras de Tarou).
      Lee byte a byte (little endian), así que es igual en la SNES y en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  pantallas/prueba_carga.c y tests/prueba_p16.c.

    HARDWARE QUE TOCA
      ninguno (lógica pura).

    VARIABLES: ninguna.
    → glosario: Pieza de 16×16
---------------------------------------------------------------------------------*/
#ifndef P16_H
#define P16_H

#include "sistema/tipos.h"

#define P16_REGISTRO 8               /* Bytes de un registro del índice.                  */

/* Un cuadro: 8 bytes. */
typedef struct
{
    u16 piezas;                      /* Desplazamiento de su tabla (dx, dy) en la tira.   */
    u16 tiles;                       /* Desplazamiento de sus tiles (n·128 bytes).        */
    u16 espejo;                      /* Desplazamiento de su tabla espejada (a la izquierda). */
    u8 n;                            /* Piezas del cuadro.                                */
    u8 libre;                        /* Relleno.                                          */
} P16Cuadro;

/* Lee el registro del cuadro `cuadro` de la tira que empieza en `tira`. */
void p16_cuadro(const u8 *tira, u8 cuadro, P16Cuadro *c);

#endif /* P16_H */
