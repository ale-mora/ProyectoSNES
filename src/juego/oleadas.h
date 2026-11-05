/*---------------------------------------------------------------------------------
    oleadas.h — Las zonas del beat 'em up: dónde se bloquea la cámara y cuándo se libera.

    PROPÓSITO
      Un beat 'em up de carril avanza por ZONAS (docs/diseno-beat-em-up.md §3): la
      cámara sigue al jugador hasta el punto de una zona, se BLOQUEA, aparece una oleada
      de enemigos y hasta vencerla no se puede seguir; después se libera («GO →») hasta la
      zona siguiente. Este módulo es solo esa máquina de estados, sin enemigos todavía:
      la tabla de zonas dice en qué posición de la cámara se bloquea cada una, y
      `oleadas_vencer` es lo que llamará la lógica de enemigos cuando no quede ninguno
      (hoy, A en la pantalla de la fase 1 hace de «oleada vencida»).
      Lógica pura, sin snes.h: se prueba en el PC (tests/prueba_oleadas.c).

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  pantallas/pantalla_fase1.c (el límite de la cámara) y tests/prueba_oleadas.c.
                Más adelante: enemigos/ (qué sale en cada oleada).

    HARDWARE QUE TOCA
      ninguno.

      Desde el 2026-11-05 la fase tiene cuatro ESCENARIOS (aldea, bosque de bambú,
      monasterio y dojo; cada uno se carga aparte, con su fondo) y cada escenario tiene su
      tabla: los puntos de bloqueo, su ancho (la aldea 2048 px, los otros 1024) y QUÉ sale en
      cada oleada (hasta 3 tipos de enemigos/ninja.h).

    VARIABLES (campos de Oleadas; 4 bytes)
      actual     u8   zona en curso (zonas = ya no quedan)
      bloqueada  u8   1 mientras la oleada de la zona `actual` está viva
      escenario  u8   ESCENARIO_* (qué tabla se usa)
      zonas      u8   cuántas zonas tiene ese escenario
    → glosario: Zona, Oleada, Cámara
---------------------------------------------------------------------------------*/
#ifndef OLEADAS_H
#define OLEADAS_H

#include "sistema/tipos.h"

#define OLEADAS_ZONAS 4                  /* Zonas como mucho en un escenario.                 */
#define OLEADA_TIPOS 3                   /* Enemigos como mucho en una oleada (las 3 ranuras). */

#define ESCENARIO_ALDEA 0                /* Los cuatro escenarios de la fase 1, en orden.     */
#define ESCENARIO_BOSQUE 1
#define ESCENARIO_MONASTERIO 2
#define ESCENARIO_DOJO 3
#define ESCENARIOS 4

typedef struct
{
    u8 actual;
    u8 bloqueada;
    u8 escenario;
    u8 zonas;
} Oleadas;

/* Antes de la primera zona de la aldea, sin bloqueo. */
void oleadas_iniciar(Oleadas *o);

/* Igual, en el escenario `escenario` (ESCENARIO_*). */
void oleadas_iniciar_escenario(Oleadas *o, u8 escenario);

/* El ancho del escenario en píxeles (2048 la aldea, 1024 los demás). */
u16 oleadas_ancho(u8 escenario);

/* 1 cuando ya se vencieron todas las oleadas del escenario. */
u8 oleadas_terminadas(const Oleadas *o);

/* Los tipos de enemigo (NINJA_*) de la oleada en curso y cuántos son (1..3). */
const u8 *oleadas_tipos(const Oleadas *o);
u8 oleadas_cuantos(const Oleadas *o);

/* El punto de una zona recortado a donde la cámara puede llegar (CAMARA_MAX): un punto más
   allá del final del nivel se bloquea en el final, en vez de no bloquearse nunca. */
u16 oleadas_alcanzable(u16 punto);

/* Hasta dónde puede llegar la cámara: la posición de la zona en curso o, si ya no quedan
   zonas, el final del escenario (su ancho menos una pantalla). */
u16 oleadas_limite(const Oleadas *o);

/* Un frame: si la cámara llegó al punto de la zona y no está bloqueada, la bloquea y
   devuelve 1 (en ese frame salen los enemigos); si no, 0. */
u8 oleadas_actualizar(Oleadas *o, u16 camara);

/* La oleada de la zona en curso está vencida: se libera y pasa a la siguiente. Sin
   efecto si no estaba bloqueada. */
void oleadas_vencer(Oleadas *o);

#endif /* OLEADAS_H */
