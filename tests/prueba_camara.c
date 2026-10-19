/*---------------------------------------------------------------------------------
    prueba_camara.c — Pruebas de juego/camara.c y juego/nivel.c.

    PROPÓSITO
      Fijar con ejemplos la regla de la cámara (docs/diseno-demo-jugable.md §6):
      camara = x_pix + 48 - 128 (48 = mitad de la celda de 96 px), recortada a
      0..ANCHO_NIVEL - ANCHO_PANTALLA.
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
    COMPROBAR(camara_para(80) == 0);              /* 80 + 48 - 128 = 0: justo el límite.  */
    COMPROBAR(camara_para(81) == 1);              /* Un píxel más y empieza a seguirlo.   */
    COMPROBAR(camara_para(200) == 120);           /* 200 + 48 - 128.                      */

    /* En el borde derecho se detiene en ANCHO_NIVEL - ANCHO_PANTALLA = 256. */
    COMPROBAR(camara_para(336) == 256);           /* 336 + 48 - 128 = 256: el límite.     */
    COMPROBAR(camara_para(416) == 256);           /* Más allá (el tope de Tarou), sigue en 256. */

    /* Suelo plano: la misma altura en todo el nivel. */
    COMPROBAR(nivel_suelo_en(0) == 208);
    COMPROBAR(nivel_suelo_en(500) == 208);
    COMPROBAR(SUELO_Y == 208);
    COMPROBAR(ANCHO_NIVEL == 512);
    COMPROBAR(ANCHO_PANTALLA == 256);
    COMPROBAR(ANCHO_CELDA == 96);                 /* Celda de 96x64: el perro de Pixellab */
    COMPROBAR(ALTO_CELDA == 64);                  /* mide hasta 81 px de ancho al correr. */
}
