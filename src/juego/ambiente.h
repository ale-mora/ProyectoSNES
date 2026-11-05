/*---------------------------------------------------------------------------------
    ambiente.h — El humo de los tejados y los pájaros de la aldea: dónde va cada
    figura en la pantalla.

    PROPÓSITO
      Lógica pura (sin snes.h, probada en el PC: tests/prueba_ambiente.c) de los
      adornos que se mueven solos en la fase 1 (2026-11-05):
        - HUMO: de los tejados que dice res/aldea/aldea_datos.h (ALDEA_HUMOS) suben
          bocanadas que se apartan un poco a la derecha (el viento); cada una nace
          pequeña (cuadro 0), se hace penacho (cuadro 1) y, al subir 64 px, vuelve a
          nacer en el tejado. Dos tejados a la vista como mucho, dos bocanadas cada uno.
        - PÁJAROS: una bandada de tres que cruza el cielo hacia la derecha, en el plano
          de las montañas (la cámara los mueve a la mitad, como a la capa lejana),
          aleteando y subiendo y bajando un poco; al salir, da la vuelta (512 px).
      La pantalla le pasa la cámara cada frame (solo mientras el mundo avanza: en pausa
      se quedan quietos) y juego/ambiente_sprite.c copia el resultado a la tabla de
      sprites.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: juego/ambiente.c (usa aldea_datos.h con ALDEA_HUMOS_TABLAS).
      Lo usan:  juego/ambiente_sprite.c, pantallas/pantalla_fase1.c,
                tests/prueba_ambiente.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos)
      Figura (8 bytes: potencia de 2)
        x, y      s16  esquina de arriba a la izquierda de su pieza de 16x16, en la pantalla
        cuadro    u8   0 o 1 (bocanada / penacho; alas arriba / abajo)
        visible   u8   1 si se dibuja
      Ambiente
        fig       Figura[AMBIENTE_FIGURAS]  las 4 bocanadas (0..3) y los 3 pájaros (4..6)
        reloj     u16  frames desde que se entró
        bandada   u16  x de la bandada en el plano de las montañas (avanza 1 px cada 2 frames)
    → glosario: Sprite, Parallax
---------------------------------------------------------------------------------*/
#ifndef AMBIENTE_H
#define AMBIENTE_H

#include "sistema/tipos.h"

#define AMBIENTE_FUENTES 2               /* Tejados con humo a la vez (a la vista).       */
#define AMBIENTE_BOCANADAS 2             /* Bocanadas por tejado.                         */
#define AMBIENTE_HUMO (AMBIENTE_FUENTES * AMBIENTE_BOCANADAS)   /* Figuras 0..3.          */
#define AMBIENTE_PAJAROS 3               /* Figuras 4..6.                                 */
#define AMBIENTE_FIGURAS (AMBIENTE_HUMO + AMBIENTE_PAJAROS)
#define AMBIENTE_SUBIDA 64               /* Lo que sube una bocanada antes de renacer.    */
#define AMBIENTE_PENACHO 24              /* Desde aquí, la bocanada es penacho.           */
#define AMBIENTE_VUELTA 511              /* La bandada da la vuelta cada 512 px (máscara). */

typedef struct
{
    s16 x;
    s16 y;
    u8 cuadro;
    u8 visible;
    u16 libre;
} Figura;

typedef struct
{
    Figura fig[AMBIENTE_FIGURAS];
    u16 reloj;
    u16 bandada;
} Ambiente;

/* Todo apagado, el reloj a 0 y la bandada a la izquierda. */
void ambiente_iniciar(Ambiente *a);

/* Un frame: avanza el reloj y coloca las figuras para la cámara `camara`. */
void ambiente_actualizar(Ambiente *a, u16 camara);

#endif /* AMBIENTE_H */
