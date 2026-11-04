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

    /* Con la cámara a menos de NINJA_ENTRADA del principio del nivel, el de la izquierda nace
       en x = 0 (el borde del nivel): antes, cámara − 40 daba la vuelta en u16 (10 → 65506) y
       el ninja aparecía al otro lado del nivel. */
    ninja_derrotar_todos(n);
    ninja_oleada(n, 10, 1);
    COMPROBAR(n[0].x == 0 && n[0].mira_izq == 0);
    COMPROBAR(n[1].x == 10 + 256 + NINJA_ENTRADA);
    ninja_derrotar_todos(n);
    ninja_oleada(n, NINJA_ENTRADA, 0);               /* Justo en el límite: x = 0, sin recorte. */
    COMPROBAR(n[0].x == 0);
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

    /* Mientras viene de fuera no se detiene a NINJA_DISTANCIA ni en el objetivo: sigue hacia
       dentro de la pantalla (hacia donde mira al nacer) hasta entrar. */
    ninja_iniciar_todos(n);
    n[0].vivo = 1; n[0].x = 600; n[0].y = 200; n[0].reloj = 0; n[0].mira_izq = 1;
    for (f = 0; f < 30; f++)
    {
        ninja_actualizar(n, 580, 200);               /* A 20 px del objetivo (< 40)...      */
    }
    COMPROBAR(n[0].x == 570 && n[0].mira_izq == 1);  /* ...y lo pasa: aún no ha entrado.    */

    /* Tarou pegado al borde izquierdo (cámara + MARGEN_PANTALLA = 24) y un ninja que llega por la
       izquierda: el ninja no puede pararse fuera ni encima de Tarou sin entrar (antes se quedaba
       en x = 280 < 288, sin entrar nunca y sin atacar). */
    {
        u16 cam = 256;
        ninja_iniciar_todos(n);
        ninja_oleada(n, cam, 0);                     /* [0] nace a la izquierda.            */
        for (f = 0; f < 200; f++)
        {
            ninja_actualizar(n, cam + 24, 200);
            ninja_limitar_x(n, cam + 32, cam + 224);
        }
        COMPROBAR(n[0].dentro == 1);
        COMPROBAR(n[0].x >= cam + 32 && n[0].x <= cam + 224);
        n[0].espera = 0;
        COMPROBAR(ninja_intentar_atacar(n, cam + 24, 200, 1) == 1);   /* Y ya puede atacar. */
    }

    /* El primer paso de y es en frames impares del reloj (reparte los pasos). */
    for (k = 0; k < NINJAS_MAX; k++)
    {
        n[k].vivo = 0;
    }
    COMPROBAR(ninja_vivos(n) == 0);

    /* Los dibujos de «Ninja 1» (2026-11-04). */
    ninja_iniciar_todos(n);
    n[0].vivo = 1; n[0].vida = NINJA_VIDA; n[0].estado = NINJA_PARADO;
    for (k = 0; k < 64; k++)                         /* Parado: la guardia, en bucle.       */
    {
        n[0].reloj = k;
        COMPROBAR(ninja_dibujo(&n[0]) < NINJA_DIB_CAMINAR);
    }
    n[0].estado = NINJA_ANDANDO;                     /* Andando: sus 6 dibujos.             */
    for (k = 0; k < NINJA_CUADROS; k++)
    {
        n[0].cuadro = k;
        COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_CAMINAR + k);
    }
    n[0].estado = NINJA_ATACANDO;                    /* El puño fuera justo mientras golpea. */
    for (f = 0; f < NINJA_ATAQUE_FRAMES; f++)
    {
        n[0].timer = (u8)f;
        k = ninja_dibujo(&n[0]);
        COMPROBAR(k >= NINJA_DIB_ATAQUE && k < NINJA_DIB_RECIBIR);
        if (f >= NINJA_GOLPE_DESDE && f < NINJA_GOLPE_HASTA)
        {
            COMPROBAR(k == NINJA_DIB_ATAQUE + 2);
        }
    }
    n[0].estado = NINJA_RECIBIR;                     /* Recibir: avanza y acaba en el último. */
    n[0].timer = NINJA_RECIBIR_FRAMES;
    COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_RECIBIR);
    n[0].timer = 1;
    COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_RECIBIR + 5);
    /* Derribado con vida: cae, en el suelo, se levanta; nunca parpadea. */
    n[0].estado = NINJA_DERRIBADO;
    n[0].timer = NINJA_DERRIBO_FRAMES;
    COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_DERRIBADO);
    n[0].timer = NINJA_DERRIBO_FRAMES - 30;
    COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_DERRIBADO + 6);   /* En el suelo.               */
    n[0].timer = 1;
    COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_LEVANTARSE + 4);  /* Ya de pie.                 */
    for (f = 1; f <= NINJA_DERRIBO_FRAMES; f++)
    {
        n[0].timer = (u8)f;
        n[0].reloj = (u8)f;
        k = ninja_dibujo(&n[0]);
        COMPROBAR(k >= NINJA_DIB_DERRIBADO && k < NINJA_DIB_KO && !ninja_parpadea(&n[0]));
    }
    /* Sin vida: el KO, tumbado al final, parpadea y no sale de su tira. */
    n[0].vida = 0;
    n[0].timer = NINJA_DERRIBO_FRAMES;
    COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_KO && !ninja_parpadea(&n[0]));
    n[0].timer = 1;
    COMPROBAR(ninja_dibujo(&n[0]) == NINJA_DIB_KO + 6);
    n[0].reloj = 2;
    COMPROBAR(ninja_parpadea(&n[0]));
    for (f = 1; f <= NINJA_DERRIBO_FRAMES; f++)
    {
        n[0].timer = (u8)f;
        k = ninja_dibujo(&n[0]);
        COMPROBAR(k >= NINJA_DIB_KO && k < NINJA_DIBUJOS);
    }

    /* Los tres tipos (2026-11-04): la ranura k es el tipo k, con su vida y su daño. */
    ninja_iniciar_todos(n);
    COMPROBAR(ninja_oleada(n, 0, 2) == 3);
    COMPROBAR(n[0].tipo == NINJA_NORMAL && n[0].vida == 4 && ninja_danio(&n[0]) == 2);
    COMPROBAR(n[1].tipo == NINJA_MEDIANO && n[1].vida == 6 && ninja_danio(&n[1]) == 3);
    COMPROBAR(n[2].tipo == NINJA_GRANDE && n[2].vida == 8 && ninja_danio(&n[2]) == 4);
    /* Zona 1: un normal y un mediano. */
    ninja_iniciar_todos(n);
    COMPROBAR(ninja_oleada(n, 0, 1) == 2 && n[0].tipo == NINJA_NORMAL && n[1].tipo == NINJA_MEDIANO);
    /* El grande avanza 3 de cada 4 frames y no deja de andar (sin saltar a la guardia). */
    ninja_iniciar_todos(n);
    n[2].vivo = 1; n[2].tipo = NINJA_GRANDE; n[2].dentro = 1; n[2].x = 500; n[2].y = 200;
    n[2].reloj = 0; n[2].estado = NINJA_PARADO;
    for (f = 0; f < 40; f++)
    {
        ninja_actualizar(&n[2], 300, 200);
        COMPROBAR(n[2].estado == NINJA_ANDANDO);
    }
    COMPROBAR(n[2].x == 500 - 30);
}
