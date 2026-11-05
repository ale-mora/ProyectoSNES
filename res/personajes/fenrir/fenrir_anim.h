/*---------------------------------------------------------------------------------
    fenrir_anim.h — Cuántos cuadros tiene cada animación de fenrir.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      FENRIR_<ANIMACION>_CUADROS  1..10  cuadros de 112x112 de esa tira
      FENRIR_ANIMACIONES          2      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef FENRIR_ANIM_H
#define FENRIR_ANIM_H

#define FENRIR_ENTRADA_CUADROS 4
#define FENRIR_ATAQUE_CUADROS 6

#define FENRIR_ANIMACIONES 2

#endif /* FENRIR_ANIM_H */
