/*---------------------------------------------------------------------------------
    metasprite.c — Recorrer y espejar tablas metasprite de gfx4snes -T (lógica pura).

    PROPÓSITO
      Ver metasprite.h. Dos bucles que paran en el terminador (dx = 0xFF80) o,
      como red de seguridad, en la cuarta pieza. Sin <snes.h>: se compila con
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
        espejo[n].dx = (u16)(METASPRITE_ESPEJO_DX - tabla[n].dx);   /* 0 <-> 32.           */
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
