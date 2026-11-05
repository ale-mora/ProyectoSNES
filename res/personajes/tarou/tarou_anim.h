/*---------------------------------------------------------------------------------
    tarou_anim.h — Cuántos cuadros tiene cada animación de tarou.

    PROPÓSITO
      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los
      marcadores): no editar a mano. Así el código no necesita saber cuántos
      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.

    SE CONECTA CON
      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      TAROU_<ANIMACION>_CUADROS  1..10  cuadros de 96x64 de esa tira
      TAROU_ANIMACIONES          20      cuántas animaciones hay
---------------------------------------------------------------------------------*/
#ifndef TAROU_ANIM_H
#define TAROU_ANIM_H

#define TAROU_REPOSO_CUADROS 9
#define TAROU_CAMINAR_CUADROS 8
#define TAROU_CORRER_CUADROS 8
#define TAROU_SALTAR_CUADROS 3
#define TAROU_CAER_CUADROS 3
#define TAROU_AGACHADO_CUADROS 1
#define TAROU_AGACHADO_CAMINAR_CUADROS 8
#define TAROU_DESENVAINAR_CUADROS 9
#define TAROU_ENVAINAR_CUADROS 9
#define TAROU_ATACAR_CUADROS 1
#define TAROU_ECHARSE_CUADROS 6
#define TAROU_DORMIDO_CUADROS 9
#define TAROU_DESPERTAR_CUADROS 10
#define TAROU_GUARDIA_CUADROS 1
#define TAROU_GUARDIA_BU_CUADROS 8
#define TAROU_GOLPE_1_CUADROS 5
#define TAROU_GOLPE_2_CUADROS 5
#define TAROU_GOLPE_3_CUADROS 7
#define TAROU_RECIBIR_CUADROS 5
#define TAROU_KO_CUADROS 6

#define TAROU_ANIMACIONES 20

#endif /* TAROU_ANIM_H */
