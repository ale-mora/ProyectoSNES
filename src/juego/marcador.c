/*---------------------------------------------------------------------------------
    marcador.c — Puntuación, vidas y continues (ver marcador.h).

    PROPÓSITO
      Sumar sin `long`: el resto se suma y, si pasa de 9999, se resta 10 000 y se
      cuenta un «mil» más. Como cada suma es menor de 10 000, basta un acarreo.
      Las vidas extra se dan cuando `miles` llega a 5 y a 15, una sola vez cada
      una (la marca `extra_50k` / `extra_150k` lo recuerda).

    SE CONECTA CON
      Incluye:  juego/marcador.h, juego/cifras.h (VIDAS_INICIALES, CONTINUES_INICIALES).
      Lo usan:  tests/prueba_marcador.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna propia.
    → glosario: Vida extra, Continue
---------------------------------------------------------------------------------*/
#include "juego/marcador.h"
#include "juego/cifras.h"

void marcador_iniciar(Marcador *m)
{
    m->resto = 0;
    m->miles = 0;
    m->vidas = VIDAS_INICIALES;
    m->continues = CONTINUES_INICIALES;
    m->extra_50k = 0;
    m->extra_150k = 0;
}

u8 marcador_sumar(Marcador *m, u16 puntos)
{
    u8 dadas;                        /* Vidas extra que da esta suma.                      */

    dadas = 0;
    m->resto = m->resto + puntos;
    if (m->resto >= 10000)           /* Acarreo: se cuenta un mil más.                     */
    {
        m->resto = m->resto - 10000;
        m->miles++;
    }

    if (!m->extra_50k && m->miles >= MARCADOR_EXTRA_50K_MILES)
    {
        m->extra_50k = 1;
        m->vidas++;
        dadas++;
    }
    if (!m->extra_150k && m->miles >= MARCADOR_EXTRA_150K_MILES)
    {
        m->extra_150k = 1;
        m->vidas++;
        dadas++;
    }
    return dadas;
}

u8 marcador_perder_vida(Marcador *m)
{
    if (m->vidas > 0)
        m->vidas--;
    return m->vidas == 0;            /* Sin vidas: se acabó la partida.                    */
}

u8 marcador_continuar(Marcador *m)
{
    if (m->continues == 0)
        return 0;
    m->continues--;
    m->vidas = VIDAS_INICIALES;      /* Vuelve con las vidas de inicio.                    */
    return 1;
}
