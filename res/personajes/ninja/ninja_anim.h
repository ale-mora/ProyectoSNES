/*---------------------------------------------------------------------------------
    ninja_anim.h — Cuántos cuadros tiene cada animación de ninja.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      NINJA_<ANIMACION>_CUADROS  1..10  cuadros de 96x96 de esa tira
      NINJA_ANIMACIONES          14      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef NINJA_ANIM_H
#define NINJA_ANIM_H

#define NINJA_GUARDIA_CUADROS 8
#define NINJA_CAMINAR_CUADROS 6
#define NINJA_ATAQUE_1_CUADROS 3
#define NINJA_ATAQUE_2_CUADROS 8
#define NINJA_BLOQUEO_CUADROS 2
#define NINJA_AGARRE_CUADROS 2
#define NINJA_LANZAMIENTO_CUADROS 6
#define NINJA_RECIBIR_CUADROS 6
#define NINJA_DERRIBADO_CUADROS 7
#define NINJA_EN_SUELO_CUADROS 1
#define NINJA_LEVANTARSE_CUADROS 5
#define NINJA_AGARRADO_CUADROS 2
#define NINJA_KO_CUADROS 7
#define NINJA_PATADA_CUADROS 7

#define NINJA_ANIMACIONES 14

#endif /* NINJA_ANIM_H */
