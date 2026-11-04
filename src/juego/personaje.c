/*---------------------------------------------------------------------------------
    personaje.c — Tablas de animación de Tarou y Bárbara y la elección de animación
    (ver personaje.h).

    PROPÓSITO
      Tarou no tiene arte de beat 'em up: usa las tiras del plataformas (reposo para
      la guardia y al recibir, un dibujo fijo para los tres golpes, caer para el KO),
      con los mismos ritmos que animaciones.c. Bárbara sí: una tira por animación.
      Sus golpes reparten los dibujos en la duración del golpe (GOLPE_FRAMES = 14,
      GOLPE_ULTIMO_FRAMES = 20, mundo.h) a 3 frames por dibujo, así el combate no
      cambia de números (decisión del 2026-11-04).

    SE CONECTA CON
      Incluye:  juego/personaje.h, juego/animaciones.h (enum Estado), tarou_anim.h y
                barbara_anim.h (generados por scripts/preparar_personaje.py).
      Lo usan:  ver personaje.h.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      personaje_elegido      u8          el de la partida (ver personaje.h)
      ANIMS_TAROU/_BARBARA   AnimProta[] sus tablas (static const: se quedan en la ROM)
    → glosario: Tabla de animaciones
---------------------------------------------------------------------------------*/
#include "juego/personaje.h"
#include "juego/animaciones.h"
#include "tarou_anim.h"
#include "barbara_anim.h"

u8 personaje_elegido;

/* Tarou: las tiras del plataformas (ritmos de animaciones.c). */
static const AnimProta ANIMS_TAROU[ANIM_PROTA_CANTIDAD] = {
    {ESTADO_PARADO, TAROU_REPOSO_CUADROS, 12, 1},     /* ANIM_GUARDIA: el reposo.          */
    {ESTADO_CAMINANDO, TAROU_CAMINAR_CUADROS, 6, 1},  /* ANIM_CAMINAR                      */
    {ESTADO_SALTANDO, TAROU_SALTAR_CUADROS, 6, 0},    /* ANIM_SALTAR                       */
    {ESTADO_CAYENDO, TAROU_CAER_CUADROS, 6, 0},       /* ANIM_CAER                         */
    {ESTADO_ATACANDO, TAROU_ATACAR_CUADROS, 20, 0},   /* ANIM_GOLPE_1: un dibujo fijo.     */
    {ESTADO_ATACANDO, TAROU_ATACAR_CUADROS, 20, 0},   /* ANIM_GOLPE_2                      */
    {ESTADO_ATACANDO, TAROU_ATACAR_CUADROS, 20, 0},   /* ANIM_GOLPE_3                      */
    {ESTADO_PARADO, TAROU_REPOSO_CUADROS, 12, 1},     /* ANIM_RECIBIR: reposo (parpadea).  */
    {ESTADO_CAYENDO, TAROU_CAER_CUADROS, 6, 0},       /* ANIM_KO: el dibujo de caer.       */
};

/* Bárbara: su arte de beat 'em up. */
static const AnimProta ANIMS_BARBARA[ANIM_PROTA_CANTIDAD] = {
    {BARBARA_TIRA_GUARDIA, BARBARA_GUARDIA_CUADROS, 8, 1},
    {BARBARA_TIRA_CAMINAR, BARBARA_CAMINAR_CUADROS, 6, 1},
    {BARBARA_TIRA_SALTAR, BARBARA_SALTAR_CUADROS, 6, 0},
    {BARBARA_TIRA_CAER, BARBARA_CAER_CUADROS, 6, 0},
    {BARBARA_TIRA_GOLPE_1, BARBARA_GOLPE_1_CUADROS, 3, 0},   /* 5 dibujos en 14 frames.   */
    {BARBARA_TIRA_GOLPE_2, BARBARA_GOLPE_2_CUADROS, 3, 0},   /* 5 dibujos en 14 frames.   */
    {BARBARA_TIRA_GOLPE_3, BARBARA_GOLPE_3_CUADROS, 3, 0},   /* 7 dibujos en 20 frames.   */
    {BARBARA_TIRA_RECIBIR, BARBARA_RECIBIR_CUADROS, 3, 0},   /* 6 dibujos en 16 frames.   */
    {BARBARA_TIRA_KO, BARBARA_KO_CUADROS, 6, 0},             /* Cae y se queda tumbada.   */
};

const AnimProta *personaje_anims(u8 personaje)
{
    return personaje == PERSONAJE_BARBARA ? ANIMS_BARBARA : ANIMS_TAROU;
}

u8 personaje_ancho(u8 personaje)
{
    return (u8)(personaje == PERSONAJE_BARBARA ? 112 : 96);
}

u8 personaje_alto(u8 personaje)
{
    return (u8)(personaje == PERSONAJE_BARBARA ? 96 : 64);
}

u16 personaje_alcance(u8 personaje)
{
    return (u16)(personaje == PERSONAJE_BARBARA ? ALCANCE_BARBARA : ALCANCE_TAROU);
}

u8 personaje_anim_de(const Cuerpo *c)
{
    switch (c->estado)
    {
        case CUERPO_ANDANDO:   return ANIM_CAMINAR;
        case CUERPO_SUBIENDO:  return ANIM_SALTAR;
        case CUERPO_BAJANDO:   return ANIM_CAER;
        case CUERPO_ATACANDO:                        /* El golpe del combo: 1, 2 o 3.       */
            if (c->golpe >= 3) return ANIM_GOLPE_3;
            if (c->golpe == 2) return ANIM_GOLPE_2;
            return ANIM_GOLPE_1;
        case CUERPO_RECIBIR:   return ANIM_RECIBIR;
        case CUERPO_DERRIBADO: return ANIM_KO;       /* Solo se cae sin vida (mundo.c).     */
        default:               return ANIM_GUARDIA;
    }
}

void personaje_animar(AnimEstado *e, const AnimProta *tabla, u8 anim)
{
    const AnimProta *d;

    if (anim != e->anim)
    {
        e->anim = anim;                              /* Otra animación: desde el principio. */
        e->cuadro = 0;
        e->reloj = 0;
        return;
    }
    d = tabla + e->anim;                             /* 4 bytes por entrada: sin tcc__mul.  */
    e->reloj++;
    if (e->reloj >= d->frames)
    {
        e->reloj = 0;
        e->cuadro++;
        if (e->cuadro >= d->cuadros)
        {
            e->cuadro = (u8)(d->bucle ? 0 : d->cuadros - 1);
        }
    }
}
