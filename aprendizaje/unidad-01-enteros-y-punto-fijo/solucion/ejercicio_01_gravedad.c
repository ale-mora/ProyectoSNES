/*---------------------------------------------------------------------------------
    ejercicio_01_gravedad.c (SOLUCIÓN) — Ejercicio de la unidad 01: el salto de
    Tarou en punto fijo 12.4.

    PROPÓSITO
      Simular frame a frame el salto completo de Tarou con las constantes reales
      del juego (VEL_SALTO = -96, GRAVEDAD = 4) y en el mismo orden que
      tarou_actualizar() en src/juego/tarou.c: primero la gravedad cambia la
      velocidad, después la velocidad mueve la posición. Imprime cada frame,
      el apogeo, la altura máxima y el frame en que aterriza. Al final calcula
      la altura que saldría con el orden al revés, para ver que el orden importa.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), sistema/puntofijo.h (PF, PF_ENTERO; trae
                sistema/tipos.h).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   tarou_actualizar (paso 2: gravedad) y mover_y (paso 4: y += vy)
                en src/juego/tarou.c; prueba_apogeo_y_altura y prueba_aterrizaje
                en tests/prueba_tarou.c.
      Compilar: gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I..\..\..\src -o ejercicio_01.exe ejercicio_01_gravedad.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      y_fp        s16  PF(75)..PF(144)  posición vertical de la celda, en 12.4
      vy_fp       s16  -92..92          velocidad vertical en 12.4 (negativa = sube)
      y_min_fp    s16  PF(75)..PF(144)  la y más pequeña vista (el punto más alto)
      frame_apogeo u8  23               frame en que se alcanzó y_min_fp
      frame        u8  1..47            número de frame
      y2_fp, vy2_fp s16                 lo mismo con el orden al revés (TODO 6)

    → conceptos (README de la unidad 01): punto fijo 12.4, PF, PF_ENTERO
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include "sistema/puntofijo.h"       /* PF(n) y PF_ENTERO(x): las del juego.              */

/*
 * SALIDA ESPERADA (exacta): ver capturas/salida_ejercicio_01_gravedad.txt. Las
 * primeras y las últimas líneas son:
 *   frame  1: vy=-92 y_fp=2212 y=138 px
 *   frame  2: vy=-88 y_fp=2124 y=132 px
 *   ...
 *   frame 47: vy=92 y_fp=2304 y=144 px
 *   Apogeo en el frame 23: y minima 75 px
 *   Altura maxima: 69 px
 *   Aterriza en el frame 47
 *   Con el orden al reves: 75 px
 */

/*--- Las constantes de src/juego/tarou.h (cópialas tal cual) -----------------------*/
#define GRAVEDAD 4                   /* 0,25 px por frame cada frame.                     */
#define VEL_SALTO (-96)              /* -6 px por frame al despegar.                      */
#define Y_SUELO 144                  /* y de la celda con los pies en el suelo (208 - 64).*/
#define FRAMES_MAX 60                /* Seguro: el bucle nunca da más de 60 vueltas.      */

int main(void)                       /* El programa empieza aquí.                         */
{
    s16 y_fp;                        /* Posición vertical en 12.4.                        */
    s16 vy_fp;                       /* Velocidad vertical en 12.4.                       */
    s16 y_min_fp;                    /* El punto más alto (la y más pequeña).             */
    u8 frame_apogeo = 0;             /* Cuándo se alcanzó.                                */
    u8 frame = 0;                    /* Frames desde el despegue.                         */
    s16 y2_fp;                       /* TODO 6: posición con el orden al revés.           */
    s16 vy2_fp;                      /* TODO 6: velocidad con el orden al revés.          */
    s16 y2_min_fp;                   /* TODO 6: su punto más alto.                        */

    /* TODO 1: la celda empieza en el suelo y despega con VEL_SALTO. */
    y_fp = PF(Y_SUELO);              /* 144 px = 2304 en 12.4.                            */
    vy_fp = VEL_SALTO;               /* -96: ya está en 12.4 (no PF(-6): ver README).     */
    y_min_fp = y_fp;                 /* De momento, lo más alto es el punto de partida.   */

    do
    {
        frame++;                     /* Un frame más.                                     */

        /* TODO 2: el orden de tarou_actualizar: primero la gravedad, después mover. */
        vy_fp = (s16)(vy_fp + GRAVEDAD);   /* Paso 2 de tarou_actualizar.                */
        y_fp = (s16)(y_fp + vy_fp);        /* Paso 4 (mover_y): y += vy.                 */

        /* TODO 3: aterrizar. Si los pies llegan al suelo (o lo pasan), la celda se
           pone exactamente en Y_SUELO, como hace mover_y con PF(fila - ALTO_CELDA). */
        if (y_fp >= PF(Y_SUELO))
        {
            y_fp = PF(Y_SUELO);      /* Pegado al suelo, sin fracción.                    */
        }

        /* TODO 4: recordar el punto más alto y su frame. Se compara en punto fijo. */
        if (y_fp < y_min_fp)
        {
            y_min_fp = y_fp;         /* Nuevo punto más alto.                             */
            frame_apogeo = frame;    /* ...en este frame.                                 */
        }

        printf("frame %2u: vy=%d y_fp=%d y=%u px\n",
               (unsigned)frame, vy_fp, y_fp, (unsigned)PF_ENTERO(y_fp));
    } while (y_fp < PF(Y_SUELO) && frame < FRAMES_MAX);  /* Hasta tocar el suelo.         */

    /* TODO 5: la altura en píxeles es lo que subió la celda. y_min_fp es >= 0, así
       que PF_ENTERO es seguro aquí. */
    printf("Apogeo en el frame %u: y minima %u px\n", (unsigned)frame_apogeo,
           (unsigned)PF_ENTERO(y_min_fp));
    printf("Altura maxima: %u px\n", (unsigned)(Y_SUELO - PF_ENTERO(y_min_fp)));
    printf("Aterriza en el frame %u\n", (unsigned)frame);

    /* TODO 6: el mismo salto con el orden al revés (mover y DESPUÉS sumar la
       gravedad). Solo hace falta la subida: mientras vy2 sea negativa. */
    y2_fp = PF(Y_SUELO);             /* Mismo punto de partida...                         */
    vy2_fp = VEL_SALTO;              /* ...y misma velocidad inicial.                     */
    y2_min_fp = y2_fp;
    while (vy2_fp < 0)               /* Mientras suba.                                    */
    {
        y2_fp = (s16)(y2_fp + vy2_fp);     /* Primero mover...                            */
        vy2_fp = (s16)(vy2_fp + GRAVEDAD); /* ...después la gravedad.                     */
        if (y2_fp < y2_min_fp)
        {
            y2_min_fp = y2_fp;
        }
    }
    printf("Con el orden al reves: %u px\n", (unsigned)(Y_SUELO - PF_ENTERO(y2_min_fp)));

    return 0;                        /* Terminó bien.                                     */
}
