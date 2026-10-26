/*---------------------------------------------------------------------------------
    animaciones.h — Los estados del protagonista y la tabla de animaciones (una
    animación por estado).

    PROPÓSITO
      El enum Estado es la lista cerrada de "qué está haciendo Tarou": uno solo a
      la vez. Como cada estado tiene su animación (relación 1:1 de la spec §7),
      la misma constante sirve para indexar ANIMACIONES[] (cuántos cuadros,
      cada cuántos frames cambia, si hace bucle) y NOMBRES_ESTADO[] (texto para
      el HUD de depuración). La cantidad de cuadros no se escribe aquí: la
      genera el pipeline en tarou_anim.h a partir del arte.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/animaciones.c (define las tablas), juego/tarou.c (máquina
                de estados y avance de cuadros), juego/tarou_sprite.c (qué tira
                dibujar), juego/hud_debug.c (nombres), tests/prueba_animacion.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (declaradas aquí, definidas en animaciones.c)
      ANIMACIONES[ESTADO_CANTIDAD]     AnimDef      cuadros, frames_por_cuadro, bucle
      NOMBRES_ESTADO[ESTADO_CANTIDAD]  const char*  "PARADO", "CAMINANDO", ...

    → glosario: Estado, enum, struct, Máquina de estados
---------------------------------------------------------------------------------*/
#ifndef ANIMACIONES_H
#define ANIMACIONES_H

#include "sistema/tipos.h"

/* El orden importa: es el índice en las dos tablas y en las tiras de tarou_sprite.c. */
typedef enum
{
    ESTADO_PARADO = 0,          /* reposo                                              */
    ESTADO_CAMINANDO,           /* caminar                                             */
    ESTADO_CORRIENDO,           /* correr                                              */
    ESTADO_SALTANDO,            /* saltar (subiendo)                                   */
    ESTADO_CAYENDO,             /* caer                                                */
    ESTADO_AGACHADO,            /* agachado, quieto                                    */
    ESTADO_AGACHADO_CAMINANDO,  /* agachado_caminar                                    */
    ESTADO_DESENVAINANDO,       /* desenvainar                                         */
    ESTADO_ENVAINANDO,          /* envainar                                            */
    ESTADO_ATACANDO,            /* atacar                                              */
    ESTADO_ECHANDOSE,           /* echarse (va a dormir)                               */
    ESTADO_DORMIDO,             /* dormido (bucle)                                     */
    ESTADO_DESPERTANDO,         /* despertar                                           */
    ESTADO_CANTIDAD             /* no es un estado: cuántos hay (13)                   */
} Estado;

typedef struct
{
    u8 cuadros;                 /* cuántos cuadros de 64x64 tiene la tira              */
    u8 frames_por_cuadro;       /* cada cuántos frames avanza al siguiente cuadro      */
    u8 bucle;                   /* 1 = vuelve al primero al terminar; 0 = se queda     */
} AnimDef;

extern const AnimDef ANIMACIONES[ESTADO_CANTIDAD];
extern const char *const NOMBRES_ESTADO[ESTADO_CANTIDAD];

#endif /* ANIMACIONES_H */
