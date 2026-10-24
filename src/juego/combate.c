/*---------------------------------------------------------------------------------
    combate.c — Cajas, profundidad e impacto (ver combate.h).

    PROPÓSITO
      De la celda al mundo: los pies están en el centro de la última fila de la
      celda, así que la esquina de la celda está en (x − mitad_ancho, y_dibujo −
      alto). Mirando al oeste la caja se espeja dentro de la celda:
      x' = ancho − x − ancho_caja. Después, una comparación de rectángulos, la
      regla de profundidad y el daño. También aquí, las tablas por rango de
      juego/cifras.h (static const: así se quedan en la ROM, CLAUDE.md).

    SE CONECTA CON
      Incluye:  juego/combate.h, juego/cifras.h.
      Lo usan:  tests/prueba_combate.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tablas en ROM)
      vida_ninja[4]    u8   16 / 24 / 32 / 48     vida por rango
      bonus_dano[4]    u8   0 / 1 / 2 / 3         daño extra por rango
      puntos_ninja[4]  u16  100 / 150 / 200 / 300 puntos al vencerlo
    → glosario: Caja de golpe / caja de daño, Parada de impacto, Rango
---------------------------------------------------------------------------------*/
#include "juego/combate.h"
#include "juego/cifras.h"

static const u8 vida_ninja[RANGOS] = {16, 24, 32, 48};      /* Blanco, amarillo, verde, rojo. */
static const u8 bonus_dano[RANGOS] = {0, 1, 2, 3};          /* Se suma al daño base.          */
static const u16 puntos_ninja[RANGOS] = {100, 150, 200, 300}; /* Al vencerlo.                 */

u8 cifras_vida_ninja(u8 rango)
{
    return vida_ninja[rango & 3];                    /* & 3: nunca se sale de la tabla.     */
}

u8 cifras_bonus_dano(u8 rango)
{
    return bonus_dano[rango & 3];
}

u16 cifras_puntos_ninja(u8 rango)
{
    return puntos_ninja[rango & 3];
}

void combate_caja_mundo(const CajaCuadro *c, const Celda *celda, const Cuerpo *cuerpo,
                        u8 izquierda, CajaMundo *salida)
{
    s16 x_celda;                                     /* Esquina de la celda en el mundo.    */
    s16 y_celda;
    s16 x_en_celda;                                  /* x de la caja dentro de la celda (con signo:
                                                        una caja que sobresale da negativo, no 236). */

    x_celda = (s16)PF_ENTERO(cuerpo->x_fp) - celda->mitad_ancho; /* Pies en el centro.      */
    y_celda = (s16)mundo_y_dibujo(cuerpo) - celda->alto;         /* Pies en la última fila. */

    x_en_celda = c->x;                               /* Mirando al este, tal cual.          */
    if (izquierda)
        x_en_celda = (s16)celda->ancho - (s16)c->x - (s16)c->ancho;   /* Mirando al oeste, espejada. */

    salida->x = x_celda + x_en_celda;                /* Al mundo.                           */
    salida->y = y_celda + c->y;
    salida->ancho = c->ancho;
    salida->alto = c->alto;
}

u8 combate_se_cruzan(const CajaMundo *a, const CajaMundo *b)
{
    if (a->ancho == 0 || b->ancho == 0)
        return 0;                                    /* Una caja vacía no toca nada.        */
    if (a->x + a->ancho <= b->x)
        return 0;                                    /* a está entera a la izquierda.       */
    if (b->x + b->ancho <= a->x)
        return 0;                                    /* a está entera a la derecha.         */
    if (a->y + a->alto <= b->y)
        return 0;                                    /* a está entera encima.               */
    if (b->y + b->alto <= a->y)
        return 0;                                    /* a está entera debajo.               */
    return 1;                                        /* Se pisan.                           */
}

u8 combate_misma_profundidad(const Cuerpo *a, const Cuerpo *b)
{
    s16 dy;                                          /* Diferencia de profundidad.          */

    dy = a->y_fp - b->y_fp;                          /* En punto fijo: puede ser negativa.  */
    if (dy < 0)
        dy = -dy;                                    /* Valor absoluto, a mano.             */
    return dy <= TOLERANCIA_Y;
}

u8 combate_golpear(const CajaMundo *golpe, const CajaCuadro *datos, const Cuerpo *atacante,
                   Golpeable *atacante_g, const CajaMundo *objetivo_caja,
                   const Cuerpo *objetivo, Golpeable *objetivo_g, u8 *golpeados,
                   u8 bit_objetivo)
{
    if (*golpeados & bit_objetivo)
        return GOLPE_FALLA;                          /* Este ataque ya le dio.              */
    if (objetivo_g->vida == 0 || objetivo_g->invulnerable)
        return GOLPE_FALLA;                          /* Vencido o protegido.                */
    if (!combate_misma_profundidad(atacante, objetivo))
        return GOLPE_FALLA;                          /* Lejos en el carril: pasa de largo.  */
    if (!combate_se_cruzan(golpe, objetivo_caja))
        return GOLPE_FALLA;                          /* Las cajas no se tocan.              */

    *golpeados |= bit_objetivo;                      /* Marcado: no le vuelve a dar.        */
    objetivo_g->destello = FRAMES_DESTELLO;          /* Parpadeo en blanco.                 */

    if (datos->dano >= objetivo_g->vida)             /* El golpe se lo lleva todo:          */
    {
        objetivo_g->vida = 0;                        /* sin vida (u8: nunca bajo 0),        */
        atacante_g->parada = FRAMES_PARADA;          /* con parada, que es el último,       */
        objetivo_g->parada = FRAMES_PARADA;
        return GOLPE_VENCE;                          /* KO.                                 */
    }
    objetivo_g->vida -= datos->dano;                 /* Quita su daño.                      */

    if (datos->efecto == EFECTO_DERRIBO)             /* Golpe fuerte:                       */
    {
        atacante_g->parada = FRAMES_PARADA;          /* los dos se congelan,                */
        objetivo_g->parada = FRAMES_PARADA;
        return GOLPE_DERRIBO;                        /* y el objetivo cae.                  */
    }
    return GOLPE_RETROCESO;                          /* Golpe normal: retrocede.            */
}

void combate_tic(Golpeable *g)
{
    if (g->invulnerable)
        g->invulnerable--;                           /* Cada contador baja hasta 0.         */
    if (g->destello)
        g->destello--;
    if (g->parada)
        g->parada--;
}

u8 combate_pagar_especial(u8 vida)
{
    if (vida <= COSTE_ESPECIAL)
        return 1;                                    /* Nunca mata: deja 1.                 */
    return vida - COSTE_ESPECIAL;                    /* Paga su coste.                      */
}
