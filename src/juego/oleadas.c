/*---------------------------------------------------------------------------------
    oleadas.c — Las oleadas de una zona (ver oleadas.h).

    PROPÓSITO
      Una máquina de estados pequeña: ESPERANDO → (ESCENA_ANTES) → COMBATE →
      (ESCENA_DESPUES) → ESPERANDO, y al acabar la última, COMPLETA. Lo delicado es
      el frame en que sale el último enemigo: quien llama todavía no lo ha creado,
      así que `vivos` aún no lo cuenta; por eso un frame con SPAWN nunca puede dar la
      oleada por vencida (si no, una oleada acabaría antes de empezar).

    SE CONECTA CON
      Incluye:  juego/oleadas.h.
      Lo usan:  tests/prueba_oleadas.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna propia.
    → glosario: Oleada, Bloqueo de cámara
---------------------------------------------------------------------------------*/
#include "juego/oleadas.h"

void oleadas_iniciar(Oleadas *o, u8 zona)
{
    o->zona = zona;
    o->oleada = 0;
    o->estado = OL_ESPERANDO;
    o->aparecidos = 0;
    o->espera = 0;
    o->go = 0;
    o->escena = ESC_NADA;
    o->spawn_tipo = 0;
    o->spawn_rango = 0;
    o->spawn_lado = LADO_DERECHA;
    o->spawn_x = 0;
}

/* La oleada en curso (o la próxima). Solo se llama con oleada < n_oleadas. */
static const Oleada *actual(const Oleadas *o)
{
    return &fase1_zona(o->zona)->oleadas[o->oleada];
}

/* Empieza el combate de la oleada: el primer enemigo sale en el frame siguiente. */
static void empezar_combate(Oleadas *o)
{
    o->estado = OL_COMBATE;
    o->aparecidos = 0;
    o->espera = 0;
    o->escena = ESC_NADA;
}

/* La oleada ha terminado del todo (vencida y con su escena ya jugada): pasa a la siguiente. */
static u8 cerrar_oleada(Oleadas *o)
{
    const Zona *z;

    z = fase1_zona(o->zona);
    o->oleada++;
    o->aparecidos = 0;
    o->escena = ESC_NADA;
    o->go = FRAMES_GO;               /* La flecha: hay que seguir avanzando.               */
    if (o->oleada >= z->n_oleadas)
    {
        o->estado = OL_COMPLETA;
        return OL_EV_GO | OL_EV_ZONA;
    }
    o->estado = OL_ESPERANDO;
    return OL_EV_GO;
}

/* Todos han salido y ninguno vive: vencida. Si lleva escena detrás, se juega antes de cerrar. */
static u8 vencer_oleada(Oleadas *o)
{
    const Oleada *w;

    w = actual(o);
    if (w->escena_despues != ESC_NADA)
    {
        o->estado = OL_ESCENA_DESPUES;
        o->escena = w->escena_despues;
        return OL_EV_ESCENA;
    }
    return cerrar_oleada(o);
}

/* Sale el enemigo `aparecidos` de la oleada: rellena spawn_* y deja pasar FRAMES_ENTRE_ENEMIGOS. */
static void sacar_enemigo(Oleadas *o, const Oleada *w, u16 camara)
{
    u8 lado;

    o->spawn_tipo = w->e[o->aparecidos].tipo;
    o->spawn_rango = w->e[o->aparecidos].rango;
    lado = w->lado;
    if (lado == LADO_ALTERNO)
        lado = (o->aparecidos & 1) ? LADO_IZQUIERDA : LADO_DERECHA;   /* El primero, por la derecha. */
    o->spawn_lado = lado;
    if (lado == LADO_DERECHA)
    {
        o->spawn_x = camara + ANCHO_PANTALLA + MARGEN_SPAWN;          /* Fuera, por la derecha.   */
    }
    else if (camara > MARGEN_SPAWN + 16)
    {
        o->spawn_x = camara - MARGEN_SPAWN;                           /* Fuera, por la izquierda. */
    }
    else
    {
        o->spawn_x = 16;             /* Al borde del mundo no hay «fuera» por la izquierda.      */
    }
    o->aparecidos++;
    o->espera = FRAMES_ENTRE_ENEMIGOS;
}

u8 oleadas_actualizar(Oleadas *o, u16 camara, u8 vivos)
{
    const Zona *z;
    const Oleada *w;

    if (o->go)
        o->go--;                     /* La flecha se apaga sola.                           */
    z = fase1_zona(o->zona);

    switch (o->estado)
    {
    case OL_ESPERANDO:
        if (o->oleada >= z->n_oleadas)
            return OL_EV_NADA;
        w = actual(o);
        if (camara < w->x)
            return OL_EV_NADA;       /* Aún no se ha llegado.                              */
        if (w->escena_antes != ESC_NADA)
        {
            o->estado = OL_ESCENA_ANTES;
            o->escena = w->escena_antes;
            return OL_EV_ESCENA;
        }
        empezar_combate(o);
        return OL_EV_NADA;

    case OL_COMBATE:
        w = actual(o);
        if (o->aparecidos < w->n)
        {
            if (o->espera)
                o->espera--;
            if (o->espera == 0)
            {
                sacar_enemigo(o, w, camara);
                return OL_EV_SPAWN;  /* Este frame no puede dar la oleada por vencida.     */
            }
            return OL_EV_NADA;
        }
        if (vivos == 0)
            return vencer_oleada(o);
        return OL_EV_NADA;

    default:
        return OL_EV_NADA;           /* Escenas y zona completa: no hay nada que decidir.  */
    }
}

u8 oleadas_escena_terminada(Oleadas *o)
{
    if (o->estado == OL_ESCENA_ANTES)
    {
        empezar_combate(o);
        return OL_EV_NADA;
    }
    if (o->estado == OL_ESCENA_DESPUES)
        return cerrar_oleada(o);
    return OL_EV_NADA;               /* No había escena en marcha.                         */
}

u16 oleadas_limite_camara(const Oleadas *o)
{
    const Zona *z;
    u16 fin;                         /* Hasta dónde llega la cámara en esta zona.          */

    z = fase1_zona(o->zona);
    fin = z->longitud - ANCHO_PANTALLA;
    if (o->estado == OL_COMPLETA || o->oleada >= z->n_oleadas)
        return fin;
    if (actual(o)->x < fin)
        return actual(o)->x;         /* Bloqueada en la x de la oleada (en curso o próxima). */
    return fin;
}

u8 oleadas_go_activo(const Oleadas *o)
{
    return o->go != 0;
}
