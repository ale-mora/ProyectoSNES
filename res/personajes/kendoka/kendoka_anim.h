/*---------------------------------------------------------------------------------
    kendoka_anim.h — Cuántos cuadros tiene cada animación de kendoka.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      KENDOKA_<ANIMACION>_CUADROS  1..10  cuadros de 128x112 de esa tira
      KENDOKA_ANIMACIONES          10      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef KENDOKA_ANIM_H
#define KENDOKA_ANIM_H

#define KENDOKA_GUARDIA_CUADROS 6
#define KENDOKA_CAMINAR_CUADROS 6
#define KENDOKA_ATAQUE_CUADROS 6
#define KENDOKA_BLOQUEO_CUADROS 2
#define KENDOKA_AGARRE_CUADROS 2
#define KENDOKA_LANZAMIENTO_CUADROS 5
#define KENDOKA_RECIBIR_CUADROS 5
#define KENDOKA_DERRIBADO_CUADROS 6
#define KENDOKA_LEVANTARSE_CUADROS 5
#define KENDOKA_KO_CUADROS 6

#define KENDOKA_ANIMACIONES 10

#endif /* KENDOKA_ANIM_H */
