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

    VARIABLES (campos de Oleadas; 4 bytes)
      actual     u8   zona en curso (OLEADAS_ZONAS = ya no quedan)
      bloqueada  u8   1 mientras la oleada de la zona `actual` está viva
    → glosario: Zona, Oleada, Cámara
---------------------------------------------------------------------------------*/
#ifndef OLEADAS_H
#define OLEADAS_H

#include "sistema/tipos.h"

#define OLEADAS_ZONAS 4                  /* Zonas de la fase 1 (hoy provisionales).           */

typedef struct
{
    u8 actual;
    u8 bloqueada;
    u8 libre[2];                         /* Relleno hasta 4 bytes.                            */
} Oleadas;

/* Antes de la primera zona, sin bloqueo. */
void oleadas_iniciar(Oleadas *o);

/* Hasta dónde puede llegar la cámara: la posición de la zona en curso, o 0xFFFF si ya
   no quedan zonas (solo manda el final del nivel). */
u16 oleadas_limite(const Oleadas *o);

/* Un frame: si la cámara llegó al punto de la zona y no está bloqueada, la bloquea y
   devuelve 1 (en ese frame salen los enemigos); si no, 0. */
u8 oleadas_actualizar(Oleadas *o, u16 camara);

/* La oleada de la zona en curso está vencida: se libera y pasa a la siguiente. Sin
   efecto si no estaba bloqueada. */
void oleadas_vencer(Oleadas *o);

#endif /* OLEADAS_H */
