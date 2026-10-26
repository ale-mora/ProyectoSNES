/*---------------------------------------------------------------------------------
    demo_24_oleada.c — Demo de la unidad 24: una oleada de ninjas SIMULADA con el
    código real del juego.

    PROPÓSITO
      Usa tal cual enemigos/ninja.c, juego/oleadas.c y juego/camara.c (compilados en el PC
      con -DPRUEBAS_PC) y repite, en `un_cuadro`, el bloque de la fase 1 que reparte los
      turnos (src/pantallas/pantalla_fase1.c, el `for` que llama a ninja_actualizar,
      ninja_limitar_x y ninja_intentar_atacar). Cuatro partes:

        A. Dos zonas seguidas con un «jugador» de juguete: Tarou camina a la derecha, la
           cámara se bloquea en el punto de la zona, salen los ninjas, y el jugador los
           derriba de uno en uno (ninja_recibir) hasta que no queda ninguno; entonces
           oleadas_vencer libera la cámara. Se imprime cada EVENTO con su cuadro.
        B. El mismo escenario, la oleada de 3 ninjas, cuadro a cuadro: una columna por
           ranura con la letra del estado (tabla más abajo) y cuántos atacan a la vez.
        C. El testigo de ataque: la misma oleada con 1, 2 y 3 turnos a la vez, 900 cuadros
           y Tarou quieto: máximo simultáneo y tajos que da cada ranura.
        D. Esa oleada de 3 como tira de estados (una letra cada 6 cuadros) con 1 y 2 turnos.

      El jugador de juguete NO usa mundo.c ni combate.c: es un guion de la propia demo.
      Tarou no recibe daño aquí (no hay vida), así que la oleada no se acaba sola.

      Letras del estado: '.' ranura libre · 'a' andando fuera de pantalla · 'A' andando ya
      dentro · 'p' parado (esperando) · 'T' dando un tajo · 'r' aturdido · 'D' derribado.

    SE CONECTA CON
      Incluye:  <stdio.h>, enemigos/ninja.h, juego/oleadas.h, juego/camara.h,
                juego/nivel.h (ANCHO_PANTALLA).
      Compilar (desde aprendizaje/unidad-24-enemigos-y-oleadas/):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_24.exe
            demo_24_oleada.c ../../src/enemigos/ninja.c ../../src/juego/oleadas.c
            ../../src/juego/camara.c
      Uso:  demo_24.exe            las tres partes
            demo_24.exe a|b|c|d    una sola

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      ninjas   Ninja[3]  las tres ranuras fijas (enemigos/ninja.h)
      oleadas  Oleadas   zona en curso y si la cámara está bloqueada
      camara   u16       scroll; sube solo con camara_avanzar
      tarou_x  u16       x de los pies del Tarou de juguete; tarou_y fija el carril

    → glosario: Oleada, Zona, IA, Máquina de estados, Carril
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf.                                     */
#include "enemigos/ninja.h"              /* Ninja, ninja_oleada, ninja_actualizar...    */
#include "juego/oleadas.h"               /* Oleadas, oleadas_actualizar...              */
#include "juego/camara.h"                /* camara_avanzar.                             */
#include "juego/nivel.h"                 /* ANCHO_PANTALLA.                             */

#define MARGEN_NINJA 32                  /* El mismo valor que pantalla_fase1.c.        */
#define TAROU_Y 200                      /* El carril de en medio (Y_INICIAL del juego). */

static Ninja ninjas[NINJAS_MAX];
static Oleadas oleadas;
static u16 camara;
static u16 tarou_x;
static u8 tarou_y = TAROU_Y;
static u8 tajos[NINJAS_MAX];             /* Parte C: tajos empezados por ranura.        */
static u8 max_a_la_vez;                  /* Parte C: el máximo de atacantes simultáneos. */

/*--- letra: un carácter por estado, para la tabla cuadro a cuadro ---------------------*/
static char letra(const Ninja *n)
{
    if (!n->vivo)
    {
        return '.';
    }
    switch (n->estado)
    {
    case NINJA_ANDANDO:
        return (char)(n->dentro ? 'A' : 'a');
    case NINJA_PARADO:
        return 'p';
    case NINJA_ATACANDO:
        return 'T';
    case NINJA_RECIBIR:
        return 'r';
    default:
        return 'D';
    }
}

/*--- un_cuadro: el bloque de IA de pantalla_fase1.c, con los turnos como parámetro ----
    `turnos_max` hace de NINJA_TURNOS (en el juego es una constante). Devuelve cuántos
    tajos EMPEZARON este cuadro. */
static u8 un_cuadro(u8 turnos_max, u8 *empezados)
{
    u8 k, turnos, nuevos = 0;

    turnos = ninja_atacando(ninjas);                 /* Cuántos dan un tajo ahora.          */
    for (k = 0; k < NINJAS_MAX; k++)
    {
        empezados[k] = 0;
        if (ninjas[k].vivo)
        {
            ninja_actualizar(ninjas + k, tarou_x, tarou_y);
            ninja_limitar_x(ninjas + k, (u16)(camara + MARGEN_NINJA),
                            (u16)(camara + ANCHO_PANTALLA - MARGEN_NINJA));
            if (ninja_intentar_atacar(ninjas + k, tarou_x, tarou_y, (u8)(turnos < turnos_max)))
            {
                turnos++;                            /* El turno queda cogido.              */
                empezados[k] = 1;
                nuevos++;
            }
        }
    }
    return nuevos;
}

/*--- Partes A y B: dos zonas con un jugador de juguete ---------------------------------
    `mostrar_eventos` imprime los sucesos (A); `cuadro_a_cuadro` imprime la tabla (B)
    entre los cuadros [desde, hasta) cada `cada` cuadros. */
static void recorrido(u8 mostrar_eventos, u8 cuadro_a_cuadro, u16 desde, u16 hasta, u16 cada)
{
    u16 f, limite;
    u8 k, dentro_antes[NINJAS_MAX], estado_antes[NINJAS_MAX];
    u8 empezados[NINJAS_MAX], a_la_vez, libre;
    u16 proximo_derribo = 0;                         /* Cuadro en que el jugador derriba al siguiente. */

    ninja_iniciar_todos(ninjas);
    oleadas_iniciar(&oleadas);
    camara = 0;
    tarou_x = 64;                                    /* X_INICIAL del juego.                */
    for (k = 0; k < NINJAS_MAX; k++)
    {
        dentro_antes[k] = 0;
        estado_antes[k] = NINJA_PARADO;
    }
    for (f = 0; f < hasta; f++)
    {
        /*--- El jugador de juguete: anda a la derecha salvo con la cámara bloqueada ----*/
        if (!oleadas.bloqueada)
        {
            tarou_x = (u16)(tarou_x + 2);            /* PASO_X de mundo.h.                  */
        }
        else if (tarou_x < (u16)(camara + 128))
        {
            tarou_x = (u16)(tarou_x + 2);            /* Si quedó atrás, vuelve al centro.   */
        }
        /*--- La cámara y la zona (en el orden de pantalla_fase1.c) -------------------*/
        limite = oleadas_limite(&oleadas);
        camara = camara_avanzar(camara, tarou_x, limite);
        if (oleadas_actualizar(&oleadas, camara))
        {
            u8 nacidos = ninja_oleada(ninjas, camara, oleadas.actual);

            proximo_derribo = (u16)(f + 330);        /* Les da tiempo a llegar y a atacar.  */
            if (mostrar_eventos)
            {
                printf("[f=%4u] la camara llega a %u: ZONA %u BLOQUEADA, nacen %u ninja(s)\n",
                       f, camara, oleadas.actual, nacidos);
            }
        }
        /*--- El jugador derriba al ninja de la ranura más baja que siga vivo -----------*/
        if (oleadas.bloqueada && proximo_derribo != 0 && f >= proximo_derribo)
        {
            for (k = 0; k < NINJAS_MAX; k++)
            {
                if (ninjas[k].vivo && ninjas[k].estado != NINJA_DERRIBADO)
                {
                    ninja_recibir(ninjas + k, NINJA_VIDA, 1, (u8)(ninjas[k].x < tarou_x), 1);
                    if (mostrar_eventos)
                    {
                        printf("[f=%4u] el jugador derriba al ninja %u\n", f, k);
                    }
                    break;
                }
            }
            proximo_derribo = (u16)(f + 60);
        }
        /*--- La IA de los ninjas ---------------------------------------------------*/
        un_cuadro(NINJA_TURNOS, empezados);
        a_la_vez = ninja_atacando(ninjas);
        for (k = 0; k < NINJAS_MAX; k++)
        {
            if (mostrar_eventos)
            {
                if (ninjas[k].vivo && ninjas[k].dentro && !dentro_antes[k])
                {
                    printf("[f=%4u] el ninja %u ENTRA en pantalla (x=%u, camara=%u): ya no sale\n",
                           f, k, ninjas[k].x, camara);
                }
                if (empezados[k])
                {
                    printf("[f=%4u] el ninja %u coge turno y da un tajo (atacan %u de %u)\n",
                           f, k, a_la_vez, NINJA_TURNOS);
                }
                if (estado_antes[k] == NINJA_ATACANDO && ninjas[k].estado == NINJA_PARADO)
                {
                    printf("[f=%4u] el ninja %u acaba el tajo y descansa %u cuadros\n", f, k,
                           ninjas[k].espera);
                }
            }
            dentro_antes[k] = (u8)(ninjas[k].vivo && ninjas[k].dentro);
            estado_antes[k] = ninjas[k].estado;
        }
        libre = (u8)(oleadas.bloqueada && ninja_vivos(ninjas) == 0);
        if (libre)                                   /* Oleada sin ninjas: vencida.         */
        {
            oleadas_vencer(&oleadas);
            proximo_derribo = 0;
            if (mostrar_eventos)
            {
                printf("[f=%4u] no queda ninguno: oleada VENCIDA, camara libre (siguiente zona %u)\n",
                       f, oleadas.actual);
            }
        }
        if (cuadro_a_cuadro && f >= desde && ((f - desde) % cada) == 0)
        {
            printf("f=%4u cam=%4u tarou=%4u | ", f, camara, tarou_x);
            for (k = 0; k < NINJAS_MAX; k++)
            {
                printf("N%u:%c x=%4u y=%3u esp=%2u  ", k, letra(ninjas + k),
                       ninjas[k].vivo ? ninjas[k].x : 0, ninjas[k].vivo ? ninjas[k].y : 0,
                       ninjas[k].vivo ? ninjas[k].espera : 0);
            }
            printf("| atacan %u\n", a_la_vez);
        }
        if (oleadas.actual >= 2 && !oleadas.bloqueada && mostrar_eventos)
        {
            break;                                   /* Dos zonas hechas: basta.            */
        }
    }
}

/*--- Parte C: el testigo con 1, 2 y 3 turnos a la vez ----------------------------------*/
static void parte_c(void)
{
    u8 turnos_max, k, empezados[NINJAS_MAX];
    u16 f;

    printf("Parte C: oleada de 3 ninjas (zona 2), Tarou quieto en x=384, 900 cuadros\n");
    printf("  turnos | max a la vez | tajos ranura 0 | ranura 1 | ranura 2 | total\n");
    for (turnos_max = 1; turnos_max <= 3; turnos_max++)
    {
        ninja_iniciar_todos(ninjas);
        camara = 256;
        tarou_x = 384;                               /* El centro de la pantalla.           */
        ninja_oleada(ninjas, camara, 2);
        max_a_la_vez = 0;
        for (k = 0; k < NINJAS_MAX; k++)
        {
            tajos[k] = 0;
        }
        for (f = 0; f < 900; f++)
        {
            un_cuadro(turnos_max, empezados);
            for (k = 0; k < NINJAS_MAX; k++)
            {
                tajos[k] = (u8)(tajos[k] + empezados[k]);
            }
            if (ninja_atacando(ninjas) > max_a_la_vez)
            {
                max_a_la_vez = ninja_atacando(ninjas);
            }
        }
        printf("  %6u | %12u | %14u | %8u | %8u | %5u\n", turnos_max, max_a_la_vez, tajos[0],
               tajos[1], tajos[2], (unsigned)(tajos[0] + tajos[1] + tajos[2]));
    }
    printf("  (en el juego NINJA_TURNOS = %u)\n", NINJA_TURNOS);
}

/*--- Parte D: la misma oleada de 3, como tira de estados (1 carácter = 6 cuadros) ------*/
static void parte_d(void)
{
    u8 turnos_max, k, empezados[NINJAS_MAX];
    u16 f;
    char tira[NINJAS_MAX][101];
    char vez[101];

    printf("Parte D: oleada de 3 ninjas, Tarou quieto; 1 caracter = 6 cuadros, 600 cuadros\n");
    for (turnos_max = 1; turnos_max <= 2; turnos_max++)
    {
        ninja_iniciar_todos(ninjas);
        camara = 256;
        tarou_x = 384;
        ninja_oleada(ninjas, camara, 2);
        for (f = 0; f < 600; f++)
        {
            un_cuadro(turnos_max, empezados);
            if (f % 6 == 5)                          /* Una muestra cada 6 cuadros.         */
            {
                for (k = 0; k < NINJAS_MAX; k++)
                {
                    tira[k][f / 6] = letra(ninjas + k);
                }
                vez[f / 6] = (char)('0' + ninja_atacando(ninjas));
            }
        }
        for (k = 0; k < NINJAS_MAX; k++)
        {
            tira[k][100] = 0;
        }
        vez[100] = 0;
        printf("  turnos = %u\n", turnos_max);
        for (k = 0; k < NINJAS_MAX; k++)
        {
            printf("    N%u %s\n", k, tira[k]);
        }
        printf("    # %s   (cuantos atacan)\n", vez);
    }
}

int main(int argc, char **argv)
{
    char parte = (char)(argc > 1 ? argv[1][0] : 0);

    if (parte == 0 || parte == 'a')
    {
        printf("Parte A: dos zonas con un jugador de juguete (eventos)\n");
        recorrido(1, 0, 0, 2000, 1);
        printf("\n");
    }
    if (parte == 0 || parte == 'b')
    {
        printf("Parte B: la zona 0 (1 ninja) y la zona 1 (2 ninjas), un cuadro de cada 20\n");
        printf("         entre el cuadro 140 y el 760; N0..N2 son las ranuras\n");
        recorrido(0, 1, 140, 760, 20);
        printf("\n");
    }
    if (parte == 0 || parte == 'c')
    {
        parte_c();
        printf("\n");
    }
    if (parte == 0 || parte == 'd')
    {
        parte_d();
    }
    return 0;
}
