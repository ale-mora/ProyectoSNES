/*---------------------------------------------------------------------------------
    tarou_anim.h (copia fija para pruebas) — Cuántos cuadros tiene cada animación
    de Tarou, con los valores de los marcadores.

    PROPÓSITO
      En la ROM, animaciones.c incluye el tarou_anim.h que GENERA el pipeline en
      res/personajes/tarou/ (cambia con el arte). Las pruebas de PC necesitan
      valores conocidos y estables: esta copia los fija a los cuadros por
      defecto de scripts/preparar_personaje.py. tests/Makefile pone -I. antes
      que -I../src, así que aquí gana esta copia.

    SE CONECTA CON
      Lo usan: ../src/juego/animaciones.c (solo al compilar las pruebas),
               tests/prueba_animacion.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      TAROU_<ANIMACION>_CUADROS  1..15  cuadros de 64x64 de esa tira
      TAROU_ANIMACIONES          13     cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef TAROU_ANIM_H
#define TAROU_ANIM_H

#define TAROU_REPOSO_CUADROS 4
#define TAROU_CAMINAR_CUADROS 8
#define TAROU_CORRER_CUADROS 8
#define TAROU_SALTAR_CUADROS 2
#define TAROU_CAER_CUADROS 2
#define TAROU_AGACHADO_CUADROS 2
#define TAROU_AGACHADO_CAMINAR_CUADROS 6
#define TAROU_DESENVAINAR_CUADROS 4
#define TAROU_ENVAINAR_CUADROS 4
#define TAROU_ATACAR_CUADROS 5
#define TAROU_ECHARSE_CUADROS 4
#define TAROU_DORMIDO_CUADROS 4
#define TAROU_DESPERTAR_CUADROS 4

#define TAROU_ANIMACIONES 13

#endif /* TAROU_ANIM_H */
