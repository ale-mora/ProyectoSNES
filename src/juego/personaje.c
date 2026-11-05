/*---------------------------------------------------------------------------------
    personaje.c — Tablas de animación de Tarou y Bárbara y la elección de animación
    (ver personaje.h).

    PROPÓSITO
      Tarou tiene arte de beat 'em up para la guardia, los tres golpes, recibir y el KO
      (2026-11-04); camina, salta y cae con las tiras del plataformas. Bárbara: una tira
      por animación.
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

/* Tarou: su arte de beat 'em up (2026-11-04) para la guardia, los golpes, recibir y el KO;
   caminar, saltar y caer siguen siendo las tiras del plataformas (ritmos de animaciones.c).
   Los golpes reparten sus dibujos a 3 frames, como los de Bárbara. */
static const AnimProta ANIMS_TAROU[ANIM_PROTA_CANTIDAD] = {
    {TAROU_TIRA_GUARDIA_BU, TAROU_GUARDIA_BU_CUADROS, 6, 1},   /* ANIM_GUARDIA              */
    {ESTADO_CAMINANDO, TAROU_CAMINAR_CUADROS, 4, 1},          /* ANIM_CAMINAR (6 hasta el 2026-11-05: más ritmo). */
    {ESTADO_SALTANDO, TAROU_SALTAR_CUADROS, 6, 0},            /* ANIM_SALTAR               */
    {ESTADO_CAYENDO, TAROU_CAER_CUADROS, 6, 0},               /* ANIM_CAER                 */
    {TAROU_TIRA_GOLPE_1, TAROU_GOLPE_1_CUADROS, 2, 0},        /* 5 dibujos en 12 frames.   */
    {TAROU_TIRA_GOLPE_2, TAROU_GOLPE_2_CUADROS, 2, 0},        /* 5 dibujos en 12 frames.   */
    {TAROU_TIRA_GOLPE_3, TAROU_GOLPE_3_CUADROS, 2, 0},        /* 7 en 16: salto y mordisco. */
    {TAROU_TIRA_RECIBIR, TAROU_RECIBIR_CUADROS, 2, 0},        /* 5 dibujos en 12 frames.   */
    {TAROU_TIRA_KO, TAROU_KO_CUADROS, 6, 0},                  /* Cae y se queda tumbado.   */
    /* Los movimientos nuevos: los que ya tienen arte (punto 8 del 2026-11-05: golpe 4,
       derribado, levantarse y victoria) con su tira; los demás, con las que ya tenía. */
    {ESTADO_CORRIENDO, TAROU_CORRER_CUADROS, 3, 1},           /* ANIM_CORRER (la del plataformas). */
    {TAROU_TIRA_GOLPE_3, TAROU_GOLPE_3_CUADROS, 2, 0},        /* ANIM_AEREO                */
    {TAROU_TIRA_GUARDIA_BU, 1, 8, 1},                         /* ANIM_AGARRE               */
    {TAROU_TIRA_GOLPE_1, TAROU_GOLPE_1_CUADROS, 2, 0},        /* ANIM_GOLPE_AGARRE         */
    {TAROU_TIRA_GOLPE_2, TAROU_GOLPE_2_CUADROS, 3, 0},        /* ANIM_LANZAMIENTO          */
    {TAROU_TIRA_GOLPE_3, TAROU_GOLPE_3_CUADROS, 3, 0},        /* ANIM_ESPECIAL             */
    {TAROU_TIRA_GOLPE_1, TAROU_GOLPE_1_CUADROS, 2, 0},        /* ANIM_LANZAR_ARMA          */
    {TAROU_TIRA_DERRIBADO, TAROU_DERRIBADO_CUADROS, 5, 0},    /* ANIM_DERRIBADO: 30 frames. */
    {TAROU_TIRA_LEVANTARSE, TAROU_LEVANTARSE_CUADROS, 2, 0},  /* ANIM_LEVANTARSE: 12 de 16. */
    {TAROU_TIRA_RECIBIR, TAROU_RECIBIR_CUADROS, 6, 1},        /* ANIM_AGARRADA             */
    {TAROU_TIRA_VICTORIA, TAROU_VICTORIA_CUADROS, 6, 0},      /* ANIM_VICTORIA             */
    {TAROU_TIRA_GOLPE_4, TAROU_GOLPE_4_CUADROS, 2, 0},        /* ANIM_GOLPE_4: 6 en 16.    */
};

/* Bárbara: su arte de beat 'em up. */
static const AnimProta ANIMS_BARBARA[ANIM_PROTA_CANTIDAD] = {
    {BARBARA_TIRA_GUARDIA, BARBARA_GUARDIA_CUADROS, 6, 1},
    {BARBARA_TIRA_CAMINAR, BARBARA_CAMINAR_CUADROS, 4, 1},   /* (6 hasta el 2026-11-05: más ritmo.) */
    {BARBARA_TIRA_SALTAR, BARBARA_SALTAR_CUADROS, 6, 0},
    {BARBARA_TIRA_CAER, BARBARA_CAER_CUADROS, 6, 0},
    {BARBARA_TIRA_GOLPE_1, BARBARA_GOLPE_1_CUADROS, 2, 0},   /* 5 dibujos en 12 frames.   */
    {BARBARA_TIRA_GOLPE_2, BARBARA_GOLPE_2_CUADROS, 2, 0},   /* 5 dibujos en 12 frames.   */
    {BARBARA_TIRA_GOLPE_3, BARBARA_GOLPE_3_CUADROS, 2, 0},   /* 7 dibujos en 16 frames.   */
    {BARBARA_TIRA_RECIBIR, BARBARA_RECIBIR_CUADROS, 2, 0},   /* 6 dibujos en 12 frames.   */
    {BARBARA_TIRA_KO, BARBARA_KO_CUADROS, 6, 0},             /* Cae y se queda tumbada.   */
    {BARBARA_TIRA_CORRER, BARBARA_CORRER_CUADROS, 3, 1},     /* ANIM_CORRER               */
    {BARBARA_TIRA_AEREO, BARBARA_AEREO_CUADROS, 3, 0},       /* ANIM_AEREO                */
    {BARBARA_TIRA_AGARRE, BARBARA_AGARRE_CUADROS, 6, 1},     /* ANIM_AGARRE               */
    {BARBARA_TIRA_GOLPE_AGARRE, BARBARA_GOLPE_AGARRE_CUADROS, 4, 0},   /* 3 en 12 frames. */
    {BARBARA_TIRA_LANZAMIENTO, BARBARA_LANZAMIENTO_CUADROS, 2, 0},     /* 7 en 18 frames. */
    {BARBARA_TIRA_ESPECIAL, BARBARA_ESPECIAL_CUADROS, 3, 0},           /* 8 en 24 frames. */
    {BARBARA_TIRA_LANZAR_ARMA, BARBARA_LANZAR_ARMA_CUADROS, 2, 0},     /* 5 en 12 frames. */
    {BARBARA_TIRA_DERRIBADA, BARBARA_DERRIBADA_CUADROS, 4, 0},         /* 7 en 30 frames. */
    {BARBARA_TIRA_LEVANTARSE, BARBARA_LEVANTARSE_CUADROS, 3, 0},       /* 5 en 16 frames. */
    {BARBARA_TIRA_AGARRADA, BARBARA_AGARRADA_CUADROS, 8, 1},
    {BARBARA_TIRA_VICTORIA, BARBARA_VICTORIA_CUADROS, 6, 0},
    {BARBARA_TIRA_GOLPE_3, BARBARA_GOLPE_3_CUADROS, 3, 0},   /* ANIM_GOLPE_4 (no lo usa). */
};

/* Los apoyos: la carrera de entrada en bucle para todo, y el ataque en ANIM_ESPECIAL
   (pantalla_fase1.c les pone CUERPO_CORRIENDO y luego CUERPO_ESPECIAL). */
#define APOYO_ENTRA {APOYO_TIRA_ENTRADA, 4, 4, 1}
static const AnimProta ANIMS_APOYO[ANIM_PROTA_CANTIDAD] = {
    APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA,
    APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA,
    APOYO_ENTRA, APOYO_ENTRA,
    {APOYO_TIRA_ATAQUE, 6, 6, 0},                            /* ANIM_ESPECIAL: 36 frames. */
    APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA, APOYO_ENTRA,
};

/* La celda de cada uno (Tarou, Bárbara, Lucinda, Rex, Fenrir). */
static const u8 ANCHO_CELDA[5] = {96, 112, 96, 96, 112};
static const u8 ALTO_CELDA[5] = {64, 96, 96, 96, 112};

const AnimProta *personaje_anims(u8 personaje)
{
    if (personaje == PERSONAJE_TAROU)
    {
        return ANIMS_TAROU;
    }
    return personaje == PERSONAJE_BARBARA ? ANIMS_BARBARA : ANIMS_APOYO;
}

u8 personaje_ancho(u8 personaje)
{
    return ANCHO_CELDA[personaje];
}

u8 personaje_alto(u8 personaje)
{
    return ALTO_CELDA[personaje];
}

u16 personaje_alcance(u8 personaje)
{
    return (u16)(personaje == PERSONAJE_BARBARA ? ALCANCE_BARBARA : ALCANCE_TAROU);
}

u8 personaje_golpes(u8 personaje)
{
    return (u8)(personaje == PERSONAJE_BARBARA ? GOLPES : GOLPES_TAROU);
}

u8 personaje_anim_de(const Cuerpo *c)
{
    switch (c->estado)
    {
        case CUERPO_ANDANDO:   return ANIM_CAMINAR;
        case CUERPO_CORRIENDO: return ANIM_CORRER;
        case CUERPO_AEREO:     return ANIM_AEREO;
        case CUERPO_AGARRANDO: return ANIM_AGARRE;
        case CUERPO_GOLPE_AGARRE: return ANIM_GOLPE_AGARRE;
        case CUERPO_LANZANDO:  return ANIM_LANZAMIENTO;
        case CUERPO_ESPECIAL:  return ANIM_ESPECIAL;
        case CUERPO_LANZAR_ARMA: return ANIM_LANZAR_ARMA;
        case CUERPO_LEVANTARSE: return ANIM_LEVANTARSE;
        case CUERPO_AGARRADO:  return ANIM_AGARRADA;
        case CUERPO_VICTORIA:  return ANIM_VICTORIA;
        case CUERPO_SUBIENDO:  return ANIM_SALTAR;
        case CUERPO_BAJANDO:   return ANIM_CAER;
        case CUERPO_ATACANDO:                        /* El golpe del combo: 1..4.           */
            if (c->golpe >= 4) return ANIM_GOLPE_4;
            if (c->golpe >= 3) return ANIM_GOLPE_3;
            if (c->golpe == 2) return ANIM_GOLPE_2;
            return ANIM_GOLPE_1;
        case CUERPO_RECIBIR:   return ANIM_RECIBIR;
        case CUERPO_DERRIBADO: return (u8)(c->vida == 0 ? ANIM_KO : ANIM_DERRIBADO);
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
