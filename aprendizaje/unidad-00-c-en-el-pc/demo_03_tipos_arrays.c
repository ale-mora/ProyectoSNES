/*---------------------------------------------------------------------------------
    demo_03_tipos_arrays.c — Demo 3 de la unidad 00: arrays y tipos de ancho fijo.

    PROPÓSITO
      Un array de 5 puntajes recorrido con un for para sumarlos, escrito con
      los tipos del juego (u8, u16) en vez de int. Las cuatro líneas typedef
      de arriba son las mismas que src/sistema/tipos.h usa en el PC (rama
      PRUEBAS_PC). Al final imprime cuántos bytes ocupa cada tipo: int mide 4
      bytes con gcc y solo 2 con tcc816, el compilador de la SNES; u16 mide 2
      en los dos. Es la base del ejercicio 3.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t, uint16_t, int8_t, int16_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/sistema/tipos.h:33-38 (los mismos typedef).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_03.exe demo_03_tipos_arrays.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      puntajePorNivel   u16[5]   0..65535 cada uno   puntaje de cada nivel
      total             u16      0..65535            suma de los 5
      i                 u8       0..4                índice del for
      vidas             u8       0..255              guardado en un solo byte

    → conceptos (README de la unidad 00): array, índice, typedef, u8/u16/s16, sizeof
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae los enteros de tamaño fijo del C estándar.   */

/*--- Los nombres cortos del juego: copiados de src/sistema/tipos.h (rama PC) ---------*/
typedef uint8_t u8;                  /* u8:  sin signo, 8 bits  (1 byte):  0..255.        */
typedef uint16_t u16;                /* u16: sin signo, 16 bits (2 bytes): 0..65535.      */
typedef int8_t s8;                   /* s8:  con signo, 8 bits:  -128..127.               */
typedef int16_t s16;                 /* s16: con signo, 16 bits: -32768..32767.           */

int main(void)                       /* main devuelve int: lo exige el C estándar.        */
{
    /* Un array de 5 u16: el puntaje que Tarou consigue en cada nivel. */
    u16 puntajePorNivel[5] = {120, 340, 90, 500, 275};

    /* Los índices van de 0 a (tamaño - 1): este array tiene 5 elementos,
       así que los índices válidos son 0, 1, 2, 3 y 4. */
    printf("Puntaje del nivel 1 (indice 0): %d\n", puntajePorNivel[0]);
    printf("Puntaje del nivel 3 (indice 2): %d\n", puntajePorNivel[2]);

    /* Recorrer todo el array con un for, sumando el puntaje total. */
    u16 total = 0;                   /* El acumulador empieza en 0.                       */
    for (u8 i = 0; i < 5; i++)       /* i es u8: un índice de 0 a 4 cabe en un byte.      */
    {
        total = total + puntajePorNivel[i];   /* Suma el elemento i.                      */
    }
    printf("Puntaje total: %d\n", total);

    /* u8 ocupa 1 solo byte (0..255): de sobra para contar vidas. */
    u8 vidas = 3;
    printf("Vidas (guardadas en 1 byte): %d\n", vidas);

    /* sizeof dice cuántos bytes ocupa un tipo o una variable. %u imprime un
       número sin signo; el (unsigned) adapta lo que devuelve sizeof a %u. */
    printf("Bytes: u8=%u u16=%u s16=%u int=%u\n",
           (unsigned)sizeof(u8), (unsigned)sizeof(u16),
           (unsigned)sizeof(s16), (unsigned)sizeof(int));   /* int: 4 en gcc, 2 en tcc816. */
    printf("Bytes del array: %u (5 elementos de 2 bytes)\n",
           (unsigned)sizeof(puntajePorNivel));

    return 0;                        /* Terminó bien.                                     */
}
