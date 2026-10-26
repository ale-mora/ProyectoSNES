/*---------------------------------------------------------------------------------
    barbara.h — Bárbara, la segunda protagonista (Revisión 2).

    PROPÓSITO
      PENDIENTE: Revisión 2 — lógica pura de Bárbara, hermana de tarou.c: su struct, sus constantes de movimiento y su máquina de estados (sus mecánicas se diseñarán entonces).
      Este archivo es un STUB: compila, reserva el sitio del subsistema y documenta
      el contrato previsto. No contiene lógica; la única función real,
      barbara_pendiente(), existe para que el enlazador vea el objeto.

    SE CONECTA CON
      Incluirá: sistema/tipos.h, sistema/puntofijo.h, juego/mando.h, juego/animaciones.h.
      Lo usarán: juego/cambio_protagonista.c, pantallas/pantalla_nivel1.c, tests/.

    HARDWARE QUE TOCA
      ninguno (lógica pura, como tarou.c).

    VARIABLES (globales y estáticas de este archivo)
      ninguna todavía.

    → glosario: Stub, Protagonista
---------------------------------------------------------------------------------*/
#ifndef BARBARA_H
#define BARBARA_H

#include "sistema/tipos.h"

/* Contrato previsto (Revisión 2), calcado del de Tarou:
typedef struct { ... } Barbara;
void barbara_iniciar(Barbara *b, u16 x_pix, u16 suelo_y);
void barbara_actualizar(Barbara *b, const Mando *m, u16 suelo_y);
void barbara_caja_cuerpo(const Barbara *b, Caja *c);
u8   barbara_caja_golpe(const Barbara *b, Caja *c); */

/* Única función real del stub: no hace nada. Se quitará al implementar el módulo. */
void barbara_pendiente(void);

#endif /* BARBARA_H */
