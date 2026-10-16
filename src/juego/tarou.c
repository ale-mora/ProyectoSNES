/*---------------------------------------------------------------------------------
    tarou.c — Física y máquina de estados del protagonista. Lógica pura.

    PROPÓSITO
      Implementa tarou_actualizar(): a partir de lo que el jugador quiere (Mando)
      y de la altura del suelo, decide el estado del frame, mueve a Tarou con
      punto fijo 12.4 y recorta la posición a los límites del nivel. Todo lo que
      aquí se calcula es verificable en el PC (tests/prueba_tarou.c): ni una
      línea toca la PPU, la OAM ni el mando real.

      Orden de cada frame (docs/diseno-demo-jugable.md §5): transiciones de
      estado → aceleración o frenado en x → corte del salto y gravedad en y →
      integrar → recortar x → aterrizar → (avanzar la animación: tarea 3.7).

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

/*--- saltar: despegue desde un estado de suelo ---------------------------------------
    Da velocidad vertical hacia arriba (negativa) y conserva vx: correr alarga el
    salto. La gravedad empieza a actuar en este mismo frame. */
static void saltar(Tarou *t)
{
    t->vy_fp = VEL_SALTO;                       /* -96: 6 px por frame hacia arriba.        */
    t->en_suelo = 0;
    cambiar_estado(t, ESTADO_SALTANDO);
}

/*--- de pie: PARADO, CAMINANDO y CORRIENDO ------------------------------------------
    Prioridad del mismo frame (§4): R > A > B > Abajo > dirección. Cada "return"
    es un botón que gana a los de abajo. */
static void manejar_pie(Tarou *t, const Mando *m)
{
    u8 direccion = (u8)(m->izquierda || m->derecha);  /* 1 si hay una dirección pulsada. */

    /* Contador de sueño: solo cuenta PARADO y sin actividad; cualquier botón lo
       pone a cero. A los FRAMES_PARA_DORMIR (10 s) se echa a dormir (§4). */
    if (t->estado == ESTADO_PARADO && !m->actividad)
    {
        t->frames_quieto++;
        if (t->frames_quieto >= FRAMES_PARA_DORMIR)
        {
            t->frames_quieto = 0;
            cambiar_estado(t, ESTADO_ECHANDOSE);
            return;
        }
    }
    else
    {
        t->frames_quieto = 0;
    }

    if (m->espada_pulsado)                      /* R: sacar o guardar la espada, quieto.    */
    {
        t->vx_fp = 0;
        cambiar_estado(t, t->espada == ESPADA_FUERA ? ESTADO_ENVAINANDO : ESTADO_DESENVAINANDO);
        return;
    }
    if (m->atacar_pulsado && t->espada == ESPADA_FUERA) /* A: solo con la espada fuera.  */
    {
        t->vx_fp = 0;                           /* El ataque frena en seco.                 */
        cambiar_estado(t, ESTADO_ATACANDO);
        return;
    }
    if (m->saltar_pulsado)                      /* B: salta conservando vx.                 */
    {
        saltar(t);
        return;
    }
    if (m->abajo)                               /* Abajo: se agacha y se detiene.           */
    {
        t->vx_fp = 0;
        cambiar_estado(t, ESTADO_AGACHADO);
        return;
    }

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

/*--- agachado: AGACHADO y AGACHADO_CAMINANDO ---------------------------------------
    Solo B, soltar Abajo y la cruceta hacen algo; R y A se ignoran (§4). */
static void manejar_agachado(Tarou *t, const Mando *m)
{
    u8 direccion = (u8)(m->izquierda || m->derecha);

    if (m->saltar_pulsado)                      /* Puede saltar agachado (se levanta).      */
    {
        saltar(t);
        return;
    }
    if (!m->abajo)                              /* Soltar Abajo: de pie, andando o parado.  */
    {
        cambiar_estado(t, t->estado == ESTADO_AGACHADO_CAMINANDO ? ESTADO_CAMINANDO : ESTADO_PARADO);
        return;
    }

    if (direccion)
    {
        if (t->estado != ESTADO_AGACHADO_CAMINANDO)
        {
            cambiar_estado(t, ESTADO_AGACHADO_CAMINANDO);
        }
        acelerar(t, m, ACEL_SUELO, FRENADO_SUELO, VEL_AGACHADO_MAX);   /* Y no cuenta.   */
    }
    else
    {
        frenar(t, FRENADO_SUELO);
        if (t->vx_fp == 0 && t->estado != ESTADO_AGACHADO)
        {
            cambiar_estado(t, ESTADO_AGACHADO);
        }
    }
}

/*--- espada: DESENVAINANDO, ENVAINANDO y ATACANDO ignoran el mando -------------------
    Esperan a que su animación termine (anim_terminada lo pone avanzar_animacion
    al final del frame anterior) y entonces vuelven a PARADO. */
static void manejar_espada(Tarou *t)
{
    if (!t->anim_terminada)
    {
        return;
    }
    if (t->estado == ESTADO_DESENVAINANDO)
    {
        t->espada = ESPADA_FUERA;               /* La espada cuenta como fuera al terminar. */
    }
    else if (t->estado == ESTADO_ENVAINANDO)
    {
        t->espada = ESPADA_ENVAINADA;
    }
    cambiar_estado(t, ESTADO_PARADO);
}

/*--- sueño: ECHANDOSE, DORMIDO y DESPERTANDO -----------------------------------------
    Cualquier actividad despierta (también mientras se está echando: directo a
    DESPERTANDO, nunca pasa por DORMIDO). Despertar ignora el mando hasta que
    termina su animación; por eso un salto pulsado al despertar no salta. */
static void manejar_sueno(Tarou *t, const Mando *m)
{
    t->frames_quieto = 0;                       /* Al despertar se vuelve a contar de cero. */
    switch (t->estado)
    {
        case ESTADO_ECHANDOSE:
            if (m->actividad)
            {
                cambiar_estado(t, ESTADO_DESPERTANDO);
            }
            else if (t->anim_terminada)
            {
                cambiar_estado(t, ESTADO_DORMIDO);
            }
            break;
        case ESTADO_DORMIDO:
            if (m->actividad)
            {
                cambiar_estado(t, ESTADO_DESPERTANDO);
            }
            break;
        default:                                /* ESTADO_DESPERTANDO                       */
            if (t->anim_terminada)
            {
                cambiar_estado(t, ESTADO_PARADO);
            }
            break;
    }
}

/*--- avanzar_animacion: un frame más del cuadro actual; cambia de cuadro cada
     frames_por_cuadro; al final, vuelve al primero (bucle) o se queda en el último
     y marca anim_terminada. Se llama al FINAL de cada frame (§5). ------------------*/
static void avanzar_animacion(Tarou *t)
{
    const AnimDef *a = &ANIMACIONES[t->estado];
    t->frames_cuadro++;
    if (t->frames_cuadro >= a->frames_por_cuadro)
    {
        t->frames_cuadro = 0;
        t->cuadro++;
        if (t->cuadro >= a->cuadros)
        {
            if (a->bucle)
            {
                t->cuadro = 0;                  /* Reposo, caminar...: vuelve a empezar.    */
            }
            else
            {
                t->cuadro = (u8)(a->cuadros - 1); /* Ataque, salto...: se queda en el último.*/
                t->anim_terminada = 1;
            }
        }
    }
}

/*--- aire: solo control aéreo (la gravedad va aparte, en tarou_actualizar) ----------
    Con una dirección pulsada acelera despacio (ACEL_AIRE); sin dirección no hay
    frenado: en el aire se conserva la velocidad que se llevaba. */
static void actualizar_aire(Tarou *t, const Mando *m)
{
    if (m->izquierda || m->derecha)
    {
        acelerar(t, m, ACEL_AIRE, ACEL_AIRE, m->correr ? VEL_CORRER_MAX : VEL_CAMINAR_MAX);
    }
}

/*--- aterrizar: al tocar el suelo, el estado depende del mando y de vx (§4) --------*/
static void aterrizar(Tarou *t, const Mando *m)
{
    t->vy_fp = 0;
    t->en_suelo = 1;
    if (m->abajo)
    {
        cambiar_estado(t, ESTADO_AGACHADO);     /* Abajo mantenida: cae agachado.           */
    }
    else if (t->vx_fp != 0)
    {
        cambiar_estado(t, m->correr ? ESTADO_CORRIENDO : ESTADO_CAMINANDO);
    }
    else
    {
        cambiar_estado(t, ESTADO_PARADO);
    }
}

void tarou_actualizar(Tarou *t, const Mando *m, u16 suelo_y)
{
    s16 suelo_fp = PF(suelo_y - ANCHO_CELDA);   /* y de la celda cuando los pies tocan.   */

    /*--- 1. Transiciones y aceleración según el estado ---------------------------*/
    switch (t->estado)
    {
        case ESTADO_PARADO:
        case ESTADO_CAMINANDO:
        case ESTADO_CORRIENDO:
            manejar_pie(t, m);
            break;
        case ESTADO_AGACHADO:
        case ESTADO_AGACHADO_CAMINANDO:
            manejar_agachado(t, m);
            break;
        case ESTADO_SALTANDO:
        case ESTADO_CAYENDO:
            actualizar_aire(t, m);
            break;
        case ESTADO_DESENVAINANDO:
        case ESTADO_ENVAINANDO:
        case ESTADO_ATACANDO:
            manejar_espada(t);
            break;
        case ESTADO_ECHANDOSE:
        case ESTADO_DORMIDO:
        case ESTADO_DESPERTANDO:
            manejar_sueno(t, m);
            break;
        default:
            break;                              /* Valor imposible: no hacer nada.        */
    }

    /*--- 2. En el aire: corte del salto, gravedad y paso de subir a caer -----------*/
    if (!t->en_suelo)
    {
        if (t->estado == ESTADO_SALTANDO && !m->saltar && t->vy_fp < VEL_SALTO_CORTE)
        {
            t->vy_fp = VEL_SALTO_CORTE;         /* Soltar B recorta la subida (altura variable).*/
        }
        t->vy_fp += GRAVEDAD;                   /* Cada frame cae un poco más rápido...        */
        if (t->vy_fp > VEL_TERMINAL)
        {
            t->vy_fp = VEL_TERMINAL;            /* ...hasta la velocidad terminal.             */
        }
        if (t->estado == ESTADO_SALTANDO && t->vy_fp >= 0)
        {
            cambiar_estado(t, ESTADO_CAYENDO);  /* Apogeo: deja de subir.                      */
        }
    }

    /*--- 3. Integrar x y recortar al nivel ------------------------------------------*/
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

    /*--- 4. Integrar y y aterrizar ---------------------------------------------------*/
    t->y_fp += t->vy_fp;
    if (!t->en_suelo && t->y_fp >= suelo_fp)
    {
        t->y_fp = suelo_fp;                     /* Nunca se hunde en el suelo.               */
        aterrizar(t, m);
    }

    /*--- 5. Avanzar la animación del estado en que termina el frame ------------------*/
    avanzar_animacion(t);
}

/*=== Cajas (§5): coordenadas del mundo, relativas a la celda de 64x64 ===============*/

void tarou_caja_cuerpo(const Tarou *t, Caja *c)
{
    u8 agachado = (u8)(t->estado == ESTADO_AGACHADO || t->estado == ESTADO_AGACHADO_CAMINANDO);
    c->x = (s16)(PF_ENTERO(t->x_fp) + 20);     /* Columnas 20..44 de la celda.              */
    c->ancho = 24;
    if (agachado)
    {
        c->y = (s16)(PF_ENTERO(t->y_fp) + 32); /* Agachado: solo la mitad de abajo (32..64).*/
        c->alto = 32;
    }
    else
    {
        c->y = (s16)(PF_ENTERO(t->y_fp) + 8);  /* De pie: filas 8..64.                      */
        c->alto = 56;
    }
}

u8 tarou_caja_golpe(const Tarou *t, Caja *c)
{
    if (t->estado != ESTADO_ATACANDO)
    {
        return 0;                               /* Solo hay golpe mientras ataca.            */
    }
    c->ancho = 28;
    c->alto = 16;
    c->y = (s16)(PF_ENTERO(t->y_fp) + 24);     /* Filas 24..40: la altura de la espada.     */
    if (t->direccion == DIR_DERECHA)
    {
        c->x = (s16)(PF_ENTERO(t->x_fp) + 44); /* Delante, a la derecha: columnas 44..72.   */
    }
    else
    {
        c->x = (s16)(PF_ENTERO(t->x_fp) - 8);  /* Espejo: 64 - 72 = -8 → columnas -8..20.   */
    }
    return 1;
}
