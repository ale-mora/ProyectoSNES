/*---------------------------------------------------------------------------------
    prueba_escena_publisher.c — Pruebas de la tabla del logo del publisher
    (escena/escena_publisher.c). Solución de la ampliación del ejercicio 12.

    PROPÓSITO
      Que la tabla vaya ordenada y termine en el FIN, que dure 244 frames (la
      duración de antes del motor: si no, el arranque entero se corre), que el
      logo se vea con brillo 15 y nítido entre los fundidos, que ninguna pista
      dé saltos (un `desde` mal escrito en una RAMPA) y que acabe APAGADA:
      brillo 0 (setBrightness(0) es forced blank). El mosaico acaba en 15 a
      propósito: lo pone a 0 efectos_terminar() antes de cambiar de pantalla.

    SE CONECTA CON
      Incluye:  minitest.h, escena/escena.h, escena/escena_publisher.h.
      Lo usa:   tests/pruebas.c (pruebas_escena_publisher).

    HARDWARE QUE TOCA
      ninguno (solo PC).

    VARIABLES: ninguna global.
    → glosario: Motor de escenas
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "escena/escena.h"
#include "escena/escena_publisher.h"

/* Cuánto cambió una pista entre dos frames (en 8.8, sin signo). La de
   prueba_escena.c es static: cada archivo de pruebas tiene la suya. */
static u16 salto(u16 a, u16 b)
{
    return (u16)(a > b ? a - b : b - a);
}

void pruebas_escena_publisher(void)
{
    Escena e;
    const EventoEscena *ev;
    u16 previo[ESCENA_PISTAS];
    u16 maximo[ESCENA_PISTAS];
    u16 frames = 0;
    u8 orden = 1;
    u8 sin_saltos = 1;
    u8 visible = 1;
    u8 i;

    /* Ordenada, cada rampa acaba antes del FIN, y el FIN es la última línea. */
    for (ev = ESCENA_PUBLISHER; ev->pista != ESCENA_FIN; ev++)
    {
        if (ev[1].cuadro < ev->cuadro || ev->cuadro + ev->frames > ESCENA_PUBLISHER_FRAMES)
        {
            orden = 0;
        }
    }
    COMPROBAR(orden);
    COMPROBAR(ev == &ESCENA_PUBLISHER[ESCENA_PUBLISHER_EVENTOS - 1]);
    COMPROBAR(ev->cuadro == 32 + 180 + 32);           /* La duración de antes del motor.   */

    /* El salto máximo por pista: el mayor |paso| de sus rampas más 1 entero. */
    for (i = 0; i < ESCENA_PISTAS; i++)
    {
        maximo[i] = 0;
        previo[i] = 0;
    }
    for (ev = ESCENA_PUBLISHER; ev->pista != ESCENA_FIN; ev++)
    {
        u16 p = (u16)(ev->paso < 0 ? -ev->paso : ev->paso);
        if (ev->frames != 0 && p + 256 > maximo[ev->pista])
        {
            maximo[ev->pista] = (u16)(p + 256);
        }
    }

    escena_empezar(&e, ESCENA_PUBLISHER);
    while (!e.terminada && frames < 1000)
    {
        u16 c = e.cuadro;
        escena_avanzar(&e);
        frames++;
        for (i = 0; i < ESCENA_PISTAS; i++)
        {
            if (c != 0 && salto(previo[i], e.pistas[i].valor) > maximo[i])
            {
                sin_saltos = 0;                      /* El frame 0 lleva un FIJAR.        */
            }
            previo[i] = e.pistas[i].valor;
        }
        if (c >= 31 && c < 196 &&
            (ESCENA_VALOR(&e, PISTA_BRILLO) != 15 || ESCENA_VALOR(&e, PISTA_MOSAICO) != 0))
        {
            visible = 0;                             /* Entre medias: brillo 15 y nítido. */
        }
    }
    COMPROBAR(sin_saltos);
    COMPROBAR(visible);
    COMPROBAR(frames == ESCENA_PUBLISHER_FRAMES + 1); /* 245 llamadas: la última ve el FIN. */
    COMPROBAR(e.terminada == 1);
    COMPROBAR(ESCENA_VALOR(&e, PISTA_BRILLO) == 0);   /* Acaba apagada.                    */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_MOSAICO) == 15); /* Pixelada: lo limpia efectos_terminar. */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_LUZ) == 0);      /* Sin luz ni destello.              */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_DESTELLO) == 0);
}
