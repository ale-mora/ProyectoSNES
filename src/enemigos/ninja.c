/*---------------------------------------------------------------------------------
    ninja.c — Lógica de los enemigos: los ninjas, el kendoka y el jefe (ver ninja.h).

    PROPÓSITO
      ninja_actualizar, cada frame: (0) los estados que duran (aturdido, agarrado,
      atacando, bloqueando, agarrando, lanzando, disparando, embistiendo, convocando)
      cuentan su tiempo y, en su frame, dejan un `evento`; (1) mira hacia el objetivo;
      (2) si está a más de NINJA_DISTANCIA en x, avanza NINJA_PASO_X (si aún no ha entrado
      en la pantalla, anda recto hacia dentro hasta que ninja_limitar_x lo da por dentro);
      el rápido, en cambio, se queda entre NINJA_RAPIDO_CERCA y _LEJOS; el grande y el
      kendoka dan menos pasos; (3) cada dos frames corrige un píxel su y hacia la del
      objetivo; (4) anda si se movió; (5) la animación avanza cada NINJA_FRAMES_CUADRO
      frames. El ninja normal, si ve venir un golpe de frente, a veces se pone en guardia.
      Sin divisiones ni multiplicaciones (el azar es un registro de desplazamiento de 16 bits).
      Una máquina de estados por enemigo (gamedev:game-ai): las transiciones viven en cada
      estado; la IA decide y la pantalla (y combate.c) hacen el resto.

    SE CONECTA CON
      Incluye:  enemigos/ninja.h, juego/mundo.h (el carril), juego/nivel.h (ANCHO_PANTALLA).
      Lo usan:  enemigos/ninja_sprite.c, juego/combate.c, pantallas/pantalla_fase1.c y tests/.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (estáticas)
      semilla      u16  el azar (LFSR de Galois, 0xB400)
      presa        u8   1 si el protagonista se puede agarrar este frame (ninja_contexto)
      amenaza      u8   id del golpe que da el protagonista (0 = ninguno)
      amenaza_x, amenaza_y, amenaza_izq   dónde está y hacia dónde mira
    VARIABLES (constantes)
      CARRIL_ENTRADA  u16[NINJAS_MAX]  carril de cada ranura al nacer (distintos)
      VIDA_TIPO, DANIO_TIPO           u8[NINJA_TIPOS]
    → glosario: Oleada, IA, Máquina de estados
---------------------------------------------------------------------------------*/
#include "enemigos/ninja.h"
#include "juego/mundo.h"
#include "juego/nivel.h"

/* static const: se queda en la ROM. Tres carriles repartidos entre CARRIL_Y_MIN y MAX. */
static const u16 CARRIL_ENTRADA[NINJAS_MAX] = {200, 188, 212};

/* Vida y daño de cada tipo (normal, mediano, grande, rápido, kendoka, jefe). */
static const u8 VIDA_TIPO[NINJA_TIPOS] = {NINJA_VIDA, 6, 8, 4, 16, 40};
static const u8 DANIO_TIPO[NINJA_TIPOS] = {NINJA_DANIO, 3, 4, 2, 4, 4};

/* El puñetazo, por grupos de 4 frames del ataque: carga (0), a medio camino (1), puño fuera
   (2) mientras la caja golpea (frames 12..19), y vuelta. */
static const u8 ATAQUE_DIBUJO[(NINJA_ATAQUE_FRAMES >> 2) + 1] = {0, 1, 2, 2, 1, 0, 0};

/* Recibir: 6 dibujos en sus 12 frames (por frame transcurrido). */
static const u8 RECIBIR_DIBUJO[NINJA_RECIBIR_FRAMES] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5};

/* Derribado con vida, por frame transcurrido (0..47): cae (7 dibujos a 3 frames), se queda
   en el suelo (el último de derribado) 12 frames y se levanta (5 dibujos a 3 frames).
   Hasta el 2026-11-05 eran 64 frames (20 en el suelo, 4 por dibujo al levantarse). */
static const u8 DERRIBO_DIBUJO[NINJA_DERRIBO_FRAMES] = {
    23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34};

/* Sin vida (KO): cae con su tira de KO (7 dibujos a 3 frames) y se queda tumbado. */
#define KO_CAIDA 21                      /* Frames de la caída del KO.                      */
#define KO_PARPADEO_DESDE 24             /* Desde aquí parpadea tumbado hasta irse.         */

static u16 semilla;
static u8 presa;
static u8 amenaza;
static u16 amenaza_x, amenaza_y;
static u8 amenaza_izq;

/*--- azar: el siguiente número pseudoaleatorio (0..255) ----------------------------*/
static u8 azar(void)
{
    u8 bit = (u8)(semilla & 1);

    semilla = (u16)(semilla >> 1);
    if (bit)
    {
        semilla ^= 0xB400;                           /* LFSR de Galois de 16 bits.          */
    }
    return (u8)semilla;
}

void ninja_iniciar_todos(Ninja *n)
{
    u8 k;

    semilla = 0xACE1;                                /* Siempre la misma: se repite igual.  */
    presa = 0;
    amenaza = 0;
    amenaza_x = amenaza_y = 0;
    amenaza_izq = 0;
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
        n->destello = 0;
        n->inv = 0;
        n->lanzado = 0;
        n->ataque = 0;
        n->evento = 0;
        n->convoca = 0;
        n->dados = 0;
        n->vio = 0;
        n->x = 0;
        n->y = 0;
        n++;                                         /* Siguiente ranura: sin a[k] (tcc__mul). */
    }
}

void ninja_contexto(u8 presa_libre, u8 golpe_id, u16 x, u16 y, u8 mira_izq)
{
    presa = presa_libre;
    amenaza = golpe_id;
    amenaza_x = x;
    amenaza_y = y;
    amenaza_izq = mira_izq;
}

/*--- nacer: el tipo `tipo` en la ranura k, por un lado de la pantalla -----------------*/
static void nacer(Ninja *n, u8 k, u8 tipo, u16 camara, u8 por_derecha)
{
    n->vivo = 1;
    if (por_derecha)
    {
        n->x = (u16)(camara + ANCHO_PANTALLA + NINJA_ENTRADA);   /* Por la derecha.         */
    }
    else
    {
        n->x = (u16)(camara > NINJA_ENTRADA ? camara - NINJA_ENTRADA : 0);   /* Por la      */
    }                                                /* izquierda, sin bajar de 0: en u16, 10 − 40 */
                                                     /* daría la vuelta a 65506 (el otro extremo). */
    n->y = CARRIL_ENTRADA[k];
    n->mira_izq = por_derecha;                       /* Mirando hacia dentro de la pantalla. */
    n->cuadro = 0;
    n->reloj = k;                                    /* Desfasados: no pisan el mismo paso. */
    n->estado = NINJA_PARADO;
    n->dentro = 0;                                   /* Viene de fuera de la pantalla.      */
    n->tipo = tipo;
    n->vida = VIDA_TIPO[tipo];
    n->id_golpe = 0;                                 /* Ningún tajo lo ha tocado todavía.   */
    n->timer = 0;
    n->empuje = 0;
    n->espera = (u8)(NINJA_ESPERA_INICIAL + (k << 4) - k);   /* 30, 45, 60: no atacan juntos. */
    n->conecto = 0;
    n->destello = 0;
    n->inv = 0;
    n->lanzado = 0;
    n->ataque = 0;
    n->evento = 0;
    n->convoca = NINJA_CONVOCA_CADA;
    n->dados = 0;
    n->vio = 0;
}

/*--- cabe: ¿puede ir el tipo en la ranura k? (la 0, de 16 piezas, solo los pequeños) --*/
static u8 cabe(u8 tipo, u8 k)
{
    if (k == 0)
    {
        return (u8)(tipo == NINJA_NORMAL || tipo == NINJA_RAPIDO);
    }
    if (tipo == NINJA_JEFE)
    {
        return (u8)(k == 1);                         /* Usa los huecos 1 y 2.               */
    }
    return 1;
}

u8 ninja_oleada_tipos(Ninja *n, u16 camara, const u8 *tipos, u8 cuantos)
{
    u8 i, k, nacidos = 0;
    Ninja *p;

    for (i = 0; i < cuantos; i++)
    {
        p = n;
        for (k = 0; k < NINJAS_MAX; k++)
        {
            if (!p->vivo && cabe(tipos[i], k) && !(tipos[i] == NINJA_JEFE && n[2].vivo) &&
                !(k == 2 && n[1].vivo && n[1].tipo == NINJA_JEFE))   /* El jefe usa el hueco 2. */
            {
                nacer(p, k, tipos[i], camara, (u8)(nacidos & 1));
                nacidos++;
                break;
            }
            p++;
        }
    }
    return nacidos;
}

u8 ninja_oleada(Ninja *n, u16 camara, u8 zona)
{
    static const u8 TIPOS[NINJAS_MAX] = {NINJA_NORMAL, NINJA_MEDIANO, NINJA_GRANDE};
    u8 quiere = (u8)(zona + 1);                      /* Zona 0 → 1 ninja, 1 → 2, 2 → 3...   */

    if (quiere > NINJAS_MAX)
    {
        quiere = NINJAS_MAX;
    }
    return ninja_oleada_tipos(n, camara, TIPOS, quiere);
}

/*--- aturdido: un cuadro de RECIBIR o DERRIBADO (retrocede y cuenta el tiempo) -------*/
static void aturdido(Ninja *n)
{
    u8 desliza;                                      /* Píxeles por cuadro que lo empuja.   */

    n->reloj++;                                      /* (También hace parpadear al KO.)     */
    if (n->estado == NINJA_DERRIBADO && n->lanzado)
    {
        desliza = (u8)(n->timer > NINJA_DERRIBO_FRAMES - NINJA_LANZADO_FRAMES ? NINJA_LANZADO_PASO : 0);
        if (desliza == 0)
        {
            n->lanzado = 0;                          /* Aterrizó: ya no derriba a nadie.    */
        }
    }
    else if (n->estado == NINJA_DERRIBADO)
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
            if (n->estado == NINJA_DERRIBADO)        /* Se acaba de levantar: un rato sin   */
            {                                        /* que le peguen (y parpadeando).      */
                n->inv = NINJA_INV_LEVANTARSE;
            }
            n->estado = NINJA_PARADO;                /* Se recupera y vuelve a la carga.    */
            n->cuadro = 0;
        }
    }
}

u8 ninja_recibir(Ninja *n, u8 danio, u8 derribo, u8 hacia_izq, u8 id)
{
    if (!n->vivo || n->id_golpe == id || n->estado == NINJA_DERRIBADO || n->inv != 0)
    {
        return 0;                                    /* Ya lo golpeó este tajo, está caído o */
    }                                                /* se acaba de levantar.               */
    n->id_golpe = id;
    n->destello = NINJA_DESTELLO_FRAMES;             /* En blanco unos frames: el golpe se nota. */
    n->vida = (u8)(n->vida > danio ? n->vida - danio : 0);
    n->mira_izq = (u8)(hacia_izq ? 0 : 1);           /* Mira hacia el que le pegó.          */
    n->empuje = (u8)(hacia_izq ? NINJA_EMPUJE_IZQ : NINJA_EMPUJE_DER);
    n->evento = NINJA_EVENTO_NINGUNO;
    if (derribo || n->vida == 0)
    {
        n->estado = NINJA_DERRIBADO;
        n->timer = NINJA_DERRIBO_FRAMES;
        return 1;
    }
    if (n->tipo == NINJA_JEFE && n->estado == NINJA_CARGANDO)
    {
        return 0;                                    /* El jefe embistiendo no se para.     */
    }
    n->estado = NINJA_RECIBIR;
    n->timer = NINJA_RECIBIR_FRAMES;
    return 0;
}

u8 ninja_bloquea(Ninja *n, u8 hacia_izq, u8 rompe, u8 id)
{
    u8 puede;

    if (rompe || !n->vivo || n->inv != 0 || n->id_golpe == id || n->mira_izq == hacia_izq)
    {
        return 0;                                    /* Rompe la guardia, o por la espalda. */
    }
    if (n->tipo == NINJA_KENDOKA)
    {
        puede = (u8)(n->estado == NINJA_PARADO || n->estado == NINJA_ANDANDO ||
                     n->estado == NINJA_BLOQUEO);
    }
    else
    {
        puede = (u8)(n->tipo == NINJA_NORMAL && n->estado == NINJA_BLOQUEO);
    }
    if (!puede)
    {
        return 0;
    }
    n->estado = NINJA_BLOQUEO;                       /* El golpe rebota.                    */
    n->timer = NINJA_BLOQUEO_FRAMES;
    n->id_golpe = id;
    return 1;
}

/*--- duracion: frames del ataque en curso según su tipo ------------------------------*/
static u8 duracion(const Ninja *n)
{
    if (n->ataque == NINJA_ATQ_PATADA)
    {
        return NINJA_PATADA_FRAMES;
    }
    if (n->ataque == NINJA_ATQ_PESADO)
    {
        return NINJA_PESADO_FRAMES;
    }
    return NINJA_ATAQUE_FRAMES;
}

/*--- acabar: vuelve a la carga tras una acción, con su descanso -----------------------*/
static void acabar(Ninja *n)
{
    n->estado = NINJA_PARADO;
    n->cuadro = 0;
    n->timer = 0;
    n->espera = NINJA_ESPERA;
}

/*--- durar: un frame de los estados que duran; 1 si el frame ya está hecho -----------*/
static u8 durar(Ninja *n)
{
    switch (n->estado)
    {
        case NINJA_ATACANDO:
            n->timer++;
            n->reloj++;
            if (n->timer >= duracion(n))
            {
                acabar(n);
            }
            return 1;
        case NINJA_BLOQUEO:
            n->reloj++;
            if (n->timer)
            {
                n->timer--;
            }
            if (n->timer == 0)
            {
                n->estado = NINJA_PARADO;
                n->cuadro = 0;
            }
            return 1;
        case NINJA_AGARRADO:
            n->reloj++;                              /* Agarrado: lo mueve combate.c.       */
            return 1;
        case NINJA_AGARRANDO:
            n->timer++;
            n->reloj++;
            if (n->timer >= NINJA_AGARRE_FRAMES)
            {
                n->estado = NINJA_LANZANDO;          /* Y ahora lo lanza.                   */
                n->timer = 0;
            }
            return 1;
        case NINJA_LANZANDO:
            n->timer++;
            if (n->timer == NINJA_LANZAR_SUELTA)
            {
                n->evento = NINJA_EVENTO_LANZA_PRESA;
            }
            if (n->timer >= NINJA_LANZAR_FRAMES)
            {
                acabar(n);
            }
            return 1;
        case NINJA_DISPARANDO:
            n->timer++;
            if (n->timer == NINJA_DISPARO_SUELTA)
            {
                n->evento = NINJA_EVENTO_SHURIKEN;
            }
            if (n->timer >= NINJA_DISPARO_FRAMES)
            {
                acabar(n);
            }
            return 1;
        case NINJA_CARGANDO:
            n->timer++;
            n->reloj++;
            n->x = (u16)(n->mira_izq ? n->x - NINJA_CARGA_PASO : n->x + NINJA_CARGA_PASO);
            if ((n->reloj & 3) == 0)
            {
                n->cuadro++;
                if (n->cuadro >= 6)
                {
                    n->cuadro = 0;
                }
            }
            if (n->timer >= NINJA_CARGA_FRAMES)
            {
                acabar(n);
                n->espera = (u8)(NINJA_ESPERA + 30);
            }
            return 1;
        case NINJA_CONVOCANDO:
            n->timer++;
            if (n->timer == (NINJA_CONVOCAR_FRAMES >> 1))
            {
                n->evento = NINJA_EVENTO_CONVOCAR;
            }
            if (n->timer >= NINJA_CONVOCAR_FRAMES)
            {
                acabar(n);
            }
            return 1;
        default:
            return 0;
    }
}

void ninja_actualizar(Ninja *n, u16 obj_x, u16 obj_y)
{
    u8 anda = 0;
    u16 dx;

    n->evento = NINJA_EVENTO_NINGUNO;                /* Los eventos duran un frame.         */
    if (n->destello != 0)
    {
        n->destello--;                               /* El blanco del golpe se gasta solo.  */
    }
    if (n->inv != 0)
    {
        n->inv--;                                    /* Y la invulnerabilidad de levantarse. */
    }
    if (n->estado == NINJA_RECIBIR || n->estado == NINJA_DERRIBADO)
    {
        aturdido(n);                                 /* Sin IA mientras está tocado.        */
        return;
    }
    if (durar(n))
    {
        return;                                      /* Atacando, agarrando, disparando...  */
    }
    if (n->espera != 0)
    {
        n->espera--;                                 /* El descanso se gasta andando o parado. */
    }

    /*--- 0. El normal ve venir un golpe de frente (cerca): a veces se pone en guardia ---*/
    if (n->tipo == NINJA_NORMAL && n->dentro && amenaza != 0 && amenaza != n->vio)
    {
        n->vio = amenaza;                            /* Decide una vez por golpe.           */
        dx = (u16)(amenaza_x > n->x ? amenaza_x - n->x : n->x - amenaza_x);
        if (dx < 64 && amenaza_izq != n->mira_izq && (azar() & 3) == 0)
        {
            n->estado = NINJA_BLOQUEO;
            n->timer = NINJA_BLOQUEO_FRAMES;
            n->mira_izq = (u8)(amenaza_x < n->x);    /* Hacia el que le ataca.              */
            return;
        }
    }

    /*--- 0b. El jefe enfadado llama a un ninja cada NINJA_CONVOCA_CADA x 2 frames ---*/
    if (n->tipo == NINJA_JEFE && n->vida <= NINJA_JEFE_FURIA && n->dentro)
    {
        if (n->convoca != 0 && (n->reloj & 1))
        {
            n->convoca--;
        }
        if (n->convoca == 0)
        {
            n->convoca = NINJA_CONVOCA_CADA;
            n->estado = NINJA_CONVOCANDO;
            n->timer = 0;
            return;
        }
    }

    /*--- 1 y 2. Mirar hacia el objetivo y acercarse en x ----------------------------
        El grande y el kendoka son más lentos: uno de cada cuatro (dos) frames no dan el paso. */
    dx = (u16)(n->x > obj_x ? n->x - obj_x : obj_x - n->x);
    if (!n->dentro)                                  /* Aún fuera de la pantalla: sigue recto  */
    {                                                /* hacia dentro sin pararse, hasta que     */
        n->x = (u16)(n->mira_izq ? n->x - NINJA_PASO_X     /* ninja_limitar_x lo da por dentro. */
                                 : n->x + NINJA_PASO_X);
        anda = 1;
    }
    else if (n->tipo == NINJA_RAPIDO)                /* El rápido guarda la distancia.      */
    {
        n->mira_izq = (u8)(n->x > obj_x);
        if (dx < NINJA_RAPIDO_CERCA)
        {
            n->x = (u16)(n->x > obj_x ? n->x + NINJA_PASO_X : n->x - NINJA_PASO_X);   /* Se aleja. */
            anda = 1;
        }
        else if (dx > NINJA_RAPIDO_LEJOS)
        {
            n->x = (u16)(n->x > obj_x ? n->x - NINJA_PASO_X : n->x + NINJA_PASO_X);
            anda = 1;
        }
    }
    else if ((n->tipo == NINJA_GRANDE && (n->reloj & 3) == 0) ||
             (n->tipo == NINJA_KENDOKA && (n->reloj & 1) == 0))
    {
        n->mira_izq = (u8)(n->x > obj_x);            /* Solo mira; da el paso el siguiente, */
        anda = (u8)(dx > NINJA_DISTANCIA);           /* pero sigue andando (sin saltos al dibujo de guardia). */
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
    if (n->estado == NINJA_DERRIBADO && n->lanzado)
    {
        return;                                      /* Volando: puede salirse (y vuelve).  */
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

u8 ninja_agarrable(const Ninja *n)
{
    return (u8)(n->vivo && n->dentro && n->inv == 0 && n->vida != 0 &&
                n->tipo != NINJA_KENDOKA && n->tipo != NINJA_JEFE &&
                (n->estado == NINJA_PARADO || n->estado == NINJA_ANDANDO || n->estado == NINJA_RECIBIR));
}

void ninja_agarrar(Ninja *n)
{
    n->estado = NINJA_AGARRADO;
    n->timer = 0;
    n->cuadro = 0;
}

void ninja_soltar(Ninja *n, u8 derribo, u8 hacia_izq)
{
    if (n->estado != NINJA_AGARRADO)
    {
        return;
    }
    if (derribo)
    {
        n->estado = NINJA_DERRIBADO;                 /* Tras los golpes de agarre: al suelo. */
        n->timer = NINJA_DERRIBO_FRAMES;
        n->empuje = (u8)(hacia_izq ? NINJA_EMPUJE_IZQ : NINJA_EMPUJE_DER);
        return;
    }
    n->estado = NINJA_PARADO;                        /* Se escapa y vuelve a la carga.      */
    n->espera = NINJA_ESPERA;
    n->cuadro = 0;
}

void ninja_lanzar(Ninja *n, u8 hacia_izq, u8 danio)
{
    n->vida = (u8)(n->vida > danio ? n->vida - danio : 0);
    n->estado = NINJA_DERRIBADO;
    n->timer = NINJA_DERRIBO_FRAMES;
    n->lanzado = 1;
    n->destello = NINJA_DESTELLO_FRAMES;
    n->empuje = (u8)(hacia_izq ? NINJA_EMPUJE_IZQ : NINJA_EMPUJE_DER);
    n->mira_izq = (u8)(hacia_izq ? 0 : 1);           /* Vuela de espaldas.                  */
}

u8 ninja_volando(const Ninja *n)
{
    return (u8)(n->vivo && n->estado == NINJA_DERRIBADO && n->lanzado);
}

u8 ninja_atacando(const Ninja *n)
{
    u8 k, cuantos = 0;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        cuantos = (u8)(cuantos + (n->vivo && (n->estado == NINJA_ATACANDO ||
                                              n->estado == NINJA_AGARRANDO ||
                                              n->estado == NINJA_LANZANDO ||
                                              n->estado == NINJA_DISPARANDO ||
                                              n->estado == NINJA_CARGANDO)));
        n++;
    }
    return cuantos;
}

/*--- empezar: un ataque de tipo `ataque` -------------------------------------------*/
static void empezar(Ninja *n, u8 estado, u8 ataque)
{
    n->estado = estado;
    n->ataque = ataque;
    n->timer = 0;
    n->cuadro = 0;
    n->conecto = 0;
    n->dados++;
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
    switch (n->tipo)
    {
        case NINJA_RAPIDO:                           /* A distancia y en su carril: shuriken. */
            if (dy > 4 || dx < 64 || dx > 136)
            {
                return 0;
            }
            n->mira_izq = (u8)(n->x > obj_x);
            empezar(n, NINJA_DISPARANDO, NINJA_ATQ_PUNO);
            return 1;
        case NINJA_JEFE:
            if (dx > NINJA_CARGA_DESDE && dy <= 16)  /* Lejos: embiste.                     */
            {
                n->mira_izq = (u8)(n->x > obj_x);
                empezar(n, NINJA_CARGANDO, NINJA_ATQ_PESADO);
                return 1;
            }
            if (dx > NINJA_DISTANCIA + 16 || dy > 8)
            {
                return 0;
            }
            empezar(n, NINJA_ATACANDO, NINJA_ATQ_PESADO);    /* Cerca: el tajo largo.       */
            return 1;
        case NINJA_KENDOKA:
            if (dx > NINJA_DISTANCIA + 16 || dy > 8)
            {
                return 0;
            }
            if (presa && dx <= NINJA_AGARRE_ALCANCE && (azar() & 3) == 0)
            {
                empezar(n, NINJA_AGARRANDO, NINJA_ATQ_PUNO);  /* Agarra al protagonista.   */
                return 1;
            }
            empezar(n, NINJA_ATACANDO, NINJA_ATQ_PESADO);
            return 1;
        default:
            if (dx > NINJA_DISTANCIA + 8 || dy > 8)
            {
                return 0;                            /* Aún lejos para acertar.             */
            }
            if (n->tipo == NINJA_NORMAL && presa && dx <= NINJA_AGARRE_ALCANCE && n->dados != 0 &&
                (azar() & 3) == 0)
            {
                empezar(n, NINJA_AGARRANDO, NINJA_ATQ_PUNO);
                return 1;
            }
            empezar(n, NINJA_ATACANDO,              /* El normal alterna puño y patada.    */
                    (u8)(n->tipo == NINJA_NORMAL && (n->dados & 1) ? NINJA_ATQ_PATADA : NINJA_ATQ_PUNO));
            return 1;
    }
}

u8 ninja_golpe_activo(const Ninja *n)
{
    if (!n->vivo || n->conecto)
    {
        return 0;
    }
    if (n->estado == NINJA_CARGANDO)
    {
        return 1;                                    /* La embestida golpea todo el rato.   */
    }
    if (n->estado != NINJA_ATACANDO)
    {
        return 0;
    }
    if (n->ataque == NINJA_ATQ_PATADA)
    {
        return (u8)(n->timer >= NINJA_PATADA_DESDE && n->timer < NINJA_PATADA_HASTA);
    }
    if (n->ataque == NINJA_ATQ_PESADO)
    {
        return (u8)(n->timer >= NINJA_PESADO_DESDE && n->timer < NINJA_PESADO_HASTA);
    }
    return (u8)(n->timer >= NINJA_GOLPE_DESDE && n->timer < NINJA_GOLPE_HASTA);
}

u16 ninja_alcance(const Ninja *n)
{
    if (n->estado == NINJA_CARGANDO)
    {
        return 24;                                   /* El cuerpo entero, por delante.      */
    }
    if (n->ataque == NINJA_ATQ_PATADA)
    {
        return NINJA_PATADA_ALCANCE;
    }
    if (n->ataque == NINJA_ATQ_PESADO)
    {
        return NINJA_PESADO_ALCANCE;
    }
    return NINJA_ALCANCE;
}

u8 ninja_derriba(const Ninja *n)
{
    return (u8)(n->estado == NINJA_CARGANDO || (n->estado == NINJA_ATACANDO && n->ataque == NINJA_ATQ_PESADO));
}

u8 ninja_dibujo(const Ninja *n)
{
    u8 pasado;                                       /* Frames desde que empezó a caer o a recibir. */
    u8 f;

    switch (n->estado)
    {
        case NINJA_ANDANDO:
            return (u8)(NINJA_DIB_CAMINAR + n->cuadro);
        case NINJA_ATACANDO:
            if (n->ataque == NINJA_ATQ_PATADA)
            {
                f = (u8)(n->timer >> 2);
                return (u8)(NINJA_DIB_PATADA + (f > 6 ? 6 : f));
            }
            if (n->ataque == NINJA_ATQ_PESADO)
            {
                f = (u8)(n->timer >> 2);             /* 30 frames: grupos de 4 → 0..5.      */
                return (u8)(NINJA_DIB_PESADO + (f > 5 ? 5 : f));
            }
            return (u8)(NINJA_DIB_ATAQUE + ATAQUE_DIBUJO[n->timer >> 2]);
        case NINJA_RECIBIR:
            pasado = (u8)(NINJA_RECIBIR_FRAMES - n->timer);
            if (pasado >= NINJA_RECIBIR_FRAMES)
            {
                pasado = NINJA_RECIBIR_FRAMES - 1;   /* (Ya no es potencia de 2: se recorta.) */
            }
            return (u8)(NINJA_DIB_RECIBIR + RECIBIR_DIBUJO[pasado]);
        case NINJA_DERRIBADO:
            pasado = (u8)(NINJA_DERRIBO_FRAMES - n->timer);
            if (pasado >= NINJA_DERRIBO_FRAMES)
            {
                pasado = NINJA_DERRIBO_FRAMES - 1;   /* 0..47 (recortado, no enmascarado).  */
            }
            if (n->vida != 0)
            {
                return DERRIBO_DIBUJO[pasado];
            }
            if (pasado < KO_CAIDA)                   /* 3 frames por dibujo, sin dividir:  */
            {                                        /* el de la tabla de derribo menos 23  */
                return (u8)(NINJA_DIB_KO + DERRIBO_DIBUJO[pasado] - NINJA_DIB_DERRIBADO);
            }                                        /* da 0..6 en los mismos 21 frames.   */
            return (u8)(NINJA_DIB_KO + 6);           /* Tumbado.                            */
        case NINJA_AGARRADO:
            return (u8)(NINJA_DIB_AGARRADO + ((n->reloj >> 3) & 1));
        case NINJA_BLOQUEO:
            return (u8)(NINJA_DIB_BLOQUEO + (n->timer < (NINJA_BLOQUEO_FRAMES - 3) ? 1 : 0));
        case NINJA_AGARRANDO:
            return (u8)(NINJA_DIB_AGARRE + ((n->reloj >> 3) & 1));
        case NINJA_LANZANDO:
            f = (u8)(n->timer >> 2);
            return (u8)(NINJA_DIB_LANZAR + (f > 5 ? 5 : f));
        case NINJA_DISPARANDO:
            f = (u8)(n->timer >> 2);
            return (u8)(NINJA_DIB_SHURIKEN + (f > 7 ? 7 : f));
        case NINJA_CARGANDO:
            return (u8)(NINJA_DIB_CARGA + n->cuadro);
        case NINJA_CONVOCANDO:
            f = (u8)(n->timer >> 2);
            return (u8)(NINJA_DIB_CONVOCAR + (f > 3 ? 3 : f));
        default:                                     /* Parado: la guardia, 8 frames por dibujo. */
            return (u8)(NINJA_DIB_GUARDIA + ((n->reloj >> 3) & 7));
    }
}

u8 ninja_parpadea(const Ninja *n)
{
    u8 pasado;

    if (n->inv != 0)
    {
        return (u8)((n->inv & 4) != 0);              /* Recién levantado: parpadea.         */
    }
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
