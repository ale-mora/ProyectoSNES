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

    → glosario: Colisión, Metatile
---------------------------------------------------------------------------------*/
#include "juego/colision.h"

#define ANCHO_PX (COLUMNAS_COLISION * CELDA_COLISION)   /* 2048: más allá, pared.     */
#define ALTO_PX (FILAS_COLISION * CELDA_COLISION)       /* 224: más abajo, vacío.     */

static const u8 *rejilla;      /* La rejilla elegida con colision_usar (0 = ninguna).      */

void colision_usar(const u8 *r)
{
    rejilla = r;
}

u8 colision_en(u16 x, u16 y)
{
    if (x >= ANCHO_PX)                               /* Los dos lados del nivel...          */
    {
        return COL_SOLIDO;                           /* ...son paredes.                     */
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
