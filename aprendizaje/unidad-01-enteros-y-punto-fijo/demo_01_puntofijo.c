/*---------------------------------------------------------------------------------
    demo_01_puntofijo.c — Demo 2 de la unidad 01: decimales sin decimales.

    PROPÓSITO
      Enseña, en cuatro pasos numerados en la salida, el punto fijo del juego:
      mover un valor a 1,5 px por frame en formato 12.4 y ver frame a frame la
      parte entera (el píxel que se dibuja) y la fracción (dieciseisavos);
      moverlo hacia la izquierda con una velocidad negativa; la trampa de
      PF_ENTERO con una posición negativa; y el formato 8.8 del motor de
      escenas, que sube un brillo de 0 a 15 en 32 frames (0,47 por frame).

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), sistema/puntofijo.h (PF, PF_ENTERO, PF_BITS;
                trae sistema/tipos.h: u8, u16, s16).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   x_fp y vx_fp de src/juego/tarou.h; Pista.valor y la macro
                RAMPA de src/escena/escena.h; escena_avanzar en escena.c.
      Compilar: gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I..\..\src -o demo_01_puntofijo.exe demo_01_puntofijo.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      x_fp       s16   PF(10)..PF(22)   posición en 12.4 (como Tarou.x_fp)
      vx_fp      s16   -24 / 24         velocidad en 12.4: 1,5 px por frame
      y_fp       s16   -16              una posición por encima de la pantalla
      valor      u16   0..0x0F00        brillo en 8.8 (como Pista.valor)
      paso       s16   120              cuánto sube el brillo por frame, en 8.8
      f          u8    1..32            número de frame

    → conceptos (README de la unidad 01): punto fijo 12.4 y 8.8, PF, PF_ENTERO
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include "sistema/puntofijo.h"       /* PF(n): n * 16. PF_ENTERO(x): x / 16 (x >= 0).     */

#define VEL_1_5 24                   /* 1,5 px por frame en 12.4: 1,5 * 16 = 24.          */
#define FRAMES_RAMPA 32              /* El fundido de entrada del logo dura 32 frames.    */
#define BRILLO_FINAL 15              /* Brillo máximo de la SNES.                         */
/* El paso de RAMPA(0, 32, PISTA_BRILLO, 0, 15), con la misma cuenta que escena.h. */
#define PASO_RAMPA ((s16)((((s16)BRILLO_FINAL - (s16)0) * 128 / FRAMES_RAMPA) * 2))

int main(void)                       /* El programa empieza aquí.                         */
{
    s16 x_fp = PF(10);               /* Empieza en el píxel 10: 10 * 16 = 160.            */
    s16 vx_fp = VEL_1_5;             /* Hacia la derecha, 1,5 px por frame.               */
    s16 y_fp;                        /* Para el paso 3.                                   */
    u16 valor = 0;                   /* Brillo 0 en 8.8 (byte alto = 0).                  */
    s16 paso = PASO_RAMPA;           /* 120: 120 / 256 = 0,47 por frame.                  */
    u8 f;                            /* Número de frame.                                  */

    /* 1. A 1,5 px por frame: el píxel avanza 1, 2, 1, 2... y la fracción guarda el resto. */
    printf("1. inicio: x_fp=%d -> pixel %u, fraccion %u/16\n",
           x_fp, (unsigned)PF_ENTERO(x_fp), (unsigned)(x_fp & 15));
    for (f = 1; f <= 8; f++)
    {
        x_fp = (s16)(x_fp + vx_fp);  /* posición += velocidad, igual que tarou.c.         */
        printf("1. frame %u: x_fp=%d -> pixel %u, fraccion %u/16\n",
               (unsigned)f, x_fp, (unsigned)PF_ENTERO(x_fp), (unsigned)(x_fp & 15));
    }

    /* 2. Hacia la izquierda: la velocidad es negativa, la posición sigue siendo >= 0. */
    vx_fp = (s16)-VEL_1_5;           /* -24: 1,5 px por frame hacia la izquierda.         */
    for (f = 1; f <= 4; f++)
    {
        x_fp = (s16)(x_fp + vx_fp);  /* Sumar un negativo es restar.                      */
        printf("2. frame %u: x_fp=%d -> pixel %u\n", (unsigned)f, x_fp, (unsigned)PF_ENTERO(x_fp));
    }

    /* 3. La trampa: PF_ENTERO de una posición negativa (un salto por encima de la pantalla). */
    y_fp = (s16)(PF(0) - 16);        /* Un píxel por encima de la fila 0: -16.            */
    printf("3. y_fp=%d -> PF_ENTERO da %u (no -1)\n", y_fp, (unsigned)PF_ENTERO(y_fp));
    printf("3. comparar en punto fijo: y_fp < 0 es %d (correcto)\n", y_fp < 0);

    /* 4. Formato 8.8: el byte alto es el entero y el bajo, 256avos (escena.c). */
    printf("4. paso de la rampa 0 -> 15 en %d frames: %d (= %d/256 por frame)\n",
           FRAMES_RAMPA, paso, paso);
    for (f = 1; f <= FRAMES_RAMPA; f++)
    {
        if (f < FRAMES_RAMPA)
        {
            valor = (u16)(valor + paso);              /* Un paso más.                     */
        }
        else
        {
            valor = (u16)(BRILLO_FINAL << 8);         /* Último frame: exacto (escena.c). */
        }
        if ((f & 3) == 0)                             /* Imprime uno de cada 4 frames.    */
        {
            printf("4. frame %2u: valor=0x%04X -> brillo %u\n",
                   (unsigned)f, (unsigned)valor, (unsigned)(valor >> 8));
        }
    }

    return 0;                        /* Terminó bien.                                     */
}
