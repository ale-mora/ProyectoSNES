/*---------------------------------------------------------------------------------
    prueba_recorrido.c — El tutorial entero, jugado por el guion, en el PC.

    PROPÓSITO
      Con la rejilla REAL del nivel (nivel1_datos.h) y el guion del recorrido
      (juego/guion_tutorial.c), juega cuadro a cuadro como lo haría la SNES:
      mando del guion → tarou_actualizar → partida_actualizar. Comprueba que
      llega a la salida sin reaparecer nunca, que rompe el tronco por el camino
      y que pasa por todos los puntos de control. Es la prueba de que el nivel
      se PUEDE terminar; las de prueba_colision.c y prueba_partida.c comprueban
      que cada obstáculo exige su movimiento.

    SE CONECTA CON
      Incluye:  minitest.h, juego/guion_tutorial.h, juego/partida.h,
                juego/colision.h, juego/tarou.h, juego/nivel.h,
                sistema/puntofijo.h, nivel1_datos.h.
      Lo usa:   tests/pruebas.c (pruebas_recorrido).

    HARDWARE QUE TOCA
      ninguno (solo PC).

    VARIABLES (estáticas)
      controles  u16[]  las x de los carteles
    → glosario: Punto de control
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/guion_tutorial.h"
#include "juego/partida.h"
#include "juego/colision.h"
#include "juego/tarou.h"
#include "juego/nivel.h"
#include "sistema/puntofijo.h"
#include "nivel1_datos.h"

static const u16 controles[] = NIVEL1_CONTROLES_X;

void pruebas_recorrido(void)
{
    Tarou t; Mando m; Partida p;
    u16 cuadro;
    u8 e = PARTIDA_SIGUE, reapariciones = 0, tronco_roto = 0;

    colision_usar(nivel1_colision);
    partida_iniciar(&p, NIVEL1_INICIO_X, controles, NIVEL1_CONTROLES, NIVEL1_TRONCO_X, NIVEL1_SALIDA_X);
    tarou_iniciar(&t, NIVEL1_INICIO_X - ANCHO_CELDA / 2, SUELO_Y);
    for (cuadro = 0; cuadro < GUION_TUTORIAL_FIN + 300; cuadro++)
    {
        guion_mando(GUION_TUTORIAL, GUION_TUTORIAL_TRAMOS, cuadro, &m);
        tarou_actualizar(&t, &m);
        e = partida_actualizar(&p, &t);
        if (e == PARTIDA_REAPARECER)
        {
            reapariciones++;                         /* No debería pasar nunca.             */
            break;
        }
        if (e == PARTIDA_TRONCO_ROTO)
        {
            tronco_roto = 1;
        }
        if (e == PARTIDA_SALIDA)
        {
            break;
        }
    }
    COMPROBAR(reapariciones == 0);                   /* Ni un foso ni una zarza.            */
    COMPROBAR(tronco_roto == 1);                     /* La espada abrió el paso.            */
    COMPROBAR(e == PARTIDA_SALIDA);                  /* Y llegó al final...                 */
    COMPROBAR(cuadro == GUION_TUTORIAL_FIN);         /* ...en el cuadro previsto.           */
    COMPROBAR(p.reaparicion == controles[NIVEL1_CONTROLES - 1]); /* Pasó por todos.         */
}
