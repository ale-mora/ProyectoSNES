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
      fases[LQ_ESTADOS]   Fase    frames, ventana de golpe [ini, fin) y siguiente estado
      cajas[LQ_ESTADOS]   CajaCuadro  caja de golpe del estado (ancho 0 = sin caja)

    → glosario: Estado, Combo, Derribo, Caja de golpe
---------------------------------------------------------------------------------*/
#include "juego/luchador.h"
#include "juego/cifras.h"

/* Velocidades en profundidad (y): la mitad que en x (§5.5). */
#define VEL_Y_ANDAR (VEL_ANDAR_TAROU >> 1)
#define VEL_Y_CORRER (VEL_CORRER_TAROU >> 1)
#define VEL_DERRIBO_Z 48             /* 3 px por frame hacia arriba al caer derribado.     */

typedef struct
{
    u8 frames;                       /* Duración; 0 = sin límite (acaba por otra causa).   */
    u8 ini;                          /* Primer frame con caja de golpe.                    */
    u8 fin;                          /* Primer frame sin caja de golpe.                    */
    u8 siguiente;                    /* Estado al acabar por tiempo.                       */
} Fase;

/* Tiempos y ventanas de cada estado, en el orden de LuchadorEstado. */
static const Fase fases[LQ_ESTADOS] =
{
    {0, 0, 0, LQ_QUIETO},            /* QUIETO                                             */
    {0, 0, 0, LQ_ANDAR},             /* ANDAR                                              */
    {0, 0, 0, LQ_CORRER},            /* CORRER                                             */
    {0, 0, 0, LQ_SALTO},            /* SALTO (acaba al aterrizar)                         */
    {0, 0, 255, LQ_QUIETO},          /* AEREO: activo todo el vuelo                        */
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
};

/* Cajas de golpe, mirando al este, dentro de la celda de 96x64 (los pies en el centro). */
static const CajaCuadro cajas[LQ_ESTADOS] =
{
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* QUIETO */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* ANDAR */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* CORRER */
    {0, 0, 0, 0, 0, 0, {0, 0}},                                                       /* SALTO */
    {40, 24, 36, 24, DANO_AEREO_TAROU, EFECTO_DERRIBO, {0, 0}},                       /* AEREO */
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
};

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
        L->g.invulnerable = fases[nuevo].frames;
    }
    if (nuevo == LQ_LEVANTARSE)      /* Invulnerable al levantarse (§5.5).                 */
        L->g.invulnerable = FRAMES_INVULNERABLE;

    /* Quieto salvo en los estados que se mueven solos (andar, correr, saltar, volar). */
    if (nuevo != LQ_ANDAR && nuevo != LQ_CORRER && nuevo != LQ_SALTO && nuevo != LQ_AEREO
        && nuevo != LQ_LANZADO && nuevo != LQ_CARRERA_ATAQUE)
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
                L->c.vx_fp = -VEL_CORRER_TAROU;
            else
                L->c.vx_fp = VEL_CORRER_TAROU;
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
        cruceta_a_velocidad(L, m, VEL_CORRER_TAROU, VEL_Y_CORRER);
    }
    else
    {
        if (L->estado != LQ_ANDAR)
            cambiar(L, LQ_ANDAR);
        cruceta_a_velocidad(L, m, VEL_ANDAR_TAROU, VEL_Y_ANDAR);
    }
}

/* En el salto: se controla con la cruceta (sin correr) y A da el ataque aéreo. */
static void atender_salto(Luchador *L, const Mando *m)
{
    if (m->atacar_pulsado)
    {
        cambiar(L, LQ_AEREO);
        return;
    }
    cruceta_a_velocidad(L, m, VEL_ANDAR_TAROU, VEL_Y_ANDAR);
}

/* En un golpe del combo: A en la ventana de encadenado (tras su último cuadro activo). */
static void atender_combo(Luchador *L, const Mando *m)
{
    const Fase *f;

    f = &fases[L->estado];
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
    f = &fases[L->estado];
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
        if (L->estado == LQ_SALTO || L->estado == LQ_AEREO)
            cambiar(L, LQ_QUIETO);
        else if (L->estado == LQ_DERRIBADO || L->estado == LQ_LANZADO)
            cambiar(L, LQ_EN_SUELO);
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

    f = &fases[L->estado];
    caja = &cajas[L->estado];
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
