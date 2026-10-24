/*---------------------------------------------------------------------------------
    demo_01_logica.c — Demo 1 de la unidad 00: variables, if/else y for.

    PROPÓSITO
      Un programa completo para leer y ejecutar antes del ejercicio 1: Tarou
      empieza con 3 vidas, choca con 5 obstáculos seguidos y el programa
      cuenta lo que le queda. Muestra una variable que cambia, un for que
      repite y un if/else que elige.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_01.exe demo_01_logica.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      vidas       int    0..3     vidas que le quedan a Tarou
      obstaculo   int    1..5     número del obstáculo de esta vuelta del for

    → conceptos (README de la unidad 00): variable, if, for
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */

int main(void)                       /* El programa empieza aquí.                         */
{
    int vidas = 3;                   /* Declara la variable y le da su valor inicial.     */

    printf("Tarou empieza con %d vidas.\n", vidas);   /* %d se reemplaza por el valor.  */

    for (int obstaculo = 1; obstaculo <= 5; obstaculo++) {   /* Inicio; condición; paso. */
        if (vidas > 0) {             /* Solo si le queda alguna vida...                   */
            vidas = vidas - 1;       /* ...pierde una (la variable cambia de valor).      */
            printf("Obstaculo %d: Tarou choco. Vidas restantes: %d\n", obstaculo, vidas);
        } else {                     /* Si no le quedaba ninguna:                         */
            printf("Obstaculo %d: Tarou ya no tiene vidas. Fin del intento.\n", obstaculo);
        }
    }                                /* Aquí "obstaculo" ya no existe: vivía en el for.   */

    if (vidas > 0) {                 /* Al final, una sola decisión con lo que quedó.     */
        printf("Tarou termino el nivel con %d vidas de sobra.\n", vidas);
    } else {
        printf("Game Over.\n");
    }

    return 0;                        /* Terminó bien.                                     */
}
