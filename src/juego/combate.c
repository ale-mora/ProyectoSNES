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

u8 combate_resolver(const Cuerpo *c, Ninja *n, u16 alcance)
{
    u8 k, antes, parada = 0;
    u8 derribo = combate_derriba(c->golpe);

    if (!mundo_golpe_activo(c))
    {
        return 0;
    }
    for (k = 0; k < NINJAS_MAX; k++)
    {
        if (n->vivo && combate_caja(c->x, c->y, c->mira_izq, alcance, COMBATE_PROFUNDIDAD, n->x, n->y))
        {
            antes = n->id_golpe;                     /* Para saber si este tajo era nuevo.  */
            if (ninja_recibir(n, combate_danio(c->golpe), derribo, c->mira_izq, c->id))
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
