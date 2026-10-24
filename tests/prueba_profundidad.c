/*---------------------------------------------------------------------------------
    prueba_profundidad.c — Pruebas de juego/profundidad.c.

    PROPÓSITO
      Fijar la regla del orden por profundidad del beat 'em up
      (docs/diseno-beat-em-up.md §3): el que tiene los pies más abajo en la
      pantalla (y mayor) va PRIMERO, porque en la OAM la entrada más baja tapa
      a las demás. Con y iguales se respeta el orden anterior (orden estable:
      dos enemigos a la misma altura no se cambian de lugar cada frame, que
      parpadearía).

    SE CONECTA CON
      Incluye:  minitest.h, juego/profundidad.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_profundidad).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Orden por profundidad, OAM
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/profundidad.h"

void pruebas_profundidad(void)
{
    u16 y[4];
    u8 orden[4];

    /* Desordenados: el de más abajo (y = 190) primero. */
    y[0] = 150; y[1] = 190; y[2] = 170; y[3] = 160;
    orden[0] = 0; orden[1] = 1; orden[2] = 2; orden[3] = 3;
    profundidad_ordenar(orden, y, 4);
    COMPROBAR(orden[0] == 1);
    COMPROBAR(orden[1] == 2);
    COMPROBAR(orden[2] == 3);
    COMPROBAR(orden[3] == 0);

    /* Ya ordenados: no cambia nada (el caso de casi todos los frames). */
    profundidad_ordenar(orden, y, 4);
    COMPROBAR(orden[0] == 1 && orden[1] == 2 && orden[2] == 3 && orden[3] == 0);

    /* Empates: se queda el orden que traían (estable). */
    y[0] = 176; y[1] = 176; y[2] = 180; y[3] = 176;
    orden[0] = 3; orden[1] = 0; orden[2] = 1; orden[3] = 2;
    profundidad_ordenar(orden, y, 4);
    COMPROBAR(orden[0] == 2);
    COMPROBAR(orden[1] == 3);
    COMPROBAR(orden[2] == 0);
    COMPROBAR(orden[3] == 1);

    /* Uno solo y ninguno: no se toca. */
    orden[0] = 2;
    profundidad_ordenar(orden, y, 1);
    COMPROBAR(orden[0] == 2);
    profundidad_ordenar(orden, y, 0);
    COMPROBAR(orden[0] == 2);
}
