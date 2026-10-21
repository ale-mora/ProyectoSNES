/*---------------------------------------------------------------------------------
    escena_nintendo.h — La coreografía del logo "Nintendo" (pantalla 1).

    PROPÓSITO
      Declara la tabla de eventos que usa pantalla_nintendo.c y la duración
      total de la escena. La duración es la que tenía la pantalla antes del
      motor de escenas: 32 frames de fade_in, 180 de espera (3 s) y 32 de
      fade_out = 244. Si cambia, la secuencia de arranque entera se corre y el
      título deja de salir a los ~48-50 s (los scripts de captura pulsan Start
      a los 55 s): tests/prueba_escena.c lo vigila.

    SE CONECTA CON
      Incluye:  escena/escena.h.
      Lo usan:  pantallas/pantalla_nintendo.c, tests/prueba_escena.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      ESCENA_NINTENDO_FRAMES   244   frame del FIN_ESCENA
      ESCENA_NINTENDO_EVENTOS  8     líneas de la tabla
      ESCENA_NINTENDO          const EventoEscena[]  la tabla (en la ROM)
    → glosario: Motor de escenas
---------------------------------------------------------------------------------*/
#ifndef ESCENA_NINTENDO_H
#define ESCENA_NINTENDO_H

#include "escena/escena.h"

#define ESCENA_NINTENDO_FRAMES (32 + 180 + 32)     /* Fade in + 3 s + fade out, como antes. */
#define ESCENA_NINTENDO_EVENTOS 8

extern const EventoEscena ESCENA_NINTENDO[ESCENA_NINTENDO_EVENTOS];

#endif /* ESCENA_NINTENDO_H */
