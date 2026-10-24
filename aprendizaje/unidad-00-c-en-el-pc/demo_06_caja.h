/*---------------------------------------------------------------------------------
    demo_06_caja.h — Cabecera de la demo 6 de la unidad 00: una guarda de inclusión.

    PROPÓSITO
      Define un tipo (Caja) y una constante. demo_06_preprocesador.c la incluye
      DOS veces a propósito: la guarda de inclusión (#ifndef / #define /
      #endif) hace que la segunda vez no se pegue nada. Sin la guarda, gcc
      daría «error: redefinition of 'struct Caja'» (la salida real está en
      capturas/error_sin_guarda.txt).

    SE CONECTA CON
      Incluye:  <stdint.h> (int16_t, uint8_t).
      Lo usan:  demo_06_preprocesador.c.
      Modelo:   src/juego/tarou.h:36-37 y :99 (la guarda TAROU_H).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tipos y constantes que define)
      Caja           struct   x, y (s16), ancho, alto (u8)
      CAJA_MAX_ANCHO #define  96 (el ancho de una celda de Tarou)

    → conceptos (README de la unidad 00): #include, guarda de inclusión
---------------------------------------------------------------------------------*/
#ifndef DEMO_06_CAJA_H               /* ¿Primera vez que se pega este archivo?            */
#define DEMO_06_CAJA_H               /* Sí: se marca, y la próxima vez se salta todo.     */

#include <stdint.h>                  /* Los .h del sistema también llevan su guarda.      */

#define CAJA_MAX_ANCHO 96            /* Una constante pública: quien incluya la ve.       */

struct Caja                          /* Una struct con etiqueta (struct Caja): definirla  */
{                                    /* dos veces en el mismo .c es un error.             */
    int16_t x, y;                    /* Esquina superior izquierda.                       */
    uint8_t ancho, alto;             /* Tamaño en píxeles.                                */
};

#endif /* DEMO_06_CAJA_H */          /* Fin de la zona protegida.                         */
