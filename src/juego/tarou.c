/*---------------------------------------------------------------------------------
    tarou.c — Física y máquina de estados del protagonista. Lógica pura.

    PROPÓSITO
      Implementa tarou_actualizar(): a partir de lo que el jugador quiere (Mando)
      y de lo que hay en el escenario (juego/colision.h), decide el estado del
      frame, mueve a Tarou con punto fijo 12.4 y lo para contra paredes, suelos,
      plataformas y techos. Todo lo que
      aquí se calcula es verificable en el PC (tests/prueba_tarou.c): ni una
      línea toca la PPU, la OAM ni el mando real.

      Orden de cada frame (docs/diseno-demo-jugable.md §5): transiciones de
      estado → aceleración o frenado en x → corte del salto y gravedad en y →
      mover en x (paredes) → mover en y (suelo, techo, caer) → avanzar la
      animación. X e Y se resuelven por separado: es la forma clásica (y la más
      sencilla) de que las esquinas no atrapen al personaje.

    SE CONECTA CON
      Incluye:  juego/tarou.h, juego/nivel.h (ANCHO_NIVEL, ANCHO_CELDA, ALTO_CELDA),
                juego/colision.h (qué hay en cada píxel), sistema/puntofijo.h.
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
#include "juego/colision.h"
#include "sistema/puntofijo.h"

#define X_MAX_FP PF(ANCHO_NIVEL - ANCHO_CELDA)   /* 31232 (1952 px): la celda no sale.  */

/*=== Cajas (§5): coordenadas del mundo, relativas a la celda de 96x64 ===============
    Medidas sobre el arte de Pixellab (perro de unos 64x58 px de pie, centrado en la
    celda, pies en la fila 64): el cuerpo deja fuera la cola y las orejas; la espada
    va en la boca, así que el golpe sale por delante del morro. */
#define CUERPO_X 24                 /* Columnas 24..76: lomo y patas, sin la cola.        */
#define CUERPO_ANCHO 52
#define CUERPO_Y 14                 /* De pie: filas 14..64.                              */
#define CUERPO_ALTO 50
#define CUERPO_Y_AGACHADO 28        /* Agachado (Sneak, unos 46 px de alto): filas 28..64.*/
#define CUERPO_ALTO_AGACHADO 36
#define GOLPE_X 60                  /* Mirando a la derecha: columnas 60..88, del morro   */
#define GOLPE_ANCHO 28              /* a la punta de la espada y algo más.                */
#define GOLPE_Y 20                  /* Filas 20..36: la altura de la boca y la espada.    */
#define GOLPE_ALTO 16

/* Los dos puntos de apoyo: dos "pies" dentro del cuerpo, a 28 px uno del otro. Tarou
   se sostiene mientras CUALQUIERA de los dos pise algo, y cae cuando los dos están
   en el aire. Con los pies tan juntos, un foso de 96 px exige saltar 68 px en el
   aire: caminando (1 px por cuadro, unos 48 cuadros de salto) no llega; corriendo
   (2 px por cuadro) sí. → docs/diseno-tutorial-movimiento.md §2 */
#define PIE_ATRAS (CUERPO_X + 12)   /* Columna 36 de la celda.                            */
#define PIE_DELANTE (CUERPO_X + 40) /* Columna 64.                                        */


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
    t->y_fp = PF(suelo_y - ALTO_CELDA);       /* La última fila de la celda es la planta. */
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

/*--- cabe_de_pie: ¿puede levantarse aquí? ------------------------------------------
    Mira si la parte de arriba del cuerpo DE PIE (las filas que el cuerpo agachado
    no ocupa) toca algo sólido, en sus dos bordes y en el centro. En el túnel del
    tutorial (48 px de alto) de pie no cabe (50 px) y agachado sí (36 px). */
static u8 cabe_de_pie(const Tarou *t)
{
    s16 x = PF_ENTERO(t->x_fp);
    s16 arriba = (s16)(PF_ENTERO(t->y_fp) + CUERPO_Y);        /* Cabeza de pie.            */
    s16 abajo = (s16)(PF_ENTERO(t->y_fp) + CUERPO_Y_AGACHADO - 1); /* Cabeza agachado, -1. */
    u8 i;
    s16 y;
    for (i = 0; i < 2; i++)                     /* Dos filas: la de arriba y la de abajo.  */
    {
        y = i ? abajo : arriba;
        if (colision_solido((u16)(x + CUERPO_X), (u16)y)
            || colision_solido((u16)(x + CUERPO_X + CUERPO_ANCHO / 2), (u16)y)
            || colision_solido((u16)(x + CUERPO_X + CUERPO_ANCHO - 1), (u16)y))
        {
            return 0;                           /* Hay techo: no cabe.                      */
        }
    }
    return 1;
}

/*--- agachado: AGACHADO y AGACHADO_CAMINANDO ---------------------------------------
    Solo B, soltar Abajo y la cruceta hacen algo; R y A se ignoran (§4). Bajo un
    techo bajo es como si Abajo siguiera pulsado: ni se levanta ni salta. */
static void manejar_agachado(Tarou *t, const Mando *m)
{
    u8 direccion = (u8)(m->izquierda || m->derecha);
    u8 cabe = cabe_de_pie(t);                   /* ¿Hay sitio para ponerse de pie?          */

    if (m->saltar_pulsado && cabe)              /* Puede saltar agachado (se levanta).      */
    {
        saltar(t);
        return;
    }
    if (!m->abajo && cabe)                      /* Soltar Abajo: de pie, andando o parado.  */
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

/*--- borde_choca: ¿toca algo sólido la columna x del cuerpo, de `arriba` a `pies`? ---
    Mira un punto cada 16 px (una celda) y el último píxel antes de los pies: así no
    se escapa ninguna celda, por fina que sea la pared. */
static u8 borde_choca(u16 x, s16 arriba, s16 pies)
{
    s16 y;
    if (arriba < 0)
    {
        arriba = 0;                             /* Por encima de la pantalla no hay nada.   */
    }
    for (y = arriba; y < pies; y += CELDA_COLISION)
    {
        if (colision_solido(x, (u16)y))
        {
            return 1;
        }
    }
    return (u8)(pies > 0 && colision_solido(x, (u16)(pies - 1)));   /* Justo sobre los pies. */
}

/*--- arriba_cuerpo: fila de la cabeza (depende de si va agachado) -------------------*/
static s16 arriba_cuerpo(const Tarou *t)
{
    if (t->estado == ESTADO_AGACHADO || t->estado == ESTADO_AGACHADO_CAMINANDO)
    {
        return (s16)(PF_ENTERO(t->y_fp) + CUERPO_Y_AGACHADO);
    }
    return (s16)(PF_ENTERO(t->y_fp) + CUERPO_Y);
}

/*--- mover_x: integrar x y pararse contra las paredes --------------------------------
    Solo cuenta lo SÓLIDO (las plataformas y las zarzas se atraviesan de lado). Si el
    borde delantero del cuerpo entra en una celda sólida, se le pega por fuera:
    (borde & 0xFFF0) es el comienzo de esa celda (borrar los 4 bits bajos = redondear
    hacia abajo a múltiplo de 16). */
static void mover_x(Tarou *t)
{
    s16 x, pies, arriba;
    u16 borde;
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
    x = PF_ENTERO(t->x_fp);
    arriba = arriba_cuerpo(t);
    pies = (s16)(PF_ENTERO(t->y_fp) + ALTO_CELDA);   /* La fila justo bajo las patas.      */
    if (t->vx_fp > 0)
    {
        borde = (u16)(x + CUERPO_X + CUERPO_ANCHO - 1);  /* Hacia la derecha: el borde derecho. */
        if (borde_choca(borde, arriba, pies))
        {
            t->x_fp = PF((s16)(borde & 0xFFF0) - CUERPO_X - CUERPO_ANCHO); /* Pegado a la pared. */
            t->vx_fp = 0;
        }
    }
    else if (t->vx_fp < 0)
    {
        borde = (u16)(x + CUERPO_X);            /* Hacia la izquierda: el borde izquierdo.  */
        if (borde_choca(borde, arriba, pies))
        {
            t->x_fp = PF((s16)(borde & 0xFFF0) + CELDA_COLISION - CUERPO_X); /* Tras la pared. */
            t->vx_fp = 0;
        }
    }
}

/*--- apoyado: ¿pisa algo alguno de los dos pies en la fila y? ------------------------*/
static u8 apoyado(s16 x, s16 y)
{
    return (u8)(colision_apoya((u16)(x + PIE_ATRAS), (u16)y)
                || colision_apoya((u16)(x + PIE_DELANTE), (u16)y));
}

/*--- mover_y: integrar y; aterrizar, caer por un borde o darse con el techo ----------*/
static void mover_y(Tarou *t, const Mando *m)
{
    s16 x = PF_ENTERO(t->x_fp);
    s16 pies_antes, pies, fila, arriba;

    if (t->en_suelo)                            /* En el suelo: ¿sigue habiendo suelo?      */
    {
        if (!apoyado(x, (s16)(PF_ENTERO(t->y_fp) + ALTO_CELDA)))
        {
            t->en_suelo = 0;                    /* Los dos pies en el aire: a caer.         */
            t->vy_fp = 0;
            cambiar_estado(t, ESTADO_CAYENDO);
        }
        return;
    }

    pies_antes = (s16)(PF_ENTERO(t->y_fp) + ALTO_CELDA);
    t->y_fp += t->vy_fp;
    if (t->vy_fp >= 0)                          /* Bajando: ¿cruza el borde de una celda que */
    {                                           /* sostiene? (una plataforma, solo así).    */
        pies = (s16)(PF_ENTERO(t->y_fp) + ALTO_CELDA);
        fila = (s16)((pies_antes + 15) & ~15);  /* Primer borde de celda en o bajo los pies. */
        for (; fila <= pies; fila += CELDA_COLISION)
        {
            if (apoyado(x, fila))
            {
                t->y_fp = PF(fila - ALTO_CELDA);   /* Los pies, justo encima del borde.     */
                aterrizar(t, m);
                return;
            }
        }
    }
    else                                        /* Subiendo: ¿la cabeza entra en algo sólido? */
    {
        arriba = (s16)(PF_ENTERO(t->y_fp) + CUERPO_Y);
        if (arriba >= 0
            && (colision_solido((u16)(x + CUERPO_X), (u16)arriba)
                || colision_solido((u16)(x + CUERPO_X + CUERPO_ANCHO / 2), (u16)arriba)
                || colision_solido((u16)(x + CUERPO_X + CUERPO_ANCHO - 1), (u16)arriba)))
        {
            /* Pegado bajo el techo; vy = 0 y el apogeo (paso 2) lo pasa a CAYENDO. */
            t->y_fp = PF((s16)(arriba & ~15) + CELDA_COLISION - CUERPO_Y);
            t->vy_fp = 0;
        }
    }
}

void tarou_actualizar(Tarou *t, const Mando *m)
{
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

    /*--- 3. Mover en x contra las paredes; 4. después en y: suelo, techo, caer ----
       Por separado y en este orden: así una esquina nunca "engancha" a Tarou. */
    mover_x(t);
    mover_y(t, m);

    /*--- 5. Avanzar la animación del estado en que termina el frame ------------------*/
    avanzar_animacion(t);
}

void tarou_caja_cuerpo(const Tarou *t, Caja *c)
{
    u8 agachado = (u8)(t->estado == ESTADO_AGACHADO || t->estado == ESTADO_AGACHADO_CAMINANDO);
    c->x = (s16)(PF_ENTERO(t->x_fp) + CUERPO_X);       /* Columnas 24..76 de la celda.      */
    c->ancho = CUERPO_ANCHO;
    if (agachado)
    {
        c->y = (s16)(PF_ENTERO(t->y_fp) + CUERPO_Y_AGACHADO); /* Más bajo y más corto.     */
        c->alto = CUERPO_ALTO_AGACHADO;
    }
    else
    {
        c->y = (s16)(PF_ENTERO(t->y_fp) + CUERPO_Y);   /* De pie: filas 14..64.             */
        c->alto = CUERPO_ALTO;
    }
}

u8 tarou_caja_golpe(const Tarou *t, Caja *c)
{
    if (t->estado != ESTADO_ATACANDO)
    {
        return 0;                               /* Solo hay golpe mientras ataca.            */
    }
    c->ancho = GOLPE_ANCHO;
    c->alto = GOLPE_ALTO;
    c->y = (s16)(PF_ENTERO(t->y_fp) + GOLPE_Y); /* Filas 20..36: boca y espada.             */
    if (t->direccion == DIR_DERECHA)
    {
        c->x = (s16)(PF_ENTERO(t->x_fp) + GOLPE_X);   /* Delante, a la derecha: 60..88.     */
    }
    else
    {
        /* Espejo dentro de la celda: la columna c pasa a 96 - c, así que 60..88 se
           convierte en 8..36 (96 - 88 = 8). Todo constante: sin cuentas por frame. */
        c->x = (s16)(PF_ENTERO(t->x_fp) + (ANCHO_CELDA - GOLPE_X - GOLPE_ANCHO));
    }
    return 1;
}
