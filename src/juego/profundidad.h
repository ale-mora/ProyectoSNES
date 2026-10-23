/*---------------------------------------------------------------------------------
    profundidad.h — Orden de dibujo por profundidad del beat 'em up.

    PROPÓSITO
      En un beat 'em up de carril (docs/diseno-beat-em-up.md §3), el que tiene
      los pies más abajo en la pantalla está más cerca de la cámara y tiene que
      tapar a los demás. En la SNES, entre dos sprites que se pisan gana el de
      la entrada MÁS BAJA de la OAM (el bit de prioridad solo decide contra los
      fondos). Así que cada frame se ordenan las entidades por la y de sus pies,
      de mayor a menor, y se escriben en la OAM en ese orden.
      Ordena por INSERCIÓN: con ≤ 8 entidades que casi nunca cambian de orden
      entre un frame y el siguiente, cuesta una comparación por entidad. Es
      estable: con la misma y se queda el orden que traían, para que dos
      enemigos a la misma altura no se cambien de lugar cada frame.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/sprites.c (sprites_ordenar) y
                tests/prueba_profundidad.c.

    HARDWARE QUE TOCA
      ninguno (lógica pura, probada en el PC).

    VARIABLES: ninguna.
    → glosario: Orden por profundidad, OAM
---------------------------------------------------------------------------------*/
#ifndef PROFUNDIDAD_H
#define PROFUNDIDAD_H

#include "sistema/tipos.h"

/* Reordena `orden` (n índices de entidades) para que y[orden[0]] >= y[orden[1]] >= ...
   `y` es la línea de pantalla de los pies de cada entidad, indexada por entidad. */
void profundidad_ordenar(u8 *orden, const u16 *y, u8 n);

#endif /* PROFUNDIDAD_H */
