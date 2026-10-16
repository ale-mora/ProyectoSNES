/*---------------------------------------------------------------------------------
    camara.h — La cámara: qué trozo del nivel se ve en pantalla.

    PROPÓSITO
      La SNES no mueve "objetos": mueve los fondos escribiendo un desplazamiento
      (scroll) en la PPU. La cámara es ese desplazamiento. camara_para() lo
      calcula a partir de la x del protagonista: lo mantiene centrado y se
      detiene en los bordes del nivel.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  pantallas/pantalla_nivel1.c (bgSetScroll con el resultado),
                juego/tarou_sprite.c y juego/hud_debug.c (x_pantalla = x - camara),
                tests/prueba_camara.c.

    HARDWARE QUE TOCA
      ninguno: lógica pura. El scroll lo escribe pantalla_nivel1.c.

    VARIABLES: ninguna.
    → glosario: Cámara, Parallax, BG / capas
---------------------------------------------------------------------------------*/
#ifndef CAMARA_H
#define CAMARA_H

#include "sistema/tipos.h"

/* Desplazamiento horizontal de la cámara para un protagonista cuya celda empieza
   en x_pix: su centro (x + 32) queda en el centro de la pantalla (128), recortado
   a 0..ANCHO_NIVEL - ANCHO_PANTALLA. Devuelve píxeles enteros. */
u16 camara_para(u16 x_pix);

#endif /* CAMARA_H */
