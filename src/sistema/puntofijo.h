/*---------------------------------------------------------------------------------
    puntofijo.h — Aritmética de punto fijo 12.4 para posiciones y velocidades.

    PROPÓSITO
      La SNES no tiene decimales (no hay unidad de coma flotante). Para mover a
      Tarou 1,5 píxeles por frame guardamos los números multiplicados por 16: un
      s16 con 12 bits de parte entera y 4 de fracción (1/16 de píxel). 16 es
      "1,0 px", 24 es "1,5 px", 7168 es "448 px". Sumar, restar y comparar
      funcionan igual que con enteros; el píxel se obtiene desplazando 4 bits.
      Regla del proyecto: las POSICIONES nunca son negativas (se recortan antes
      de convertir) y las VELOCIDADES, que sí pueden serlo, nunca se convierten
      a píxel. El >> de un s16 es aritmético en tcc816 (conserva el signo; medido
      en la unidad 01 de aprendizaje/); el peligro está en el cast a (u16) de
      PF_ENTERO, que convierte un negativo en un número enorme.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/tarou.c, juego/camara.c, juego/tarou_sprite.c,
                juego/hud_debug.c, tests/prueba_puntofijo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (macros)
      PF_BITS        4           bits de fracción
      PF(n)          s16         entero n → punto fijo (n * 16)
      PF_ENTERO(x)   u16         punto fijo x (>= 0) → parte entera (x / 16)

    → glosario: Punto fijo, #define, s16
---------------------------------------------------------------------------------*/
#ifndef PUNTOFIJO_H
#define PUNTOFIJO_H

#include "sistema/tipos.h"

#define PF_BITS 4                                   /* 4 bits de fracción = 1/16 px.       */
#define PF(n) ((s16)((n) << PF_BITS))               /* 1 → 16, 448 → 7168.                 */
#define PF_ENTERO(x) ((u16)((x) >> PF_BITS))        /* Solo para x >= 0: 7183 → 448.       */

#endif /* PUNTOFIJO_H */
