/*---------------------------------------------------------------------------------
    solucion/ejercicio_03_tipos_arrays.c — Solución del ejercicio 3 de la unidad 00.

    PROPÓSITO
      Recorre el array de vidas perdidas, imprime cada nivel, suma el total y
      cuenta los niveles perfectos. Un solo for hace las tres cosas. Todo es
      u8, como en el juego: los valores son pequeños y caben en un byte.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o solucion_03.exe solucion/ejercicio_03_tipos_arrays.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      vidasPerdidas     u8[5]   0..3 cada una      vidas perdidas en cada nivel
      total             u8      0..15              suma de todas (cabe en un byte)
      nivelesPerfectos  u8      0..5               niveles sin vidas perdidas
      i                 u8      0..4               índice del nivel que se mira

    → conceptos (README de la unidad 00): array, índice, typedef, u8
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae uint8_t.                                     */

typedef uint8_t u8;                  /* El nombre del juego (src/sistema/tipos.h): 0..255. */

#define NIVELES 5                    /* Cuántos niveles hay: el tamaño del array.         */

int main(void)                       /* El programa empieza aquí.                         */
{
    /* TODO 1: el array, con sus 5 valores entre llaves y en orden. */
    u8 vidasPerdidas[NIVELES] = {2, 0, 1, 0, 3};

    /* TODO 2: los dos acumuladores empiezan en 0 (sin "= 0" valdrían basura). */
    u8 total = 0;                    /* Suma de las vidas perdidas.                       */
    u8 nivelesPerfectos = 0;         /* Niveles con 0 vidas perdidas.                     */

    /* TODO 3: i va de 0 a NIVELES - 1; "i < NIVELES" para justo antes del 5. */
    for (u8 i = 0; i < NIVELES; i++)
    {
        printf("Nivel %d: %d vidas perdidas\n", i + 1, vidasPerdidas[i]);  /* Nivel = índice + 1. */
        total = total + vidasPerdidas[i];                                  /* Acumula.            */
        if (vidasPerdidas[i] == 0)   /* "==" compara; "=" asignaría 0 y nunca sería cierto. */
        {
            nivelesPerfectos = nivelesPerfectos + 1;                       /* Cuenta uno más.     */
        }
    }

    /* TODO 4: los resultados, una vez terminado el recorrido. */
    printf("Total de vidas perdidas: %d\n", total);
    printf("Niveles perfectos (0 vidas perdidas): %d\n", nivelesPerfectos);

    return 0;                        /* Terminó bien.                                     */
}
