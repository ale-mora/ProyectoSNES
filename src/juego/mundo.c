/*---------------------------------------------------------------------------------
    mundo.c — Movimiento en el carril: x, y y el salto con z (ver mundo.h).

    PROPÓSITO
      Cada frame, en este orden: (1) si está en el suelo y se pulsa B, empieza el salto;
      (2) en el suelo, la dirección fija hacia dónde mira; en el aire se conserva;
      (3) x se mueve solo en el suelo (el impulso del salto se queda: no se "conduce"
      el salto en x); y se puede mover siempre; (4) la gravedad cambia vz y vz cambia z;
      al llegar a z <= 0 aterriza. (5) x e y se recortan a los límites del carril.
      No hay divisiones ni multiplicaciones: solo sumas, restas y comparaciones.

    SE CONECTA CON
      Incluye:  juego/mundo.h.
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_mundo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo en la struct Cuerpo).
    → glosario: Carril, Altura (z), Punto fijo
---------------------------------------------------------------------------------*/
#include "juego/mundo.h"

void mundo_iniciar(Cuerpo *c, u16 x, u16 y)
{
    c->x = x;                                        /* Donde nace.                         */
    c->y = y;
    c->z = 0;                                        /* En el suelo.                        */
    c->vz = 0;
    c->mira_izq = 0;                                 /* Mirando a la derecha.               */
    c->estado = CUERPO_PARADO;
    c->libre[0] = 0;
    c->libre[1] = 0;
}

void mundo_limitar_x(Cuerpo *c, u16 x_min, u16 x_max)
{
    if (c->x < x_min)
    {
        c->x = x_min;
    }
    else if (c->x > x_max)
    {
        c->x = x_max;
    }
}

u16 mundo_altura(const Cuerpo *c)
{
    return (u16)(c->z >> 4);                         /* 12.4 a píxeles (z nunca es negativa). */
}

void mundo_actualizar(Cuerpo *c, const Mando *m)
{
    u8 aire = (u8)(c->z > 0 || c->vz < 0);           /* ¿Ya está en el aire?                */

    /*--- 1. Salto: solo desde el suelo ---------------------------------------------*/
    if (!aire && m->saltar_pulsado)
    {
        c->vz = (s16)(-SALTO_V0);                    /* Hacia arriba (z crece al bajar vz). */
        c->z = 1;                                    /* Ya no está en el suelo.             */
        aire = 1;
    }

    /*--- 2. x: andar solo en el suelo; el aire conserva la dirección y el impulso ---*/
    if (!aire)
    {
        if (m->izquierda && !m->derecha)
        {
            c->x = (u16)(c->x - PASO_X);
            c->mira_izq = 1;
        }
        else if (m->derecha && !m->izquierda)
        {
            c->x = (u16)(c->x + PASO_X);
            c->mira_izq = 0;
        }
    }
    else
    {
        c->x = (u16)(c->x + (c->mira_izq ? -PASO_X : PASO_X)); /* Sigue su rumbo (o quieto). */
    }

    /*--- 3. y: la profundidad se puede cambiar también en el aire ------------------*/
    if (m->arriba && !m->abajo)
    {
        c->y = (u16)(c->y - PASO_Y);
    }
    else if (m->abajo && !m->arriba)
    {
        c->y = (u16)(c->y + PASO_Y);
    }

    /*--- 4. Gravedad: la altura sube mientras vz < 0 y baja después ----------------*/
    if (aire)
    {
        c->z = (s16)(c->z - c->vz);                  /* vz < 0 → z crece (sube).            */
        c->vz = (s16)(c->vz + GRAVEDAD);
        if (c->z <= 0)                               /* Tocó el suelo.                      */
        {
            c->z = 0;
            c->vz = 0;
            aire = 0;
        }
    }

    /*--- 5. Límites del carril -----------------------------------------------------*/
    if (c->x < MUNDO_X_MIN || c->x > 0x8000)         /* 0x8000: el u16 dio la vuelta por abajo. */
    {
        c->x = MUNDO_X_MIN;
    }
    else if (c->x > MUNDO_X_MAX)
    {
        c->x = MUNDO_X_MAX;
    }
    if (c->y < CARRIL_Y_MIN)
    {
        c->y = CARRIL_Y_MIN;
    }
    else if (c->y > CARRIL_Y_MAX)
    {
        c->y = CARRIL_Y_MAX;
    }

    /*--- 6. Estado (lo que elige la animación) -------------------------------------*/
    if (aire)
    {
        c->estado = (u8)(c->vz < 0 ? CUERPO_SUBIENDO : CUERPO_BAJANDO);
    }
    else if (m->izquierda != m->derecha || m->arriba != m->abajo)
    {
        c->estado = CUERPO_ANDANDO;
    }
    else
    {
        c->estado = CUERPO_PARADO;
    }
}
