/*---------------------------------------------------------------------------------
    luchador.c — Tarou en combate (ver luchador.h).

    PROPÓSITO
      Cada estado tiene una fila en `fases` (cuántos frames dura, en qué frames
      pega, a qué estado pasa al acabar) y una caja en `cajas` (qué pega). Cada
      frame: se cuentan los contadores, se mira si el estado ha terminado por
      tiempo, se atiende al mando según el estado y se mueve el cuerpo. Los
      estados sin tiempo (AEREO, SALTO, LANZADO, DERRIBADO, KO, AGARRADO...)
      terminan al aterrizar o cuando otro luchador los cambia.

    SE CONECTA CON
      Incluye:  juego/luchador.h, juego/cifras.h.
      Lo usan:  tests/prueba_luchador.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tablas en ROM)
      fases_tarou/_barbara[LQ_ESTADOS]  Fase        frames, ventana de golpe [ini, fin) y siguiente
      cajas_tarou/_barbara[LQ_ESTADOS]  CajaCuadro   caja de golpe del estado (ancho 0 = sin caja)

    → glosario: Estado, Combo, Derribo, Caja de golpe
---------------------------------------------------------------------------------*/
#include "juego/luchador.h"
#include "juego/cifras.h"

#define VEL_DERRIBO_Z 48             /* 3 px por frame hacia arriba al caer derribado.     */

typedef struct
{
    u8 frames;                       /* Duración; 0 = sin límite (acaba por otra causa).   */
    u8 ini;                          /* Primer frame con caja de golpe.                    */
    u8 fin;                          /* Primer frame sin caja de golpe.                    */
    u8 siguiente;                    /* Estado al acabar por tiempo.                       */
} Fase;

/* Tiempos y ventanas de cada estado, en el orden de LuchadorEstado. */
static const Fase fases_tarou[LQ_ESTADOS] =
{
    {0, 0, 0, LQ_QUIETO},            /* QUIETO                                             */
    {0, 0, 0, LQ_ANDAR},             /* ANDAR                                              */
    {0, 0, 0, LQ_CORRER},            /* CORRER                                             */
    {0, 0, 0, LQ_SALTO},            /* SALTO (acaba al aterrizar)                         */
    {0, 0, 255, LQ_QUIETO},          /* AEREO: activo todo el vuelo                        */
    {0, 0, 255, LQ_QUIETO},          /* PATADA: activa todo el vuelo                       */
    {18, 6, 12, LQ_QUIETO},          /* CARRERA_ATAQUE                                     */
    {20, 4, 8, LQ_GOLPE_2},          /* GOLPE_1: la ventana de encadenado son los 12 últimos */
    {20, 4, 8, LQ_GOLPE_3},          /* GOLPE_2                                            */
    {22, 5, 9, LQ_GOLPE_4},          /* GOLPE_3                                            */
    {22, 6, 10, LQ_QUIETO},          /* GOLPE_4: el mordisco cierra el combo               */
    {24, 6, 18, LQ_QUIETO},          /* ESPECIAL                                           */
    {0, 0, 0, LQ_AGARRAR},           /* AGARRAR (acaba al soltar)                          */
    {14, 3, 8, LQ_AGARRAR},          /* GOLPE_AGARRE: vuelve a agarrar                     */
    {20, 8, 12, LQ_QUIETO},          /* LANZAR                                             */
    {0, 0, 0, LQ_AGARRADO},          /* AGARRADO (acaba al soltar)                         */
    {0, 0, 0, LQ_LANZADO},           /* LANZADO (acaba al aterrizar)                       */
    {FRAMES_RECIBIR, 0, 0, LQ_QUIETO}, /* RECIBIR                                          */
    {0, 0, 0, LQ_EN_SUELO},          /* DERRIBADO (acaba al aterrizar)                     */
    {30, 0, 0, LQ_LEVANTARSE},       /* EN_SUELO                                           */
    {36, 0, 0, LQ_QUIETO},           /* LEVANTARSE                                         */
    {0, 0, 0, LQ_KO},                /* KO (acaba por FRAMES_KO, en su propio caso)        */
    {0, 0, 0, LQ_ENTRADA},           /* ENTRADA: cae hasta aterrizar                       */
    {FRAMES_ENTRADA_GOLPE, 0, 0, LQ_QUIETO}, /* ENTRADA_GOLPE                              */
    {0, 0, 0, LQ_INVOCAR},           /* INVOCAR: dura lo que dure el apoyo (acaba por fuera) */
};

/* Cajas de golpe, mirando al este, dentro de la celda de 96x64 (los pies en el centro). */
static const CajaCuadro cajas_tarou[LQ_ESTADOS] =
{
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* QUIETO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* ANDAR */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* CORRER */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* SALTO */
    {40, 24, 36, 24, DANO_AEREO_TAROU, EFECTO_DERRIBO, {0, 0}},                       /* AEREO */
    {40, 24, 56, 24, DANO_PATADA_TAROU, EFECTO_DERRIBO, {0, 0}},                      /* PATADA: más larga */
    {48, 16, 40, 20, DANO_AEREO_TAROU, EFECTO_DERRIBO, {0, 0}},                       /* CARRERA_ATAQUE */
    {56, 20, 28, 16, DANO_COMBO_TAROU, EFECTO_RETROCESO, {0, 0}},                     /* GOLPE_1 */
    {56, 20, 28, 16, DANO_COMBO_TAROU, EFECTO_RETROCESO, {0, 0}},                     /* GOLPE_2 */
    {56, 20, 28, 16, DANO_COMBO_TAROU, EFECTO_RETROCESO, {0, 0}},                     /* GOLPE_3 */
    {56, 20, 30, 18, DANO_MORDISCO_TAROU, EFECTO_DERRIBO, {0, 0}},                    /* GOLPE_4 */
    {0, 0, 96, 64, DANO_ESPECIAL, EFECTO_RETROCESO, {0, 0}},                          /* ESPECIAL */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* AGARRAR */
    {52, 24, 20, 20, DANO_AGARRE_TAROU, EFECTO_RETROCESO, {0, 0}},                    /* GOLPE_AGARRE */
    {56, 24, 20, 20, DANO_LANZAR_TAROU, EFECTO_DERRIBO, {0, 0}},                      /* LANZAR */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* AGARRADO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* LANZADO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* RECIBIR */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* DERRIBADO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* EN_SUELO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* LEVANTARSE */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* KO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* ENTRADA */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* ENTRADA_GOLPE (se resuelve por distancia) */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* INVOCAR */
};

/* Bárbara: katana lenta y larga (celda de 96x96, pies en el centro; spec §4). Combo de 3 golpes
   (24/24/30 frames, ventana de encadenado de 12); el tajo final cierra el combo y derriba. */
static const Fase fases_barbara[LQ_ESTADOS] =
{
    {0, 0, 0, LQ_QUIETO},            /* QUIETO                                             */
    {0, 0, 0, LQ_ANDAR},             /* ANDAR                                              */
    {0, 0, 0, LQ_CORRER},            /* CORRER                                             */
    {0, 0, 0, LQ_SALTO},             /* SALTO                                              */
    {0, 0, 255, LQ_QUIETO},          /* AEREO                                              */
    {0, 0, 255, LQ_QUIETO},          /* PATADA                                             */
    {20, 8, 14, LQ_QUIETO},          /* CARRERA_ATAQUE: la embestida con el hombro         */
    {24, 6, 12, LQ_GOLPE_2},         /* GOLPE_1                                            */
    {24, 6, 12, LQ_GOLPE_3},         /* GOLPE_2                                            */
    {30, 8, 14, LQ_QUIETO},          /* GOLPE_3: el tajo cierra el combo (no hay golpe 4)  */
    {0, 0, 0, LQ_QUIETO},            /* GOLPE_4: Bárbara no lo usa                         */
    {24, 6, 18, LQ_QUIETO},          /* ESPECIAL                                           */
    {0, 0, 0, LQ_AGARRAR},           /* AGARRAR                                            */
    {16, 4, 10, LQ_AGARRAR},         /* GOLPE_AGARRE: empuñadura                           */
    {22, 8, 12, LQ_QUIETO},          /* LANZAR: judo                                       */
    {0, 0, 0, LQ_AGARRADO},          /* AGARRADO                                           */
    {0, 0, 0, LQ_LANZADO},           /* LANZADO                                            */
    {FRAMES_RECIBIR, 0, 0, LQ_QUIETO}, /* RECIBIR                                          */
    {0, 0, 0, LQ_EN_SUELO},          /* DERRIBADO                                          */
    {30, 0, 0, LQ_LEVANTARSE},       /* EN_SUELO                                           */
    {36, 0, 0, LQ_QUIETO},           /* LEVANTARSE                                         */
    {0, 0, 0, LQ_KO},                /* KO                                                 */
    {0, 0, 0, LQ_ENTRADA},           /* ENTRADA                                            */
    {FRAMES_ENTRADA_GOLPE, 0, 0, LQ_QUIETO}, /* ENTRADA_GOLPE                              */
    {0, 0, 0, LQ_INVOCAR},           /* INVOCAR: dura lo que dure el apoyo (acaba por fuera) */
};

/* Cajas de Bárbara en su celda de 96x96 (alcance hasta 48 px por delante de los pies: el borde
   de la celda). El especial es un corte circular de 128 px centrado en los pies: x = -16. */
static const CajaCuadro cajas_barbara[LQ_ESTADOS] =
{
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* QUIETO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* ANDAR */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* CORRER */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* SALTO */
    {52, 44, 40, 28, DANO_AEREO_BARBARA, EFECTO_DERRIBO, {0, 0}},                     /* AEREO */
    {44, 44, 52, 28, DANO_PATADA_BARBARA, EFECTO_DERRIBO, {0, 0}},                    /* PATADA */
    {40, 24, 56, 40, DANO_AEREO_BARBARA, EFECTO_DERRIBO, {0, 0}},                     /* CARRERA_ATAQUE */
    {52, 36, 44, 20, DANO_COMBO_BARBARA, EFECTO_RETROCESO, {0, 0}},                   /* GOLPE_1 */
    {52, 36, 44, 20, DANO_COMBO_BARBARA, EFECTO_RETROCESO, {0, 0}},                   /* GOLPE_2 */
    {44, 24, 52, 44, DANO_TAJO_BARBARA, EFECTO_DERRIBO, {0, 0}},                      /* GOLPE_3 */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* GOLPE_4 (no existe) */
    {-16, 8, 128, 84, DANO_ESPECIAL, EFECTO_RETROCESO, {0, 0}},                       /* ESPECIAL */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* AGARRAR */
    {52, 40, 28, 24, DANO_AGARRE_BARBARA, EFECTO_RETROCESO, {0, 0}},                  /* GOLPE_AGARRE */
    {52, 40, 28, 24, DANO_LANZAR_BARBARA, EFECTO_DERRIBO, {0, 0}},                    /* LANZAR */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* AGARRADO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* LANZADO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* RECIBIR */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* DERRIBADO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* EN_SUELO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* LEVANTARSE */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* KO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* ENTRADA */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* ENTRADA_GOLPE */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* INVOCAR */
};

/* Las tablas y velocidades de cada personaje. */
static const Fase *fases_de(const Luchador *L)
{
    if (L->quien == QUIEN_BARBARA)
        return fases_barbara;
    return fases_tarou;
}

static const CajaCuadro *cajas_de(const Luchador *L)
{
    if (L->quien == QUIEN_BARBARA)
        return cajas_barbara;
    return cajas_tarou;
}

static s16 vel_andar(const Luchador *L)
{
    if (L->quien == QUIEN_BARBARA)
        return VEL_ANDAR_BARBARA;
    return VEL_ANDAR_TAROU;
}

static s16 vel_correr(const Luchador *L)
{
    if (L->quien == QUIEN_BARBARA)
        return VEL_CORRER_BARBARA;
    return VEL_CORRER_TAROU;
}

/*--- Ayudas ------------------------------------------------------------------------*/

/* Los golpes del combo: si no conectan, o no se encadenan, el combo se corta. */
static u8 es_combo(u8 estado)
{
    return estado >= LQ_GOLPE_1 && estado <= LQ_GOLPE_4;
}

/* Cambia de estado y pone lo que ese estado necesita al empezar. */
static void cambiar(Luchador *L, u8 nuevo)
{
    L->estado = nuevo;
    L->tiempo = 0;                   /* Cada estado empieza a contar desde cero.           */
    L->encadenar = 0;
    L->conecto = 0;

    if (nuevo == LQ_ESPECIAL)        /* Invulnerable durante toda la animación.            */
    {
        L->pagado = 0;
        L->g.invulnerable = fases_de(L)[nuevo].frames;
    }
    if (nuevo == LQ_LEVANTARSE)      /* Invulnerable al levantarse (§5.5).                 */
        L->g.invulnerable = FRAMES_INVULNERABLE;

    /* Quieto salvo en los estados que se mueven solos (andar, correr, saltar, volar). */
    if (nuevo != LQ_ANDAR && nuevo != LQ_CORRER && nuevo != LQ_SALTO && nuevo != LQ_AEREO
        && nuevo != LQ_PATADA && nuevo != LQ_LANZADO && nuevo != LQ_CARRERA_ATAQUE)
    {
        L->c.vx_fp = 0;
        L->c.vy_fp = 0;
    }
}

/* La cruceta da la velocidad en x e y y mira hacia donde va en x. */
static void cruceta_a_velocidad(Luchador *L, const Mando *m, s16 vel_x, s16 vel_y)
{
    L->c.vx_fp = 0;
    L->c.vy_fp = 0;
    if (m->derecha && !m->izquierda)
    {
        L->c.vx_fp = vel_x;
        L->mira_izquierda = 0;
    }
    if (m->izquierda && !m->derecha)
    {
        L->c.vx_fp = -vel_x;
        L->mira_izquierda = 1;
    }
    if (m->arriba && !m->abajo)
        L->c.vy_fp = -vel_y;         /* Arriba es hacia el fondo: y menor.                 */
    if (m->abajo && !m->arriba)
        L->c.vy_fp = vel_y;
}

/*--- Qué hace cada estado con el mando ----------------------------------------------*/

/* Quieto, andando o corriendo: aquí se decide casi todo lo que puede empezar. */
static void atender_parado(Luchador *L, const Mando *m, u8 corre)
{
    u8 moviendo;                     /* 1 si hay cruceta en cualquier dirección.           */

    if (m->atacar_pulsado && m->saltar && L->c.en_suelo)  /* A+B: especial.               */
    {
        cambiar(L, LQ_ESPECIAL);
        return;
    }
    if (m->saltar_pulsado && L->c.en_suelo)               /* B: salto.                    */
    {
        mundo_saltar(&L->c);
        cambiar(L, LQ_SALTO);
        return;
    }
    if (m->atacar_pulsado)                                /* A: combo, o tajo corriendo.  */
    {
        if (L->estado == LQ_CORRER)
        {
            cambiar(L, LQ_CARRERA_ATAQUE);
            if (L->mira_izquierda)
                L->c.vx_fp = -vel_correr(L);
            else
                L->c.vx_fp = vel_correr(L);
            L->c.vy_fp = 0;
        }
        else
        {
            cambiar(L, LQ_GOLPE_1);
        }
        return;
    }

    moviendo = (u8)(m->izquierda | m->derecha | m->arriba | m->abajo);
    if (!moviendo)
    {
        if (L->estado != LQ_QUIETO)
            cambiar(L, LQ_QUIETO);
        cruceta_a_velocidad(L, m, 0, 0);
        return;
    }
    if (corre)                       /* El doble toque solo existe con cruceta horizontal.  */
    {
        if (L->estado != LQ_CORRER)
            cambiar(L, LQ_CORRER);
        cruceta_a_velocidad(L, m, vel_correr(L), vel_correr(L) >> 1);
    }
    else
    {
        if (L->estado != LQ_ANDAR)
            cambiar(L, LQ_ANDAR);
        cruceta_a_velocidad(L, m, vel_andar(L), vel_andar(L) >> 1);
    }
}

/* En el salto: se controla con la cruceta (sin correr) y A da el ataque aéreo. */
static void atender_salto(Luchador *L, const Mando *m)
{
    u8 hacia_delante;                /* 1 si la cruceta apunta hacia donde mira.           */

    if (m->atacar_pulsado)
    {
        hacia_delante = L->mira_izquierda ? m->izquierda : m->derecha;
        if (hacia_delante)
            cambiar(L, LQ_PATADA);   /* A + cruceta hacia delante: patada voladora.        */
        else
            cambiar(L, LQ_AEREO);    /* A sin cruceta: ataque aéreo de siempre.            */
        return;
    }
    cruceta_a_velocidad(L, m, vel_andar(L), vel_andar(L) >> 1);
}

/* En un golpe del combo: A en la ventana de encadenado (tras su último cuadro activo). */
static void atender_combo(Luchador *L, const Mando *m)
{
    const Fase *f;

    f = &fases_de(L)[L->estado];
    if (m->atacar_pulsado && L->tiempo >= f->fin && L->tiempo < f->frames)
        L->encadenar = 1;
}

/* Agarrando: A da un zarpazo (hasta 3 seguidos); atrás + A lanza al agarrado. */
static void atender_agarre(Luchador *L, const Mando *m)
{
    u8 atras;                        /* La cruceta hacia donde mira es «hacia delante»;
                                        la contraria, atrás.                              */

    if (!m->atacar_pulsado)
        return;
    atras = L->mira_izquierda ? m->derecha : m->izquierda;
    if (atras)
    {
        cambiar(L, LQ_LANZAR);
        return;
    }
    if (L->golpes_agarre < MAX_GOLPES_AGARRE)
    {
        L->golpes_agarre++;
        cambiar(L, LQ_GOLPE_AGARRE);
    }
}

/*--- Funciones públicas ------------------------------------------------------------*/

void luchador_iniciar(Luchador *L, u16 x, u16 y, u8 vida)
{
    luchador_iniciar_como(L, QUIEN_TAROU, x, y, vida);   /* Por defecto, Tarou.             */
}

void luchador_iniciar_como(Luchador *L, u8 quien, u16 x, u16 y, u8 vida)
{
    L->quien = quien;                /* Antes que nada: decide las tablas que se usan.     */
    mundo_colocar(&L->c, x, y);
    L->g.vida = vida;
    L->g.invulnerable = 0;
    L->g.destello = 0;
    L->g.parada = 0;
    doble_toque_iniciar(&L->d);
    L->estado = LQ_QUIETO;
    L->tiempo = 0;
    L->encadenar = 0;
    L->conecto = 0;
    L->pagado = 0;
    L->golpes_agarre = 0;
    L->mira_izquierda = 0;
    L->vencido = 0;
}

void luchador_actualizar(Luchador *L, const Mando *m, const Carril *carril)
{
    u8 congelado;                    /* 1 si lo para una parada de impacto este frame.     */
    u8 corre;                        /* 1 si el doble toque pide correr.                   */
    u8 aterrizo;                     /* 1 si mundo_mover lo ha posado este frame.          */
    const Fase *f;

    congelado = L->g.parada != 0;
    combate_tic(&L->g);              /* Baja invulnerable, destello y parada.              */
    if (congelado)
        return;                      /* Parado: el mundo sigue, él no se mueve.            */

    if (L->tiempo < 255)
        L->tiempo++;
    corre = doble_toque_actualizar(&L->d, m->izquierda, m->derecha);

    /* Si el estado ha agotado su tiempo, pasa al siguiente antes de atender el mando. */
    f = &fases_de(L)[L->estado];
    if (f->frames != 0 && L->tiempo >= f->frames)
    {
        if (es_combo(L->estado) && !(L->encadenar && L->conecto))
            cambiar(L, LQ_QUIETO);   /* Sin encadenar (o sin haber dado): se corta el combo.*/
        else
            cambiar(L, f->siguiente);
    }

    switch (L->estado)
    {
    case LQ_QUIETO:
    case LQ_ANDAR:
    case LQ_CORRER:
        atender_parado(L, m, corre);
        break;
    case LQ_SALTO:
        atender_salto(L, m);
        break;
    case LQ_AEREO:
    case LQ_PATADA:
        break;                       /* Sigue el vuelo: solo cuenta aterrizar.             */
    case LQ_GOLPE_1:
    case LQ_GOLPE_2:
    case LQ_GOLPE_3:
    case LQ_GOLPE_4:
        atender_combo(L, m);
        break;
    case LQ_AGARRAR:
        atender_agarre(L, m);
        break;
    case LQ_KO:
        if (L->tiempo >= FRAMES_KO)
            L->vencido = 1;          /* Fuera de la pelea.                                 */
        break;
    default:
        break;                       /* Estados forzados: nada que decidir.                */
    }

    aterrizo = mundo_mover(&L->c, carril);
    if (aterrizo)
    {
        if (L->estado == LQ_SALTO || L->estado == LQ_AEREO || L->estado == LQ_PATADA)
            cambiar(L, LQ_QUIETO);
        else if (L->estado == LQ_DERRIBADO || L->estado == LQ_LANZADO)
            cambiar(L, LQ_EN_SUELO);
        else if (L->estado == LQ_ENTRADA)
            cambiar(L, LQ_ENTRADA_GOLPE);   /* Aterrizó el que entra: el golpe de entrada.   */
    }
}

void luchador_marcar_conecto(Luchador *L)
{
    L->conecto = 1;
    if (L->estado == LQ_ESPECIAL && !L->pagado)
    {
        L->g.vida = combate_pagar_especial(L->g.vida);   /* El especial solo cuesta si conecta. */
        L->pagado = 1;
    }
}

void luchador_reaccionar(Luchador *L, u8 resultado)
{
    if (resultado == GOLPE_RETROCESO)
    {
        if (L->c.en_suelo)
            cambiar(L, LQ_RECIBIR);
        else
            cambiar(L, LQ_DERRIBADO);   /* En el aire, un golpe derriba (sigue cayendo).     */
    }
    else if (resultado == GOLPE_DERRIBO)
    {
        cambiar(L, LQ_DERRIBADO);
        L->c.en_suelo = 0;
        L->c.vz_fp = VEL_DERRIBO_Z;
    }
    else if (resultado == GOLPE_VENCE)
    {
        cambiar(L, LQ_KO);
        L->c.en_suelo = 0;
        L->c.vz_fp = VEL_DERRIBO_Z;
    }
}

const CajaCuadro *luchador_caja_golpe(const Luchador *L)
{
    const Fase *f;
    const CajaCuadro *caja;

    f = &fases_de(L)[L->estado];
    caja = &cajas_de(L)[L->estado];
    if (caja->ancho == 0)
        return 0;                    /* Este estado no pega.                               */
    if (L->tiempo < f->ini || L->tiempo >= f->fin)
        return 0;                    /* Pega en otros frames.                              */
    return caja;
}

void luchador_agarrar(Luchador *L)
{
    cambiar(L, LQ_AGARRAR);
    L->golpes_agarre = 0;            /* Cada agarre empieza con sus 3 golpes.              */
}

void luchador_soltar(Luchador *L)
{
    if (L->estado == LQ_AGARRAR || L->estado == LQ_GOLPE_AGARRE || L->estado == LQ_AGARRADO)
        cambiar(L, LQ_QUIETO);
}

void luchador_ser_agarrado(Luchador *L)
{
    cambiar(L, LQ_AGARRADO);
}

void luchador_lanzar(Luchador *L, s16 vx_fp, s16 vz_fp)
{
    cambiar(L, LQ_LANZADO);
    L->c.en_suelo = 0;
    L->c.vx_fp = vx_fp;
    L->c.vy_fp = 0;
    L->c.vz_fp = vz_fp;
}

void luchador_celda(const Luchador *L, Celda *celda)
{
    celda->ancho = 96;               /* Las dos celdas son de 96 px de ancho.              */
    celda->mitad_ancho = 48;         /* Los pies, en el centro.                            */
    if (L->quien == QUIEN_BARBARA)
        celda->alto = 96;            /* Bárbara: 96x96 (decisión 33 revisada, 2026-10-24). */
    else
        celda->alto = 64;            /* Tarou: 96x64.                                      */
}

void luchador_entrar(Luchador *L, s16 x_fp, s16 y_fp, u8 invulnerable)
{
    L->c.x_fp = x_fp;                /* Cae justo donde estaba el que sale.                */
    L->c.y_fp = y_fp;
    L->c.z_fp = PF(ALTURA_ENTRADA_RELEVO);
    L->c.vx_fp = 0;
    L->c.vy_fp = 0;
    L->c.vz_fp = 0;                  /* Cae con la gravedad de siempre.                    */
    L->c.en_suelo = 0;
    L->g.parada = 0;
    L->golpes_agarre = 0;
    L->vencido = 0;                  /* Si salió por un KO, vuelve a estar en pie.         */
    cambiar(L, LQ_ENTRADA);
    L->g.invulnerable = invulnerable; /* Después de cambiar: cambiar no lo toca aquí.       */
}

u8 luchador_entrada_activa(const Luchador *L)
{
    return L->estado == LQ_ENTRADA_GOLPE && L->tiempo == 0;   /* Solo el frame del aterrizaje. */
}

u8 luchador_puede_salir(const Luchador *L)
{
    switch (L->estado)
    {
    case LQ_RECIBIR:
    case LQ_DERRIBADO:
    case LQ_EN_SUELO:
    case LQ_LEVANTARSE:
    case LQ_AGARRADO:
    case LQ_LANZADO:
    case LQ_KO:
    case LQ_ENTRADA:
    case LQ_ENTRADA_GOLPE:
    case LQ_INVOCAR:                 /* Llamando a un apoyo: no hay relevo a medias.        */
    case LQ_AGARRAR:                 /* Agarrando: soltar a un enemigo a medias no tiene sentido. */
    case LQ_GOLPE_AGARRE:
    case LQ_LANZAR:
        return 0;
    default:
        return 1;
    }
}

void luchador_invocar(Luchador *L)
{
    cambiar(L, LQ_INVOCAR);          /* Quieto: cambiar() le quita la velocidad.            */
    L->g.invulnerable = 255;         /* Invulnerable hasta que acabe el apoyo (máx. 255 f). */
}

void luchador_terminar_invocar(Luchador *L)
{
    if (L->estado != LQ_INVOCAR)
        return;                      /* Solo si seguía invocando.                           */
    L->g.invulnerable = 0;
    cambiar(L, LQ_QUIETO);
}

u8 luchador_puede_invocar(const Luchador *L)
{
    return L->c.en_suelo && luchador_puede_salir(L);
}
