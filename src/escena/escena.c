/*---------------------------------------------------------------------------------
    escena.c — El secuenciador de escenas (ver escena.h).

    PROPÓSITO
      Cada frame hace tres cosas, en este orden:
        1. Arranca los eventos cuyo `cuadro` es el frame actual (la tabla va
           ordenada, así que basta con mirar el siguiente y avanzar).
        2. Mueve cada pista que tenga frames pendientes: suma su paso y, en el
           último frame, pone el valor EXACTO de destino (así el brillo acaba
           en 15 y no en 14,9).
        3. Avanza el reloj.
      Un evento de `frames` frames que empieza en el frame c llega a su valor
      final en el frame c + frames - 1.

    SE CONECTA CON
      Incluye:  escena/escena.h.
      Lo usan:  pantallas/pantalla_nintendo.c, tests/prueba_escena.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo vive en la Escena que pasa quien llama).
    → glosario: Motor de escenas, Punto fijo
---------------------------------------------------------------------------------*/
#include "escena/escena.h"

void escena_empezar(Escena *e, const EventoEscena *tabla)
{
    Pista *p = e->pistas;
    u8 n;

    for (n = ESCENA_PISTAS; n != 0; n--, p++)        /* Todas las pistas quietas en 0.      */
    {
        p->valor = 0;
        p->paso = 0;
        p->quedan = 0;
        p->hasta = 0;
    }
    e->proximo = tabla;                              /* El primer evento de la tabla.       */
    e->cuadro = 0;
    e->terminada = 0;
}

void escena_avanzar(Escena *e)
{
    const EventoEscena *ev = e->proximo;
    Pista *p;
    u8 n;

    if (e->terminada)                                /* Ya acabó: los valores se quedan.    */
    {
        return;
    }

    /*--- 1. Arrancar los eventos de este frame ----------------------------------- */
    /* <= y no ==: si la tabla tuviera un evento fuera de orden, con == la
       escena se quedaría esperándolo para siempre (la pantalla, congelada);
       con <= arranca tarde y la escena sigue. Las pruebas exigen el orden. */
    while (ev->cuadro <= e->cuadro)
    {
        if (ev->pista == ESCENA_FIN)                 /* Fin: no se pasa de aquí, y los      */
        {                                            /* valores y el reloj se quedan como   */
            e->proximo = ev;                         /* estaban en el frame anterior.       */
            e->terminada = 1;
            return;
        }
        p = &e->pistas[ev->pista];                   /* Pista es de 8 bytes: índice x 8 =   */
                                                     /* tres desplazamientos, sin tcc__mul. */
        p->hasta = ev->hasta;
        p->paso = ev->paso;
        p->quedan = ev->frames;
        if (ev->frames == 0)                         /* FIJAR: el valor, ya.                */
        {
            p->valor = (u16)((u16)ev->hasta << 8);
        }
        ev++;                                        /* Siguiente evento (8 bytes más).     */
    }
    e->proximo = ev;

    /*--- 2. Mover las pistas en curso -------------------------------------------- */
    for (p = e->pistas, n = ESCENA_PISTAS; n != 0; n--, p++)
    {
        if (p->quedan != 0)
        {
            p->quedan--;
            if (p->quedan != 0)
            {
                p->valor = (u16)(p->valor + p->paso);    /* Un paso más (8.8).          */
            }
            else
            {
                p->valor = (u16)((u16)p->hasta << 8);    /* Último frame: exacto.       */
            }
        }
    }

    /*--- 3. Un frame más ---------------------------------------------------------- */
    e->cuadro++;
}
