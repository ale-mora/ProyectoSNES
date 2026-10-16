/*---------------------------------------------------------------------------------
    prueba_animacion.c — Pruebas de juego/animaciones.h/.c y de juego/mando.h.

    PROPÓSITO
      Comprobar que la tabla ANIMACIONES tiene, por estado, los cuadros que
      genera el pipeline (tarou_anim.h) y los frames por cuadro y bucles de
      docs/diseno-demo-jugable.md §7; que los nombres de los estados (para el
      HUD) coinciden con el enum; y que la struct Mando compila en el PC.

    SE CONECTA CON
      Incluye:  minitest.h, juego/animaciones.h, juego/mando.h, <string.h>.
      Lo usan:  tests/pruebas.c (llama a pruebas_animacion).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Estado, enum, struct
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/animaciones.h"
#include "juego/mando.h"

void pruebas_animacion(void)
{
    Mando m;

    /* Cuadros: vienen de tarou_anim.h (aquí, la copia fija de tests/). */
    COMPROBAR(ANIMACIONES[ESTADO_PARADO].cuadros == 4);
    COMPROBAR(ANIMACIONES[ESTADO_CAMINANDO].cuadros == 8);
    COMPROBAR(ANIMACIONES[ESTADO_ATACANDO].cuadros == 5);
    COMPROBAR(ANIMACIONES[ESTADO_AGACHADO_CAMINANDO].cuadros == 6);

    /* Frames por cuadro y bucle: tabla de §7. */
    COMPROBAR(ANIMACIONES[ESTADO_PARADO].frames_por_cuadro == 12);
    COMPROBAR(ANIMACIONES[ESTADO_CAMINANDO].frames_por_cuadro == 6);
    COMPROBAR(ANIMACIONES[ESTADO_CORRIENDO].frames_por_cuadro == 4);
    COMPROBAR(ANIMACIONES[ESTADO_DORMIDO].frames_por_cuadro == 20);
    COMPROBAR(ANIMACIONES[ESTADO_ATACANDO].bucle == 0);
    COMPROBAR(ANIMACIONES[ESTADO_SALTANDO].bucle == 0);
    COMPROBAR(ANIMACIONES[ESTADO_DORMIDO].bucle == 1);
    COMPROBAR(ANIMACIONES[ESTADO_CAMINANDO].bucle == 1);

    /* El enum cuenta 13 estados y el último nombre coincide. */
    COMPROBAR(ESTADO_CANTIDAD == 13);
    COMPROBAR(strcmp(NOMBRES_ESTADO[ESTADO_AGACHADO_CAMINANDO], "AGACHADO_CAMINANDO") == 0);
    COMPROBAR(strcmp(NOMBRES_ESTADO[ESTADO_DESPERTANDO], "DESPERTANDO") == 0);

    /* La estructura del mando existe y arranca a cero cuando se inicializa así. */
    memset(&m, 0, sizeof(m));
    COMPROBAR(m.derecha == 0 && m.saltar_pulsado == 0 && m.actividad == 0);
}
