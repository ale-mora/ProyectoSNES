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
   en x_pix: su centro (x + 48) queda en el centro de la pantalla (128), recortado
   a 0..ANCHO_NIVEL - ANCHO_PANTALLA. Devuelve píxeles enteros. */
u16 camara_para(u16 x_pix);

/*--- Cámara del beat 'em up (docs/diseno-beat-em-up.md §3, «zonas por oleadas») -------
    No retrocede nunca (como en Final Fight) y no pasa del `limite` que pone la zona en
    curso (juego/oleadas.c: la cámara se bloquea hasta vencer la oleada). */
#define CAMARA_PASO 2        /* Píxeles por frame como mucho: fondo_nivel_actualizar no se
                                salta ninguna columna de 8 px y no se adelanta al protagonista. */

/* Un frame de cámara: persigue la x del CENTRO del protagonista (queda en el centro de la
   pantalla, 128) pero solo hacia delante, a CAMARA_PASO por frame, sin pasar de `limite`
   (0xFFFF = sin límite) ni del final del nivel (ANCHO_NIVEL − ANCHO_PANTALLA). Devuelve la
   cámara nueva. */
u16 camara_avanzar(u16 camara, u16 x_centro, u16 limite);

#endif /* CAMARA_H */
