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

u8 combate_en_caja(const Cuerpo *c, u16 tx, u16 ty)
{
    u16 dx, dy;

    /* Profundidad: |ty − y| <= COMBATE_PROFUNDIDAD (sin valor absoluto: dos comparaciones). */
    dy = (u16)(ty > c->y ? ty - c->y : c->y - ty);
    if (dy > COMBATE_PROFUNDIDAD)
    {
        return 0;
    }
    /* x: «delante» es hacia donde mira; el objetivo cuenta desde COMBATE_ATRAS detrás de los
       pies hasta COMBATE_ALCANCE delante. */
    if (c->mira_izq)
    {
        if (tx > c->x)                               /* Detrás (a la derecha): solo el margen. */
        {
            dx = (u16)(tx - c->x);
            return (u8)(dx <= COMBATE_ATRAS);
        }
        dx = (u16)(c->x - tx);                       /* Delante (a la izquierda).           */
        return (u8)(dx <= COMBATE_ALCANCE);
    }
    if (tx < c->x)                                   /* Detrás (a la izquierda).            */
    {
        dx = (u16)(c->x - tx);
        return (u8)(dx <= COMBATE_ATRAS);
    }
    dx = (u16)(tx - c->x);                           /* Delante (a la derecha).             */
    return (u8)(dx <= COMBATE_ALCANCE);
}

u8 combate_resolver(const Cuerpo *c, Ninja *n)
{
    u8 k, antes, parada = 0;
    u8 derribo = combate_derriba(c->golpe);

    if (!mundo_golpe_activo(c))
    {
        return 0;
    }
    for (k = 0; k < NINJAS_MAX; k++)
    {
        if (n->vivo && combate_en_caja(c, n->x, n->y))
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
