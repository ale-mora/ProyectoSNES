/*---------------------------------------------------------------------------------
    nivel.h — Medidas del nivel 1 y la altura del suelo.

    PROPÓSITO
      Un solo sitio para las constantes del escenario (ancho del nivel, ancho de
      la pantalla, celda del protagonista, fila del suelo base) y para la cuenta
      que dice en qué
      posición del mapa de la VRAM va cada columna del nivel (el nivel mide
      2048 px y el mapa de la VRAM solo 512: se reutiliza como un anillo).

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/camara.c (ANCHO_NIVEL, ANCHO_PANTALLA), juego/tarou.c
                (ANCHO_CELDA), juego/fondo_nivel.c (nivel_columna_vram),
                pantallas/pantalla_nivel1.c, tests/prueba_camara.c.

    HARDWARE QUE TOCA
      ninguno: lógica pura (se compila también en el PC).

    VARIABLES (constantes)
      ANCHO_NIVEL     2048 px   ancho del nivel 1 (el tutorial: 8 pantallas)
      COLUMNAS_NIVEL  256       columnas de 8 px del nivel (ANCHO_NIVEL / 8)
      ANCHO_PANTALLA  256  px   ancho visible de la SNES
      ANCHO_CELDA     96   px   ancho de la celda del sprite del protagonista
      ALTO_CELDA      64   px   alto de la celda (su última fila es la de los pies)
      SUELO_Y         176  px   borde de arriba del suelo BASE (filas de celdas 11..13); lo
                                que hay encima lo dice la rejilla de juego/colision.h

    → glosario: Nivel, Escenario, Suelo, Streaming de columnas
---------------------------------------------------------------------------------*/
#ifndef NIVEL_H
#define NIVEL_H

#include "sistema/tipos.h"

#define ANCHO_NIVEL 2048     /* Ocho pantallas: el tutorial (diseno-tutorial-movimiento). */
#define COLUMNAS_NIVEL 256   /* Columnas de tiles de 8 px: 2048 / 8.                    */
#define ANCHO_PANTALLA 256   /* Resolución horizontal de la SNES.                       */
#define ANCHO_CELDA 96       /* Tarou (perro de Pixellab) mide hasta 81 px al correr:   */
#define ALTO_CELDA 64        /* celda de 96x64 = 3x2 piezas de 32x32 (§7).             */
#define SUELO_Y 176          /* Suelo base: 48 px (3 filas de celdas de 16) hasta la   */
                             /* 223. Donde empieza Tarou y la referencia de los dibujos. */

/* Columna `col` del nivel (0..255) → desplazamiento, en palabras, de su primera fila
   dentro del mapa de 64x32 de la VRAM. El mapa de 64x32 son DOS páginas de 32x32
   seguidas (0x400 palabras cada una): la columna va a la página (col / 32) % 2 y,
   dentro de ella, a la posición col % 32. Las columnas 0, 64, 128 y 192 caen en el
   mismo sitio: el mapa es un anillo y el scroll de la PPU también da la vuelta a los
   512 px, así que lo que se ve siempre coincide. */
u16 nivel_columna_vram(u16 col);

#endif /* NIVEL_H */
