/*---------------------------------------------------------------------------------
    ninja_rapido_anim.h — Cuántos cuadros tiene cada animación de ninja_rapido.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      NINJA_RAPIDO_<ANIMACION>_CUADROS  1..10  cuadros de 96x96 de esa tira
      NINJA_RAPIDO_ANIMACIONES          14      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef NINJA_RAPIDO_ANIM_H
#define NINJA_RAPIDO_ANIM_H

#define NINJA_RAPIDO_GUARDIA_CUADROS 8
#define NINJA_RAPIDO_CAMINAR_CUADROS 6
#define NINJA_RAPIDO_ATAQUE_1_CUADROS 3
#define NINJA_RAPIDO_ATAQUE_2_CUADROS 8
#define NINJA_RAPIDO_BLOQUEO_CUADROS 2
#define NINJA_RAPIDO_AGARRE_CUADROS 2
#define NINJA_RAPIDO_LANZAMIENTO_CUADROS 6
#define NINJA_RAPIDO_RECIBIR_CUADROS 6
#define NINJA_RAPIDO_DERRIBADO_CUADROS 7
#define NINJA_RAPIDO_EN_SUELO_CUADROS 1
#define NINJA_RAPIDO_LEVANTARSE_CUADROS 5
#define NINJA_RAPIDO_AGARRADO_CUADROS 2
#define NINJA_RAPIDO_KO_CUADROS 7
#define NINJA_RAPIDO_PATADA_CUADROS 7

#define NINJA_RAPIDO_ANIMACIONES 14

#endif /* NINJA_RAPIDO_ANIM_H */
