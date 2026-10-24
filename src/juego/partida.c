/*---------------------------------------------------------------------------------
    partida.c — Las reglas del tutorial (ver partida.h).

    PROPÓSITO
      Comprobar, una vez por frame y en este orden: ¿cayó o se pinchó? (gana a
      todo lo demás), ¿la espada toca el tronco?, ¿ha llegado a la salida?, y
      apuntar el último punto de control por el que ha pasado.

    SE CONECTA CON
      Incluye:  juego/partida.h, juego/colision.h, juego/nivel.h (ANCHO_CELDA,
                SUELO_Y), sistema/puntofijo.h.
      Lo usan:  pantallas/pantalla_nivel1.c, tests/prueba_partida.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo viaja en la struct Partida).
    → glosario: Punto de control, AABB
---------------------------------------------------------------------------------*/
#include "juego/partida.h"
#include "juego/colision.h"
#include "juego/nivel.h"
#include "sistema/puntofijo.h"

/*--- se_tocan: ¿se solapan dos rectángulos? (prueba AABB) -------------------------
    Dos cajas alineadas con los ejes se solapan si cada una empieza antes de que
    acabe la otra, en x y en y. Es la prueba de colisión más barata que hay. */
static u8 se_tocan(s16 ax, s16 ay, s16 aw, s16 ah, s16 bx, s16 by, s16 bw, s16 bh)
{
    return (u8)(ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah);
}

void partida_iniciar(Partida *p, u16 inicio_x, const u16 *controles, u8 n_controles,
                     u16 tronco_x, u16 salida_x)
{
    p->controles = controles;
    p->n_controles = n_controles;
    p->reaparicion = inicio_x;                       /* Antes del primer cartel: el inicio. */
    p->siguiente = 0;                                /* El primero por pasar.               */
    p->tronco_x = tronco_x;
    p->tronco_entero = (u8)(tronco_x != 0);
    p->salida_x = salida_x;
    if (p->tronco_entero)                            /* El tronco, apoyado en el suelo base. */
    {
        colision_bloque(tronco_x, SUELO_Y - TRONCO_ALTO, TRONCO_ANCHO, TRONCO_ALTO);
    }
}

u8 partida_actualizar(Partida *p, const Tarou *t)
{
    Caja c;
    u16 centro;

    /*--- 1. ¿Cayó por un foso o tocó una zarza? Reaparecer gana a todo. ----------*/
    /* En punto fijo y con signo: saltando desde una plataforma la celda sube por encima
       de la pantalla (y negativa), y PF_ENTERO, que es solo para x >= 0, daría un
       número enorme: ¡una "caída" en lo alto de un salto! */
    if (t->y_fp >= PF(Y_CAIDA))
    {
        return PARTIDA_REAPARECER;
    }
    tarou_caja_cuerpo(t, &c);                        /* La fila de las patas, de punta a    */
    if (colision_fila_es(c.x, (s16)(c.x + c.ancho - 1), (s16)(c.y + c.alto - 1), COL_ZARZA))
    {                                                /* punta: ¿pisa o roza una zarza?      */
        return PARTIDA_REAPARECER;
    }

    /*--- 2. ¿La espada toca el tronco? -------------------------------------------*/
    if (p->tronco_entero && tarou_caja_golpe(t, &c)
        && se_tocan(c.x, c.y, c.ancho, c.alto, (s16)p->tronco_x, SUELO_Y - TRONCO_ALTO,
                    TRONCO_ANCHO, TRONCO_ALTO))
    {
        p->tronco_entero = 0;
        colision_bloque(0, 0, 0, 0);                 /* Ya no es sólido.                    */
        return PARTIDA_TRONCO_ROTO;
    }

    /*--- 3. Puntos de control y salida, por el centro de la celda -----------------*/
    centro = (u16)(PF_ENTERO(t->x_fp) + ANCHO_CELDA / 2);
    /* Los controles van de izquierda a derecha: basta mirar el SIGUIENTE (una
       comparación por frame en vez de recorrerlos todos). */
    if (p->siguiente < p->n_controles && centro >= p->controles[p->siguiente])
    {
        p->reaparicion = p->controles[p->siguiente]; /* Pasado: se apunta...                */
        p->siguiente++;                              /* ...y se espera al siguiente.        */
    }
    if (centro >= p->salida_x)
    {
        return PARTIDA_SALIDA;
    }
    return PARTIDA_SIGUE;
}
