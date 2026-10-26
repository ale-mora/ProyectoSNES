/*---------------------------------------------------------------------------------
    sprites.c — La tabla de entidades de la capa de sprites (ver sprites.h).

    PROPÓSITO
      Lo que se hace en C, que es poco y barato: activar las entradas y ordenar
      por profundidad. Las entradas las rellena el juego a través de un puntero
      (la struct mide 16 bytes: &sprites_ent[k] es un desplazamiento). El trabajo
      de cada pieza lo hace sprites_dibujar(), en sprites_oam.asm.

    SE CONECTA CON
      Incluye:  <snes.h> (oamSetXY, oamSetEx), juego/sprites.h,
                juego/profundidad.h.
      Lo usan:  pantallas/prueba_carga.c.

    HARDWARE QUE TOCA
      Solo al iniciar: el búfer de la OAM del SDK (las 128 entradas, escondidas).

    VARIABLES: las de sprites.h (globales: el .asm las lee por su nombre).
    → glosario: OAM, Orden por profundidad
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/sprites.h"
#include "juego/profundidad.h"

SpriteEnt sprites_ent[SPRITES_MAX];
u16 sprites_y[SPRITES_MAX];
u8 sprites_orden[SPRITES_MAX];
u8 sprites_n;
u8 sprites_oam_antes;

void sprites_iniciar(void)
{
    u16 e;

    for (e = 0; e < SPRITES_OAM_MAX * 4; e += 4)
    {
        oamSetEx(e, OBJ_SMALL, OBJ_SHOW);            /* x bit 8 = 0, tamaño pequeño...      */
        oamSetXY(e, 0, 224);                         /* ...y por debajo de la pantalla.     */
    }
    sprites_n = 0;                                   /* (La WRAM no empieza a cero.)        */
    sprites_oam_antes = 0;
}

void sprites_usar(u8 n)
{
    u8 k;

    if (n > SPRITES_MAX)
    {
        n = SPRITES_MAX;
    }
    for (k = 0; k < n; k++)
    {
        sprites_orden[k] = k;                        /* Orden de partida: 0..n-1.           */
    }
    sprites_n = n;
}

void sprites_ordenar(void)
{
    profundidad_ordenar(sprites_orden, sprites_y, sprites_n);
}
