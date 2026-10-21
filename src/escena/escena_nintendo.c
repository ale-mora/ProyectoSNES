/*---------------------------------------------------------------------------------
    escena_nintendo.c — Los eventos del logo "Nintendo" (ver escena_nintendo.h).

    PROPÓSITO
      Se lee como un guion de cine, en frames (60 = 1 s):
        0-31    el logo se enciende (brillo 0 → 15) mientras se enfoca: empieza
                pixelado (mosaico 15) y queda nítido en el frame 47;
        70-125  una banda de luz cruza el logo en diagonal, de izquierda a
                derecha; la luz se enciende al entrar y se apaga antes de salir;
        212-243 se apaga (brillo 15 → 0);
        244     fin: la pantalla pasa al logo del publisher.

    SE CONECTA CON
      Incluye:  escena/escena_nintendo.h.
      Lo usan:  pantallas/pantalla_nintendo.c, tests/prueba_escena.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales)
      ESCENA_NINTENDO  const EventoEscena[8]  en la ROM (const)
    → glosario: Motor de escenas
---------------------------------------------------------------------------------*/
#include "escena/escena_nintendo.h"

const EventoEscena ESCENA_NINTENDO[ESCENA_NINTENDO_EVENTOS] = {
    FIJAR(0, PISTA_MOSAICO, 15),                     /* Arranca con bloques de 16x16...     */
    RAMPA(0, 48, PISTA_MOSAICO, 15, 0),              /* ...que se enfocan en 48 frames.     */
    RAMPA(0, 32, PISTA_BRILLO, 0, 15),               /* Fundido de entrada (lo de fade_in). */
    RAMPA(70, 56, PISTA_DESTELLO, 0, 255),           /* La banda cruza el logo.             */
    RAMPA(70, 14, PISTA_LUZ, 0, 14),                 /* La luz se enciende al entrar...     */
    RAMPA(112, 14, PISTA_LUZ, 14, 0),                /* ...y se apaga antes de salir.       */
    RAMPA(212, 32, PISTA_BRILLO, 15, 0),             /* Fundido de salida.                  */
    FIN_ESCENA(ESCENA_NINTENDO_FRAMES),              /* 244: lo mismo que duraba antes.     */
};
