/*---------------------------------------------------------------------------------
    barbara_anim.h — Cuántos cuadros tiene cada animación de barbara.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      BARBARA_<ANIMACION>_CUADROS  1..10  cuadros de 144x112 de esa tira
      BARBARA_ANIMACIONES          25      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef BARBARA_ANIM_H
#define BARBARA_ANIM_H

#define BARBARA_GUARDIA_CUADROS 8
#define BARBARA_CAMINAR_CUADROS 8
#define BARBARA_CORRER_CUADROS 8
#define BARBARA_SALTAR_CUADROS 3
#define BARBARA_CAER_CUADROS 3
#define BARBARA_GOLPE_1_CUADROS 5
#define BARBARA_GOLPE_2_CUADROS 5
#define BARBARA_GOLPE_3_CUADROS 7
#define BARBARA_AEREO_CUADROS 5
#define BARBARA_AGARRE_CUADROS 2
#define BARBARA_GOLPE_AGARRE_CUADROS 3
#define BARBARA_LANZAMIENTO_CUADROS 7
#define BARBARA_AGARRADA_CUADROS 2
#define BARBARA_RECIBIR_CUADROS 6
#define BARBARA_DERRIBADA_CUADROS 7
#define BARBARA_EN_SUELO_CUADROS 1
#define BARBARA_LEVANTARSE_CUADROS 5
#define BARBARA_ESPECIAL_CUADROS 6
#define BARBARA_LANZAR_ARMA_CUADROS 5
#define BARBARA_RECOGER_CUADROS 4
#define BARBARA_ENTRADA_RELEVO_CUADROS 6
#define BARBARA_INVOCAR_CUADROS 4
#define BARBARA_KO_CUADROS 5
#define BARBARA_VICTORIA_CUADROS 6
#define BARBARA_INTRO_VENGANZA_CUADROS 4

#define BARBARA_ANIMACIONES 25

#endif /* BARBARA_ANIM_H */
