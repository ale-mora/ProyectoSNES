/*---------------------------------------------------------------------------------
    escena_publisher.c — Los eventos del logo del publisher (ver escena_publisher.h).

    PROPÓSITO
      Se lee como un guion, en frames (60 = 1 s):
        0-31     el logo se enciende (brillo 0 → 15) y a la vez se enfoca:
                 empieza pixelado (mosaico 15) y queda nítido en el frame 31;
        32-195   quieto;
        196-243  se pixela otra vez (mosaico 0 → 15, 48 frames, el mismo ritmo
                 que setMosaicEffect) y, desde el 212, se apaga (brillo 15 → 0);
        244      fin: brillo 0. pantalla_publisher.c llama a efectos_terminar
                 (mosaico a 0) y pasa a la pantalla de los desarrolladores.

    SE CONECTA CON
      Incluye:  escena/escena_publisher.h.
      Lo usan:  pantallas/pantalla_publisher.c, tests/prueba_escena_publisher.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales)
      ESCENA_PUBLISHER  const EventoEscena[6]  la tabla (en la WRAM: const global)
    → glosario: Motor de escenas, Mosaico, Fade / brillo
---------------------------------------------------------------------------------*/
#include "escena/escena_publisher.h"

const EventoEscena ESCENA_PUBLISHER[ESCENA_PUBLISHER_EVENTOS] = {
    FIJAR(0, PISTA_MOSAICO, 15),                     /* Arranca con bloques de 16x16...     */
    RAMPA(0, 32, PISTA_MOSAICO, 15, 0),              /* ...que se enfocan en 32 frames...   */
    RAMPA(0, 32, PISTA_BRILLO, 0, 15),               /* ...mientras se enciende.            */
    RAMPA(196, 48, PISTA_MOSAICO, 0, 15),            /* Se pixela al salir (como la intro). */
    RAMPA(212, 32, PISTA_BRILLO, 15, 0),             /* Fundido de salida.                  */
    FIN_ESCENA(ESCENA_PUBLISHER_FRAMES),             /* 244: lo mismo que duraba antes.     */
};
