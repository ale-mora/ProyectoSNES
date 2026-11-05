/*---------------------------------------------------------------------------------
    destello.h — El destello de pantalla al recibir un golpe (ejercicio de la unidad 25).

    PROPÓSITO
      Lógica pura (sin snes.h, probada en el PC: tests/prueba_destello.c): cuando la vida
      de la protagonista baja de un frame al siguiente, la pantalla se aclara unos frames
      con un brillo que baja hasta 0. La pantalla (pantallas/pantalla_fase1.c) pone ese
      brillo en el color fijo (COLDATA) y lo SUMA a los fondos con la matemática de color.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: juego/destello.c.
      Lo usan:  pantallas/pantalla_fase1.c, tests/prueba_destello.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Destello; 2 bytes)
      frames  u8  frames de destello que quedan (0 = ninguno)
      vida    u8  la vida del frame anterior (para ver si bajó)
    → glosario: Matemática de color
---------------------------------------------------------------------------------*/
#ifndef DESTELLO_H
#define DESTELLO_H

#include "sistema/tipos.h"

#ifndef DESTELLO_FRAMES
#define DESTELLO_FRAMES 4                /* Cuánto dura (con -D, más largo para capturarlo). */
#endif
#define DESTELLO_MAXIMO 16               /* Brillo del primer frame (0..31 en COLDATA).   */

typedef struct
{
    u8 frames;
    u8 vida;
} Destello;

/* Sin destello, partiendo de la vida `vida`. */
void destello_iniciar(Destello *d, u8 vida);

/* Un frame con la vida de ahora: devuelve el brillo de este frame (0 = sin destello). */
u8 destello_actualizar(Destello *d, u8 vida);

#endif /* DESTELLO_H */
