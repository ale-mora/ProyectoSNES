/*---------------------------------------------------------------------------------
    ambiente.c — El humo y los pájaros de la aldea (ver ambiente.h).

    PROPÓSITO
      Sin divisiones ni multiplicaciones por variables: la fase de cada bocanada es
      el reloj a la mitad más un desfase fijo, cortado con una máscara (& 63); la
      bandada da la vuelta con & 511 y sube y baja con una tabla de 16 pasos.

    SE CONECTA CON
      Incluye:  juego/ambiente.h, aldea_datos.h (con ALDEA_HUMOS_TABLAS: las tablas
                aldea_humo_x/_y de los tejados, generadas por scripts/preparar_aldea.py).
      Lo usan:  ver ambiente.h.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      PAJARO_DX  u8[3]   separación de cada pájaro en la bandada
      PAJARO_Y   u8[3]   altura de cada uno en la pantalla
      VAIVEN     u8[16]  cuánto baja en cada paso del vaivén (0..3 px)
    → glosario: Sprite, Parallax
---------------------------------------------------------------------------------*/
#include "juego/ambiente.h"
#define ALDEA_HUMOS_TABLAS
#include "aldea_datos.h"

#define HUMO_VISTA 280                   /* Un tejado está a la vista si x + 16 - cámara < 280. */
#define HUMO_MARGEN 16                   /* (La bocanada mide 16 y nace 8 a la izquierda.) */
#define HUMO_ALTO 12                     /* La bocanada nace 12 px por encima del tejado. */
#define PAJARO_VISTA 272                 /* x en el plano < 272: se ve (entra por x = -16). */

static const u8 PAJARO_DX[AMBIENTE_PAJAROS] = {0, 22, 44};
static const u8 PAJARO_Y[AMBIENTE_PAJAROS] = {20, 25, 16};   /* En el cielo, por encima de la
                                                                 barra de vida (fila 40): el pájaro ocupa las
                                                                 filas 4..11 de su pieza, + 3 de vaivén. */
static const u8 VAIVEN[16] = {0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 2, 2, 1, 1, 0, 0};

void ambiente_iniciar(Ambiente *a)
{
    u8 k;

    for (k = 0; k < AMBIENTE_FIGURAS; k++)
    {
        a->fig[k].x = 0;
        a->fig[k].y = 0;
        a->fig[k].cuadro = 0;
        a->fig[k].visible = 0;
        a->fig[k].libre = 0;
    }
    a->reloj = 0;
    a->bandada = 0;
}

/*--- humo_de: las dos bocanadas del tejado j en las figuras f y f + 1 -----------------*/
static void humo_de(Ambiente *a, Figura *f, u8 j, u16 camara)
{
    u8 b, fase;

    for (b = 0; b < AMBIENTE_BOCANADAS; b++)
    {
        fase = (u8)(((a->reloj >> 1) + (b << 5)) & (AMBIENTE_SUBIDA - 1));   /* 0..63. */
        f->x = (s16)((s16)(aldea_humo_x[j] - camara) - 8 + (fase >> 3));      /* El viento. */
        f->y = (s16)((s16)aldea_humo_y[j] - HUMO_ALTO - fase);                 /* Sube.      */
        f->cuadro = (u8)(fase >= AMBIENTE_PENACHO);
        f->visible = 1;
        f++;
    }
}

void ambiente_actualizar(Ambiente *a, u16 camara)
{
    Figura *f;
    u8 j, k, puestos;
    u16 sx;

    a->reloj++;
    if (a->reloj & 1)
    {
        a->bandada++;                                /* Medio píxel por frame.              */
    }

    /*--- El humo: los dos primeros tejados a la vista ---*/
    f = a->fig;
    puestos = 0;
    for (j = 0; j < ALDEA_HUMOS && puestos < AMBIENTE_FUENTES; j++)
    {
        if ((u16)(aldea_humo_x[j] + HUMO_MARGEN - camara) < HUMO_VISTA)
        {
            humo_de(a, f, j, camara);
            f += AMBIENTE_BOCANADAS;
            puestos++;
        }
    }
    for (; puestos < AMBIENTE_FUENTES; puestos++)    /* Los que no tienen tejado: apagados. */
    {
        for (k = 0; k < AMBIENTE_BOCANADAS; k++)
        {
            f->visible = 0;
            f++;
        }
    }

    /*--- Los pájaros: en el plano de las montañas (cámara a la mitad) ---*/
    for (k = 0; k < AMBIENTE_PAJAROS; k++)
    {
        sx = (u16)((a->bandada + PAJARO_DX[k] - (camara >> 1)) & AMBIENTE_VUELTA);
        f->visible = (u8)(sx < PAJARO_VISTA);
        f->x = (s16)((s16)sx - 16);
        f->y = (s16)(PAJARO_Y[k] + VAIVEN[((a->reloj >> 2) + (k << 2) + k) & 15]);
        f->cuadro = (u8)(((a->reloj >> 3) + k) & 1);
        f++;
    }
}
