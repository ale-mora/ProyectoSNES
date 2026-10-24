/*---------------------------------------------------------------------------------
    cambio_protagonista.h — El relevo entre Tarou y Bárbara (spec §4, al estilo
    de Donkey Kong Country).

    PROPÓSITO
      Se juega con uno y el otro espera fuera: nunca hay dos protagonistas en
      pantalla. L cambia de protagonista si pasó el enfriamiento (120 frames), el
      que sale no está en un estado bloqueado (recibiendo, derribado, agarrado...
      luchador_puede_salir) y el que entra tiene vida. El que sale desaparece en una
      nube; el que entra cae desde arriba sobre el mismo sitio, invulnerable, y al
      aterrizar da el golpe de entrada (4 de daño y empuje a menos de 48 px, que
      resuelve quien llama con luchador_entrada_activa y combate_empuje_cercano).
      El que espera fuera recupera 1 punto de vida cada 120 frames hasta 48 de 64.
      Si el activo cae KO (su KO termina), entra el otro AL INSTANTE, sin enfriamiento
      y con 60 frames de invulnerabilidad. Si el otro también está a 0, se pierde una
      vida (RELEVO_PIERDE_VIDA) y quien llama reinicia a los dos.
      No incluye la barra de ataque compartida: va con los apoyos.
      Lógica pura, probada en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/luchador.h, juego/mando.h, juego/mundo.h.
      Lo usan:  tests/prueba_relevo.c y, cuando exista, la pantalla de la fase.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Relevo)
      tarou, barbara   Luchador  los dos protagonistas (los dos existen siempre)
      activo           u8        QUIEN_TAROU o QUIEN_BARBARA: el que se juega
      enfriamiento     u8        frames hasta poder hacer otro relevo
      cura             u8        frames hasta la próxima cura del que espera fuera
      nube             u8        frames que le quedan a la nube de salida (0 = sin nube)
      nube_x, nube_y   s16       dónde está la nube (punto fijo 12.4, los pies del que salió)

    → glosario: Relevo, Protagonista, Golpe de entrada
---------------------------------------------------------------------------------*/
#ifndef CAMBIO_PROTAGONISTA_H
#define CAMBIO_PROTAGONISTA_H

#include "sistema/tipos.h"
#include "juego/luchador.h"
#include "juego/mando.h"
#include "juego/mundo.h"

#define FRAMES_NUBE 20               /* Lo que dura la nube del que sale.                  */
#define FRAMES_ENTRADA_INVULNERABLE 36  /* Caída (~25) + golpe de entrada (8) + margen.    */

/* Lo que pasó este frame. */
enum { RELEVO_NADA = 0, RELEVO_CAMBIO = 1, RELEVO_PIERDE_VIDA = 2 };

typedef struct
{
    Luchador tarou;
    Luchador barbara;
    u8 activo;                       /* QUIEN_TAROU o QUIEN_BARBARA.                       */
    u8 enfriamiento;                 /* Frames hasta poder hacer otro relevo.              */
    u8 cura;                         /* Frames hasta la próxima cura del que espera fuera. */
    u8 nube;                         /* Frames que le quedan a la nube (0 = ninguna).      */
    s16 nube_x, nube_y;              /* Dónde está la nube.                                */
} Relevo;

/* Empieza con Tarou activo en (x, y) y Bárbara fuera, los dos con vida completa. */
void relevo_iniciar(Relevo *r, u16 x, u16 y);

/* El que se está jugando y el que espera fuera. */
Luchador *relevo_activo(Relevo *r);
Luchador *relevo_fuera(Relevo *r);

/* Un frame: atiende a L, cura al de fuera, cambia si el activo ha caído y actualiza al activo
   con el mando. Devuelve RELEVO_NADA, RELEVO_CAMBIO o RELEVO_PIERDE_VIDA. */
u8 relevo_actualizar(Relevo *r, const Mando *m, const Carril *carril);

#endif /* CAMBIO_PROTAGONISTA_H */
