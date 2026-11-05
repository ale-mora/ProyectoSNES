/*---------------------------------------------------------------------------------
    ejercicio_25_destello.c — ESQUELETO del ejercicio 25: cópialo al clon como
    src/juego/destello.c. Así como está compila y siempre devuelve 0: con tu prueba
    (TODO 1) escrita, las pruebas deben salir en ROJO antes de completarlo.

    PROPÓSITO
      Ver ejercicio_25_destello.h.

    SE CONECTA CON
      Incluye:  juego/destello.h.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo va en el Destello del que llama).
    → glosario: Matemática de color
---------------------------------------------------------------------------------*/
#include "juego/destello.h"

void destello_iniciar(Destello *d, u8 vida)
{
    d->frames = 0;                                   /* Sin destello.                       */
    (void)vida;                                      /* TODO 3: apúntala.                   */
}

u8 destello_actualizar(Destello *d, u8 vida)
{
    (void)d;
    (void)vida;
    /* TODO 4: si la vida bajó, frames = DESTELLO_FRAMES (otro golpe a mitad: de nuevo).
       TODO 5: apunta la vida de ahora (también si subió: reiniciar la fase no es un golpe).
       TODO 6: sin frames, 0. Con frames: brillo = frames · 4 con tope DESTELLO_MAXIMO
               (sin multiplicar: un desplazamiento), frames--, y devuelve el brillo. */
    return 0;
}
