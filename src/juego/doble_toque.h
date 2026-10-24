/*---------------------------------------------------------------------------------
    doble_toque.h — Correr con un doble toque de la cruceta (beat 'em up).

    PROPÓSITO
      En la fase no hay botón de correr (Y queda libre, decisión 30): se corre
      pulsando dos veces seguidas la misma dirección horizontal
      (docs/diseno-beat-em-up.md §2). Se guarda la última dirección pulsada y
      los frames desde que se soltó; si la MISMA vuelve a pulsarse antes de
      VENTANA_DOBLE_TOQUE frames, se corre hasta soltarla. Es lógica pura: el
      mando (juego/mando.c) solo lee bits, y esto decide.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/doble_toque.c y tests/prueba_doble_toque.c (y, cuando
                exista, el luchador del prototipo de combate).

    HARDWARE QUE TOCA
      ninguno (lógica pura, probada en el PC).

    VARIABLES (campos de DobleToque)
      dir_previa      u8  TOQUE_NINGUNA/IZQUIERDA/DERECHA  dirección mantenida el frame anterior
      ultima_dir      u8  ídem                             última dirección que se pulsó
      frames_suelta   u8  0..255 (se satura)               frames desde que se soltó ultima_dir
      corriendo       u8  0 / 1                            1 mientras dura la carrera

    → glosario: Doble toque, struct
---------------------------------------------------------------------------------*/
#ifndef DOBLE_TOQUE_H
#define DOBLE_TOQUE_H

#include "sistema/tipos.h"

/*--- Dirección horizontal de la cruceta ----------------------------------------------*/
enum { TOQUE_NINGUNA = 0, TOQUE_IZQUIERDA = 1, TOQUE_DERECHA = 2 };

typedef struct
{
    u8 dir_previa;                   /* Lo que se mantenía el frame anterior.              */
    u8 ultima_dir;                   /* La última dirección pulsada.                       */
    u8 frames_suelta;                /* Frames desde que se soltó (255 = hace mucho).      */
    u8 corriendo;                    /* 1 mientras se corre.                               */
} DobleToque;

/* Lo deja como al entrar en la fase: sin dirección y sin correr. */
void doble_toque_iniciar(DobleToque *d);

/* Un frame. `izquierda` y `derecha` son los bits mantenidos de la cruceta (0/1).
   Devuelve 1 si este frame se corre (en la dirección mantenida). */
u8 doble_toque_actualizar(DobleToque *d, u8 izquierda, u8 derecha);

#endif /* DOBLE_TOQUE_H */
