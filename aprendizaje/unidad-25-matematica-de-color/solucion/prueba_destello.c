/*---------------------------------------------------------------------------------
    prueba_destello.c — Pruebas de juego/destello.c (el destello al recibir un golpe).

    PROPÓSITO
      Fijar la regla del ejercicio 25: cuando la vida baja, la pantalla se aclara
      DESTELLO_FRAMES frames con un brillo que baja de 4 en 4 hasta 0; si llega otro
      golpe a mitad, vuelve a empezar; si la vida sube (al reiniciar la fase) o se
      queda igual, no hay destello.

    SE CONECTA CON
      Incluye:  minitest.h, juego/destello.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_destello).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Matemática de color
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/destello.h"

void pruebas_destello(void)
{
    Destello d;
    u8 f;

    /* Sin golpes, nunca hay destello. */
    destello_iniciar(&d, 24);
    for (f = 0; f < 10; f++)
    {
        COMPROBAR(destello_actualizar(&d, 24) == 0);
    }

    /* Un golpe (24 -> 22): 16, 12, 8, 4 y luego 0. */
    COMPROBAR(destello_actualizar(&d, 22) == 16);
    COMPROBAR(destello_actualizar(&d, 22) == 12);
    COMPROBAR(destello_actualizar(&d, 22) == 8);
    COMPROBAR(destello_actualizar(&d, 22) == 4);
    COMPROBAR(destello_actualizar(&d, 22) == 0);
    COMPROBAR(destello_actualizar(&d, 22) == 0);

    /* Otro golpe a mitad del destello: empieza de nuevo. */
    destello_actualizar(&d, 20);
    destello_actualizar(&d, 20);
    COMPROBAR(destello_actualizar(&d, 18) == 16);

    /* La vida sube (reiniciar la fase): no es un golpe. */
    destello_iniciar(&d, 2);
    COMPROBAR(destello_actualizar(&d, 24) == 0);
    COMPROBAR(destello_actualizar(&d, 22) == 16);    /* Y el siguiente golpe sí cuenta. */

    /* El brillo nunca pasa de 16 (con DESTELLO_FRAMES más grande, para capturas). */
    COMPROBAR(DESTELLO_FRAMES >= 4);
}
