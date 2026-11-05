/*---------------------------------------------------------------------------------
    objetos_sprite.c — Los objetos de la fase en la tabla de sprites (ver .h).

    PROPÓSITO
      Cada entidad es UNA pieza de 16x16 (tabla obj_pieza1) salvo los rompibles, que son
      cuatro (obj_pieza4: (0,0) (16,0) (0,16) (16,16), en los tiles 448, 450, 452 y 454).
      El shuriken gira cambiando sus bits de volteo cada 4 frames.
      OPACOS: la paleta 7 entra en la matemática de color de la escena (la media suma que
      hace translúcidos la sombra, el humo y los pájaros) y los barriles se veían como
      fantasmas; las paletas opacas (0..3) están llenas. Así que se apaga la matemática
      SOLO donde están: las ventanas 1 y 2 de la PPU son rectángulos que el canal 5 de HDMA
      (modo 4: WH0..WH3 en cada tramo de líneas) pone, en cada tramo de líneas, alrededor
      de lo DIBUJADO de cada objeto a la vista (su caja, OBJ_CAJA_* de preparar_objetos.py:
      rompibles, ítem, proyectiles, astillas y chispa; si en unas líneas hay más de dos, se
      unen los más cercanos), y CGWSEL dice «matemática solo FUERA de las ventanas»: la
      sombra solo pierde la transparencia si pasa justo por lo dibujado de un objeto. La tabla se escribe en la que el HDMA no lee
      y objetos_sprite_vblank cambia el puntero (doble búfer).

    SE CONECTA CON
      Incluye:  <snes.h>, objetos/objetos_sprite.h, juego/sprites.h, juego/dma_lista.h,
                sistema/comun.h, objetos_datos.h.
      Lo usa:   pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      Ver objetos_sprite.h.

    VARIABLES (estáticas)
      reloj     u8          frames (giro del shuriken, chispa)
      tablas    u8[2][96]   las dos tablas HDMA de las ventanas (5 bytes por tramo y el 0)
      activa    u8          la que lee el HDMA; lista: 1 = la otra está nueva
      rs        Ventana[8]  los rectángulos de este frame; rects: cuántos
      caja_rompible u8[4]   lo dibujado del rompible de este escenario
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
#define BANCO_WRAM 0x7E              /* Banco de las variables de C.                       */
#define LINEAS 224                   /* Líneas de la pantalla.                             */
#define VACIA_IZQ 255                /* Ventana vacía: izquierda > derecha.                */
#define VACIA_DER 0
#define VENTANAS_MAX 8               /* Objetos que pueden pedir ventana a la vez.          */

typedef struct
{
    u8 x0, x1, y0, y1;               /* Un rectángulo de pantalla (inclusivo).             */
} Ventana;

#define ENT_ROMPIBLE OBJETOS_ENT     /* 15, 16                                            */
#define ENT_ITEM (OBJETOS_ENT + 2)   /* 17                                                */
#define ENT_PROYECTIL (OBJETOS_ENT + 3)   /* 18, 19                                       */
#define ENT_ASTILLA (OBJETOS_ENT + 5)     /* 20, 21                                       */
#define ENT_CHISPA (OBJETOS_ENT + 7)      /* 22                                           */

static u8 reloj;
static u8 tablas[2][96];             /* Hasta 18 tramos de 5 bytes y el 0 final.       */
static u8 activa, lista, rects;
static Ventana rs[VENTANAS_MAX];     /* Los rectángulos de este frame.                    */
static u8 caja_rompible[4];          /* La caja del rompible de este escenario.           */

/* Lo dibujado de cada objeto dentro de su sprite (x0, y0, x1, y1; preparar_objetos.py). */
static const u8 CAJA_ROMPIBLE[3][4] = {{OBJ_CAJA_BARRIL}, {OBJ_CAJA_CAJA}, {OBJ_CAJA_JARRA}};
static const u8 CAJA_ITEM[8][4] = {                  /* Por TipoItem (0 = nada).               */
    {0, 0, 0, 0}, {OBJ_CAJA_ONIGIRI}, {OBJ_CAJA_PESCADO}, {OBJ_CAJA_VIDA},
    {OBJ_CAJA_SHURIKEN}, {OBJ_CAJA_KUNAI}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const u8 CAJA_ASTILLAS[4] = {OBJ_CAJA_ASTILLAS};
static const u8 CAJA_CHISPA[2][4] = {{OBJ_CAJA_CHISPA_B}, {OBJ_CAJA_CHISPA_A}};

/*--- rect: apunta el rectángulo de pantalla de lo dibujado de un objeto ---------------
    (x0, y0): la esquina del sprite (x en u16 «con signo»: puede asomar por la izquierda);
    caja: lo dibujado dentro. Las líneas van una antes de lo dibujado a la última (el sprite
    se ve una línea más abajo de su y). Se recorta a la pantalla. */
static void rect(u16 x0, u16 y0, const u8 *caja)
{
    s16 izq = (s16)((s16)x0 + caja[0]), der = (s16)((s16)x0 + caja[2]);
    s16 arr = (s16)((s16)y0 + caja[1]), aba = (s16)((s16)y0 + caja[3] + 1);
    Ventana *v;

    if (rects >= VENTANAS_MAX || der < 0 || izq > 255 || aba < 0 || arr >= LINEAS)
    {
        return;
    }
    v = rs + rects;                                  /* 4 bytes: rs[k] es un desplazamiento. */
    v->x0 = (u8)(izq < 0 ? 0 : izq);
    v->x1 = (u8)(der > 255 ? 255 : der);
    v->y0 = (u8)(arr < 0 ? 0 : arr);
    v->y1 = (u8)(aba >= LINEAS ? LINEAS - 1 : aba);
    rects++;
}

/*--- tramo: `lineas` líneas desde `y` con las dos ventanas (izq, der) dadas -----------*/
static u8 *tramo(u8 *t, u8 lineas, u8 i1, u8 d1, u8 i2, u8 d2)
{
    u8 n;

    while (lineas)
    {
        n = (u8)(lineas > 127 ? 127 : lineas);       /* Un tramo directo: hasta 127 líneas. */
        *t++ = n;
        *t++ = i1;                                   /* WH0, WH1: ventana 1.                */
        *t++ = d1;
        *t++ = i2;                                   /* WH2, WH3: ventana 2.                */
        *t++ = d2;
        lineas = (u8)(lineas - n);
    }
    return t;
}

/*--- ventanas: la tabla HDMA de este frame en la que no se está leyendo ---------------
    Los cortes son los bordes de arriba y de abajo de los rectángulos; entre dos cortes
    seguidos los mismos objetos ocupan las líneas. En cada tramo se juntan sus rangos de x
    (ordenados; los que se tocan, en uno) y, si quedan más de dos (solo hay dos ventanas),
    se unen los dos más cercanos hasta que queden dos. */
static void ventanas(void)
{
    u8 cortes[2 * VENTANAS_MAX + 1], n = 0, k, j, v, m, c;
    u8 si[VENTANAS_MAX], sd[VENTANAS_MAX];
    u8 *t = tablas[activa ^ 1];
    Ventana *r;

    cortes[n++] = 0;
    r = rs;
    for (k = 0; k < rects; k++)
    {
        cortes[n++] = r->y0;
        cortes[n++] = (u8)(r->y1 + 1);
        r++;
    }
    for (k = 1; k < n; k++)                          /* Ordenar (inserción: son pocos).     */
    {
        v = cortes[k];
        j = k;
        while (j > 0 && cortes[j - 1] > v)
        {
            cortes[j] = cortes[j - 1];
            j--;
        }
        cortes[j] = v;
    }
    for (k = 0; k < n; k++)
    {
        c = cortes[k];
        v = (u8)(k + 1 < n ? cortes[k + 1] : LINEAS);   /* Hasta el corte siguiente.      */
        if (v > LINEAS)
        {
            v = LINEAS;
        }
        if (v <= c)
        {
            continue;                                /* Corte repetido: tramo vacío.        */
        }
        m = 0;                                       /* Los rangos de x en estas líneas,    */
        r = rs;                                      /* ordenados por el borde izquierdo.   */
        for (j = 0; j < rects; j++)
        {
            if (c >= r->y0 && c <= r->y1)
            {
                si[m] = r->x0;
                sd[m] = r->x1;
                m++;
            }
            r++;
        }
        for (j = 1; j < m; j++)
        {
            u8 a = si[j], b = sd[j], h = j;
            while (h > 0 && si[h - 1] > a)
            {
                si[h] = si[h - 1];
                sd[h] = sd[h - 1];
                h--;
            }
            si[h] = a;
            sd[h] = b;
        }
        while (m > 2)                                /* Más de dos: unir los más cercanos.  */
        {
            u8 mejor = 0, hueco = 255, h;
            for (j = 0; j + 1 < m; j++)
            {
                h = (u8)(si[j + 1] > sd[j] ? si[j + 1] - sd[j] : 0);
                if (h < hueco)
                {
                    hueco = h;
                    mejor = j;
                }
            }
            if (sd[mejor + 1] > sd[mejor])
            {
                sd[mejor] = sd[mejor + 1];
            }
            for (j = (u8)(mejor + 1); j + 1 < m; j++)
            {
                si[j] = si[j + 1];
                sd[j] = sd[j + 1];
            }
            m--;
        }
        if (m == 0)
        {
            t = tramo(t, (u8)(v - c), VACIA_IZQ, VACIA_DER, VACIA_IZQ, VACIA_DER);
        }
        else if (m == 1)
        {
            t = tramo(t, (u8)(v - c), si[0], sd[0], VACIA_IZQ, VACIA_DER);
        }
        else
        {
            t = tramo(t, (u8)(v - c), si[0], sd[0], si[1], sd[1]);
        }
    }
    *t = 0;                                          /* Fin de la tabla.                    */
    lista = 1;
}

void objetos_sprite_vblank(void)
{
    if (lista)                                       /* La tabla nueva, al canal 5: la lee  */
    {                                                /* desde la línea 0 de este frame.     */
        activa ^= 1;
        REG_A1T5LH = (u16)tablas[activa];
        lista = 0;
    }
}

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
    /* Las ventanas de color (ver el PROPÓSITO): canal 5, directo, modo 4 = 4 registros
       seguidos desde WH0 ($2126..$2129); ventanas 1 y 2 como «ventana de color» (OR) y la
       matemática solo fuera de ellas, con la pantalla secundaria de operando. */
    for (k = 0; k < 4; k++)                          /* La caja de su rompible.             */
    {
        caja_rompible[k] = CAJA_ROMPIBLE[rompible][k];
    }
    rects = 0;
    activa = 0;
    lista = 0;
    ventanas();                                      /* Una tabla vacía en la 1...          */
    activa = 1;                                      /* ...que es la que se lee.            */
    lista = 0;
    REG_DMAP5 = 0x04;
    REG_BBAD5 = 0x26;
    REG_A1T5LH = (u16)tablas[activa];
    REG_A1B5 = BANCO_WRAM;
    REG_WOBJSEL = 0xA0;                              /* Color: ventana 1 y ventana 2.        */
    REG_WOBJLOG = 0x00;                              /* Unidas con OR.                      */
    REG_CGWSEL = OBJETOS_CGWSEL;
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

static void colocar(const Objetos *o, u16 camara);

void objetos_sprite_actualizar(const Objetos *o, u16 camara)
{
    colocar(o, camara);
    ventanas();                                      /* La tabla HDMA de este frame.        */
}

/*--- colocar: las entidades de este frame y el rectángulo de cada objeto a la vista ---*/
static void colocar(const Objetos *o, u16 camara)
{
    SpriteEnt *s = sprites_ent + OBJETOS_ENT;
    const Rompible *r = o->r;
    const Proyectil *p = o->p;
    const Astilla *a = o->a;
    u8 k, puestos = 0, gira;
    u16 sx, tile;

    reloj++;
    rects = 0;
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
            rect(s->x0, s->y0, caja_rompible);       /* Sin matemática de color: opaco.     */
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
            if (s->n)
            {
                rect(s->x0, s->y0, CAJA_ITEM[o->item.tipo & 7]);
            }
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
            if (s->n)
            {
                rect(s->x0, s->y0, CAJA_ITEM[p->tipo & 7]);
            }
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
            if (s->n)
            {
                rect(s->x0, s->y0, CAJA_ASTILLAS);
            }
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
        if (s->n)
        {
            rect(s->x0, s->y0, CAJA_CHISPA[o->chispa < 4 ? 0 : 1]);
        }
    }
    else
    {
        s->n = 0;
    }
}
