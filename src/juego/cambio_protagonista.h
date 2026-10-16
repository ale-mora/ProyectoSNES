/*---------------------------------------------------------------------------------
    cambio_protagonista.h — Cambio de protagonista entre Tarou y Bárbara (Etapa 4).

    PROPÓSITO
      PENDIENTE: Etapa 4 — decidir quién está bajo el control del jugador, cambiar entre ambos (al estilo Donkey Kong Country) y hacer que el que no juega siga al otro.
      Este archivo es un STUB: compila, reserva el sitio del subsistema y documenta
      el contrato previsto. No contiene lógica; la única función real,
      cambio_protagonista_pendiente(), existe para que el enlazador vea el objeto.

    SE CONECTA CON
      Incluirá: juego/tarou.h, juego/barbara.h, juego/mando.h.
      Lo usarán: pantallas/pantalla_nivel1.c (quién recibe el mando y quién se dibuja).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales y estáticas de este archivo)
      ninguna todavía.

    → glosario: Stub, Protagonista, Jugador
---------------------------------------------------------------------------------*/
#ifndef CAMBIO_PROTAGONISTA_H
#define CAMBIO_PROTAGONISTA_H

#include "sistema/tipos.h"

/* Contrato previsto (Etapa 4):
enum { PROTAGONISTA_TAROU, PROTAGONISTA_BARBARA };
u8   protagonista_activo(void);
void protagonista_cambiar(void); */

/* Única función real del stub: no hace nada. Se quitará al implementar el módulo. */
void cambio_protagonista_pendiente(void);

#endif /* CAMBIO_PROTAGONISTA_H */
