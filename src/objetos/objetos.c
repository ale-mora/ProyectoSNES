/*---------------------------------------------------------------------------------
    objetos.c — Rompibles, ítems, proyectiles, astillas y chispa (ver objetos.h).

    PROPÓSITO
      Sin divisiones ni multiplicaciones; las tablas se recorren con punteros (8 bytes por
      elemento). Cada cosa decide por cajas simples: |dx| y |dy| contra un margen.

    SE CONECTA CON
      Incluye:  objetos/objetos.h, juego/combate.h (combate_caja).
      Lo usan:  ver objetos.h.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo en la struct Objetos del que llama).
    → glosario: Sprite, Caja de colisión / caja de golpe
---------------------------------------------------------------------------------*/
#include "objetos/objetos.h"
#include "juego/combate.h"

#define ROMPIBLE_PROFUNDIDAD 12          /* Tolerancia en y para romperlo.                */
#define RECOGER_X 16                     /* Caja para recoger un ítem.                    */
#define RECOGER_Y 10
#define PROYECTIL_X 14                   /* Caja de un proyectil contra un cuerpo.        */
#define PROYECTIL_Y 10
#define PROYECTIL_ID 253                 /* El «tajo» de los proyectiles del protagonista. */

void objetos_iniciar(Objetos *o, const u16 *xs, const u16 *ys, const u8 *contiene, u8 n)
{
    Rompible *r = o->r;
    Proyectil *p = o->p;
    Astilla *a = o->a;
    u8 k;

    if (n > ROMPIBLES_MAX)
    {
        n = ROMPIBLES_MAX;
    }
    o->n_rompibles = n;
    for (k = 0; k < ROMPIBLES_MAX; k++)
    {
        r->vivo = (u8)(k < n);
        r->x = (u16)(k < n ? xs[k] : 0);
        r->y = (u16)(k < n ? ys[k] : 0);
        r->contiene = (u8)(k < n ? contiene[k] : ITEM_NADA);
        r++;
    }
    for (k = 0; k < PROYECTILES_MAX; k++)
    {
        p->vivo = 0;
        p++;
    }
    for (k = 0; k < ASTILLAS_MAX; k++)
    {
        a->vivo = 0;
        a++;
    }
    o->item.vivo = 0;
    o->item.tipo = ITEM_NADA;
    o->chispa = 0;
    o->item_nuevo = 0;
}

void objetos_chispa(Objetos *o, u16 x, u16 y)
{
    o->chispa_x = x;
    o->chispa_y = y;
    o->chispa = CHISPA_FRAMES;
}

void objetos_lanzar(Objetos *o, u16 x, u16 y, u8 izq, u8 dueno, u8 tipo)
{
    Proyectil *p = o->p;
    u8 k;

    for (k = 0; k < PROYECTILES_MAX; k++)
    {
        if (!p->vivo)
        {
            p->x = x;
            p->y = y;
            p->izq = izq;
            p->dueno = dueno;
            p->tipo = tipo;
            p->vivo = PROYECTIL_FRAMES;              /* vivo cuenta los frames que le quedan. */
            return;
        }
        p++;
    }
}

/*--- romper: el rompible salta en astillas y suelta lo que lleva -------------------*/
static void romper(Objetos *o, Rompible *r)
{
    Astilla *a = o->a;

    r->vivo = 0;
    a->x = r->x; a->y = (s16)(r->y - 24); a->vx = -2; a->vy = -4; a->vivo = 1; a->reloj = 20;
    a++;
    a->x = r->x; a->y = (s16)(r->y - 24); a->vx = 2; a->vy = -5; a->vivo = 1; a->reloj = 20;
    if (r->contiene != ITEM_NADA)
    {
        o->item.x = r->x;
        o->item.y = r->y;
        o->item.tipo = r->contiene;
        o->item.vivo = 1;
        o->item.reloj = ITEM_VIDA_FRAMES;
        o->item_nuevo = 1;
    }
    objetos_chispa(o, r->x, (u16)(r->y - 16));
}

/*--- recoger: el protagonista pisa el ítem -------------------------------------------*/
static void recoger(Objetos *o, Cuerpo *c)
{
    Item *i = &o->item;
    u16 dx, dy;

    if (!i->vivo || c->z != 0 || c->vida == 0)
    {
        return;
    }
    dx = (u16)(i->x > c->x ? i->x - c->x : c->x - i->x);
    dy = (u16)(i->y > c->y ? i->y - c->y : c->y - i->y);
    if (dx > RECOGER_X || dy > RECOGER_Y)
    {
        return;
    }
    switch (i->tipo)
    {
        case ITEM_ONIGIRI:
            c->vida = (u8)(c->vida + CURA_ONIGIRI > VIDA_TAROU ? VIDA_TAROU : c->vida + CURA_ONIGIRI);
            break;
        case ITEM_PESCADO:
            c->vida = (u8)(c->vida + CURA_PESCADO > VIDA_TAROU ? VIDA_TAROU : c->vida + CURA_PESCADO);
            break;
        case ITEM_VIDA:
            c->vida = VIDA_TAROU;                    /* Vida extra: la barra llena.         */
            break;
        case ITEM_SHURIKEN:
            c->arma = ARMA_SHURIKEN;
            break;
        default:
            c->arma = ARMA_KUNAI;
            break;
    }
    i->vivo = 0;
}

/*--- proyectil: avanza y hiere a quien toque ----------------------------------------*/
static void mover_proyectil(Proyectil *p, Cuerpo *c, Ninja *n, u16 camara)
{
    Ninja *q;
    u16 dx, dy;
    u8 k, danio;

    p->x = (u16)(p->izq ? p->x - PROYECTIL_PASO : p->x + PROYECTIL_PASO);
    p->vivo--;
    if ((u16)(p->x - camara + 32) > 320)             /* Fuera de la pantalla (con margen).  */
    {
        p->vivo = 0;
        return;
    }
    if (p->dueno == 0)                               /* Shuriken enemigo: al protagonista.  */
    {
        dx = (u16)(p->x > c->x ? p->x - c->x : c->x - p->x);
        dy = (u16)(p->y > c->y ? p->y - c->y : c->y - p->y);
        if (dx < PROYECTIL_X && dy < PROYECTIL_Y && c->z < (24 << 4))
        {
            if (mundo_recibir(c, DANIO_SHURIKEN, p->izq))
            {
                p->vivo = 0;
            }
        }
        return;
    }
    danio = (u8)(p->tipo == ITEM_KUNAI ? DANIO_KUNAI : DANIO_SHURIKEN);
    q = n;
    for (k = 0; k < NINJAS_MAX; k++)                 /* El del protagonista: a los enemigos. */
    {
        if (q->vivo && q->estado != NINJA_DERRIBADO)
        {
            dx = (u16)(p->x > q->x ? p->x - q->x : q->x - p->x);
            dy = (u16)(p->y > q->y ? p->y - q->y : q->y - p->y);
            if (dx < PROYECTIL_X && dy < PROYECTIL_Y)
            {
                if (!ninja_bloquea(q, p->izq, 0, PROYECTIL_ID))
                {
                    q->id_golpe = 0;                 /* Cada proyectil es un golpe nuevo.   */
                    ninja_recibir(q, danio, 0, p->izq, PROYECTIL_ID);
                }
                p->vivo = 0;
                return;
            }
        }
        q++;
    }
}

void objetos_actualizar(Objetos *o, Cuerpo *c, Ninja *n, u16 alcance, u16 camara)
{
    Rompible *r = o->r;
    Proyectil *p = o->p;
    Astilla *a = o->a;
    u8 k;

    o->item_nuevo = 0;
    /*--- El golpe del protagonista rompe lo que toca ---*/
    if (mundo_golpe_activo(c))
    {
        for (k = 0; k < o->n_rompibles; k++)
        {
            if (r->vivo && combate_caja(c->x, c->y, c->mira_izq, alcance, ROMPIBLE_PROFUNDIDAD, r->x, r->y))
            {
                romper(o, r);
            }
            r++;
        }
    }
    recoger(o, c);
    if (o->item.vivo)
    {
        o->item.reloj--;
        if (o->item.reloj == 0)
        {
            o->item.vivo = 0;                        /* Nadie lo cogió: se va.              */
        }
    }
    for (k = 0; k < PROYECTILES_MAX; k++)
    {
        if (p->vivo)
        {
            mover_proyectil(p, c, n, camara);
        }
        p++;
    }
    for (k = 0; k < ASTILLAS_MAX; k++)
    {
        if (a->vivo)
        {
            a->x = (u16)(a->x + a->vx);
            a->y = (s16)(a->y + a->vy);
            a->vy++;                                 /* Gravedad.                           */
            a->reloj--;
            if (a->reloj == 0)
            {
                a->vivo = 0;
            }
        }
        a++;
    }
    if (o->chispa)
    {
        o->chispa--;
    }
}
