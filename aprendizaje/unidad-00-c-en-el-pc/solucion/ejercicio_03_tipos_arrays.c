/*---------------------------------------------------------------------------------
    solucion/ejercicio_03_tipos_arrays.c — Solución del ejercicio 3 de la unidad 00.

    PROPÓSITO
      Recorre el array de vidas perdidas, imprime cada nivel, suma el total y
      cuenta los niveles perfectos. Un solo for hace las tres cosas.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o solucion_03.exe solucion/ejercicio_03_tipos_arrays.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      vidasPerdidas     int[5]  0..3 cada una      vidas perdidas en cada nivel
      total             int     0..                suma de todas
      nivelesPerfectos  int     0..5               niveles sin vidas perdidas
      i                 int     0..4               índice del nivel que se mira

    → conceptos (README de la unidad 00): array, índice
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */

#define NIVELES 5                    /* Cuántos niveles hay: el tamaño del array.         */

int main(void)                       /* El programa empieza aquí.                         */
{
    /* TODO 1: el array, con sus 5 valores entre llaves y en orden. */
    int vidasPerdidas[NIVELES] = {2, 0, 1, 0, 3};

    /* TODO 2: los dos acumuladores empiezan en 0 (sin "= 0" valdrían basura). */
    int total = 0;                   /* Suma de las vidas perdidas.                       */
    int nivelesPerfectos = 0;        /* Niveles con 0 vidas perdidas.                     */

    /* TODO 3: i va de 0 a NIVELES - 1; "i < NIVELES" para justo antes del 5. */
    for (int i = 0; i < NIVELES; i++)
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
