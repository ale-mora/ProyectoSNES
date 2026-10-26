/*---------------------------------------------------------------------------------
    ninja.c — El ninja kendoka de prueba (ver ninja.h).

    PROPÓSITO
      Cada frame: se cuentan los contadores; si el estado tiene tiempo y lo ha
      agotado, pasa al siguiente; si está quieto o andando, mira si el objetivo
      está en rango y si puede atacar (permiso del llamador y enfriamiento a 0);
      si no, se acerca en x y en y. Luego se mueve el cuerpo y se aterriza si
      estaba derribado. Las cifras (vida, daño por rango, enfriamiento) vienen de
      juego/cifras.h; la caja del shinai, de una tabla fija.

    SE CONECTA CON
      Incluye:  enemigos/ninja.h, juego/cifras.h.
      Lo usan:  tests/prueba_ninja.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tablas en ROM)
      fases[NQ_ESTADOS]  tiempos de cada estado (frames, ventana de golpe, siguiente)
      cajas_shinai       caja de golpe del shinai (la de daño va en el propio código)
    → glosario: Rango, Turnos de ataque, Caja de golpe, Derribo
---------------------------------------------------------------------------------*/
#include "enemigos/ninja.h"
#include "juego/cifras.h"

#define ALCANCE_NINJA PF(28)         /* A 28 px en x (pies a pies) ya puede pegar.         */
#define VEL_NINJA_Y (VEL_NINJA >> 1) /* Profundidad a la mitad, como Tarou.                */
#define TOLERANCIA_Y_NINJA (TOLERANCIA_Y >> 1)  /* Se alinea en y hasta 4 px.          */
#define FRAMES_RECIBIR_NINJA 12
#define FRAMES_KO_NINJA 40
#define VEL_DERRIBO_NINJA 48         /* 3 px por frame hacia arriba al caer derribado.     */

typedef struct
{
    u8 frames;                       /* Duración; 0 = sin límite.                          */
    u8 ini;                          /* Primer frame con caja de golpe.                    */
    u8 fin;                          /* Primer frame sin caja de golpe.                    */
    u8 siguiente;                    /* Estado al acabar por tiempo.                       */
} Fase;

static const Fase fases[NQ_ESTADOS] =
{
    {0, 0, 0, NQ_QUIETO},            /* QUIETO                                             */
    {0, 0, 0, NQ_ACERCARSE},         /* ACERCARSE                                          */
    {24, 8, 12, NQ_QUIETO},          /* ATACAR: el shinai pega en 8..11                    */
    {FRAMES_RECIBIR_NINJA, 0, 0, NQ_QUIETO}, /* RECIBIR                                    */
    {0, 0, 0, NQ_EN_SUELO},          /* DERRIBADO (acaba al aterrizar)                     */
    {30, 0, 0, NQ_LEVANTARSE},       /* EN_SUELO                                           */
    {36, 0, 0, NQ_QUIETO},           /* LEVANTARSE                                         */
    {0, 0, 0, NQ_KO},                /* KO (vencido por tiempo, en su propio caso)         */
};

/* Valor absoluto de un s16 (la diferencia de posiciones puede ser negativa). */
static s16 abs16(s16 v)
{
    if (v < 0)
        return -v;
    return v;
}

/* Cambia de estado y empieza a contar desde cero. */
static void cambiar(Ninja *n, u8 nuevo)
{
    n->estado = nuevo;
    n->tiempo = 0;
    if (nuevo != NQ_ACERCARSE && nuevo != NQ_DERRIBADO && nuevo != NQ_RECIBIR)
    {
        n->c.vx_fp = 0;              /* Quieto en su sitio salvo en los estados que se mueven. */
        n->c.vy_fp = 0;
    }
    if (nuevo == NQ_LEVANTARSE)
        n->g.invulnerable = FRAMES_INVULNERABLE;
}

/* Hacia el objetivo: x hasta el alcance, y hasta alinearse. Mira hacia él. */
static void acercar(Ninja *n, const Cuerpo *objetivo)
{
    s16 dx;                          /* Objetivo menos ninja, en x.                        */
    s16 dy;                          /* Objetivo menos ninja, en y.                        */

    dx = objetivo->x_fp - n->c.x_fp;
    dy = objetivo->y_fp - n->c.y_fp;
    n->mira_izquierda = dx < 0;

    n->c.vx_fp = 0;
    if (abs16(dx) > ALCANCE_NINJA)
        n->c.vx_fp = dx < 0 ? -VEL_NINJA : VEL_NINJA;
    n->c.vy_fp = 0;
    if (abs16(dy) > TOLERANCIA_Y_NINJA)
        n->c.vy_fp = dy < 0 ? -VEL_NINJA_Y : VEL_NINJA_Y;
}

/* ¿Está el objetivo a tiro: en x dentro del alcance y en la misma profundidad? */
static u8 en_rango(const Ninja *n, const Cuerpo *objetivo)
{
    return abs16(objetivo->x_fp - n->c.x_fp) <= ALCANCE_NINJA
        && abs16(objetivo->y_fp - n->c.y_fp) <= TOLERANCIA_Y;
}

void ninja_iniciar(Ninja *n, u16 x, u16 y, u8 rango)
{
    mundo_colocar(&n->c, x, y);
    n->g.vida = cifras_vida_ninja(rango);
    n->g.invulnerable = 0;
    n->g.destello = 0;
    n->g.parada = 0;
    n->estado = NQ_QUIETO;
    n->tiempo = 0;
    n->rango = rango & 3;
    n->espera = 0;
    n->mira_izquierda = 0;
    n->vencido = 0;
}

void ninja_actualizar(Ninja *n, const Cuerpo *objetivo, u8 permiso, const Carril *carril)
{
    u8 congelado;                    /* 1 si lo para una parada de impacto este frame.     */
    u8 aterrizo;                     /* 1 si mundo_mover lo ha posado este frame.          */
    const Fase *f;

    congelado = n->g.parada != 0;
    combate_tic(&n->g);
    if (congelado)
        return;

    if (n->tiempo < 255)
        n->tiempo++;
    if (n->espera)
        n->espera--;

    f = &fases[n->estado];
    if (f->frames != 0 && n->tiempo >= f->frames)
        cambiar(n, f->siguiente);

    switch (n->estado)
    {
    case NQ_QUIETO:
    case NQ_ACERCARSE:
        if (en_rango(n, objetivo) && permiso && n->espera == 0)
        {
            cambiar(n, NQ_ATACAR);
            n->espera = COOLDOWN_NINJA;
            n->mira_izquierda = objetivo->x_fp < n->c.x_fp;
        }
        else if (en_rango(n, objetivo))
        {
            if (n->estado != NQ_QUIETO)
                cambiar(n, NQ_QUIETO);  /* A tiro pero sin permiso: espera de pie.        */
            n->mira_izquierda = objetivo->x_fp < n->c.x_fp;
        }
        else
        {
            if (n->estado != NQ_ACERCARSE)
                cambiar(n, NQ_ACERCARSE);
            acercar(n, objetivo);
        }
        break;
    case NQ_KO:
        if (n->tiempo >= FRAMES_KO_NINJA)
            n->vencido = 1;
        break;
    default:
        break;                       /* Estados forzados: nada que decidir.                */
    }

    aterrizo = mundo_mover(&n->c, carril);
    if (aterrizo && n->estado == NQ_DERRIBADO)
        cambiar(n, NQ_EN_SUELO);
}

void ninja_reaccionar(Ninja *n, u8 resultado)
{
    if (resultado == GOLPE_RETROCESO)
    {
        if (n->c.en_suelo)
            cambiar(n, NQ_RECIBIR);
        else
            cambiar(n, NQ_DERRIBADO);
    }
    else if (resultado == GOLPE_DERRIBO)
    {
        cambiar(n, NQ_DERRIBADO);
        n->c.en_suelo = 0;
        n->c.vz_fp = VEL_DERRIBO_NINJA;
    }
    else if (resultado == GOLPE_VENCE)
    {
        cambiar(n, NQ_KO);
        n->c.en_suelo = 0;
        n->c.vz_fp = VEL_DERRIBO_NINJA;
    }
}

u8 ninja_caja_golpe(const Ninja *n, CajaCuadro *salida)
{
    const Fase *f;

    f = &fases[n->estado];
    if (n->estado != NQ_ATACAR || n->tiempo < f->ini || n->tiempo >= f->fin)
        return 0;                    /* Fuera de los frames del shinai: no pega.           */

    salida->x = 36;                  /* Mirando al este; el espejo lo hace combate. Cabe en la
                                        celda de 64 px: 36 + 28 = 64, la espada no se sale.  */
    salida->y = 56;                  /* A la altura del pecho de un perro (Tarou).         */
    salida->ancho = 28;
    salida->alto = 20;
    salida->dano = DANO_SHINAI + cifras_bonus_dano(n->rango);
    salida->efecto = EFECTO_RETROCESO;
    salida->relleno[0] = 0;
    salida->relleno[1] = 0;
    return 1;
}

void ninja_caja_dano(const Ninja *n, CajaCuadro *salida)
{
    (void)n;                         /* El cuerpo entero es igual en todos los estados.    */
    salida->x = 16;
    salida->y = 8;
    salida->ancho = 32;
    salida->alto = 88;
    salida->dano = 0;
    salida->efecto = EFECTO_RETROCESO;
    salida->relleno[0] = 0;
    salida->relleno[1] = 0;
}
