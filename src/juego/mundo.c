/*---------------------------------------------------------------------------------
    mundo.c — Movimiento en el carril (x, y, z), combo y movimientos (ver mundo.h).

    PROPÓSITO
      Cada frame, en este orden:
        0. Si está aturdido (stun), KO (vida 0), levantándose o agarrado: solo retrocede y
           cuenta el tiempo (paso_aturdido); agarrado no hace nada (lo lleva combate.c).
        A. Acciones que duran (especial, lanzar arma, agarre, golpe de agarre, lanzamiento):
           cada una avanza su reloj y, en su frame, deja un `evento`.
        B. Golpe del combo en curso: solo avanza el golpe (paso_golpe); el cuerpo se queda quieto.
        C. Si no: A + B = especial; A con arma = lanzarla; en el suelo A empieza el combo; en el
           aire A da el ataque aéreo (una vez por salto); B salta; la dirección (con el doble
           toque para correr) mueve en x en el suelo, el aire conserva el impulso; y se puede
           cambiar siempre; la gravedad cambia vz y vz cambia z.
        D. x e y se recortan a los límites y se elige el estado (lo que anima).
      El combo (docs/diseno-beat-em-up.md §5.3): cada golpe dura GOLPE_FRAMES cuadros (el
      último, GOLPE_ULTIMO_FRAMES), su caja de golpe está activa de GOLPE_ACTIVO_DESDE a
      GOLPE_ACTIVO_HASTA (sin incluir) y, si se pulsa A a partir de GOLPE_ENCADENAR_DESDE, al
      acabar pasa al siguiente (la «cola»). Cada golpe (y cada ataque aéreo o especial) lleva
      un `id` nuevo (1..254) para que un mismo tajo no golpee dos veces al mismo enemigo.
      No hay divisiones ni multiplicaciones: solo sumas, restas y comparaciones.

    SE CONECTA CON
      Incluye:  juego/mundo.h.
      Lo usan:  pantallas/pantalla_fase1.c, juego/combate.c y las pruebas de tests/.

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
    c->vida = VIDA_TAROU;                            /* Con la barra llena.                 */
    c->inv = 0;
    c->stun = 0;
    c->empuje = 0;
    c->golpes_max = GOLPES;                          /* La pantalla lo cambia (Tarou: 4).   */
    c->corre = 0;
    c->dir = 0;
    c->suelta = 255;
    c->antes = 0;
    c->aereo = 0;
    c->agarra = 0;
    c->agarre_golpes = 0;
    c->empuja = 0;
    c->accion = 0;
    c->cobro = 0;
    c->arma = ARMA_NINGUNA;
    c->evento = EVENTO_NINGUNO;
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

/*--- cortar: lo que hacía, fuera (al recibir un golpe) -------------------------------*/
static void cortar(Cuerpo *c)
{
    c->golpe = 0;                                    /* Se le corta el golpe que diera.     */
    c->reloj = 0;
    c->cola = 0;
    c->z = 0;                                        /* Cae al suelo si estaba en el aire.  */
    c->vz = 0;
    c->corre = 0;
    c->aereo = 0;
    c->accion = 0;
    if (c->agarra)
    {
        c->evento = EVENTO_SUELTA_AGARRADO;          /* Soltó al que tenía agarrado.        */
    }
}

/*--- puede_recibir: no le pegan invulnerable, en el suelo, girando ni levantándose --*/
static u8 puede_recibir(const Cuerpo *c)
{
    return (u8)(c->inv == 0 && c->vida != 0 && c->estado != CUERPO_DERRIBADO &&
                c->estado != CUERPO_ESPECIAL && c->estado != CUERPO_LEVANTARSE);
}

u8 mundo_recibir(Cuerpo *c, u8 danio, u8 hacia_izq)
{
    if (!puede_recibir(c))
    {
        return 0;                                    /* Invulnerable o ya KO: el golpe no cuenta. */
    }
    c->vida = (u8)(c->vida > danio ? c->vida - danio : 0);
    cortar(c);
    c->empuje = (u8)(hacia_izq ? 1 : 2);             /* Lo empuja hacia donde va el golpe.  */
    c->inv = INVULNERABLE_FRAMES;
    if (c->vida == 0)
    {
        c->estado = CUERPO_DERRIBADO;
        c->stun = DERRIBO_FRAMES;
    }
    else
    {
        c->estado = CUERPO_RECIBIR;
        c->stun = RECIBIR_FRAMES;
    }
    return 1;
}

u8 mundo_derribar(Cuerpo *c, u8 danio, u8 hacia_izq)
{
    if (c->estado != CUERPO_AGARRADO && !puede_recibir(c))
    {
        return 0;                                    /* (Agarrado sí: lo están lanzando.)   */
    }
    c->vida = (u8)(c->vida > danio ? c->vida - danio : 0);
    cortar(c);
    c->empuje = (u8)(hacia_izq ? 1 : 2);
    c->inv = 0;                                      /* Caído no le pegan; la invulnerabilidad */
    c->estado = CUERPO_DERRIBADO;                    /* empieza al levantarse.              */
    c->stun = DERRIBO_FRAMES;
    return 1;
}

u8 mundo_golpe_activo(const Cuerpo *c)
{
    if (c->estado == CUERPO_ESPECIAL)
    {
        return (u8)(c->accion >= ESPECIAL_ACTIVO_DESDE && c->accion < ESPECIAL_ACTIVO_HASTA);
    }
    if (c->aereo == 1)
    {
        return (u8)(c->accion >= AEREO_ACTIVO_DESDE);
    }
    return (u8)(c->golpe != 0 && c->reloj >= GOLPE_ACTIVO_DESDE && c->reloj < GOLPE_ACTIVO_HASTA);
}

u8 mundo_golpe_danio(const Cuerpo *c)
{
    if (c->estado == CUERPO_ESPECIAL || c->aereo == 1)
    {
        return 2;
    }
    return (u8)(c->golpe >= c->golpes_max ? 2 : 1);  /* 1, 1(, 1), 2.                       */
}

u8 mundo_golpe_derriba(const Cuerpo *c)
{
    return (u8)(c->estado == CUERPO_ESPECIAL || c->aereo == 1 || c->golpe >= c->golpes_max);
}

u8 mundo_golpe_alrededor(const Cuerpo *c)
{
    return (u8)(c->estado == CUERPO_ESPECIAL);
}

void mundo_cobrar_especial(Cuerpo *c)
{
    if (c->cobro)
    {
        return;                                      /* Una vez por especial.               */
    }
    c->cobro = 1;
    c->vida = (u8)(c->vida > COSTE_ESPECIAL ? c->vida - COSTE_ESPECIAL : 1);   /* Nunca mata. */
}

/*--- nuevo_id: el siguiente identificador de golpe, siempre en 1..254 --------------*/
static u8 nuevo_id(u8 id)
{
    return (u8)(id >= 254 ? 1 : id + 1);             /* 0 y 255 no se usan: un ninja recién nacido tiene 0. */
}

/*--- paso_golpe: un cuadro del golpe en curso ----------------------------------------*/
static void paso_golpe(Cuerpo *c, const Mando *m)
{
    u8 duracion = (u8)(c->golpe >= c->golpes_max ? GOLPE_ULTIMO_FRAMES : GOLPE_FRAMES);

    c->reloj++;
    if (m->atacar_pulsado && c->reloj >= GOLPE_ENCADENAR_DESDE && c->golpe < c->golpes_max)
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

/*--- paso_aturdido: un cuadro de RECIBIR, DERRIBADO o LEVANTARSE --------------------*/
static void paso_aturdido(Cuerpo *c)
{
    u8 desliza;                                      /* Píxeles por cuadro que lo empuja.   */

    if (c->estado == CUERPO_LEVANTARSE)
    {
        c->accion++;
        if (c->accion >= LEVANTARSE_FRAMES)
        {
            c->estado = CUERPO_PARADO;               /* De pie: un rato invulnerable.       */
            c->accion = 0;
            c->inv = INVULNERABLE_FRAMES;
        }
        return;
    }
    if (c->estado == CUERPO_DERRIBADO)
    {
        desliza = (u8)(c->stun > DERRIBO_FRAMES - 10 ? 3 : 0);    /* Sale despedido.       */
    }
    else
    {
        desliza = (u8)(c->stun > RECIBIR_FRAMES - 6 ? 2 : 0);     /* Un pequeño retroceso. */
    }
    if (c->empuje == 1)
    {
        c->x = (u16)(c->x > desliza ? c->x - desliza : 0);
    }
    else
    {
        c->x = (u16)(c->x + desliza);
    }
    if (c->stun != 0)
    {
        c->stun--;
    }
    if (c->stun == 0 && c->vida != 0)
    {
        if (c->estado == CUERPO_DERRIBADO)
        {
            c->estado = CUERPO_LEVANTARSE;           /* Se levanta (y luego, invulnerable). */
            c->accion = 0;
        }
        else
        {
            c->estado = CUERPO_PARADO;               /* Se recupera.                        */
        }
    }
}

/*--- doble_toque: la dirección de este frame y, si se repite a tiempo, correr --------*/
static u8 doble_toque(Cuerpo *c, const Mando *m)
{
    u8 d = 0;

    if (m->izquierda && !m->derecha)
    {
        d = 1;
    }
    else if (m->derecha && !m->izquierda)
    {
        d = 2;
    }
    if (d == 0)
    {
        c->corre = 0;                                /* Al soltar, deja de correr.          */
        if (c->suelta != 255)
        {
            c->suelta++;
        }
    }
    else if (d != c->antes)                          /* Acaba de pulsar esta dirección.     */
    {
        c->corre = (u8)(d == c->dir && c->suelta <= VENTANA_DOBLE_TOQUE);
        c->dir = d;
        c->suelta = 0;
    }
    c->antes = d;
    return d;
}

/*--- acciones: un frame de lo que dura (devuelve 1 si el frame ya está hecho) ---------*/
static u8 acciones(Cuerpo *c, const Mando *m)
{
    u8 atras;

    switch (c->estado)
    {
        case CUERPO_ESPECIAL:
            c->accion++;
            if (c->accion >= ESPECIAL_FRAMES)
            {
                c->estado = CUERPO_PARADO;
                c->accion = 0;
            }
            return 1;

        case CUERPO_LANZAR_ARMA:
            c->accion++;
            if (c->accion == LANZAR_ARMA_SUELTA)
            {
                c->evento = EVENTO_LANZA_ARMA;       /* objetos.c crea el proyectil.        */
            }
            if (c->accion >= LANZAR_ARMA_FRAMES)
            {
                c->estado = CUERPO_PARADO;
                c->accion = 0;
            }
            return 1;

        case CUERPO_AGARRANDO:
            c->accion++;
            atras = (u8)(c->mira_izq ? (m->derecha && !m->izquierda) : (m->izquierda && !m->derecha));
            if (m->atacar_pulsado && atras)
            {
                c->estado = CUERPO_LANZANDO;         /* Atrás + A: lanzamiento.             */
                c->accion = 0;
            }
            else if (m->atacar_pulsado)
            {
                c->estado = CUERPO_GOLPE_AGARRE;     /* A: golpe de agarre.                 */
                c->accion = 0;
            }
            else if (c->accion >= AGARRE_MAX)
            {
                c->estado = CUERPO_PARADO;           /* Se le escapa.                       */
                c->accion = 0;
                c->evento = EVENTO_SUELTA_AGARRADO;
            }
            return 1;

        case CUERPO_GOLPE_AGARRE:
            c->accion++;
            if (c->accion == GOLPE_AGARRE_IMPACTO)
            {
                c->evento = EVENTO_GOLPE_AGARRE;
            }
            if (c->accion >= GOLPE_AGARRE_FRAMES)
            {
                c->agarre_golpes++;
                c->accion = 0;
                if (c->agarre_golpes >= AGARRE_GOLPES)
                {
                    c->estado = CUERPO_PARADO;       /* Tras el tercero lo suelta (derribado). */
                    c->evento = EVENTO_SUELTA_AGARRADO;
                }
                else
                {
                    c->estado = CUERPO_AGARRANDO;    /* Sigue agarrado.                     */
                }
            }
            return 1;

        case CUERPO_LANZANDO:
            c->accion++;
            if (c->accion == LANZAR_SUELTA)
            {
                c->evento = EVENTO_LANZA_AGARRADO;
            }
            if (c->accion >= LANZAR_FRAMES)
            {
                c->estado = CUERPO_PARADO;
                c->accion = 0;
            }
            return 1;

        default:
            return 0;
    }
}

void mundo_actualizar(Cuerpo *c, const Mando *m)
{
    u8 aire = (u8)(c->z > 0 || c->vz < 0);           /* ¿Ya está en el aire?                */
    u8 d, paso;

    c->evento = EVENTO_NINGUNO;                      /* Los eventos duran un frame.         */
    if (c->inv != 0)
    {
        c->inv--;                                    /* La invulnerabilidad se gasta sola.  */
    }
    if (c->estado == CUERPO_AGARRADO || c->estado == CUERPO_VICTORIA)
    {
        return;                                      /* Lo lleva otro (combate.c / la pantalla). */
    }
    if (c->stun != 0 || c->vida == 0 || c->estado == CUERPO_LEVANTARSE ||
        (c->estado == CUERPO_DERRIBADO))
    {
        paso_aturdido(c);
        if (c->x < MUNDO_X_MIN || c->x > 0x8000)
        {
            c->x = MUNDO_X_MIN;                      /* Los empujones no sacan del nivel.   */
        }
        else if (c->x > MUNDO_X_MAX)
        {
            c->x = MUNDO_X_MAX;
        }
        return;
    }
    if (acciones(c, m))
    {
        return;                                      /* Especial, arma, agarre o lanzamiento. */
    }
    if (c->agarra && c->estado == CUERPO_PARADO)
    {
        c->agarra = 0;                               /* (combate.c ya lo soltó.)            */
        c->agarre_golpes = 0;
    }

    /*--- B. Golpe en curso: el cuerpo se queda quieto ---------------------------------*/
    if (c->golpe != 0)
    {
        paso_golpe(c, m);
    }
    else
    {
        /*--- C1. Especial, arma, golpe, ataque aéreo o salto -------------------------*/
        if (!aire && ((m->atacar_pulsado && m->saltar) || (m->saltar_pulsado && m->atacar)))
        {
            c->estado = CUERPO_ESPECIAL;             /* A + B: el especial.                 */
            c->accion = 0;
            c->cobro = 0;
            c->corre = 0;
            c->id = nuevo_id(c->id);
            return;
        }
        if (m->atacar_pulsado && c->arma != ARMA_NINGUNA && !aire)
        {
            c->estado = CUERPO_LANZAR_ARMA;          /* Con arma: A la lanza.               */
            c->accion = 0;
            c->corre = 0;
            return;
        }
        if (!aire && m->atacar_pulsado)
        {
            c->golpe = 1;
            c->reloj = 0;
            c->cola = 0;
            c->corre = 0;
            c->id = nuevo_id(c->id);
        }
        else if (aire && m->atacar_pulsado && c->aereo == 0)
        {
            c->aereo = 1;                            /* Ataque aéreo: una vez por salto.    */
            c->accion = 0;
            c->id = nuevo_id(c->id);
        }
        else if (!aire && m->saltar_pulsado)
        {
            c->vz = (s16)(-SALTO_V0);                /* Hacia arriba (z crece al bajar vz). */
            c->z = 1;                                /* Ya no está en el suelo.             */
            aire = 1;
            c->aereo = 0;
        }
    }

    d = doble_toque(c, m);
    if (c->golpe == 0)
    {
        /*--- C2. x: andar o correr en el suelo; el aire conserva la dirección e impulso -*/
        paso = (u8)(c->corre ? PASO_CORRER : PASO_X);
        if (!aire)
        {
            if (d == 1)
            {
                c->x = (u16)(c->x - paso);
                c->mira_izq = 1;
            }
            else if (d == 2)
            {
                c->x = (u16)(c->x + paso);
                c->mira_izq = 0;
            }
        }
        else
        {
            c->x = (u16)(c->mira_izq ? c->x - paso : c->x + paso);   /* Sigue su rumbo.     */
            if (c->aereo == 1)
            {
                c->accion++;                         /* El ataque aéreo cuenta sus cuadros. */
            }
        }

        /*--- C3. y: la profundidad se puede cambiar también en el aire ----------------*/
        if (m->arriba && !m->abajo)
        {
            c->y = (u16)(c->y - PASO_Y);
        }
        else if (m->abajo && !m->arriba)
        {
            c->y = (u16)(c->y + PASO_Y);
        }

        /*--- C4. Gravedad: la altura sube mientras vz < 0 y baja después --------------*/
        if (aire)
        {
            c->z = (s16)(c->z - c->vz);              /* vz < 0 → z crece (sube).            */
            c->vz = (s16)(c->vz + GRAVEDAD);
            if (c->z <= 0)                           /* Tocó el suelo.                      */
            {
                c->z = 0;
                c->vz = 0;
                aire = 0;
                c->aereo = 0;                        /* El ataque aéreo acaba al aterrizar. */
            }
        }
    }

    /*--- D. Límites del carril y estado -------------------------------------------------*/
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
        c->estado = (u8)(c->aereo == 1 ? CUERPO_AEREO : (c->vz < 0 ? CUERPO_SUBIENDO : CUERPO_BAJANDO));
    }
    else if (d != 0 && c->corre)
    {
        c->estado = CUERPO_CORRIENDO;
    }
    else if (d != 0 || m->arriba != m->abajo)
    {
        c->estado = CUERPO_ANDANDO;
    }
    else
    {
        c->estado = CUERPO_PARADO;
    }
}
