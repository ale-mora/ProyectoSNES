/*---------------------------------------------------------------------------------
    prueba_camara.c — Pruebas de juego/camara.c y juego/nivel.c.

    PROPÓSITO
      Fijar con ejemplos la regla de la cámara (docs/diseno-demo-jugable.md §6):
      camara = x_pix + 32 - 128, recortada a 0..ANCHO_NIVEL - ANCHO_PANTALLA.
      Y la del suelo (§5): en la Revisión 1 es plano, siempre SUELO_Y.

    SE CONECTA CON
      Incluye:  minitest.h, juego/camara.h, juego/nivel.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_camara).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Cámara, Suelo, Nivel
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/camara.h"
#include "juego/nivel.h"

void pruebas_camara(void)
{
    /* Mientras Tarou está cerca del borde izquierdo, la cámara no se mueve. */
    COMPROBAR(camara_para(0) == 0);
    COMPROBAR(camara_para(96) == 0);              /* 96 + 32 - 128 = 0: justo el límite.  */
    COMPROBAR(camara_para(97) == 1);              /* Un píxel más y empieza a seguirlo.   */
    COMPROBAR(camara_para(200) == 104);           /* 200 + 32 - 128.                      */

    /* En el borde derecho se detiene en ANCHO_NIVEL - ANCHO_PANTALLA = 256. */
    COMPROBAR(camara_para(352) == 256);           /* 352 + 32 - 128 = 256: el límite.     */
    COMPROBAR(camara_para(448) == 256);           /* Más allá, sigue en 256.              */

    /* Suelo plano: la misma altura en todo el nivel. */
    COMPROBAR(nivel_suelo_en(0) == 208);
    COMPROBAR(nivel_suelo_en(500) == 208);
    COMPROBAR(SUELO_Y == 208);
    COMPROBAR(ANCHO_NIVEL == 512);
    COMPROBAR(ANCHO_PANTALLA == 256);
    COMPROBAR(ANCHO_CELDA == 64);
}
