/*---------------------------------------------------------------------------------
    destello.c — El destello al recibir un golpe (ver destello.h).

    PROPÓSITO
      Sin tablas ni divisiones: el brillo es frames · 4, con tope DESTELLO_MAXIMO (así
      con DESTELLO_FRAMES = 4 sale 16, 12, 8, 4; con más frames, se queda en 16 al
      principio y luego baja igual).

    SE CONECTA CON
      Incluye:  juego/destello.h.
      Lo usan:  ver destello.h.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo va en el Destello del que llama).
    → glosario: Matemática de color
---------------------------------------------------------------------------------*/
#include "juego/destello.h"

void destello_iniciar(Destello *d, u8 vida)
{
    d->frames = 0;
    d->vida = vida;
}

u8 destello_actualizar(Destello *d, u8 vida)
{
    u8 brillo;

    if (vida < d->vida)                              /* La vida bajó: un golpe.             */
    {
        d->frames = DESTELLO_FRAMES;                 /* (Otro golpe a mitad: de nuevo.)     */
    }
    d->vida = vida;                                  /* Si sube (reiniciar), solo se apunta. */
    if (d->frames == 0)
    {
        return 0;
    }
    brillo = (u8)(d->frames << 2);                   /* 4 por frame que queda...            */
    if (brillo > DESTELLO_MAXIMO)
    {
        brillo = DESTELLO_MAXIMO;                    /* ...con tope.                        */
    }
    d->frames--;
    return brillo;
}
