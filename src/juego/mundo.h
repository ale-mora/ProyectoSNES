/*---------------------------------------------------------------------------------
    mundo.h — El mundo del beat 'em up en x/y/z: posición, carril y salto.

    PROPÓSITO
      Cada luchador (protagonista, ninja, jefe) tiene un Cuerpo: x es el avance
      por la fase, y la profundidad en el carril (la línea de pantalla de los
      pies) y z la altura sobre el suelo (docs/diseno-beat-em-up.md §3). Se
      dibuja en `y − z`; la sombra, siempre en y. Aquí se mueve el cuerpo un
      frame: x e y se recortan al carril, y z cae con la gravedad hasta el
      suelo. En el prototipo el carril es una franja fija (Carril); cuando
      lleguen las zonas de Tiled, sus bordes saldrán por columna.
      Todo en punto fijo 12.4, como el resto del juego. x, y y z nunca son
      negativas (se recortan); las velocidades sí pueden serlo.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, sistema/puntofijo.h.
      Lo usan:  juego/mundo.c, juego/combate.c (posiciones de las cajas) y
                tests/prueba_mundo.c.

    HARDWARE QUE TOCA
      ninguno (lógica pura, probada en el PC).

    VARIABLES (campos de Cuerpo, 16 bytes: potencia de 2 para indexar sin tcc__mul)
      x_fp, y_fp, z_fp     s16  >= 0, punto fijo 12.4    posición (pies) y altura
      vx_fp, vy_fp, vz_fp  s16  punto fijo 12.4          velocidad por frame (vz > 0 = sube)
      en_suelo             u8   0 / 1                    1 si z == 0 y no está subiendo
      relleno              u8[3]                         hasta 16 bytes
    VARIABLES (campos de Carril)
      x_min_fp, x_max_fp   s16  punto fijo 12.4          por dónde se puede andar en x
      y_arriba_fp          s16  punto fijo 12.4          borde de arriba (fondo) del carril
      y_abajo_fp           s16  punto fijo 12.4          borde de abajo (cerca de la cámara)

    → glosario: Carril, Profundidad (y) y altura (z), Punto fijo, struct
---------------------------------------------------------------------------------*/
#ifndef MUNDO_H
#define MUNDO_H

#include "sistema/tipos.h"
#include "sistema/puntofijo.h"

typedef struct
{
    s16 x_fp, y_fp, z_fp;            /* Pies en el mundo (x, y) y altura (z).              */
    s16 vx_fp, vy_fp, vz_fp;         /* Velocidad por frame; vz positiva sube.             */
    u8 en_suelo;                     /* 1 si está apoyado.                                 */
    u8 relleno[3];                   /* Hasta 16 bytes: a[i] es un desplazamiento.         */
} Cuerpo;

typedef struct
{
    s16 x_min_fp, x_max_fp;          /* Límites del avance.                                */
    s16 y_arriba_fp, y_abajo_fp;     /* Límites de la profundidad.                         */
} Carril;

/* Coloca el cuerpo quieto y apoyado en (x, y) del mundo, en píxeles. */
void mundo_colocar(Cuerpo *c, u16 x, u16 y);

/* Si está apoyado, despega con VEL_SALTO_Z. Devuelve 1 si saltó. */
u8 mundo_saltar(Cuerpo *c);

/* Un frame: suma la velocidad, recorta x e y al carril y aplica la gravedad a z.
   Devuelve 1 en el frame en que aterriza (z vuelve a 0 desde el aire). */
u8 mundo_mover(Cuerpo *c, const Carril *carril);

/* Línea de pantalla del dibujo, en píxeles del mundo: y − z (0 si z > y). */
u16 mundo_y_dibujo(const Cuerpo *c);

#endif /* MUNDO_H */
