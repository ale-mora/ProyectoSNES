/*---------------------------------------------------------------------------------
    colision.h — ¿Qué hay en este punto del nivel? La rejilla de colisión.

    PROPÓSITO
      El nivel está dibujado en Tiled sobre una rejilla de celdas de 16x16 px
      (128 columnas x 14 filas) y cada celda guarda qué ES: vacío, sólido,
      plataforma de un solo sentido o zarza. La física de Tarou solo pregunta
      "¿qué hay en el píxel (x, y)?": este módulo responde mirando la celda que
      contiene ese píxel. Sin dibujo ni hardware: es lógica pura, y en el PC
      las pruebas le dan rejillas pequeñas hechas a mano o la del nivel real.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/tarou.c (paredes, suelo, techo, agacharse bajo un techo),
                pantallas/pantalla_nivel1.c (colision_usar con la rejilla de la
                ROM), tests/prueba_colision.c y tests/prueba_tarou.c.
      Datos:    la rejilla la genera scripts/preparar_nivel.py
                (res/nivel1/colision.bin, en data.asm como nivel1_colision).

    HARDWARE QUE TOCA
      ninguno: lógica pura (se compila también en el PC).

    VARIABLES (constantes)
      CELDA_COLISION  16    lado de una celda en píxeles
      COLUMNAS_COLISION 128 celdas de ancho (2048 px)
      FILAS_COLISION  14    celdas de alto (224 px: la pantalla entera)
      COL_VACIO 0, COL_SOLIDO 1, COL_PLATAFORMA 2, COL_ZARZA 3   lo que hay en una celda

    → glosario: Colisión, AABB, Plataforma de un solo sentido, Metatile
---------------------------------------------------------------------------------*/
#ifndef COLISION_H
#define COLISION_H

#include "sistema/tipos.h"

#define CELDA_COLISION 16      /* Celdas de 16x16 px, como el arte de Pixellab.            */
#define COLUMNAS_COLISION 128  /* 2048 / 16.                                               */
#define FILAS_COLISION 14      /* 224 / 16: la altura de la pantalla.                      */

#define COL_VACIO 0            /* Aire: se atraviesa y no sostiene.                         */
#define COL_SOLIDO 1           /* Tierra: no se atraviesa por ningún lado.                  */
#define COL_PLATAFORMA 2       /* De un solo sentido: sostiene al caer encima; desde abajo
                                  y de lado se atraviesa.                                   */
#define COL_ZARZA 3            /* Se atraviesa, pero hace daño (reaparecer).                */

/* Elige la rejilla con la que se responde (COLUMNAS x FILAS bytes, fila a fila). */
void colision_usar(const u8 *rejilla);

/* Lo que hay en el píxel (x, y) del mundo. Fuera del nivel: a los lados (x >= 2048,
   y también "x negativa", que en u16 da la vuelta a un número enorme) es SÓLIDO, una
   pared; por debajo (y >= 224) y por encima (y negativa) es VACÍO: se cae por los
   fosos y se puede saltar más arriba del borde de la pantalla. */
u8 colision_en(u16 x, u16 y);

/* 1 si el píxel (x, y) es sólido (atajo de colision_en(x, y) == COL_SOLIDO). */
u8 colision_solido(u16 x, u16 y);

/* 1 si un pie en (x, y) se apoya: sólido o plataforma. */
u8 colision_apoya(u16 x, u16 y);

#endif /* COLISION_H */
