/*---------------------------------------------------------------------------------
    minitest.h — Arnés mínimo de pruebas en PC para la lógica pura del juego.

    PROPÓSITO
      La SNES no tiene cómo ejecutar pruebas, pero la lógica del protagonista, la
      cámara y el nivel no tocan el hardware: se compilan también con gcc y se
      comprueban aquí. COMPROBAR(cond) cuenta la prueba y, si falla, imprime el
      archivo, la línea y la condición. No hay más: ni fixtures ni mocks.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), sistema/tipos.h (u16).
      Lo usan:  tests/pruebas.c (define los contadores y llama a cada suite) y
                cada tests/prueba_*.c.

    HARDWARE QUE TOCA
      ninguno (solo PC).

    VARIABLES (declaradas aquí, definidas en pruebas.c)
      pruebas_total     u16  0..65535  comprobaciones ejecutadas
      pruebas_fallidas  u16  0..65535  comprobaciones que fallaron

    → glosario: #define, extern
---------------------------------------------------------------------------------*/
#ifndef MINITEST_H
#define MINITEST_H

#include <stdio.h>
#include "sistema/tipos.h"

extern u16 pruebas_total;
extern u16 pruebas_fallidas;

/* do { ... } while (0) hace que la macro se comporte como una sola sentencia,
   también dentro de un if sin llaves. #cond convierte la condición en texto. */
#define COMPROBAR(cond)                                                       \
    do                                                                        \
    {                                                                         \
        pruebas_total++;                                                      \
        if (!(cond))                                                          \
        {                                                                     \
            pruebas_fallidas++;                                               \
            printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond);          \
        }                                                                     \
    } while (0)

#endif /* MINITEST_H */
