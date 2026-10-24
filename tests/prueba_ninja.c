/*---------------------------------------------------------------------------------
    prueba_ninja.c — Pruebas de enemigos/ninja.c (el ninja básico).

    PROPÓSITO
      Fijar la IA mínima: la oleada ocupa (zona + 1) ranuras (máximo 3), alternando
      izquierda y derecha fuera de la pantalla, en carriles distintos; cada ninja mira y
      camina hacia el protagonista a 1 px por frame hasta NINJA_DISTANCIA, ajusta su y,
      anima entre 0 y NINJA_CUADROS − 1 y se queda parado y en el cuadro 0 al llegar.

    SE CONECTA CON
      Incluye:  minitest.h, enemigos/ninja.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_ninja).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Oleada, IA
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "enemigos/ninja.h"

void pruebas_ninja(void)
{
    Ninja n[NINJAS_MAX];
    u16 f;
    u8 k, cuadro_max;

    ninja_iniciar_todos(n);
    COMPROBAR(ninja_vivos(n) == 0);

    /* Zona 0: un ninja; zona 1: dos; zona 2 y 5: tres (el máximo). */
    COMPROBAR(ninja_oleada(n, 300, 0) == 1);
    COMPROBAR(ninja_vivos(n) == 1);
    ninja_derrotar_todos(n);
    COMPROBAR(ninja_vivos(n) == 0);
    COMPROBAR(ninja_oleada(n, 300, 1) == 2);
    COMPROBAR(ninja_vivos(n) == 2);
    ninja_derrotar_todos(n);
    COMPROBAR(ninja_oleada(n, 300, 5) == NINJAS_MAX);
    COMPROBAR(ninja_vivos(n) == NINJAS_MAX);

    /* Nacen fuera de la pantalla: el primero a la izquierda, el segundo a la derecha. */
    COMPROBAR(n[0].x == 300 - NINJA_ENTRADA && n[0].mira_izq == 0);
    COMPROBAR(n[1].x == 300 + 256 + NINJA_ENTRADA && n[1].mira_izq == 1);
    COMPROBAR(n[0].y != n[1].y && n[1].y != n[2].y && n[0].y != n[2].y);  /* Carriles distintos. */

    /* Una oleada con ranuras ocupadas solo rellena las libres. */
    ninja_derrotar_todos(n);
    ninja_oleada(n, 300, 0);                         /* Ocupa la ranura 0.                  */
    COMPROBAR(ninja_oleada(n, 300, 2) == 2);         /* Pide 3 y solo hay 2 libres.         */
    COMPROBAR(ninja_vivos(n) == 3);

    /* Camina hacia el protagonista y se detiene a NINJA_DISTANCIA. */
    ninja_iniciar_todos(n);
    n[0].vivo = 1; n[0].x = 100; n[0].y = 200; n[0].cuadro = 0; n[0].reloj = 0;
    n[0].dentro = 1;                                 /* Ya en pantalla: se detiene a la distancia. */
    cuadro_max = 0;
    for (f = 0; f < 400; f++)
    {
        ninja_actualizar(n, 400, 200);               /* El objetivo, a la derecha.          */
        if (n[0].cuadro > cuadro_max)
        {
            cuadro_max = n[0].cuadro;
        }
    }
    COMPROBAR(n[0].x == 400 - NINJA_DISTANCIA);      /* Se paró a la distancia.             */
    COMPROBAR(n[0].mira_izq == 0);
    COMPROBAR(n[0].estado == NINJA_PARADO && n[0].cuadro == 0);
    COMPROBAR(cuadro_max == NINJA_CUADROS - 1);      /* Recorrió todos los cuadros de andar. */

    /* El objetivo a la izquierda: se vuelve y se acerca por ese lado. */
    ninja_actualizar(n, 50, 200);
    COMPROBAR(n[0].mira_izq == 1 && n[0].estado == NINJA_ANDANDO);

    /* Ajusta su y de uno en uno hasta el carril del objetivo, sin pasarse. */
    ninja_iniciar_todos(n);
    n[0].vivo = 1; n[0].x = 300; n[0].y = 190; n[0].reloj = 0; n[0].dentro = 1;
    for (f = 0; f < 100; f++)
    {
        ninja_actualizar(n, 300, 210);               /* Misma x: no avanza en x.            */
    }
    COMPROBAR(n[0].y == 210 && n[0].x == 300);
    for (f = 0; f < 100; f++)
    {
        ninja_actualizar(n, 300, 186);               /* Y ahora sube.                       */
    }
    COMPROBAR(n[0].y == 186);

    /* Dentro de la pantalla ya no sale: viene de fuera sin recortes y, al entrar, se limita. */
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 1);                         /* [0] a la izquierda; [1] a la derecha. */
    COMPROBAR(n[1].x == 300 + 256 + NINJA_ENTRADA && n[1].dentro == 0);
    ninja_limitar_x(n + 1, 316, 540);                /* Fuera (x = 596): no se toca.        */
    COMPROBAR(n[1].x == 300 + 256 + NINJA_ENTRADA && n[1].dentro == 0);
    n[1].x = 500;
    ninja_limitar_x(n + 1, 316, 540);                /* Entra: ahora cuenta como dentro.    */
    COMPROBAR(n[1].dentro == 1 && n[1].x == 500);
    n[1].x = 600;                                    /* Intenta salir por la derecha...     */
    ninja_limitar_x(n + 1, 316, 540);
    COMPROBAR(n[1].x == 540);                        /* ...y se queda en el borde.          */
    n[1].x = 100;                                    /* O por la izquierda.                 */
    ninja_limitar_x(n + 1, 316, 540);
    COMPROBAR(n[1].x == 316);

    /* Mientras viene de fuera no se detiene a NINJA_DISTANCIA: sigue hasta entrar. */
    ninja_iniciar_todos(n);
    n[0].vivo = 1; n[0].x = 600; n[0].y = 200; n[0].reloj = 0;
    for (f = 0; f < 20; f++)
    {
        ninja_actualizar(n, 580, 200);               /* A 20 px del objetivo (< 40)...      */
    }
    COMPROBAR(n[0].x == 580);                        /* ...sigue andando hasta alcanzarlo.  */

    /* El primer paso de y es en frames impares del reloj (reparte los pasos). */
    for (k = 0; k < NINJAS_MAX; k++)
    {
        n[k].vivo = 0;
    }
    COMPROBAR(ninja_vivos(n) == 0);
}
