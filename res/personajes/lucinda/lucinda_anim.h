/*---------------------------------------------------------------------------------
    lucinda_anim.h — Cuántos cuadros tiene cada animación de lucinda.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      LUCINDA_<ANIMACION>_CUADROS  1..10  cuadros de 96x96 de esa tira
      LUCINDA_ANIMACIONES          2      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef LUCINDA_ANIM_H
#define LUCINDA_ANIM_H

#define LUCINDA_ENTRADA_CUADROS 4
#define LUCINDA_ATAQUE_CUADROS 6

#define LUCINDA_ANIMACIONES 2

#endif /* LUCINDA_ANIM_H */
