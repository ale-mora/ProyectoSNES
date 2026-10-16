/*---------------------------------------------------------------------------------
    tarou.c — Física y máquina de estados del protagonista. Lógica pura.

    PROPÓSITO
      Implementa tarou_actualizar(): a partir de lo que el jugador quiere (Mando)
      y de la altura del suelo, decide el estado del frame, mueve a Tarou con
      punto fijo 12.4 y recorta la posición a los límites del nivel. Todo lo que
      aquí se calcula es verificable en el PC (tests/prueba_tarou.c): ni una
      línea toca la PPU, la OAM ni el mando real.

      Orden de cada frame (docs/diseno-demo-jugable.md §5): transiciones de
      estado → aceleración o frenado en x → (gravedad en y) → integrar → recortar
      x → (aterrizar) → (avanzar la animación). Los pasos entre paréntesis llegan
      en las tareas 3.5 a 3.7 del plan.

    SE CONECTA CON
      Incluye:  juego/tarou.h, juego/nivel.h (ANCHO_NIVEL, ANCHO_CELDA),
                sistema/puntofijo.h.
      Lo usan:  pantallas/pantalla_nivel1.c (un tarou_actualizar por frame),
                tests/prueba_tarou.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales y estáticas de este archivo)
      ninguna: todo el estado viaja dentro de la struct Tarou que recibe cada
      función (así podría haber dos protagonistas sin duplicar código).

    → glosario: Estado, Máquina de estados, Punto fijo, Velocidad, aceleración y frenado
---------------------------------------------------------------------------------*/
#include "juego/tarou.h"
#include "juego/nivel.h"
#include "sistema/puntofijo.h"

#define X_MAX_FP PF(ANCHO_NIVEL - ANCHO_CELDA)   /* 7168: la celda no sale del nivel.   */

/*--- cambiar_estado: entra en `nuevo` y reinicia la animación ----------------------*/
static void cambiar_estado(Tarou *t, u8 nuevo)
{
    t->estado = nuevo;
    t->cuadro = 0;                   /* Toda animación empieza por su primer cuadro.           */
    t->frames_cuadro = 0;
    t->anim_terminada = 0;
}

void tarou_iniciar(Tarou *t, u16 x_pix, u16 suelo_y)
{
    t->x_fp = PF(x_pix);
    t->y_fp = PF(suelo_y - ANCHO_CELDA);      /* La última fila de la celda es la planta. */
    t->vx_fp = 0;
    t->vy_fp = 0;
    t->espada = ESPADA_ENVAINADA;
    t->direccion = DIR_DERECHA;
    t->en_suelo = 1;
    t->frames_quieto = 0;
    cambiar_estado(t, ESTADO_PARADO);
}

/*--- acelerar: empuja vx hacia +max o -max según la dirección pulsada ---------------
    Si vx ya supera el máximo (por ejemplo venía corriendo y ahora solo camina),
    no acelera: frena poco a poco hasta ese máximo. Así soltar Y no corta la
    velocidad de golpe. */
static void acelerar(Tarou *t, const Mando *m, s16 acel, s16 frenado, s16 max)
{
    s16 objetivo;
    if (m->derecha)
    {
        t->direccion = DIR_DERECHA;
        objetivo = max;
    }
    else
    {
        t->direccion = DIR_IZQUIERDA;
        objetivo = (s16)-max;
    }

    if (objetivo > 0)
    {
        if (t->vx_fp < objetivo)                /* Por debajo del máximo: acelera.         */
        {
            t->vx_fp += acel;
            if (t->vx_fp > objetivo) t->vx_fp = objetivo;
        }
        else if (t->vx_fp > objetivo)           /* Por encima (venía corriendo): frena.    */
        {
            t->vx_fp -= frenado;
            if (t->vx_fp < objetivo) t->vx_fp = objetivo;
        }
    }
    else
    {
        if (t->vx_fp > objetivo)                /* Hacia la izquierda, espejo de lo anterior.*/
        {
            t->vx_fp -= acel;
            if (t->vx_fp < objetivo) t->vx_fp = objetivo;
        }
        else if (t->vx_fp < objetivo)
        {
            t->vx_fp += frenado;
            if (t->vx_fp > objetivo) t->vx_fp = objetivo;
        }
    }
}

/*--- frenar: acerca vx a 0 de `frenado` en `frenado`, sin pasarse ------------------*/
static void frenar(Tarou *t, s16 frenado)
{
    if (t->vx_fp > 0)
    {
        t->vx_fp -= frenado;
        if (t->vx_fp < 0) t->vx_fp = 0;
    }
    else if (t->vx_fp < 0)
    {
        t->vx_fp += frenado;
        if (t->vx_fp > 0) t->vx_fp = 0;
    }
}

/*--- suelo: los estados de pie en los que el jugador manda sobre x -----------------*/
static void actualizar_suelo(Tarou *t, const Mando *m)
{
    u8 direccion = (u8)(m->izquierda || m->derecha);  /* 1 si hay una dirección pulsada. */

    if (direccion)
    {
        /* Caminar o correr según Y; el cambio de estado reinicia la animación solo
           si el estado es distinto, para que correr→caminar no "salte" de cuadro. */
        u8 nuevo = m->correr ? ESTADO_CORRIENDO : ESTADO_CAMINANDO;
        if (t->estado != nuevo)
        {
            cambiar_estado(t, nuevo);
        }
        acelerar(t, m, ACEL_SUELO, FRENADO_SUELO, m->correr ? VEL_CORRER_MAX : VEL_CAMINAR_MAX);
    }
    else
    {
        frenar(t, FRENADO_SUELO);
        if (t->vx_fp == 0 && t->estado != ESTADO_PARADO)
        {
            cambiar_estado(t, ESTADO_PARADO);
        }
    }
}

void tarou_actualizar(Tarou *t, const Mando *m, u16 suelo_y)
{
    (void)suelo_y;                              /* Lo usará el aterrizaje (tarea 3.5).    */

    /*--- 1. Transiciones y aceleración según el estado ---------------------------*/
    switch (t->estado)
    {
        case ESTADO_PARADO:
        case ESTADO_CAMINANDO:
        case ESTADO_CORRIENDO:
            actualizar_suelo(t, m);
            break;
        default:
            break;                              /* El resto de estados llega en 3.5-3.7.  */
    }

    /*--- 2. Integrar x y recortar al nivel ------------------------------------------*/
    t->x_fp += t->vx_fp;
    if (t->x_fp < 0)
    {
        t->x_fp = 0;                            /* Borde izquierdo: ni un píxel más allá... */
        t->vx_fp = 0;                           /* ...y la velocidad no se acumula.          */
    }
    if (t->x_fp > X_MAX_FP)
    {
        t->x_fp = X_MAX_FP;                     /* Borde derecho.                            */
        t->vx_fp = 0;
    }
}
