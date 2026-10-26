/*---------------------------------------------------------------------------------
    ninja.c — Lógica del ninja básico (ver ninja.h).

    PROPÓSITO
      ninja_actualizar, cada frame: (1) mira hacia el objetivo; (2) si está a más de
      NINJA_DISTANCIA en x, avanza NINJA_PASO_X (si aún no ha entrado en la pantalla, no se
      detiene hasta entrar: ver ninja_limitar_x); (3) cada dos frames corrige un píxel su y
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
            n->x = (u16)((nacidos & 1) ? camara + ANCHO_PANTALLA + NINJA_ENTRADA
                                       : camara - NINJA_ENTRADA);   /* Izquierda o derecha. */
            n->y = CARRIL_ENTRADA[k];
            n->mira_izq = (u8)(nacidos & 1);         /* Mirando hacia dentro de la pantalla. */
            n->cuadro = 0;
            n->reloj = k;                            /* Desfasados: no pisan el mismo paso. */
            n->estado = NINJA_PARADO;
            n->dentro = 0;                           /* Viene de fuera de la pantalla.      */
            n->vida = NINJA_VIDA;
            n->id_golpe = 0;                         /* Ningún tajo lo ha tocado todavía.   */
            n->timer = 0;
            n->empuje = 0;
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

    n->reloj++;                                      /* (También hace parpadear el dibujo.) */
    if (n->estado == NINJA_DERRIBADO)
    {
        desliza = (u8)(n->timer > NINJA_DERRIBO_FRAMES - 14 ? 3 : 0);   /* Sale despedido.   */
    }
    else
    {
        desliza = (u8)(n->timer > NINJA_RECIBIR_FRAMES - 6 ? 2 : 0);    /* Un pequeño retroceso. */
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

    /*--- 1 y 2. Mirar hacia el objetivo y acercarse en x hasta NINJA_DISTANCIA ------*/
    if (n->x > obj_x)
    {
        n->mira_izq = 1;
        if (n->x - obj_x > NINJA_DISTANCIA || (!n->dentro && n->x != obj_x))   /* Hasta que entre en     */
        {                                            /* pantalla no se detiene (ninja_limitar_x). */
            n->x = (u16)(n->x - NINJA_PASO_X);
            anda = 1;
        }
    }
    else
    {
        n->mira_izq = 0;
        if (obj_x - n->x > NINJA_DISTANCIA || (!n->dentro && n->x != obj_x))
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
