/*---------------------------------------------------------------------------------
    prueba_ambiente.c — Pruebas de juego/ambiente.c (el humo y los pájaros de la aldea).

    PROPÓSITO
      Fijar lo que se ve: el humo sale del tejado que está a la vista (y de ninguno
      cuando no hay), cada bocanada sube 64 px apartándose a la derecha y renace, pasa
      de bocanada a penacho, nunca hay más de dos tejados; la bandada cruza el cielo a
      medio píxel por frame, la cámara la mueve a la mitad, aletea y da la vuelta.
      Corre sobre las tablas REALES de res/aldea/aldea_datos.h.

    SE CONECTA CON
      Incluye:  minitest.h, juego/ambiente.h, aldea_datos.h (con ALDEA_HUMOS_TABLAS).
      Lo usan:  tests/pruebas.c (llama a pruebas_ambiente).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Sprite, Parallax
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/ambiente.h"
#define ALDEA_HUMOS_TABLAS
#include "aldea_datos.h"

void pruebas_ambiente(void)
{
    Ambiente a;
    u16 f, camara;
    s16 y0, x0;
    u8 k, vistos, cambios, ultimo;

    /* Al empezar, nada a la vista hasta el primer frame. */
    ambiente_iniciar(&a);
    for (k = 0; k < AMBIENTE_FIGURAS; k++)
    {
        COMPROBAR(!a.fig[k].visible);
    }

    /* Con la cámara en 0, el primer tejado echa humo justo encima de él. */
    ambiente_actualizar(&a, 0);
    COMPROBAR(a.fig[0].visible && a.fig[1].visible);
    COMPROBAR(a.fig[0].x >= (s16)aldea_humo_x[0] - 8 && a.fig[0].x <= (s16)aldea_humo_x[0]);
    COMPROBAR(a.fig[0].y < (s16)aldea_humo_y[0]);
    COMPROBAR(a.fig[0].y != a.fig[1].y);                 /* Las dos bocanadas, desfasadas. */

    /* Una bocanada sube de 1 en 1 cada 2 frames, se aparta a la derecha y renace. */
    ambiente_iniciar(&a);
    ambiente_actualizar(&a, 0);
    y0 = a.fig[0].y;
    x0 = a.fig[0].x;
    cambios = 0;
    ultimo = a.fig[0].cuadro;
    for (f = 0; f < 2 * AMBIENTE_SUBIDA - 2; f++)
    {
        ambiente_actualizar(&a, 0);
        COMPROBAR(a.fig[0].y <= y0 && a.fig[0].x >= x0);
        if (a.fig[0].cuadro != ultimo)
        {
            cambios++;
            ultimo = a.fig[0].cuadro;
        }
    }
    COMPROBAR(a.fig[0].y == y0 - (AMBIENTE_SUBIDA - 1) && a.fig[0].cuadro == 1);
    COMPROBAR(cambios == 1);                              /* Bocanada → penacho, una vez.  */
    ambiente_actualizar(&a, 0);
    ambiente_actualizar(&a, 0);
    COMPROBAR(a.fig[0].y == y0 && a.fig[0].cuadro == 0);  /* Renace en el tejado.          */

    /* A lo largo de la fase: nunca más de 2 tejados, y siempre al menos uno a la vista
       cuando un tejado de la tabla cae en la pantalla. */
    ambiente_iniciar(&a);
    for (camara = 0; camara <= 1792; camara += 16)
    {
        ambiente_actualizar(&a, camara);
        vistos = 0;
        for (k = 0; k < AMBIENTE_HUMO; k++)
        {
            if (a.fig[k].visible)
            {
                vistos++;
                COMPROBAR(a.fig[k].x > -25 && a.fig[k].x < 264);
            }
        }
        COMPROBAR(vistos <= AMBIENTE_HUMO);
        for (k = 0; k < ALDEA_HUMOS; k++)
        {
            if (aldea_humo_x[k] >= camara + 16 && aldea_humo_x[k] < camara + 240)
            {
                COMPROBAR(a.fig[0].visible);
            }
        }
    }

    /* Con la cámara al final, el último tejado (x 2008) sigue echando humo. */
    ambiente_actualizar(&a, 1792);
    COMPROBAR(a.fig[0].visible);

    /* Los pájaros: medio píxel por frame hacia la derecha; la cámara los mueve a la
       mitad; aletean; dan la vuelta. */
    ambiente_iniciar(&a);
    ambiente_actualizar(&a, 0);
    x0 = a.fig[AMBIENTE_HUMO].x;
    COMPROBAR(a.fig[AMBIENTE_HUMO].visible && a.fig[AMBIENTE_HUMO + 1].x > x0);
    for (f = 0; f < 40; f++)
    {
        ambiente_actualizar(&a, 0);
    }
    COMPROBAR(a.fig[AMBIENTE_HUMO].x == x0 + 20);
    COMPROBAR(a.fig[AMBIENTE_HUMO].y >= 20 && a.fig[AMBIENTE_HUMO].y <= 23);
    cambios = 0;
    ultimo = a.fig[AMBIENTE_HUMO].cuadro;
    for (f = 0; f < 32; f++)
    {
        ambiente_actualizar(&a, 0);
        if (a.fig[AMBIENTE_HUMO].cuadro != ultimo)
        {
            cambios++;
            ultimo = a.fig[AMBIENTE_HUMO].cuadro;
        }
    }
    COMPROBAR(cambios == 4);                              /* Aletea cada 8 frames.         */
    x0 = a.fig[AMBIENTE_HUMO].x;
    ambiente_actualizar(&a, 64);                          /* La cámara avanza 64: el pájaro */
    COMPROBAR(a.fig[AMBIENTE_HUMO].x == x0 - 32);         /* retrocede 32 (frame par: no avanza). */
    ambiente_iniciar(&a);
    for (f = 0; f < 1024; f++)                            /* 512 px: la vuelta entera.     */
    {
        ambiente_actualizar(&a, 0);
    }
    COMPROBAR(a.bandada == 512);
    vistos = 0;
    for (f = 0; f < 1024; f++)                            /* En una vuelta, a la vista algo */
    {                                                     /* más de la mitad del tiempo.   */
        ambiente_actualizar(&a, 0);
        vistos = (u8)(vistos + (f < 1024 && a.fig[AMBIENTE_HUMO].visible && (f & 7) == 0));
    }
    COMPROBAR(vistos > 60 && vistos < 75);
}
