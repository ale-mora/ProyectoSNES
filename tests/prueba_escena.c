/*---------------------------------------------------------------------------------
    prueba_escena.c — Pruebas del motor de escenas (escena/escena.c) y de la
    tabla del logo Nintendo (escena/escena_nintendo.c).

    PROPÓSITO
      Comprobar que las rampas llegan EXACTAS a su destino, que FIJAR actúa en
      el mismo frame, que la escena termina en el frame de FIN_ESCENA y, para
      la tabla real, que dura lo mismo que la pantalla de antes (si no, todo el
      arranque se corre), que ningún valor da saltos (un `desde` mal escrito en
      una RAMPA se nota así) y que al acabar no queda ningún efecto encendido.

    SE CONECTA CON
      Incluye:  minitest.h, escena/escena.h, escena/escena_nintendo.h.
      Lo usa:   tests/pruebas.c (pruebas_escena).

    HARDWARE QUE TOCA
      ninguno (solo PC).

    VARIABLES (estáticas)
      PRUEBA  const EventoEscena[]  una tabla pequeña de prueba
    → glosario: Motor de escenas
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "escena/escena.h"
#include "escena/escena_nintendo.h"

static const EventoEscena PRUEBA[] = {
    RAMPA(0, 32, PISTA_BRILLO, 0, 15),
    FIJAR(2, PISTA_MOSAICO, 9),
    RAMPA(2, 10, PISTA_MOSAICO, 9, 0),
    RAMPA(40, 3, PISTA_BRILLO, 15, 0),
    FIN_ESCENA(50),
};

/* Avanza `n` frames. */
static void avanzar(Escena *e, u16 n)
{
    for (; n != 0; n--)
    {
        escena_avanzar(e);
    }
}

static void rampas_y_fin(void)
{
    Escena e;
    u8 antes;
    u16 i;

    escena_empezar(&e, PRUEBA);
    COMPROBAR(ESCENA_VALOR(&e, PISTA_BRILLO) == 0);

    /* La rampa de brillo sube sin bajar nunca y llega a 15 en el frame 31. */
    antes = 0;
    for (i = 0; i < 31; i++)
    {
        escena_avanzar(&e);
        COMPROBAR(ESCENA_VALOR(&e, PISTA_BRILLO) >= antes);
        antes = ESCENA_VALOR(&e, PISTA_BRILLO);
    }
    COMPROBAR(ESCENA_VALOR(&e, PISTA_BRILLO) == 14);   /* Frame 30: aún no.               */
    escena_avanzar(&e);                                /* Frame 31: el último de la rampa. */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_BRILLO) == 15);
    COMPROBAR(e.pistas[PISTA_BRILLO].valor == 15 * 256);   /* Exacto, sin decimales.      */

    /* FIJAR(2) + RAMPA(2): el mosaico saltó a 9 y bajó a 0 en el frame 11. */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_MOSAICO) == 0);

    /* La rampa de bajada (frames 40-42) acaba en 0 exacto. */
    avanzar(&e, 40 - 32);                              /* Frames 32..39.                  */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_BRILLO) == 15);
    avanzar(&e, 3);                                    /* Frames 40..42.                  */
    COMPROBAR(e.pistas[PISTA_BRILLO].valor == 0);

    /* El FIN está en el frame 50: el avance número 51 lo procesa. */
    avanzar(&e, 50 - 43);                              /* Frames 43..49.                  */
    COMPROBAR(e.terminada == 0);
    escena_avanzar(&e);                                /* Frame 50.                       */
    COMPROBAR(e.terminada == 1);
    avanzar(&e, 5);                                    /* Ya no se mueve nada.            */
    COMPROBAR(e.cuadro == 50);
}

static void fijar_al_instante(void)
{
    static const EventoEscena T[] = {FIJAR(0, PISTA_LUZ, 20), FIN_ESCENA(1)};
    Escena e;

    escena_empezar(&e, T);
    escena_avanzar(&e);
    COMPROBAR(ESCENA_VALOR(&e, PISTA_LUZ) == 20);      /* En el mismo frame 0.            */
}

/* Un evento fuera de orden no congela la escena: arranca tarde y se llega al FIN. */
static void fuera_de_orden(void)
{
    static const EventoEscena T[] = {FIJAR(5, PISTA_LUZ, 7), FIJAR(3, PISTA_MOSAICO, 4),
                                     FIN_ESCENA(8)};
    Escena e;

    escena_empezar(&e, T);
    avanzar(&e, 6);                                    /* Frames 0..5.                    */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_LUZ) == 7);
    COMPROBAR(ESCENA_VALOR(&e, PISTA_MOSAICO) == 4);   /* El del frame 3, en el 5.        */
    avanzar(&e, 3);                                    /* Frames 6..8.                    */
    COMPROBAR(e.terminada == 1);
}

/* Cuánto cambió una pista entre dos frames (en 8.8, sin signo). */
static u16 salto(u16 a, u16 b)
{
    return (u16)(a > b ? a - b : b - a);
}

static void tabla_nintendo(void)
{
    Escena e;
    const EventoEscena *ev;
    u16 previo[ESCENA_PISTAS];
    u16 maximo[ESCENA_PISTAS];
    u16 frames = 0;
    u8 orden = 1;
    u8 sin_saltos = 1;
    u8 brillo_lleno = 1;
    u8 i;

    /* La tabla va ordenada por frame y cada rampa termina antes del FIN. */
    for (ev = ESCENA_NINTENDO; ev->pista != ESCENA_FIN; ev++)
    {
        if (ev[1].cuadro < ev->cuadro || ev->cuadro + ev->frames > ESCENA_NINTENDO_FRAMES)
        {
            orden = 0;
        }
    }
    COMPROBAR(orden);
    COMPROBAR(ev == &ESCENA_NINTENDO[ESCENA_NINTENDO_EVENTOS - 1]);   /* FIN, el último. */
    COMPROBAR(ev->cuadro == 32 + 180 + 32);           /* La duración de antes del motor. */

    /* El salto máximo permitido por pista: el mayor |paso| de sus eventos más
       un poco (el redondeo del último frame). Un FIJAR puede saltar lo que quiera. */
    for (i = 0; i < ESCENA_PISTAS; i++)
    {
        maximo[i] = 0;
    }
    for (ev = ESCENA_NINTENDO; ev->pista != ESCENA_FIN; ev++)
    {
        u16 p = (u16)(ev->paso < 0 ? -ev->paso : ev->paso);
        if (ev->frames != 0 && p + 256 > maximo[ev->pista])
        {
            maximo[ev->pista] = (u16)(p + 256);
        }
    }

    escena_empezar(&e, ESCENA_NINTENDO);
    for (i = 0; i < ESCENA_PISTAS; i++)
    {
        previo[i] = 0;
    }
    while (!e.terminada && frames < 1000)
    {
        u16 c = e.cuadro;
        escena_avanzar(&e);
        frames++;
        for (i = 0; i < ESCENA_PISTAS; i++)
        {
            /* El frame 0 lleva el FIJAR del mosaico: ahí sí hay salto. */
            if (c != 0 && salto(previo[i], e.pistas[i].valor) > maximo[i])
            {
                sin_saltos = 0;
                printf("  salto en la pista %u, frame %u\n", (unsigned)i, (unsigned)c);
            }
            previo[i] = e.pistas[i].valor;
        }
        if (c >= 31 && c < 212 && ESCENA_VALOR(&e, PISTA_BRILLO) != 15)
        {
            brillo_lleno = 0;                          /* Entre fundidos, brillo al máximo. */
        }
        if (c == 47)
        {
            COMPROBAR(ESCENA_VALOR(&e, PISTA_MOSAICO) == 0);   /* Ya nítido.            */
        }
        if (c == 69)
        {
            COMPROBAR(ESCENA_VALOR(&e, PISTA_LUZ) == 0);       /* Aún sin destello.      */
        }
        if (c == 90)
        {
            COMPROBAR(ESCENA_VALOR(&e, PISTA_LUZ) == 14);      /* Destello encendido.    */
        }
    }
    COMPROBAR(sin_saltos);
    COMPROBAR(brillo_lleno);
    COMPROBAR(frames == ESCENA_NINTENDO_FRAMES + 1);  /* 245 llamadas: la última ve el FIN. */
    COMPROBAR(e.terminada == 1);

    /* Al acabar no queda nada encendido: negro, nítido y sin luz. */
    COMPROBAR(ESCENA_VALOR(&e, PISTA_BRILLO) == 0);
    COMPROBAR(ESCENA_VALOR(&e, PISTA_MOSAICO) == 0);
    COMPROBAR(ESCENA_VALOR(&e, PISTA_LUZ) == 0);
    COMPROBAR(ESCENA_VALOR(&e, PISTA_DESTELLO) == 255);   /* La banda acabó fuera, a la derecha. */
}

void pruebas_escena(void)
{
    rampas_y_fin();
    fijar_al_instante();
    fuera_de_orden();
    tabla_nintendo();
}
