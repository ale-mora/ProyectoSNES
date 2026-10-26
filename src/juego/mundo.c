/*---------------------------------------------------------------------------------
    mundo.c — Movimiento en el carril (x, y, z) y combo de golpes (ver mundo.h).

    PROPÓSITO
      Cada frame, en este orden:
        A. Si hay un golpe en curso (c->golpe != 0): solo avanza el golpe (paso_golpe) y el
           cuerpo se queda quieto; no se anda ni se salta hasta acabar el combo.
        B. Si no: (1) en el suelo, A empieza el golpe 1; si no, B salta; (2) en el suelo, la
           dirección fija hacia dónde mira; en el aire se conserva el impulso; (3) y (la
           profundidad) se puede cambiar siempre; (4) la gravedad cambia vz y vz cambia z.
        C. x e y se recortan a los límites y se elige el estado (lo que anima).
      El combo (docs/diseno-beat-em-up.md §5.3): cada golpe dura GOLPE_FRAMES cuadros (el
      último, GOLPE_ULTIMO_FRAMES), su caja de golpe está activa de GOLPE_ACTIVO_DESDE a
      GOLPE_ACTIVO_HASTA (sin incluir) y, si se pulsa A a partir de GOLPE_ENCADENAR_DESDE, al
      acabar pasa al siguiente (la «cola»). Si no se pulsó, el combo termina. Cada golpe lleva
      un `id` nuevo (1..254) para que un mismo tajo no golpee dos veces al mismo enemigo.
      No hay divisiones ni multiplicaciones: solo sumas, restas y comparaciones.

    SE CONECTA CON
      Incluye:  juego/mundo.h.
      Lo usan:  pantallas/pantalla_fase1.c, juego/combate.c y tests/prueba_mundo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo en la struct Cuerpo).
    → glosario: Carril, Altura (z), Punto fijo, Combo
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
    c->golpe = 0;                                    /* Sin golpe en curso.                 */
    c->reloj = 0;
    c->cola = 0;
    c->id = 0;
}

u16 mundo_altura(const Cuerpo *c)
{
    return (u16)(c->z >> 4);                         /* 12.4 a píxeles (z nunca es negativa). */
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

u8 mundo_golpe_activo(const Cuerpo *c)
{
    return (u8)(c->golpe != 0 && c->reloj >= GOLPE_ACTIVO_DESDE && c->reloj < GOLPE_ACTIVO_HASTA);
}

/*--- nuevo_id: el siguiente identificador de golpe, siempre en 1..254 --------------*/
static u8 nuevo_id(u8 id)
{
    return (u8)(id >= 254 ? 1 : id + 1);             /* 0 y 255 no se usan: un ninja recién nacido tiene 0. */
}

/*--- paso_golpe: un cuadro del golpe en curso ----------------------------------------*/
static void paso_golpe(Cuerpo *c, const Mando *m)
{
    u8 duracion = (u8)(c->golpe >= GOLPES ? GOLPE_ULTIMO_FRAMES : GOLPE_FRAMES);

    c->reloj++;
    if (m->atacar_pulsado && c->reloj >= GOLPE_ENCADENAR_DESDE && c->golpe < GOLPES)
    {
        c->cola = 1;                                 /* A a tiempo: encadena al acabar.     */
    }
    if (c->reloj >= duracion)                        /* Acabó este golpe.                   */
    {
        if (c->cola)
        {
            c->golpe++;                              /* Pasa al siguiente del combo...      */
            c->id = nuevo_id(c->id);                 /* ...que es otro tajo.                */
        }
        else
        {
            c->golpe = 0;                            /* El combo se corta.                  */
        }
        c->reloj = 0;
        c->cola = 0;
    }
}

void mundo_actualizar(Cuerpo *c, const Mando *m)
{
    u8 aire = (u8)(c->z > 0 || c->vz < 0);           /* ¿Ya está en el aire?                */

    /*--- A. Golpe en curso: el cuerpo se queda quieto ---------------------------------*/
    if (c->golpe != 0)
    {
        paso_golpe(c, m);
    }
    else
    {
        /*--- B1. Empezar un golpe (en el suelo) o saltar ------------------------------*/
        if (!aire && m->atacar_pulsado)
        {
            c->golpe = 1;
            c->reloj = 0;
            c->cola = 0;
            c->id = nuevo_id(c->id);
        }
        else if (!aire && m->saltar_pulsado)
        {
            c->vz = (s16)(-SALTO_V0);                /* Hacia arriba (z crece al bajar vz). */
            c->z = 1;                                /* Ya no está en el suelo.             */
            aire = 1;
        }
    }

    if (c->golpe == 0)
    {
        /*--- B2. x: andar solo en el suelo; el aire conserva la dirección y el impulso -*/
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
            c->x = (u16)(c->x + (c->mira_izq ? -PASO_X : PASO_X)); /* Sigue su rumbo.       */
        }

        /*--- B3. y: la profundidad se puede cambiar también en el aire ----------------*/
        if (m->arriba && !m->abajo)
        {
            c->y = (u16)(c->y - PASO_Y);
        }
        else if (m->abajo && !m->arriba)
        {
            c->y = (u16)(c->y + PASO_Y);
        }

        /*--- B4. Gravedad: la altura sube mientras vz < 0 y baja después --------------*/
        if (aire)
        {
            c->z = (s16)(c->z - c->vz);              /* vz < 0 → z crece (sube).            */
            c->vz = (s16)(c->vz + GRAVEDAD);
            if (c->z <= 0)                           /* Tocó el suelo.                      */
            {
                c->z = 0;
                c->vz = 0;
                aire = 0;
            }
        }
    }

    /*--- C. Límites del carril y estado -------------------------------------------------*/
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

    if (c->golpe != 0)
    {
        c->estado = CUERPO_ATACANDO;
    }
    else if (aire)
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
