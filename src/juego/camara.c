/*---------------------------------------------------------------------------------
    camara.c — Cálculo de la cámara con recorte en los bordes del nivel.

    PROPÓSITO
      camara = x_pix + 48 - 128, recortada a [0, ANCHO_NIVEL - ANCHO_PANTALLA]
      (48 = ANCHO_CELDA / 2: el centro de la celda de 96 px)
      (docs/diseno-demo-jugable.md §6). Sin suavizado: la cámara sigue al
      protagonista píxel a píxel, que es lo que hacen la mayoría de plataformas
      de 16 bits.

    SE CONECTA CON
      Incluye:  juego/camara.h, juego/nivel.h (ANCHO_NIVEL, ANCHO_PANTALLA, ANCHO_CELDA).
      Lo usan:  pantallas/pantalla_nivel1.c, tests/prueba_camara.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Cámara, s16
---------------------------------------------------------------------------------*/
#include "juego/camara.h"
#include "juego/nivel.h"

#define CAMARA_MAX (ANCHO_NIVEL - ANCHO_PANTALLA)   /* 256: la última posición con nivel. */

u16 camara_para(u16 x_pix)
{
    /* En s16 para que la resta pueda dar negativo cerca del borde izquierdo:
       con u16, 10 + 48 - 128 "daría la vuelta" a 65466 en vez de -70. */
    s16 camara = (s16)x_pix + (ANCHO_CELDA / 2) - (ANCHO_PANTALLA / 2);
    if (camara < 0)
    {
        camara = 0;                     /* Borde izquierdo: la cámara no retrocede.   */
    }
    if (camara > CAMARA_MAX)
    {
        camara = CAMARA_MAX;            /* Borde derecho: ya no hay más nivel.        */
    }
    return (u16)camara;
}
