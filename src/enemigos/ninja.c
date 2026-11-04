/*---------------------------------------------------------------------------------
    ninja.c — Lógica del ninja básico (ver ninja.h).

    PROPÓSITO
      ninja_actualizar, cada frame: (1) mira hacia el objetivo; (2) si está a más de
      NINJA_DISTANCIA en x, avanza NINJA_PASO_X (si aún no ha entrado en la pantalla, anda recto
      hacia dentro, hacia donde mira al nacer, sin pararse ni sobre el objetivo, hasta que
      ninja_limitar_x lo da por dentro: si Tarou está pegado al borde, más cerca de él que
      MARGEN_NINJA, el ninja que lo seguía se quedaba fuera para siempre); (3) cada dos frames corrige un píxel su y
      hacia la del objetivo (así se colocan en su carril sin ir en diagonal rápida); (4) anda
      si se movió y se queda en el cuadro 0 si no; (5) la animación avanza de cuadro cada
      NINJA_FRAMES_CUADRO frames (potencia de 2: se mira con un AND, no con %). Sin
      divisiones ni multiplicaciones.

    SE CONECTA CON
      Incluye:  enemigos/ninja.h, juego/mundo.h (el carril), juego/nivel.h (ANCHO_PANTALLA).
      Lo usan:  enemigos/ninja_sprite.c, pantallas/pantalla_fase1.c y tests/prueba_ninja.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      CARRIL_ENTRADA  u16[NINJAS_MAX]  carril de cada ranura al nacer (distintos)
    → glosario: Oleada, IA
---------------------------------------------------------------------------------*/
#include "enemigos/ninja.h"
#include "juego/mundo.h"
#include "juego/nivel.h"

/* static const: se queda en la ROM. Tres carriles repartidos entre CARRIL_Y_MIN y MAX. */
static const u16 CARRIL_ENTRADA[NINJAS_MAX] = {200, 188, 212};

/* Vida y daño de cada tipo (normal, mediano, grande). */
static const u8 VIDA_TIPO[NINJA_TIPOS] = {NINJA_VIDA, 6, 8};
static const u8 DANIO_TIPO[NINJA_TIPOS] = {NINJA_DANIO, 3, 4};

/* El puñetazo, por grupos de 4 frames del ataque: carga (0), a medio camino (1), puño fuera
   (2) mientras la caja golpea (frames 12..19), y vuelta. */
static const u8 ATAQUE_DIBUJO[NINJA_ATAQUE_FRAMES >> 2] = {0, 0, 1, 2, 2, 1, 0};

/* Recibir: 6 dibujos en sus 16 frames (por frame transcurrido). */
static const u8 RECIBIR_DIBUJO[NINJA_RECIBIR_FRAMES] = {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5};

/* Derribado con vida, por frame transcurrido (0..63): cae (7 dibujos a 3 frames), se queda
   en el suelo (el último de derribado) 20 frames y se levanta (5 dibujos a 4 frames). */
static const u8 DERRIBO_DIBUJO[NINJA_DERRIBO_FRAMES] = {
    23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    30, 30, 30, 30, 31, 31, 31, 31, 32, 32, 32, 32, 33, 33, 33, 33, 34, 34, 34, 34,
    34, 34, 34};

/* Sin vida (KO): cae con su tira de KO (7 dibujos a 3 frames) y se queda tumbado. */
#define KO_CAIDA 21                      /* Frames de la caída del KO.                      */
#define KO_PARPADEO_DESDE 24             /* Desde aquí parpadea tumbado hasta irse.         */

void ninja_iniciar_todos(Ninja *n)
{
    u8 k;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        n->vivo = 0;
        n->dentro = 0;
        n->vida = 0;
        n->id_golpe = 0;
        n->timer = 0;
        n->empuje = 0;
        n->espera = 0;
        n->conecto = 0;
        n->tipo = k;
        n->x = 0;
        n->y = 0;
        n++;                                         /* Siguiente ranura: sin a[k] (tcc__mul). */
    }
}

u8 ninja_oleada(Ninja *n, u16 camara, u8 zona)
{
    u8 k, quiere, nacidos;

    quiere = (u8)(zona + 1);                         /* Zona 0 → 1 ninja, 1 → 2, 2 → 3...   */
    if (quiere > NINJAS_MAX)
    {
        quiere = NINJAS_MAX;
    }
    nacidos = 0;
    for (k = 0; k < NINJAS_MAX && nacidos < quiere; k++)
    {
        if (!n->vivo)
        {
            n->vivo = 1;
            if (nacidos & 1)
            {
                n->x = (u16)(camara + ANCHO_PANTALLA + NINJA_ENTRADA);   /* Por la derecha. */
            }
            else
            {
                n->x = (u16)(camara > NINJA_ENTRADA ? camara - NINJA_ENTRADA : 0);   /* Por la  */
            }                                        /* izquierda, sin bajar de 0: en u16, 10 − 40 */
                                                     /* daría la vuelta a 65506 (el otro extremo). */
            n->y = CARRIL_ENTRADA[k];
            n->mira_izq = (u8)(nacidos & 1);         /* Mirando hacia dentro de la pantalla. */
            n->cuadro = 0;
            n->reloj = k;                            /* Desfasados: no pisan el mismo paso. */
            n->estado = NINJA_PARADO;
            n->dentro = 0;                           /* Viene de fuera de la pantalla.      */
            n->tipo = k;                             /* La ranura k es el tipo k.           */
            n->vida = VIDA_TIPO[k];
            n->id_golpe = 0;                         /* Ningún tajo lo ha tocado todavía.   */
            n->timer = 0;
            n->empuje = 0;
            n->espera = (u8)(NINJA_ESPERA_INICIAL + (k << 4) - k);   /* 30, 45, 60: no atacan juntos. */
            n->conecto = 0;
            nacidos++;
        }
        n++;
    }
    return nacidos;
}

/*--- aturdido: un cuadro de RECIBIR o DERRIBADO (retrocede y cuenta el tiempo) -------*/
static void aturdido(Ninja *n)
{
    u8 desliza;                                      /* Píxeles por cuadro que lo empuja.   */

    n->reloj++;                                      /* (También hace parpadear al KO.)     */
    if (n->estado == NINJA_DERRIBADO)
    {
        desliza = (u8)(n->timer > NINJA_DERRIBO_FRAMES - 14 ? 3 : 0);   /* Sale despedido.   */
    }
    else
    {
        desliza = (u8)(n->timer > NINJA_RECIBIR_FRAMES - 4 ? 1 : 0);    /* Retroceso de 4 px: el combo de 3 golpes sigue a su alcance. */
    }
    if (n->empuje == NINJA_EMPUJE_IZQ)
    {
        n->x = (u16)(n->x > desliza ? n->x - desliza : 0);
    }
    else
    {
        n->x = (u16)(n->x + desliza);
    }
    n->timer--;
    if (n->timer == 0)
    {
        if (n->vida == 0)
        {
            n->vivo = 0;                             /* KO: desaparece.                     */
        }
        else
        {
            n->estado = NINJA_PARADO;                /* Se recupera y vuelve a la carga.    */
            n->cuadro = 0;
        }
    }
}

u8 ninja_recibir(Ninja *n, u8 danio, u8 derribo, u8 hacia_izq, u8 id)
{
    if (!n->vivo || n->id_golpe == id || n->estado == NINJA_DERRIBADO)
    {
        return 0;                                    /* Ya lo golpeó este tajo, o está caído. */
    }
    n->id_golpe = id;
    n->vida = (u8)(n->vida > danio ? n->vida - danio : 0);
    n->mira_izq = (u8)(hacia_izq ? 0 : 1);           /* Mira hacia el que le pegó.          */
    n->empuje = (u8)(hacia_izq ? NINJA_EMPUJE_IZQ : NINJA_EMPUJE_DER);
    if (derribo || n->vida == 0)
    {
        n->estado = NINJA_DERRIBADO;
        n->timer = NINJA_DERRIBO_FRAMES;
        return 1;
    }
    n->estado = NINJA_RECIBIR;
    n->timer = NINJA_RECIBIR_FRAMES;
    return 0;
}

void ninja_actualizar(Ninja *n, u16 obj_x, u16 obj_y)
{
    u8 anda = 0;

    if (n->estado == NINJA_RECIBIR || n->estado == NINJA_DERRIBADO)
    {
        aturdido(n);                                 /* Sin IA mientras está tocado.        */
        return;
    }
    if (n->estado == NINJA_ATACANDO)                 /* El tajo sigue hasta acabar.         */
    {
        n->timer++;
        n->reloj++;
        if (n->timer >= NINJA_ATAQUE_FRAMES)
        {
            n->estado = NINJA_PARADO;                /* Acabó: descansa antes del siguiente. */
            n->cuadro = 0;
            n->timer = 0;
            n->espera = NINJA_ESPERA;
        }
        return;
    }
    if (n->espera != 0)
    {
        n->espera--;                                 /* El descanso se gasta andando o parado. */
    }

    /*--- 1 y 2. Mirar hacia el objetivo y acercarse en x hasta NINJA_DISTANCIA ------
        El grande es más lento: uno de cada cuatro frames no da el paso en x. */
    if (n->tipo == NINJA_GRANDE && (n->reloj & 3) == 0 && n->dentro)
    {
        n->mira_izq = (u8)(n->x > obj_x);            /* Solo mira; da el paso el siguiente, */
        anda = (u8)((n->x > obj_x ? n->x - obj_x : obj_x - n->x) > NINJA_DISTANCIA);   /* pero */
    }                                                /* sigue andando (sin saltos al dibujo de guardia). */
    else if (!n->dentro)                                  /* Aún fuera de la pantalla: sigue recto  */
    {                                                /* hacia dentro (hacia donde mira desde que */
        n->x = (u16)(n->mira_izq ? n->x - NINJA_PASO_X     /* nació) sin pararse, ni siquiera */
                                 : n->x + NINJA_PASO_X);   /* sobre el objetivo, hasta que     */
        anda = 1;                                    /* ninja_limitar_x lo da por dentro.      */
    }
    else if (n->x > obj_x)
    {
        n->mira_izq = 1;
        if (n->x - obj_x > NINJA_DISTANCIA)
        {
            n->x = (u16)(n->x - NINJA_PASO_X);
            anda = 1;
        }
    }
    else
    {
        n->mira_izq = 0;
        if (obj_x - n->x > NINJA_DISTANCIA)
        {
            n->x = (u16)(n->x + NINJA_PASO_X);
            anda = 1;
        }
    }

    /*--- 3. y: un píxel cada dos frames hacia el carril del objetivo ----------------*/
    if ((n->reloj & 1) && n->y != obj_y)
    {
        n->y = (u16)(n->y + (n->y < obj_y ? 1 : -1));
        anda = 1;
    }

    /*--- 4 y 5. Estado y animación -------------------------------------------------*/
    n->estado = (u8)(anda ? NINJA_ANDANDO : NINJA_PARADO);
    n->reloj++;
    if (anda && (n->reloj & (NINJA_FRAMES_CUADRO - 1)) == 0)
    {
        n->cuadro++;
        if (n->cuadro >= NINJA_CUADROS)
        {
            n->cuadro = 0;
        }
    }
    else if (!anda)
    {
        n->cuadro = 0;                               /* Parado: el cuadro de reposo.        */
    }
}

void ninja_limitar_x(Ninja *n, u16 x_min, u16 x_max)
{
    if (!n->dentro)
    {
        if (n->x >= x_min && n->x <= x_max)
        {
            n->dentro = 1;                           /* Ya está en pantalla: de aquí no sale. */
        }
        return;
    }
    if (n->x < x_min)
    {
        n->x = x_min;
    }
    else if (n->x > x_max)
    {
        n->x = x_max;
    }
}

u8 ninja_atacando(const Ninja *n)
{
    u8 k, cuantos = 0;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        cuantos = (u8)(cuantos + (n->vivo && n->estado == NINJA_ATACANDO));
        n++;
    }
    return cuantos;
}

u8 ninja_intentar_atacar(Ninja *n, u16 obj_x, u16 obj_y, u8 turno_libre)
{
    u16 dx, dy;

    if (!turno_libre || !n->vivo || !n->dentro || n->estado != NINJA_PARADO || n->espera != 0)
    {
        return 0;                                    /* Sin turno, andando, o descansando.  */
    }
    dx = (u16)(n->x > obj_x ? n->x - obj_x : obj_x - n->x);
    dy = (u16)(n->y > obj_y ? n->y - obj_y : obj_y - n->y);
    if (dx > NINJA_DISTANCIA + 8 || dy > 8)
    {
        return 0;                                    /* Aún lejos para acertar.             */
    }
    n->estado = NINJA_ATACANDO;
    n->timer = 0;
    n->cuadro = 0;
    n->conecto = 0;
    return 1;
}

u8 ninja_golpe_activo(const Ninja *n)
{
    return (u8)(n->vivo && n->estado == NINJA_ATACANDO && !n->conecto &&
                n->timer >= NINJA_GOLPE_DESDE && n->timer < NINJA_GOLPE_HASTA);
}

u8 ninja_dibujo(const Ninja *n)
{
    u8 pasado;                                       /* Frames desde que empezó a caer o a recibir. */

    switch (n->estado)
    {
        case NINJA_ANDANDO:
            return (u8)(NINJA_DIB_CAMINAR + n->cuadro);
        case NINJA_ATACANDO:
            return (u8)(NINJA_DIB_ATAQUE + ATAQUE_DIBUJO[n->timer >> 2]);
        case NINJA_RECIBIR:
            pasado = (u8)(NINJA_RECIBIR_FRAMES - n->timer);
            return (u8)(NINJA_DIB_RECIBIR + RECIBIR_DIBUJO[pasado & (NINJA_RECIBIR_FRAMES - 1)]);
        case NINJA_DERRIBADO:
            pasado = (u8)(NINJA_DERRIBO_FRAMES - n->timer);
            pasado = (u8)(pasado & (NINJA_DERRIBO_FRAMES - 1));   /* 0..63 (potencia de 2).  */
            if (n->vida != 0)
            {
                return DERRIBO_DIBUJO[pasado];
            }
            if (pasado < KO_CAIDA)                   /* 3 frames por dibujo, sin dividir:  */
            {                                        /* el de la tabla de derribo menos 23  */
                return (u8)(NINJA_DIB_KO + DERRIBO_DIBUJO[pasado] - NINJA_DIB_DERRIBADO);
            }                                        /* da 0..6 en los mismos 21 frames.   */
            return (u8)(NINJA_DIB_KO + 6);           /* Tumbado.                            */
        default:                                     /* Parado: la guardia, 8 frames por dibujo. */
            return (u8)(NINJA_DIB_GUARDIA + ((n->reloj >> 3) & 7));
    }
}

u8 ninja_parpadea(const Ninja *n)
{
    u8 pasado;

    if (n->estado != NINJA_DERRIBADO || n->vida != 0)
    {
        return 0;
    }
    pasado = (u8)(NINJA_DERRIBO_FRAMES - n->timer);
    return (u8)(pasado >= KO_PARPADEO_DESDE && (n->reloj & 2));
}

u8 ninja_danio(const Ninja *n)
{
    return DANIO_TIPO[n->tipo];
}

u8 ninja_vivos(const Ninja *n)
{
    u8 k, vivos = 0;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        vivos = (u8)(vivos + n->vivo);
        n++;
    }
    return vivos;
}

void ninja_derrotar_todos(Ninja *n)
{
    u8 k;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        n->vivo = 0;
        n++;
    }
}
