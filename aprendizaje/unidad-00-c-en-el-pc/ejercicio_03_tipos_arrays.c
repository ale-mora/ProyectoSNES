/*---------------------------------------------------------------------------------
    ejercicio_03_tipos_arrays.c — Ejercicio 3 de la unidad 00: arrays.

    PROPÓSITO
      Declarar un array, recorrerlo con un for, acumular un total y usar un if
      dentro del bucle para contar cuántos elementos cumplen una condición,
      con los tipos del juego (u8) en vez de int. Es el mismo patrón con el
      que el juego recorre las piezas de un metasprite (src/juego/metasprite.c)
      o las letras de un texto.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o ejercicio_03.exe ejercicio_03_tipos_arrays.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES (las que vas a declarar)
      vidasPerdidas     u8[5]   0..3 cada una      vidas perdidas en cada nivel
      total             u8      0..15              suma de todas (cabe en un byte)
      nivelesPerfectos  u8      0..5               niveles sin vidas perdidas
      i                 u8      0..4               índice del for

    → conceptos (README de la unidad 00): array, índice, typedef, u8
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae uint8_t.                                     */

typedef uint8_t u8;                  /* El nombre del juego (src/sistema/tipos.h): 0..255. */

/*
 * EJERCICIO 3: tipos de datos y arrays
 *
 * Datos: vidas perdidas por Tarou en cada uno de los primeros 5 niveles:
 *   Nivel 1: 2 vidas perdidas
 *   Nivel 2: 0 vidas perdidas
 *   Nivel 3: 1 vida perdida
 *   Nivel 4: 0 vidas perdidas
 *   Nivel 5: 3 vidas perdidas
 *
 * SALIDA ESPERADA (exacta):
 *   Nivel 1: 2 vidas perdidas
 *   Nivel 2: 0 vidas perdidas
 *   Nivel 3: 1 vidas perdidas
 *   Nivel 4: 0 vidas perdidas
 *   Nivel 5: 3 vidas perdidas
 *   Total de vidas perdidas: 6
 *   Niveles perfectos (0 vidas perdidas): 2
 */

int main(void)                       /* El programa empieza aquí.                         */
{
    /* TODO 1: declara un array de 5 u8 llamado "vidasPerdidas" con los
               valores de arriba, en orden: {2, 0, 1, 0, 3}. Un u8 basta:
               nadie pierde más de 255 vidas en un nivel. */

    /* TODO 2: declara dos variables u8, inicializadas en 0: "total" y
               "nivelesPerfectos" (para ir acumulando dentro del bucle). */

    /* TODO 3: escribe un for con un índice u8 "i" que recorra el array del
               índice 0 al 4 y, en cada vuelta:
                 - imprima "Nivel %d: %d vidas perdidas". OJO: el número de
                   nivel es el ÍNDICE + 1 (los niveles empiezan en 1, los
                   índices en 0). Un u8 se imprime con %d igual que un
                   int: printf lo recibe ya convertido a int;
                 - sume vidasPerdidas[i] a "total";
                 - si vidasPerdidas[i] es igual a 0, sume 1 a "nivelesPerfectos". */

    /* TODO 4: cuando termine el for, imprime:
                 "Total de vidas perdidas: %d"
                 "Niveles perfectos (0 vidas perdidas): %d" */

    return 0;                        /* Terminó bien.                                     */
}
