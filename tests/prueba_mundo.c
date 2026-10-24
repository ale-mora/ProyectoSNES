/*---------------------------------------------------------------------------------
    prueba_mundo.c — Pruebas de juego/mundo.c (el cuerpo en el carril).

    PROPÓSITO
      Fijar las reglas del esqueleto del beat 'em up: x se mueve con la cruceta, y
      (profundidad) con arriba/abajo dentro de CARRIL_Y_MIN..MAX, B salta solo desde
      el suelo, el salto sube y baja y aterriza en z = 0, en el aire no se cambia de
      dirección en x pero sí se puede cambiar de carril, y los límites recortan.

    SE CONECTA CON
      Incluye:  minitest.h, juego/mundo.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_mundo).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (cada prueba usa un Cuerpo y un Mando locales).
    → glosario: Carril, Altura (z)
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/mundo.h"

static void preparar(Cuerpo *c, Mando *m)
{
    memset(m, 0, sizeof(*m));
    mundo_iniciar(c, 120, 200);
}

void pruebas_mundo(void)
{
    Cuerpo c;
    Mando m;
    u8 k, subio, max_alto;

    /* Quieto: no se mueve y está PARADO. */
    preparar(&c, &m);
    mundo_actualizar(&c, &m);
    COMPROBAR(c.x == 120 && c.y == 200 && c.z == 0);
    COMPROBAR(c.estado == CUERPO_PARADO);

    /* Derecha e izquierda: x cambia PASO_X y la mirada sigue la dirección. */
    m.derecha = 1;
    mundo_actualizar(&c, &m);
    COMPROBAR(c.x == 120 + PASO_X && c.mira_izq == 0 && c.estado == CUERPO_ANDANDO);
    m.derecha = 0; m.izquierda = 1;
    mundo_actualizar(&c, &m);
    mundo_actualizar(&c, &m);
    COMPROBAR(c.x == 120 - PASO_X && c.mira_izq == 1);
    m.derecha = 1;                                   /* Las dos a la vez se anulan.         */
    mundo_actualizar(&c, &m);
    COMPROBAR(c.x == 120 - PASO_X);

    /* El carril: arriba y abajo, y se recorta en sus extremos. */
    preparar(&c, &m);
    m.arriba = 1;
    mundo_actualizar(&c, &m);
    COMPROBAR(c.y == 200 - PASO_Y && c.estado == CUERPO_ANDANDO);
    for (k = 0; k < 100; k++)
    {
        mundo_actualizar(&c, &m);
    }
    COMPROBAR(c.y == CARRIL_Y_MIN);
    m.arriba = 0; m.abajo = 1;
    for (k = 0; k < 100; k++)
    {
        mundo_actualizar(&c, &m);
    }
    COMPROBAR(c.y == CARRIL_Y_MAX);

    /* Límites de x, sin dar la vuelta al u16 por la izquierda. */
    preparar(&c, &m);
    m.izquierda = 1;
    for (k = 0; k < 200; k++)
    {
        mundo_actualizar(&c, &m);
    }
    COMPROBAR(c.x == MUNDO_X_MIN);
    m.izquierda = 0; m.derecha = 1;
    for (k = 0; k < 200; k++)
    {
        mundo_actualizar(&c, &m);
    }
    COMPROBAR(c.x == MUNDO_X_MAX);

    /* Salto: sube, llega a ~36 px, baja y aterriza en 24 frames, de nuevo PARADO. */
    preparar(&c, &m);
    m.saltar_pulsado = 1;
    mundo_actualizar(&c, &m);
    m.saltar_pulsado = 0;
    COMPROBAR(c.z > 0 && c.estado == CUERPO_SUBIENDO);
    subio = 0; max_alto = 0;
    for (k = 0; k < 60 && c.z > 0; k++)
    {
        if (mundo_altura(&c) > max_alto)
        {
            max_alto = (u8)mundo_altura(&c);
        }
        if (c.estado == CUERPO_BAJANDO)
        {
            subio = 1;
        }
        mundo_actualizar(&c, &m);
    }
    COMPROBAR(subio);                                /* Pasó por BAJANDO.                   */
    COMPROBAR(max_alto >= 30 && max_alto <= 40);     /* Unos 36 px.                         */
    COMPROBAR(c.z == 0 && c.vz == 0 && c.estado == CUERPO_PARADO);
    COMPROBAR(k >= 20 && k <= 28);                   /* Unos 24 frames en el aire.          */

    /* En el aire no se salta otra vez, no se cambia de dirección en x, sí de carril. */
    preparar(&c, &m);
    m.derecha = 1; m.saltar_pulsado = 1;
    mundo_actualizar(&c, &m);                        /* Despega hacia la derecha.           */
    m.saltar_pulsado = 1; m.derecha = 0; m.izquierda = 1; m.abajo = 1;
    {
        s16 vz_antes = c.vz;
        u16 x_antes = c.x, y_antes = c.y;
        mundo_actualizar(&c, &m);
        COMPROBAR(c.vz == vz_antes + GRAVEDAD);      /* No volvió a empezar el salto.       */
        COMPROBAR(c.x == x_antes + PASO_X && c.mira_izq == 0); /* Sigue hacia la derecha.  */
        COMPROBAR(c.y == y_antes + PASO_Y);          /* Pero cambia de carril.              */
    }
}
