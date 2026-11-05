/*---------------------------------------------------------------------------------
    objetos_sprite.c — Los objetos de la fase en la tabla de sprites (ver .h).

    PROPÓSITO
      Cada entidad es UNA pieza de 16x16 (tabla obj_pieza1) salvo los rompibles, que son
      cuatro (obj_pieza4: (0,0) (16,0) (0,16) (16,16), en los tiles 448, 450, 452 y 454).
      El shuriken gira cambiando sus bits de volteo cada 4 frames.

    SE CONECTA CON
      Incluye:  <snes.h>, objetos/objetos_sprite.h, juego/sprites.h, juego/dma_lista.h,
                sistema/comun.h, objetos_datos.h.
      Lo usa:   pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      Ver objetos_sprite.h.

    VARIABLES (estáticas)
      reloj   u8  frames (giro del shuriken, chispa)
    → glosario: Sprite, OAM
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "objetos/objetos_sprite.h"
#include "juego/sprites.h"
#include "juego/dma_lista.h"
#include "sistema/comun.h"
#include "objetos_datos.h"

extern char objetos_til, obj_pieza1, obj_pieza4;   /* data.asm (res/objetos/).          */
extern u8 obj_banco;                                /* Banco de objetos_til y de las tablas. */

#define TILE_ROMPIBLE 448
#define TILE_ITEM 456
#define TILE_ASTILLAS 458
#define TILE_SHURIKEN 460
#define TILE_KUNAI 462
#define TILE_CHISPA 492
#define PALETA 7
#define PRIORIDAD 2
#define BYTES_PIEZA 128
#define VRAM_TILE(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))

#define ENT_ROMPIBLE OBJETOS_ENT     /* 15, 16                                            */
#define ENT_ITEM (OBJETOS_ENT + 2)   /* 17                                                */
#define ENT_PROYECTIL (OBJETOS_ENT + 3)   /* 18, 19                                       */
#define ENT_ASTILLA (OBJETOS_ENT + 5)     /* 20, 21                                       */
#define ENT_CHISPA (OBJETOS_ENT + 7)      /* 22                                           */

static u8 reloj;

/*--- pieza_a_vram: la pieza `p` del archivo al tile `t` (las dos mitades) -------------*/
static void pieza_a_vram(u8 p, u16 t)
{
    u8 *origen = (u8 *)&objetos_til + ((u16)p << 7);   /* 128 bytes por pieza.        */

    dmaCopyVram(origen, VRAM_TILE(t), 64);               /* TL, TR.                       */
    dmaCopyVram(origen + 64, VRAM_TILE(t + 16), 64);     /* BL, BR (una fila más abajo).  */
}

void objetos_sprite_cargar(u8 rompible)
{
    SpriteEnt *s;
    u8 k, base;

    base = (u8)(OBJ_PIEZA_BARRIL + (rompible << 2)); /* 4 piezas por rompible.             */
    for (k = 0; k < 4; k++)
    {
        pieza_a_vram((u8)(base + k), (u16)(TILE_ROMPIBLE + (k << 1)));
    }
    pieza_a_vram(OBJ_PIEZA_ONIGIRI, TILE_ITEM);
    pieza_a_vram(OBJ_PIEZA_ASTILLAS, TILE_ASTILLAS);
    pieza_a_vram(OBJ_PIEZA_SHURIKEN, TILE_SHURIKEN);
    pieza_a_vram(OBJ_PIEZA_KUNAI, TILE_KUNAI);
    pieza_a_vram(OBJ_PIEZA_CHISPA_A, TILE_CHISPA);
    pieza_a_vram(OBJ_PIEZA_CHISPA_B, TILE_CHISPA + 2);
    sprites_usar(OBJETOS_ENTIDADES);
    s = sprites_ent + OBJETOS_ENT;
    for (k = 0; k < 8; k++)
    {
        s->piezas = (u16)(k < 2 ? (u16)&obj_pieza4 : (u16)&obj_pieza1);
        s->banco = obj_banco;
        s->n = 0;
        s->tile = TILE_ROMPIBLE;
        s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETA));
        s->borde = 1;
        sprites_y[OBJETOS_ENT + k] = 0;
        s++;
    }
    reloj = 0;
}

/*--- una: coloca una entidad de una pieza (o la apaga si no se ve) ------------------*/
static void una(SpriteEnt *s, u16 x, s16 y, u16 tile, u8 volteo, u16 pies, u8 ent, u16 camara)
{
    u16 sx = (u16)(x - camara - 8);

    if ((u16)(sx + 16) > 272)                        /* Fuera de la pantalla.               */
    {
        s->n = 0;
        return;
    }
    s->x0 = sx;
    s->y0 = (u16)y;
    s->tile = tile;
    s->n = 1;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETA) | volteo);
    s->borde = sprites_borde(s->x0, s->y0, 16, 16);
    sprites_y[ent] = pies;
}

void objetos_sprite_actualizar(const Objetos *o, u16 camara)
{
    SpriteEnt *s = sprites_ent + OBJETOS_ENT;
    const Rompible *r = o->r;
    const Proyectil *p = o->p;
    const Astilla *a = o->a;
    u8 k, puestos = 0, gira;
    u16 sx, tile;

    reloj++;
    /*--- Los dos primeros rompibles a la vista ---*/
    for (k = 0; k < o->n_rompibles && puestos < 2; k++)
    {
        sx = (u16)(r->x - camara - 16);
        if (r->vivo && (u16)(sx + 32) < 288)
        {
            s->x0 = sx;
            s->y0 = (u16)(r->y - 32);
            s->n = 4;
            s->tile = TILE_ROMPIBLE;
            s->borde = sprites_borde(s->x0, s->y0, 32, 32);
            sprites_y[OBJETOS_ENT + puestos] = r->y;
            s++;
            puestos++;
        }
        r++;
    }
    for (; puestos < 2; puestos++)
    {
        s->n = 0;
        s++;
    }
    /*--- El ítem (si es comida nueva, su dibujo a la VRAM por la lista) ---*/
    if (o->item.vivo)
    {
        if (o->item_nuevo && o->item.tipo <= ITEM_VIDA)
        {
            tile = (u16)(OBJ_PIEZA_ONIGIRI + o->item.tipo - ITEM_ONIGIRI);
            dma_lista_anadir((u16)((u16)&objetos_til + (tile << 7)), VRAM_TILE(TILE_ITEM), 64,
                             obj_banco, DMA_VMAIN_TILES);
            dma_lista_anadir((u16)((u16)&objetos_til + (tile << 7) + 64), VRAM_TILE(TILE_ITEM + 16),
                             64, obj_banco, DMA_VMAIN_TILES);
        }
        tile = (u16)(o->item.tipo == ITEM_SHURIKEN ? TILE_SHURIKEN :
                     (o->item.tipo == ITEM_KUNAI ? TILE_KUNAI : TILE_ITEM));
        if (o->item.reloj < 60 && (reloj & 4))
        {
            s->n = 0;                                /* A punto de irse: parpadea.          */
        }
        else
        {
            una(s, o->item.x, (s16)(o->item.y - 16), tile, 0, o->item.y, ENT_ITEM, camara);
        }
    }
    else
    {
        s->n = 0;
    }
    s++;
    /*--- Proyectiles (el shuriken gira con los volteos) ---*/
    gira = (u8)(((reloj >> 2) & 1) ? SPRITE_HFLIP : 0) | (u8)(((reloj >> 3) & 1) ? 0x80 : 0);
    for (k = 0; k < PROYECTILES_MAX; k++)
    {
        if (p->vivo)
        {
            una(s, p->x, (s16)(p->y - 40), p->tipo == ITEM_KUNAI ? TILE_KUNAI : TILE_SHURIKEN,
                (u8)(p->tipo == ITEM_KUNAI ? (p->izq ? SPRITE_HFLIP : 0) : gira), p->y,
                (u8)(ENT_PROYECTIL + k), camara);
        }
        else
        {
            s->n = 0;
        }
        s++;
        p++;
    }
    /*--- Astillas ---*/
    for (k = 0; k < ASTILLAS_MAX; k++)
    {
        if (a->vivo)
        {
            una(s, a->x, a->y, TILE_ASTILLAS, (u8)(k ? SPRITE_HFLIP : 0), 0, (u8)(ENT_ASTILLA + k), camara);
        }
        else
        {
            s->n = 0;
        }
        s++;
        a++;
    }
    /*--- La chispa del golpe ---*/
    if (o->chispa)
    {
        una(s, o->chispa_x, (s16)(o->chispa_y - 48), (u16)(TILE_CHISPA + (o->chispa < 4 ? 2 : 0)), 0,
            (u16)(o->chispa_y + 1), ENT_CHISPA, camara);
    }
    else
    {
        s->n = 0;
    }
}
