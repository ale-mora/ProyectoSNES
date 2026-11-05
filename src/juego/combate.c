/*---------------------------------------------------------------------------------
    combate.c — Resolución de los golpes de Tarou contra los ninjas (ver combate.h).

    PROPÓSITO
      combate_resolver, cada frame: si el cuerpo no está en un cuadro activo de golpe, no
      pasa nada. Si lo está, recorre las ranuras de ninjas y, a cada uno vivo dentro de la caja
      de golpe (combate_en_caja) le llama a ninja_recibir con el daño del golpe en curso, si
      derriba y hacia qué lado lo empuja (hacia donde mira Tarou). ninja_recibir ignora los
      ninjas que ya recibieron este tajo, así que los cuatro cuadros activos pegan una sola vez.

    SE CONECTA CON
      Incluye:  juego/combate.h.
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_combate.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Caja de golpe, Combo, Derribo, Parada de impacto
---------------------------------------------------------------------------------*/
#include "juego/combate.h"

u8 combate_danio(u8 golpe)
{
    return (u8)(golpe >= GOLPES ? 2 : 1);            /* 1, 1, 2.                            */
}

u8 combate_derriba(u8 golpe)
{
    return (u8)(golpe >= GOLPES);                    /* El último del combo.                */
}

u8 combate_caja(u16 ax, u16 ay, u8 mira_izq, u16 alcance, u16 profundidad, u16 tx, u16 ty)
{
    u16 dx, dy;

    /* Profundidad: |ty - ay| <= profundidad (sin valor absoluto: dos comparaciones). */
    dy = (u16)(ty > ay ? ty - ay : ay - ty);
    if (dy > profundidad)
    {
        return 0;
    }
    /* x: «delante» es hacia donde mira; el objetivo cuenta desde COMBATE_ATRAS detrás de los
       pies hasta `alcance` delante. */
    if (mira_izq)
    {
        if (tx > ax)                                 /* Detrás (a la derecha): solo el margen. */
        {
            dx = (u16)(tx - ax);
            return (u8)(dx <= COMBATE_ATRAS);
        }
        dx = (u16)(ax - tx);                         /* Delante (a la izquierda).           */
        return (u8)(dx <= alcance);
    }
    if (tx < ax)                                     /* Detrás (a la izquierda).            */
    {
        dx = (u16)(ax - tx);
        return (u8)(dx <= COMBATE_ATRAS);
    }
    dx = (u16)(tx - ax);                             /* Delante (a la derecha).             */
    return (u8)(dx <= alcance);
}

u8 combate_en_caja(const Cuerpo *c, u16 tx, u16 ty)
{
    return combate_caja(c->x, c->y, c->mira_izq, COMBATE_ALCANCE, COMBATE_PROFUNDIDAD, tx, ty);
}

u8 combate_ninjas_resolver(Ninja *n, Cuerpo *c)
{
    u8 k, parada = 0;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        if (ninja_golpe_activo(n) &&
            combate_caja(n->x, n->y, n->mira_izq, NINJA_ALCANCE, NINJA_PROFUNDIDAD, c->x, c->y))
        {
            n->conecto = 1;                          /* Este tajo ya no pega más.           */
            if (mundo_recibir(c, ninja_danio(n), n->mira_izq))   /* El golpe va hacia donde mira el ninja. */
            {
                parada = (u8)(c->vida == 0 ? PARADA_DERRIBO : PARADA_GOLPE);
            }
        }
        n++;
    }
    return parada;
}

u8 combate_resolver(Cuerpo *c, Ninja *n, u16 alcance)
{
    u8 k, antes, parada = 0, hacia_izq, dentro;
    u8 derribo, danio, alrededor;

    if (!mundo_golpe_activo(c))
    {
        return 0;
    }
    derribo = mundo_golpe_derriba(c);                /* Combo, ataque aéreo o especial.     */
    danio = mundo_golpe_danio(c);
    alrededor = mundo_golpe_alrededor(c);            /* El especial: a los dos lados.       */
    for (k = 0; k < NINJAS_MAX; k++)
    {
        hacia_izq = c->mira_izq;
        dentro = (u8)(n->vivo && n->estado != NINJA_AGARRADO &&
                      combate_caja(c->x, c->y, c->mira_izq, alcance, COMBATE_PROFUNDIDAD, n->x, n->y));
        if (!dentro && alrededor && n->vivo &&
            combate_caja(c->x, c->y, (u8)!c->mira_izq, alcance, COMBATE_PROFUNDIDAD, n->x, n->y))
        {
            dentro = 1;                              /* Detrás: el giro también le alcanza. */
            hacia_izq = (u8)!c->mira_izq;
        }
        if (dentro)
        {
            antes = n->id_golpe;                     /* Para saber si este tajo era nuevo.  */
            if (alrededor && antes != c->id)
            {
                mundo_cobrar_especial(c);            /* Conectó: el especial cuesta vida.   */
            }
            if (ninja_recibir(n, danio, derribo, hacia_izq, c->id))
            {
                parada = PARADA_DERRIBO;             /* Lo derribó o lo remató.             */
            }
            else if (antes != c->id && n->id_golpe == c->id && parada == 0)
            {
                parada = PARADA_GOLPE;               /* Conectó un golpe normal.            */
            }
        }
        n++;
    }
    return parada;
}

/*--- delante: el agarrado, delante de los pies del cuerpo -----------------------------*/
static void colocar(const Cuerpo *c, Ninja *p)
{
    p->x = (u16)(c->mira_izq ? c->x - AGARRE_DELANTE : c->x + AGARRE_DELANTE);
    p->y = c->y;
    p->mira_izq = (u8)!c->mira_izq;                  /* Mirándolo.                          */
}

void combate_agarre(Cuerpo *c, Ninja *n)
{
    Ninja *p, *q;
    u8 k, j;
    u16 dx, dy;

    /*--- 1. Si ya agarra a uno: sus eventos y colocarlo ---*/
    if (c->agarra)
    {
        p = n + (c->agarra - 1);                     /* 32 bytes por ranura: sin tcc__mul.  */
        if (!p->vivo || p->estado != NINJA_AGARRADO)
        {
            c->agarra = 0;                           /* Se fue (o lo derribó otro golpe).   */
            if (c->estado == CUERPO_AGARRANDO || c->estado == CUERPO_GOLPE_AGARRE)
            {
                c->estado = CUERPO_PARADO;
                c->accion = 0;
            }
        }
        else if (c->evento == EVENTO_GOLPE_AGARRE)
        {
            p->vida = (u8)(p->vida > DANIO_GOLPE_AGARRE ? p->vida - DANIO_GOLPE_AGARRE : 0);
            p->destello = NINJA_DESTELLO_FRAMES;
            if (p->vida == 0)
            {
                ninja_lanzar(p, c->mira_izq, 0);     /* Sin vida: sale volando y cae KO.    */
                c->agarra = 0;
                c->estado = CUERPO_PARADO;
                c->accion = 0;
            }
        }
        else if (c->evento == EVENTO_LANZA_AGARRADO)
        {
            ninja_lanzar(p, c->mira_izq, DANIO_LANZAMIENTO);   /* Hacia donde mira.          */
            c->agarra = 0;
        }
        else if (c->evento == EVENTO_SUELTA_AGARRADO)
        {
            ninja_soltar(p, (u8)(c->agarre_golpes >= AGARRE_GOLPES), c->mira_izq);
            c->agarra = 0;
        }
        else if (c->estado == CUERPO_AGARRANDO || c->estado == CUERPO_GOLPE_AGARRE ||
                 c->estado == CUERPO_LANZANDO)
        {
            colocar(c, p);
        }
    }
    /*--- 2. Si anda contra uno agarrable, lo agarra a los AGARRE_EMPUJE frames ---*/
    else if (c->estado == CUERPO_ANDANDO && c->z == 0)
    {
        p = n;
        for (k = 0; k < NINJAS_MAX; k++)
        {
            dy = (u16)(p->y > c->y ? p->y - c->y : c->y - p->y);
            dx = (u16)(c->mira_izq ? c->x - p->x : p->x - c->x);   /* Delante (si no, enorme). */
            if (ninja_agarrable(p) && dy <= 6 && dx >= 8 && dx <= AGARRE_DISTANCIA)
            {
                break;
            }
            p++;
        }
        if (k < NINJAS_MAX)
        {
            c->empuja++;
            if (c->empuja >= AGARRE_EMPUJE)
            {
                ninja_agarrar(p);
                c->agarra = (u8)(k + 1);
                c->estado = CUERPO_AGARRANDO;
                c->accion = 0;
                c->agarre_golpes = 0;
                c->empuja = 0;
                colocar(c, p);
            }
        }
        else
        {
            c->empuja = 0;
        }
    }
    else
    {
        c->empuja = 0;
    }

    /*--- 3. Un ninja volando derriba a los que toca ---*/
    p = n;
    for (k = 0; k < NINJAS_MAX; k++)
    {
        if (ninja_volando(p))
        {
            q = n;
            for (j = 0; j < NINJAS_MAX; j++)
            {
                if (j != k && q->vivo && q->estado != NINJA_DERRIBADO)
                {
                    dx = (u16)(q->x > p->x ? q->x - p->x : p->x - q->x);
                    dy = (u16)(q->y > p->y ? q->y - p->y : p->y - q->y);
                    if (dx < 24 && dy < 12)
                    {
                        ninja_recibir(q, DANIO_CHOQUE, 1, (u8)(p->empuje == NINJA_EMPUJE_IZQ),
                                      NINJA_GOLPE_LANZADO);
                    }
                }
                q++;
            }
        }
        p++;
    }
}
