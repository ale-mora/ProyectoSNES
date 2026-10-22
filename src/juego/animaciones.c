/*---------------------------------------------------------------------------------
    animaciones.c — Tabla de animaciones por estado y nombres de los estados.

    PROPÓSITO
      Rellenar ANIMACIONES[] con los frames por cuadro y bucles de la spec (§7,
      "a afinar") y con los cuadros que genera el pipeline (tarou_anim.h), y
      NOMBRES_ESTADO[] con el texto que muestra el HUD. Son tablas const globales:
      tcc816 las pone en la WRAM ($7F, copiadas desde la ROM al arrancar), no en
      la ROM (solo las static const se quedan en la ROM; unidad 13).

    SE CONECTA CON
      Incluye:  juego/animaciones.h, tarou_anim.h (por ruta de inclusión: en la
                ROM, res/personajes/tarou/ vía -Ires/personajes/tarou del
                Makefile; en las pruebas, la copia fija de tests/).
      Lo usan:  juego/tarou.c, juego/tarou_sprite.c, juego/hud_debug.c, tests/.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales const, en la WRAM)
      ANIMACIONES[13]     AnimDef      ver animaciones.h
      NOMBRES_ESTADO[13]  const char*  nombres sin el prefijo ESTADO_

    → glosario: Estado, const, Frame
---------------------------------------------------------------------------------*/
#include "juego/animaciones.h"
#include "tarou_anim.h"

/* Mismo orden que el enum Estado. Columnas: cuadros, frames por cuadro, bucle. */
#define DURACION_ATAQUE 20          /* Frames que dura el golpe (1/3 s), aproximados (§4).  */

const AnimDef ANIMACIONES[ESTADO_CANTIDAD] = {
    {TAROU_REPOSO_CUADROS, 12, 1},            /* ESTADO_PARADO              */
    {TAROU_CAMINAR_CUADROS, 6, 1},            /* ESTADO_CAMINANDO           */
    {TAROU_CORRER_CUADROS, 4, 1},             /* ESTADO_CORRIENDO           */
    {TAROU_SALTAR_CUADROS, 6, 0},             /* ESTADO_SALTANDO            */
    {TAROU_CAER_CUADROS, 6, 0},               /* ESTADO_CAYENDO             */
    {TAROU_AGACHADO_CUADROS, 12, 1},          /* ESTADO_AGACHADO            */
    {TAROU_AGACHADO_CAMINAR_CUADROS, 8, 1},   /* ESTADO_AGACHADO_CAMINANDO  */
    {TAROU_DESENVAINAR_CUADROS, 5, 0},        /* ESTADO_DESENVAINANDO       */
    {TAROU_ENVAINAR_CUADROS, 5, 0},           /* ESTADO_ENVAINANDO          */
    {TAROU_ATACAR_CUADROS, DURACION_ATAQUE / TAROU_ATACAR_CUADROS, 0}, /* ESTADO_ATACANDO: el
                                                 golpe dura unos 20 frames: exacto si los cuadros
                                                 dividen a 20 (hoy 1, la pose fija Attack_1: 1 x 20);
                                                 con 3 cuadros, 3 x 6 = 18. La división es entera y
                                                 la hace el compilador: son dos constantes.  */
    {TAROU_ECHARSE_CUADROS, 8, 0},            /* ESTADO_ECHANDOSE           */
    {TAROU_DORMIDO_CUADROS, 20, 1},           /* ESTADO_DORMIDO             */
    {TAROU_DESPERTAR_CUADROS, 8, 0},          /* ESTADO_DESPERTANDO         */
};

const char *const NOMBRES_ESTADO[ESTADO_CANTIDAD] = {
    "PARADO", "CAMINANDO", "CORRIENDO", "SALTANDO", "CAYENDO", "AGACHADO",
    "AGACHADO_CAMINANDO", "DESENVAINANDO", "ENVAINANDO", "ATACANDO", "ECHANDOSE",
    "DORMIDO", "DESPERTANDO",
};
