/*---------------------------------------------------------------------------------
    metasprite.c — Recorrer y espejar tablas metasprite de gfx4snes -T (lógica pura).

    PROPÓSITO
      Ver metasprite.h. Dos bucles que paran en el terminador (dx = 0xFF80) o,
      como red de seguridad, en la sexta pieza. Sin <snes.h>: se compila con
      tcc816 para la ROM y con gcc para tests/.

    SE CONECTA CON
      Incluye:  juego/metasprite.h.
      Lo usan:  juego/tarou_sprite.c, pantallas/personajes.c, tests/prueba_metasprite.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna global (n es local a cada función).
    → glosario: Metasprite, Dirección
---------------------------------------------------------------------------------*/
#include "juego/metasprite.h"

u8 metasprite_piezas(const t_metasprite *tabla)
{
    u8 n = 0;                                        /* Piezas vistas hasta ahora.          */
    while (n < METASPRITE_PIEZAS_MAX && tabla[n].dx != METASPRITE_FIN)
    {
        n++;                                         /* Esta entrada es una pieza: siguiente.*/
    }
    return n;
}

u8 metasprite_espejar(const t_metasprite *tabla, t_metasprite *espejo)
{
    u8 n = 0;                                        /* Piezas copiadas hasta ahora.        */
    while (n < METASPRITE_PIEZAS_MAX && tabla[n].dx != METASPRITE_FIN)
    {
        /* Campo a campo: tcc816 no copia structs enteras con "=". */
        espejo[n].dx = (u16)(METASPRITE_ESPEJO_DX - tabla[n].dx);   /* 0 <-> 64, 32 = 32.  */
        espejo[n].dy = tabla[n].dy;                                 /* Misma fila.         */
        espejo[n].dtile = tabla[n].dtile;                           /* Mismo bloque.       */
        espejo[n].props = (u8)(tabla[n].props | METASPRITE_HFLIP);  /* Volteada.           */
        espejo[n].unused = 0;
        n++;
    }
    espejo[n].dx = METASPRITE_FIN;                   /* El terminador, JUSTO detrás.        */
    espejo[n].dy = 0;
    espejo[n].dtile = 0;
    espejo[n].props = 0;
    espejo[n].unused = 0;
    return n;
}

u8 metasprite_copias(const t_metasprite *tabla, u16 base_rom, u16 base_vram, CopiaBloque *salida)
{
    u8 n = metasprite_piezas(tabla);                 /* Cuántas piezas hay (0..6).          */
    u8 i = 0, copias = 0, largo, t;
    u16 bloque, fila, col, origen, vram, bytes;
    while (i < n)
    {
        fila = (u16)(tabla[i].dy >> 5);              /* Fila de la celda: 0 arriba, 1 abajo.*/
        col = (u16)(tabla[i].dx >> 5);               /* Columna de la celda: 0, 1 o 2.      */
        bloque = tabla[i].dtile;                     /* Bloque en la tira.                  */
        largo = 1;                                   /* Piezas del tramo.                   */
        /* Alargar el tramo mientras la pieza siguiente esté pegada a la derecha en la
           celda, sea el bloque siguiente y siga en la misma fila de 4 de la tira. */
        while ((u8)(i + largo) < n
               && (u16)(tabla[i + largo].dy >> 5) == fila
               && (u16)(tabla[i + largo].dx >> 5) == (u16)(col + largo)
               && tabla[i + largo].dtile == (u16)(bloque + largo)
               && ((bloque & 3) + largo) < 4)
        {
            largo++;
        }
        /* Bloque n: fila de la tira n >> 2 (2048 bytes cada una), columna n & 3 (128). */
        origen = (u16)(base_rom + ((bloque >> 2) << 11) + ((bloque & 3) << 7));
        vram = (u16)(base_vram + (((fila << 6) | (col << 2)) << 4)); /* Tile x 16 palabras. */
        bytes = (u16)((u16)largo << 7);              /* 128 bytes por bloque del tramo.     */
        for (t = 0; t < 4; t++)                      /* Las 4 filas de tiles del bloque:    */
        {                                            /* sumas, no desplazamientos (tcc816   */
            salida[copias].origen = origen;          /* hace cada << con un bucle).         */
            salida[copias].vram = vram;
            salida[copias].bytes = bytes;
            salida[copias].libre = 0;
            copias++;
            origen = (u16)(origen + 512);            /* La fila de tiles siguiente de la tira. */
            vram = (u16)(vram + 256);                /* 16 tiles más abajo en la VRAM.      */
        }
        i = (u8)(i + largo);
    }
    return copias;
}
