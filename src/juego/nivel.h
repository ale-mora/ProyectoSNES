/*---------------------------------------------------------------------------------
    nivel.h — Medidas del nivel 1 y la altura del suelo.

    PROPÓSITO
      Un solo sitio para las constantes del escenario (ancho del nivel, ancho de
      la pantalla, celda del protagonista) y para la función que responde "¿a
      qué altura está el suelo en esta x?". Hoy el suelo es plano; cuando haya
      desniveles solo cambia el interior de nivel_suelo_en().

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/camara.c (ANCHO_NIVEL, ANCHO_PANTALLA), juego/tarou.c
                (ANCHO_CELDA, el suelo), pantallas/pantalla_nivel1.c,
                tests/prueba_camara.c.

    HARDWARE QUE TOCA
      ninguno: lógica pura (se compila también en el PC).

    VARIABLES (constantes)
      ANCHO_NIVEL     512  px   ancho del nivel 1 (mapa de 64x32 tiles)
      ANCHO_PANTALLA  256  px   ancho visible de la SNES
      ANCHO_CELDA     96   px   ancho de la celda del sprite del protagonista
      ALTO_CELDA      64   px   alto de la celda (su última fila es la de los pies)
      SUELO_Y         176  px   fila donde empieza la hierba del suelo (los pies, en la 175)

    → glosario: Nivel, Escenario, Suelo
---------------------------------------------------------------------------------*/
#ifndef NIVEL_H
#define NIVEL_H

#include "sistema/tipos.h"

#define ANCHO_NIVEL 512      /* Dos pantallas: el escenario del diseño (§8).           */
#define ANCHO_PANTALLA 256   /* Resolución horizontal de la SNES.                       */
#define ANCHO_CELDA 96       /* Tarou (perro de Pixellab) mide hasta 81 px al correr:   */
#define ALTO_CELDA 64        /* celda de 96x64 = 3x2 piezas de 32x32 (§7).             */
#define SUELO_Y 176          /* Fila de la hierba de res/nivel1/fondo_cerca.png: 48 px  */
                             /* de suelo (3 filas de tiles de 16) hasta la 223 (§8).   */

/* Altura (fila y) del suelo bajo la coordenada x del mundo. Plano: siempre SUELO_Y. */
u16 nivel_suelo_en(u16 x_pix);

#endif /* NIVEL_H */
