/*---------------------------------------------------------------------------------
    gomasuri_anim.h — Cuántos cuadros tiene cada animación de gomasuri.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      GOMASURI_<ANIMACION>_CUADROS  1..10  cuadros de 144x112 de esa tira
      GOMASURI_ANIMACIONES          9      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef GOMASURI_ANIM_H
#define GOMASURI_ANIM_H

#define GOMASURI_GUARDIA_CUADROS 6
#define GOMASURI_CAMINAR_CUADROS 6
#define GOMASURI_CARGA_CUADROS 6
#define GOMASURI_TAJO_CUADROS 6
#define GOMASURI_CONVOCAR_CUADROS 4
#define GOMASURI_RECIBIR_CUADROS 5
#define GOMASURI_DERRIBADO_CUADROS 6
#define GOMASURI_LEVANTARSE_CUADROS 5
#define GOMASURI_KO_CUADROS 6

#define GOMASURI_ANIMACIONES 9

#endif /* GOMASURI_ANIM_H */
