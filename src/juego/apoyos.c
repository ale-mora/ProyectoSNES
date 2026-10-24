/*---------------------------------------------------------------------------------
    apoyos.c — Los tres apoyos y la barra de ataque (ver apoyos.h).

    PROPÓSITO
      La barra es un contador; el apoyo en curso es otro contador de frames. La
      cronología de cada apoyo es una fila de `defs`: su duración y en qué frame
      ocurre cada cosa (255 = no ocurre). Cada frame se compara el tiempo con esa
      fila y se devuelve la máscara de eventos que toca. Las filas son de 8 bytes
      para indexar por apoyo con un desplazamiento (CLAUDE.md).

    SE CONECTA CON
      Incluye:  juego/apoyos.h, juego/cifras.h.
      Lo usan:  tests/prueba_apoyos.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tablas en ROM)
      defs[APOYOS]  Def  duración y frames de los eventos de cada apoyo (PROPUESTA, a ajustar)
    → glosario: Apoyo, Barra de ataque
---------------------------------------------------------------------------------*/
#include "juego/apoyos.h"
#include "juego/cifras.h"

#define NUNCA 255                    /* «No ocurre» en la cronología.                      */

typedef struct
{
    u8 duracion;                     /* Frames que dura el apoyo.                          */
    u8 golpe[3];                     /* Frames de sus golpes (hasta 3).                    */
    u8 levantar;                     /* Frame en que levanta a los enemigos.               */
    u8 curar;                        /* Frame en que cura.                                 */
    u8 relleno[2];                   /* Hasta 8 bytes.                                     */
} Def;

/* Cronología propuesta (se ajusta jugando):
   Lucinda: 3 tajos a los 12, 24 y 36 frames; acaba a los 48.
   Rex: levanta y cura a los 20; suelta a los 60 (el golpe, la caída); acaba a los 80.
   Fenrir: la embestida cruza y pega a los 30; acaba a los 50. */
static const Def defs[APOYOS] =
{
    {48, {12, 24, 36}, NUNCA, NUNCA, {0, 0}},     /* LUCINDA */
    {80, {60, NUNCA, NUNCA}, 20, 20, {0, 0}},     /* REX     */
    {50, {30, NUNCA, NUNCA}, NUNCA, NUNCA, {0, 0}}, /* FENRIR  */
};

void apoyos_iniciar(Apoyos *a)
{
    a->golpes = 0;
    a->disponibles = 0;              /* Nadie se ha unido todavía.                         */
    a->seleccionado = APOYO_LUCINDA;
    a->activa = 0;                   /* La barra está bloqueada hasta el monasterio.       */
    a->en_curso = 0;
    a->tiempo = 0;
}

void apoyos_activar_barra(Apoyos *a)
{
    a->activa = 1;
}

void apoyos_desbloquear(Apoyos *a, u8 apoyo)
{
    if (apoyo >= APOYOS)
        return;                      /* Un apoyo que no existe.                            */
    if (a->disponibles == 0)
        a->seleccionado = apoyo;     /* El primero que llega queda elegido.                */
    a->disponibles = (u8)(a->disponibles | (1 << apoyo));
}

void apoyos_llenar(Apoyos *a)
{
    a->golpes = BARRA_LLENA;
}

u8 apoyos_golpe_conecto(Apoyos *a)
{
    if (!a->activa || a->en_curso)
        return 0;                    /* Bloqueada, o hay un apoyo actuando: no suma.       */
    if (a->golpes >= BARRA_LLENA)
        return 0;                    /* Ya está llena: no se pasa.                         */
    a->golpes++;
    return a->golpes == BARRA_LLENA; /* 1 solo en el golpe que la llena.                   */
}

void apoyos_siguiente(Apoyos *a)
{
    u8 i;
    u8 candidato;

    if (a->disponibles == 0)
        return;                      /* Ninguno al que cambiar.                            */
    candidato = a->seleccionado;
    for (i = 0; i < APOYOS; i++)
    {
        candidato++;
        if (candidato >= APOYOS)
            candidato = 0;           /* Da la vuelta.                                      */
        if (a->disponibles & (1 << candidato))
        {
            a->seleccionado = candidato;
            return;
        }
    }
}

u8 apoyos_puede_invocar(const Apoyos *a)
{
    if (!a->activa || a->en_curso)
        return 0;
    if (a->golpes < BARRA_LLENA)
        return 0;
    return (a->disponibles & (1 << a->seleccionado)) != 0;
}

u8 apoyos_invocar(Apoyos *a)
{
    if (!apoyos_puede_invocar(a))
        return 0;
    a->en_curso = (u8)(a->seleccionado + 1);   /* 0 es «ninguno», por eso el +1.           */
    a->tiempo = 0;
    a->golpes = 0;                   /* Un solo uso: la barra vuelve a 0.                  */
    return 1;
}

u8 apoyos_actualizar(Apoyos *a)
{
    const Def *d;
    u8 eventos;
    u8 k;

    if (!a->en_curso)
        return APOYO_EV_NADA;
    d = &defs[a->en_curso - 1];
    a->tiempo++;
    eventos = APOYO_EV_NADA;
    for (k = 0; k < 3; k++)
    {
        if (d->golpe[k] == a->tiempo)
            eventos = (u8)(eventos | APOYO_EV_GOLPE);
    }
    if (d->levantar == a->tiempo)
        eventos = (u8)(eventos | APOYO_EV_LEVANTAR);
    if (d->curar == a->tiempo)
        eventos = (u8)(eventos | APOYO_EV_CURAR);
    if (a->tiempo >= d->duracion)
    {
        eventos = (u8)(eventos | APOYO_EV_FIN);
        a->en_curso = 0;             /* Acabó: la barra puede volver a llenarse.           */
        a->tiempo = 0;
    }
    return eventos;
}

u8 apoyos_dano(u8 apoyo, u8 jefe)
{
    u8 dano;

    if (apoyo == APOYO_LUCINDA)
        dano = DANO_APOYO_LUCINDA;
    else if (apoyo == APOYO_REX)
        dano = DANO_APOYO_REX;
    else
        dano = DANO_APOYO_FENRIR;
    if (jefe)
        dano = (u8)(dano >> 1);      /* Contra un jefe, la mitad.                          */
    return dano;
}

u8 apoyos_derriba(u8 apoyo, u8 jefe)
{
    (void)apoyo;                     /* Ningún apoyo derriba a un jefe; a los demás, todos.  */
    return !jefe;
}

u8 apoyos_curar_vida(u8 vida, u8 apoyo)
{
    u16 nueva;                       /* En 16 bits: 64 + 16 cabe en u8, pero no hay que fiarse. */

    if (apoyo != APOYO_REX)
        return vida;                 /* Solo Rex cura.                                     */
    nueva = (u16)vida + CURA_REX;
    if (nueva > VIDA_PROTAGONISTA)
        nueva = VIDA_PROTAGONISTA;   /* Sin pasar de la vida máxima.                       */
    return (u8)nueva;
}
