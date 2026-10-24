/*---------------------------------------------------------------------------------
    combate.h — Cajas de golpe y de daño, la regla de profundidad y el impacto.

    PROPÓSITO
      Un golpe conecta si su caja de golpe se cruza con la caja de daño del
      objetivo, en la pantalla, Y las profundidades están cerca:
      |y_atacante − y_objetivo| <= TOLERANCIA_Y (docs/diseno-beat-em-up.md §3).
      Las cajas vienen por cuadro de animación, en píxeles de la celda (8 bytes
      por entrada, §5.2), y aquí se pasan al mundo según dónde está el luchador
      y hacia dónde mira. Si conecta, el objetivo pierde vida, parpadea en
      blanco (destello) y, con un golpe fuerte, los dos se congelan unos
      frames (parada de impacto). Es el corazón de combate.c: un luchador
      cambia el estado de otro (§5.1). Lógica pura, probada en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h (Cuerpo).
      Lo usan:  juego/combate.c y tests/prueba_combate.c (y el luchador del
                prototipo de combate, cuando exista).

    HARDWARE QUE TOCA
      ninguno (lógica pura).

    VARIABLES (campos de CajaCuadro, 8 bytes: lo que trae la tabla de animación)
      x, y           u8  píxeles desde la esquina superior izquierda de la celda, mirando al este
      ancho, alto    u8  tamaño en píxeles (ancho 0 = no hay caja en este cuadro)
      dano           u8  puntos de vida que quita (0 en las cajas de daño)
      efecto         u8  EFECTO_RETROCESO / EFECTO_DERRIBO
      relleno        u8[2]
    VARIABLES (campos de Celda: cómo se coloca la celda respecto a los pies)
      mitad_ancho    u8  la mitad del ancho de la celda (los pies están en el centro)
      alto           u8  alto de la celda (los pies están en la última fila)
      ancho          u8  ancho de la celda (para el espejo)
    VARIABLES (campos de CajaMundo)
      x, y           s16 esquina superior izquierda en píxeles del mundo (y de pantalla: y − z)
      ancho, alto    u8
    VARIABLES (campos de Golpeable, 4 bytes)
      vida           u8  0..255           0 = vencido
      invulnerable   u8  frames restantes sin poder ser golpeado
      destello       u8  frames restantes de parpadeo en blanco
      parada         u8  frames restantes congelado por la parada de impacto

    → glosario: Caja de golpe / caja de daño, Parada de impacto, Derribo, struct
---------------------------------------------------------------------------------*/
#ifndef COMBATE_H
#define COMBATE_H

#include "sistema/tipos.h"
#include "juego/mundo.h"

/*--- Efecto de un golpe ---------------------------------------------------------------*/
enum { EFECTO_RETROCESO = 0, EFECTO_DERRIBO = 1 };

/*--- Resultado de intentar un golpe ---------------------------------------------------*/
enum
{
    GOLPE_FALLA = 0,                 /* No se cruzan, demasiada distancia en y, o ya le dio. */
    GOLPE_RETROCESO = 1,             /* Conecta: el objetivo pasa a RECIBIR.               */
    GOLPE_DERRIBO = 2,               /* Conecta y derriba: DERRIBADO.                      */
    GOLPE_VENCE = 3                  /* Conecta y lo deja sin vida: KO.                    */
};

typedef struct
{
    s8 x;                            /* Esquina en la celda, mirando al este (con signo: una caja
                                        centrada en los pies, más ancha que la celda, empieza antes). */
    u8 y;                            /* Esquina de arriba en la celda.                     */
    u8 ancho, alto;                  /* Tamaño; ancho 0 = sin caja.                        */
    u8 dano;                         /* Vida que quita.                                    */
    u8 efecto;                       /* EFECTO_RETROCESO o EFECTO_DERRIBO.                 */
    u8 relleno[2];                   /* Hasta 8 bytes.                                     */
} CajaCuadro;

typedef struct
{
    u8 ancho, mitad_ancho, alto;     /* Tamaño de la celda del luchador.                   */
} Celda;

typedef struct
{
    s16 x, y;                        /* Esquina en el mundo (y ya es y − z).               */
    u8 ancho, alto;                  /* Tamaño.                                            */
} CajaMundo;

typedef struct
{
    u8 vida;                         /* Puntos de vida.                                    */
    u8 invulnerable;                 /* Frames sin poder recibir golpes.                   */
    u8 destello;                     /* Frames de parpadeo en blanco.                      */
    u8 parada;                       /* Frames congelado.                                  */
} Golpeable;

/* Pasa una caja de la celda al mundo para un cuerpo que mira a la izquierda
   (`izquierda` = 1, se espeja) o a la derecha. */
void combate_caja_mundo(const CajaCuadro *c, const Celda *celda, const Cuerpo *cuerpo,
                        u8 izquierda, CajaMundo *salida);

/* 1 si las dos cajas se pisan (comparten al menos un píxel). */
u8 combate_se_cruzan(const CajaMundo *a, const CajaMundo *b);

/* 1 si las profundidades están a TOLERANCIA_Y o menos. */
u8 combate_misma_profundidad(const Cuerpo *a, const Cuerpo *b);

/* Intenta el golpe `golpe` (ya en el mundo, con su daño y efecto en `datos`) contra la
   caja de daño `objetivo_caja` del objetivo. `golpeados` es la máscara de a quién ya le
   dio ESTE ataque (bit `bit_objetivo`): un ataque solo daña una vez a cada uno. Si conecta,
   quita vida, pone el destello y, con un derribo, la parada a los dos. */
u8 combate_golpear(const CajaMundo *golpe, const CajaCuadro *datos, const Cuerpo *atacante,
                   Golpeable *atacante_g, const CajaMundo *objetivo_caja,
                   const Cuerpo *objetivo, Golpeable *objetivo_g, u8 *golpeados,
                   u8 bit_objetivo);

/* 1 si `b` está a `alcance` px o menos de `a` en x Y a la misma profundidad: el golpe de entrada
   del relevo (spec §4) empuja a los que estén a menos de ALCANCE_ENTRADA_RELEVO px. */
u8 combate_empuje_cercano(const Cuerpo *a, const Cuerpo *b, u8 alcance);

/* Un frame de los contadores (invulnerable, destello, parada): bajan hasta 0. */
void combate_tic(Golpeable *g);

/* Lo que queda tras pagar el especial A+B: nunca mata (deja 1 como mínimo). */
u8 combate_pagar_especial(u8 vida);

#endif /* COMBATE_H */
