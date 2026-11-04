/*---------------------------------------------------------------------------------
    ninja_grande_anim.h — Cuántos cuadros tiene cada animación de ninja_grande.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      NINJA_GRANDE_<ANIMACION>_CUADROS  1..10  cuadros de 128x112 de esa tira
      NINJA_GRANDE_ANIMACIONES          7      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef NINJA_GRANDE_ANIM_H
#define NINJA_GRANDE_ANIM_H

#define NINJA_GRANDE_GUARDIA_CUADROS 8
#define NINJA_GRANDE_CAMINAR_CUADROS 6
#define NINJA_GRANDE_ATAQUE_1_CUADROS 3
#define NINJA_GRANDE_RECIBIR_CUADROS 6
#define NINJA_GRANDE_DERRIBADO_CUADROS 7
#define NINJA_GRANDE_LEVANTARSE_CUADROS 5
#define NINJA_GRANDE_KO_CUADROS 7

#define NINJA_GRANDE_ANIMACIONES 7

#endif /* NINJA_GRANDE_ANIM_H */
