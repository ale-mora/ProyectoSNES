/*---------------------------------------------------------------------------------
    ejercicio_25_destello.h — ESQUELETO del ejercicio 25: cópialo al clon como
    src/juego/destello.h y complétalo (ver ejercicio_25.md).

    PROPÓSITO
      Lógica pura (sin snes.h) del destello al recibir un golpe: cuando la vida baja,
      la pantalla se aclara DESTELLO_FRAMES frames con un brillo que baja hasta 0.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: juego/destello.c (tu ejercicio_25_destello.c).
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_destello.c (los escribes tú).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Destello)
      TODO 3: decide qué campos necesita (pista: dos u8 bastan).
    → glosario: Matemática de color
---------------------------------------------------------------------------------*/
#ifndef DESTELLO_H
#define DESTELLO_H

#include "sistema/tipos.h"

#ifndef DESTELLO_FRAMES
#define DESTELLO_FRAMES 4                /* Con -DDESTELLO_FRAMES=63 se puede capturar.    */
#endif
#define DESTELLO_MAXIMO 16               /* Brillo del primer frame (COLDATA: 0..31).     */

typedef struct
{
    u8 frames;                           /* TODO 3: ¿basta con esto? ¿Cómo sabes que la   */
    u8 libre;                            /*         vida BAJÓ si no recuerdas la de antes? */
} Destello;

/* Sin destello, partiendo de la vida `vida`. */
void destello_iniciar(Destello *d, u8 vida);

/* Un frame con la vida de ahora: devuelve el brillo de este frame (0 = sin destello). */
u8 destello_actualizar(Destello *d, u8 vida);

#endif /* DESTELLO_H */
