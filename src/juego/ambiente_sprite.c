/*---------------------------------------------------------------------------------
    ambiente_sprite.c — El humo y los pájaros en la tabla de sprites (ver .h).

    PROPÓSITO
      Cada figura es UNA pieza de 16x16 (tabla de piezas: un solo (0, 0)); su cuadro
      se elige cambiando el tile: la pieza k de la tira está en TILE_AMBIENTE + 2k
      (y su mitad de abajo, 16 tiles después, como en toda la capa de sprites).

    SE CONECTA CON
      Incluye:  <snes.h>, juego/ambiente_sprite.h, juego/sprites.h, sistema/comun.h.
      Lo usa:   pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      Ver ambiente_sprite.h.

    VARIABLES (constantes)
      TILE_AMBIENTE  484  la bocanada (486 penacho, 488 y 490 los pájaros)
      PALETA         7    paleta de sprites (CGRAM 128 + 7·16)
    → glosario: Sprite, OAM
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/ambiente_sprite.h"
#include "juego/sprites.h"
#include "sistema/comun.h"

extern char ambiente_til, ambiente_pal, ambiente_pieza;   /* data.asm (res/aldea/).      */
extern u8 ambiente_banco;                                   /* El banco de ambiente_pieza. */

#define TILE_AMBIENTE 484            /* Par 15, tras las 2 piezas de la sombra (480, 482). */
#define TILE_PAJARO (TILE_AMBIENTE + 4)
#define PALETA 7
#define PRIORIDAD 2                  /* Como los personajes: el primer plano los tapa.    */
#define VRAM_TILE(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))   /* 16 palabras por tile.  */

void ambiente_sprite_cargar(void)
{
    SpriteEnt *s;
    u8 k;

    dmaCopyCGram((u8 *)&ambiente_pal, 128 + (PALETA << 4), 32);
    dmaCopyVram((u8 *)&ambiente_til, VRAM_TILE(TILE_AMBIENTE), 256);              /* Arriba. */
    dmaCopyVram((u8 *)&ambiente_til + 256, VRAM_TILE(TILE_AMBIENTE + 16), 256);   /* Abajo.  */
    sprites_usar(AMBIENTE_ENTIDADES);
    s = sprites_ent + AMBIENTE_ENT;
    for (k = 0; k < AMBIENTE_FIGURAS; k++)
    {
        s->piezas = (u16)&ambiente_pieza;
        s->banco = ambiente_banco;
        s->n = 0;                                    /* Apagada hasta el primer frame.      */
        s->tile = (k < AMBIENTE_HUMO) ? TILE_AMBIENTE : TILE_PAJARO;
        s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETA));
        s->borde = 1;
        sprites_y[AMBIENTE_ENT + k] = 0;             /* y = 0: detrás de todo.              */
        s++;
    }
}

void ambiente_sprite_actualizar(const Ambiente *a)
{
    SpriteEnt *s = sprites_ent + AMBIENTE_ENT;
    const Figura *f = a->fig;
    u8 k;

    for (k = 0; k < AMBIENTE_FIGURAS; k++)
    {
        if (f->visible)
        {
            s->x0 = (u16)f->x;
            s->y0 = (u16)f->y;
            s->n = 1;
            s->tile = (u16)(((k < AMBIENTE_HUMO) ? TILE_AMBIENTE : TILE_PAJARO) + (f->cuadro << 1));
            s->borde = sprites_borde(s->x0, s->y0, 16, 16);
        }
        else
        {
            s->n = 0;
        }
        s++;
        f++;
    }
}
