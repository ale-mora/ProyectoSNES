/*---------------------------------------------------------------------------------
    ejercicio_01_gravedad.c — Ejercicio de la unidad 01: el salto de Tarou en
    punto fijo 12.4.

    PROPÓSITO
      Simular frame a frame el salto completo de Tarou con las constantes reales
      del juego (VEL_SALTO = -96, GRAVEDAD = 4) y en el mismo orden que
      tarou_actualizar() en src/juego/tarou.c: primero la gravedad cambia la
      velocidad, después la velocidad mueve la posición. Imprime cada frame,
      el apogeo, la altura máxima y el frame en que aterriza. Al final calcula
      la altura que saldría con el orden al revés, para ver que el orden importa.
      Los printf ya están escritos: solo rellena los TODO 1 a 6.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), sistema/puntofijo.h (PF, PF_ENTERO; trae
                sistema/tipos.h).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   tarou_actualizar (paso 2: gravedad) y mover_y (paso 4: y += vy)
                en src/juego/tarou.c; prueba_apogeo_y_altura y prueba_aterrizaje
                en tests/prueba_tarou.c (léelas DESPUÉS de resolverlo).
      Compilar: gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I..\..\src -o ejercicio_01.exe ejercicio_01_gravedad.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      y_fp        s16  PF(75)..PF(144)  posición vertical de la celda, en 12.4
      vy_fp       s16  -92..92          velocidad vertical en 12.4 (negativa = sube)
      y_min_fp    s16  PF(75)..PF(144)  la y más pequeña vista (el punto más alto)
      frame_apogeo u8  ?                frame en que se alcanzó y_min_fp
      frame        u8  1..47            número de frame
      y2_fp, vy2_fp s16                 lo mismo con el orden al revés (TODO 6)

    → conceptos (README de la unidad 01): punto fijo 12.4, PF, PF_ENTERO
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include "sistema/puntofijo.h"       /* PF(n) y PF_ENTERO(x): las del juego.              */

/*
 * SALIDA ESPERADA (exacta): 47 líneas "frame NN: ..." y 4 de resumen. La
 * completa está en capturas/salida_ejercicio_01_gravedad.txt; las primeras y
 * las últimas son:
 *   frame  1: vy=-92 y_fp=2212 y=138 px
 *   frame  2: vy=-88 y_fp=2124 y=132 px
 *   ...
 *   frame 47: vy=92 y_fp=2304 y=144 px
 *   Apogeo en el frame 23: y minima 75 px
 *   Altura maxima: 69 px
 *   Aterriza en el frame 47
 *   Con el orden al reves: 75 px
 *
 * Antes de escribir código, comprueba a mano el frame 1: ¿por qué vy vale -92 y
 * no -96? ¿Por qué y_fp vale 2212? (2304 es PF(144).) Y piensa: si cada frame
 * vy sube 4 y empieza en -96, ¿en qué frame deja de ser negativa?
 *
 * Con los TODO sin hacer, el programa compila sin avisos e imprime 60 líneas
 * con ceros (el seguro FRAMES_MAX corta el bucle).
 */

/*--- Las constantes de src/juego/tarou.h (cópialas tal cual) -----------------------*/
#define GRAVEDAD 4                   /* 0,25 px por frame cada frame.                     */
#define VEL_SALTO (-96)              /* -6 px por frame al despegar.                      */
#define Y_SUELO 144                  /* y de la celda con los pies en el suelo (208 - 64).*/
#define FRAMES_MAX 60                /* Seguro: el bucle nunca da más de 60 vueltas.      */

int main(void)                       /* El programa empieza aquí.                         */
{
    s16 y_fp = 0;                    /* Posición vertical en 12.4.                        */
    s16 vy_fp = 0;                   /* Velocidad vertical en 12.4.                       */
    s16 y_min_fp = 0;                /* El punto más alto (la y más pequeña).             */
    u8 frame_apogeo = 0;             /* Cuándo se alcanzó.                                */
    u8 frame = 0;                    /* Frames desde el despegue.                         */
    s16 y2_fp = 0;                   /* TODO 6: posición con el orden al revés.           */
    s16 vy2_fp = 0;                  /* TODO 6: velocidad con el orden al revés.          */
    s16 y2_min_fp = 0;               /* TODO 6: su punto más alto.                        */

    /* TODO 1: la celda empieza en el suelo (y_fp = Y_SUELO en 12.4: usa PF) y
       despega con VEL_SALTO (que YA está en 12.4). y_min_fp empieza valiendo y_fp. */

    do
    {
        frame++;                     /* Un frame más.                                     */

        /* TODO 2: el orden de tarou_actualizar: primero sumar GRAVEDAD a vy_fp,
           después sumar vy_fp a y_fp. Usa (s16)( ... ) en cada suma. */

        /* TODO 3: aterrizar. Si y_fp llega a PF(Y_SUELO) o lo pasa, ponla
           exactamente en PF(Y_SUELO) (como mover_y: sin fracción). */

        /* TODO 4: si y_fp es menor que y_min_fp, guarda y_fp en y_min_fp y el
           frame actual en frame_apogeo. Compara en punto fijo, no en píxeles. */

        printf("frame %2u: vy=%d y_fp=%d y=%u px\n",
               (unsigned)frame, vy_fp, y_fp, (unsigned)PF_ENTERO(y_fp));
    } while (y_fp < PF(Y_SUELO) && frame < FRAMES_MAX);  /* Hasta tocar el suelo.         */

    /* TODO 5: completa la cuenta de la altura: lo que subió la celda, en píxeles
       (Y_SUELO menos el píxel de y_min_fp). Ahora imprime 0. */
    printf("Apogeo en el frame %u: y minima %u px\n", (unsigned)frame_apogeo,
           (unsigned)PF_ENTERO(y_min_fp));
    printf("Altura maxima: %u px\n", 0u);
    printf("Aterriza en el frame %u\n", (unsigned)frame);

    /* TODO 6: el mismo salto con el orden al revés (primero mover y DESPUÉS sumar
       la gravedad). Prepara y2_fp, vy2_fp e y2_min_fp como en el TODO 1 y repite
       mientras vy2_fp < 0 (solo hace falta la subida), guardando el mínimo. */
    while (vy2_fp < 0)               /* Mientras suba.                                    */
    {
        vy2_fp = 0;                  /* Borra esta línea al escribir el TODO 6.           */
    }
    printf("Con el orden al reves: %u px\n", (unsigned)(Y_SUELO - PF_ENTERO(y2_min_fp)));
    (void)y2_fp;                     /* Borra esta línea cuando uses y2_fp.               */

    return 0;                        /* Terminó bien.                                     */
}
