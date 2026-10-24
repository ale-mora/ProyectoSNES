/*---------------------------------------------------------------------------------
    escena_dialogo.c — Las tablas de las escenas de diálogo (ver escena_dialogo.h).

    PROPÓSITO
      Entrar: brillo 15 → 6 y mosaico 0 → 5 en 24 frames. Salir: lo contrario. Los
      FIJAR del frame 0 ponen el valor de partida (el motor arranca con todo a 0).

    SE CONECTA CON
      Incluye:  escena/escena_dialogo.h.
      Lo usan:  pantallas/pantalla_fase.c, tests/prueba_dialogos.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: las dos tablas (en ROM por ser const dentro de un .c propio; ver
    CLAUDE.md: las globales const viven en WRAM, copiadas al arrancar, y son pequeñas).
    → glosario: Motor de escenas
---------------------------------------------------------------------------------*/
#include "escena/escena_dialogo.h"

const EventoEscena ESCENA_DIALOGO_ENTRA[ESCENA_DIALOGO_EVENTOS] = {
    FIJAR(0, PISTA_BRILLO, 15),                      /* Parte del brillo de siempre...      */
    RAMPA(0, ESCENA_DIALOGO_FRAMES, PISTA_BRILLO, 15, ESCENA_DIALOGO_BRILLO),     /* ...y baja. */
    FIJAR(0, PISTA_MOSAICO, 0),                      /* Parte nítido...                     */
    RAMPA(0, ESCENA_DIALOGO_FRAMES, PISTA_MOSAICO, 0, ESCENA_DIALOGO_MOSAICO),    /* ...y se pixela. */
    FIN_ESCENA(ESCENA_DIALOGO_FRAMES),
};

const EventoEscena ESCENA_DIALOGO_SALE[ESCENA_DIALOGO_EVENTOS] = {
    FIJAR(0, PISTA_BRILLO, ESCENA_DIALOGO_BRILLO),   /* Parte oscuro...                     */
    RAMPA(0, ESCENA_DIALOGO_FRAMES, PISTA_BRILLO, ESCENA_DIALOGO_BRILLO, 15),     /* ...y sube. */
    FIJAR(0, PISTA_MOSAICO, ESCENA_DIALOGO_MOSAICO), /* Parte pixelado...                   */
    RAMPA(0, ESCENA_DIALOGO_FRAMES, PISTA_MOSAICO, ESCENA_DIALOGO_MOSAICO, 0),    /* ...y se afila. */
    FIN_ESCENA(ESCENA_DIALOGO_FRAMES),
};
