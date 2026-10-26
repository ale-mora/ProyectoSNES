/*---------------------------------------------------------------------------------
    guion_tutorial.h — El recorrido completo del tutorial, como guion de mando.

    PROPÓSITO
      Los botones, cuadro a cuadro, que llevan a Tarou del inicio a la salida
      del nivel 1 superando cada obstáculo con su movimiento. Ajustado en el PC
      contra la física real; si alguien cambia el nivel en Tiled o la física y
      el recorrido deja de llegar, tests/prueba_recorrido.c lo dice.

    SE CONECTA CON
      Incluye:  juego/guion.h.
      Lo usan:  juego/mando.c (ROM de prueba, -DGUION_PRUEBA) y
                tests/prueba_recorrido.c.

    HARDWARE QUE TOCA
      ninguno: datos.

    VARIABLES
      GUION_TUTORIAL         TramoGuion[]  los tramos (en guion_tutorial.c)
      GUION_TUTORIAL_TRAMOS  14            cuántos son
      GUION_TUTORIAL_FIN     2270          cuadro en que llega a la salida (PC)

    → glosario: Mando, Frame
---------------------------------------------------------------------------------*/
#ifndef GUION_TUTORIAL_H
#define GUION_TUTORIAL_H

#include "juego/guion.h"

#define GUION_TUTORIAL_TRAMOS 14
#define GUION_TUTORIAL_FIN 2270

extern const TramoGuion GUION_TUTORIAL[GUION_TUTORIAL_TRAMOS];

#endif /* GUION_TUTORIAL_H */
