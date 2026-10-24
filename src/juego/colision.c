/*---------------------------------------------------------------------------------
    colision.c — La rejilla de colisión del nivel (ver colision.h).

    PROPÓSITO
      Traducir un píxel del mundo a su celda y devolver lo que guarda. Las
      celdas miden 16 px y la rejilla tiene 128 columnas, así que la cuenta
      del índice es solo de desplazamientos (sin multiplicar ni dividir, que
      en el 65816 cuestan una rutina lenta):
          columna = x >> 4          (x / 16)
          fila    = y >> 4          (y / 16)
          índice  = (fila << 7) | columna   (fila * 128 + columna)

    SE CONECTA CON
      Incluye:  juego/colision.h.
      Lo usan:  juego/tarou.c, pantallas/pantalla_nivel1.c, tests/.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (estáticas de este archivo)
      rejilla  const u8*  la rejilla en uso (ROM en la SNES, un array en el PC)
      bloque_x, bloque_y, bloque_ancho, bloque_alto  u16  el bloque (ancho 0 = no hay)

    → glosario: Colisión, Metatile
---------------------------------------------------------------------------------*/
#include "juego/colision.h"

#define ANCHO_PX (COLUMNAS_COLISION * CELDA_COLISION)   /* 2048: más allá, pared.     */
#define ALTO_PX (FILAS_COLISION * CELDA_COLISION)       /* 224: más abajo, vacío.     */

static const u8 *rejilla;      /* La rejilla elegida con colision_usar (0 = ninguna).      */
static u16 bloque_x, bloque_y; /* Esquina de arriba a la izquierda del bloque.             */
static u16 bloque_ancho, bloque_alto; /* Su tamaño; ancho 0 = no hay bloque.               */

void colision_usar(const u8 *r)
{
    rejilla = r;
    bloque_ancho = 0;                                /* Nivel nuevo: sin bloque.            */
}

void colision_bloque(u16 x, u16 y, u16 ancho, u16 alto)
{
    bloque_x = x;
    bloque_y = y;
    bloque_ancho = ancho;
    bloque_alto = alto;
}

u8 colision_en(u16 x, u16 y)
{
    if (x >= ANCHO_PX)                               /* Los dos lados del nivel...          */
    {
        return COL_SOLIDO;                           /* ...son paredes.                     */
    }
    /* ¿Dentro del bloque? Con u16, (x - bloque_x) es enorme si x < bloque_x: una sola
       comparación mira los dos lados a la vez. */
    if (bloque_ancho != 0 && (u16)(x - bloque_x) < bloque_ancho && (u16)(y - bloque_y) < bloque_alto)
    {
        return COL_SOLIDO;
    }
    if (y >= ALTO_PX || rejilla == 0)                /* Debajo (fosos), encima, o sin nivel. */
    {
        return COL_VACIO;
    }
    return rejilla[((y >> 4) << 7) | (x >> 4)];      /* fila * 128 + columna.               */
}

u8 colision_solido(u16 x, u16 y)
{
    return (u8)(colision_en(x, y) == COL_SOLIDO);
}

u8 colision_apoya(u16 x, u16 y)
{
    u8 c = colision_en(x, y);
    return (u8)(c == COL_SOLIDO || c == COL_PLATAFORMA);
}

u8 colision_columna_solida(u16 x, s16 y0, s16 y1)
{
    u16 i, fin;
    if (x >= ANCHO_PX)
    {
        return 1;                                    /* Fuera del nivel: pared.             */
    }
    if (bloque_ancho != 0 && (u16)(x - bloque_x) < bloque_ancho
        && y1 >= (s16)bloque_y && y0 < (s16)(bloque_y + bloque_alto))
    {
        return 1;                                    /* El tramo pisa el bloque.            */
    }
    if (y0 < 0)
    {
        y0 = 0;                                      /* Por encima de la pantalla: nada.    */
    }
    if (y1 >= ALTO_PX)
    {
        y1 = ALTO_PX - 1;                            /* Por debajo: nada.                   */
    }
    if (y0 > y1 || rejilla == 0)
    {
        return 0;
    }
    /* Índice de la primera celda: (fila << 7) | columna, y bajar una fila es sumar 128.
       (y & 0xFFF0) << 3 es lo mismo que (y >> 4) << 7, con menos desplazamientos. */
    i = (u16)((((u16)y0 & 0xFFF0) << 3) | (x >> 4));
    fin = (u16)((((u16)y1 & 0xFFF0) << 3) | (x >> 4));
    for (; i <= fin; i += COLUMNAS_COLISION)
    {
        if (rejilla[i] == COL_SOLIDO)
        {
            return 1;
        }
    }
    return 0;
}

u8 colision_fila_es(s16 x0, s16 x1, s16 y, u8 tipo)
{
    u16 i, fin;
    if (y < 0 || y >= ALTO_PX || rejilla == 0)
    {
        return 0;                                    /* Fuera de la pantalla: aire.         */
    }
    if (x0 < 0)
    {
        x0 = 0;
    }
    if (x1 >= ANCHO_PX)
    {
        x1 = ANCHO_PX - 1;
    }
    i = (u16)((((u16)y & 0xFFF0) << 3) | ((u16)x0 >> 4));
    fin = (u16)((((u16)y & 0xFFF0) << 3) | ((u16)x1 >> 4));
    for (; i <= fin; i++)                            /* Celdas seguidas de la misma fila.   */
    {
        if (rejilla[i] == tipo)
        {
            return 1;
        }
    }
    return 0;
}

u8 colision_apoyan(u16 xa, u16 xb, s16 y)
{
    u16 base;
    u8 c;
    if (y < 0 || y >= ALTO_PX || rejilla == 0)
    {
        return 0;                                    /* Fuera de la pantalla: nada sostiene. */
    }
    base = (u16)(((u16)y & 0xFFF0) << 3);            /* Primera celda de la fila.           */
    if (xa < ANCHO_PX)
    {
        c = rejilla[base | (xa >> 4)];
        if (c == COL_SOLIDO || c == COL_PLATAFORMA)
        {
            return 1;
        }
    }
    if (xb < ANCHO_PX)
    {
        c = rejilla[base | (xb >> 4)];
        if (c == COL_SOLIDO || c == COL_PLATAFORMA)
        {
            return 1;
        }
    }
    return 0;
}
