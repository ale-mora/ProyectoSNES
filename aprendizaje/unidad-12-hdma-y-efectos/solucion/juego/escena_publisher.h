/*---------------------------------------------------------------------------------
    escena_publisher.h — La coreografía del logo del publisher (pantalla 2).
    Solución de la ampliación del ejercicio 12 (aprendizaje/unidad-12-hdma-y-efectos).

    PROPÓSITO
      Declara la tabla de eventos de pantalla_publisher.c. Dura lo mismo que la
      pantalla antes del motor (32 frames de fade_in, 180 de espera y 32 de
      fade_out = 244), para no correr la secuencia de arranque: el título sigue
      saliendo hacia el segundo 47. tests/prueba_escena_publisher.c lo vigila.

    SE CONECTA CON
      Incluye:  escena/escena.h.
      Lo usan:  pantallas/pantalla_publisher.c, tests/prueba_escena_publisher.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      ESCENA_PUBLISHER_FRAMES   244   frame del FIN_ESCENA
      ESCENA_PUBLISHER_EVENTOS  6     líneas de la tabla
      ESCENA_PUBLISHER          const EventoEscena[]  la tabla (const global:
                                tcc816 la deja en la WRAM, copiada al arrancar)
    → glosario: Motor de escenas
---------------------------------------------------------------------------------*/
#ifndef ESCENA_PUBLISHER_H
#define ESCENA_PUBLISHER_H

#include "escena/escena.h"

#define ESCENA_PUBLISHER_FRAMES (32 + 180 + 32)    /* Fade in + 3 s + fade out, como antes. */
#define ESCENA_PUBLISHER_EVENTOS 6

extern const EventoEscena ESCENA_PUBLISHER[ESCENA_PUBLISHER_EVENTOS];

#endif /* ESCENA_PUBLISHER_H */
