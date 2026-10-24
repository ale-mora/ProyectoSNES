/*---------------------------------------------------------------------------------
    demo_07/tipos.h — Los enteros del juego, para la demo 7 de la unidad 00.

    PROPÓSITO
      Lo mismo que src/sistema/tipos.h en su rama de PC: dar los nombres u8,
      u16 y s16 a los enteros de tamaño fijo de <stdint.h>. Lo incluyen
      main.c y reloj.h; la guarda impide que se pegue dos veces en main.c
      (allí llega directamente y otra vez a través de reloj.h).

    SE CONECTA CON
      Incluye:  <stdint.h>.
      Lo usan:  demo_07/reloj.h, demo_07/main.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tipos que define)
      u8 0..255, u16 0..65535, s16 -32768..32767

    → conceptos (README de la unidad 00): typedef, guarda de inclusión
---------------------------------------------------------------------------------*/
#ifndef TIPOS_H                      /* La guarda: solo la primera vez.                   */
#define TIPOS_H

#include <stdint.h>                  /* Enteros de tamaño fijo del C estándar.            */

typedef uint8_t u8;                  /* 1 byte sin signo.                                 */
typedef uint16_t u16;                /* 2 bytes sin signo.                                */
typedef int16_t s16;                 /* 2 bytes con signo.                                */

#endif /* TIPOS_H */
