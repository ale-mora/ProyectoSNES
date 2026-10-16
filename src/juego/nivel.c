/*---------------------------------------------------------------------------------
    nivel.c — El suelo del nivel 1 (plano, por ahora).

    PROPÓSITO
      Implementa nivel_suelo_en(): la única pregunta que la física le hace al
      escenario en la Revisión 1. Existe como función, y no como constante
      suelta, para que los desniveles de la Etapa 2 no obliguen a tocar tarou.c.

    SE CONECTA CON
      Incluye:  juego/nivel.h.
      Lo usan:  juego/tarou.c (aterrizaje), pantallas/pantalla_nivel1.c,
                tests/prueba_camara.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Suelo, Nivel
---------------------------------------------------------------------------------*/
#include "juego/nivel.h"

u16 nivel_suelo_en(u16 x_pix)
{
    (void)x_pix;             /* Hoy no depende de x; evita el aviso de "parámetro sin usar".*/
    return SUELO_Y;          /* Suelo plano a la misma altura en todo el nivel.             */
}
