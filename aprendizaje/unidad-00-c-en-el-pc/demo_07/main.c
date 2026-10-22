/*---------------------------------------------------------------------------------
    demo_07/main.c — Demo 7 de la unidad 00: un programa hecho de varios archivos.

    PROPÓSITO
      Usa el reloj de reloj.h / reloj.c sin saber cómo está hecho por dentro:
      llama a sus funciones (por sus prototipos) y lee frames_totales (por su
      extern). Además enseña las tres formas de static: una variable de
      archivo sin valor inicial (vale 0 en el PC, basura en la SNES), una
      función static de este archivo y una variable static DENTRO de una
      función, que conserva su valor entre llamadas.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), tipos.h, reloj.h.
      Lo usan:  nadie: es el programa.
      Compilar (desde la carpeta de la unidad), todo de una vez:
                gcc -std=c99 -Wall -Wextra -o demo_07.exe demo_07/main.c demo_07/reloj.c
      o en dos pasos, compilar cada .c a su .o y después enlazar:
                gcc -std=c99 -Wall -Wextra -c demo_07/main.c -o main.o
                gcc -std=c99 -Wall -Wextra -c demo_07/reloj.c -o reloj.o
                gcc -o demo_07.exe main.o reloj.o

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      sin_valor  u8   static de archivo, sin inicializar   0 en el PC
      llamadas   u8   static local de informe()            cuenta las llamadas
      f          u8   0..148                               contador de los bucles

    → conceptos (README de la unidad 00): varios archivos, prototipo, extern,
      static (de archivo, de función y local), compilar y enlazar
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include "tipos.h"                   /* Primera vez: se pega.                             */
#include "reloj.h"                   /* Vuelve a incluir tipos.h: la guarda lo salta.     */

static u8 sin_valor;                 /* static de archivo SIN "= 0".                      */

/* Función static: solo existe para main.c. Su variable `llamadas` es static
   LOCAL: se crea una sola vez y conserva el valor entre llamadas (una variable
   local normal se crearía de nuevo en cada llamada). */
static void informe(void)
{
    static u8 llamadas = 0;          /* Este "= 0" se aplica una vez, no en cada llamada. */
    llamadas++;
    printf("2. informe %d: frames_totales=%d segundos=%d\n",
           llamadas, frames_totales, reloj_segundos());   /* frames_totales: por extern. */
}

int main(void)
{
    /* 1. Una static de archivo sin valor: el C estándar la pone a 0... en el PC. */
    printf("1. static sin inicializar vale %d en el PC (en la SNES: lo que hubiera en la RAM)\n",
           sin_valor);

    /* 2. El reloj, usado solo a través de su .h. */
    reloj_iniciar();                 /* Imprescindible: deja el reloj a cero.             */
    reloj_avanzar();                 /* Frame 1.                                          */
    informe();
    for (u8 f = 0; f < 59; f++)      /* Hasta el frame 60: un segundo completo.           */
    {
        reloj_avanzar();
    }
    informe();
    for (u8 f = 0; f < 90; f++)      /* Hasta el frame 150: dos segundos y medio.         */
    {
        reloj_avanzar();
    }
    informe();

    /* 3. Reiniciar el reloj no reinicia la static local de informe(): no hay
          manera de llegar a ella desde fuera de la función. */
    reloj_iniciar();
    informe();

    return 0;                        /* Terminó bien.                                     */
}
