/*---------------------------------------------------------------------------------
    personajes.h — Interfaz de los sprites de Tarou y Bárbara de la secuencia de
    arranque (solo animación de reposo, carga estática).

    PROPÓSITO
      Uso desde una pantalla:
        personajes_cargar();                         // al entrar (Mode 3/7 pisan la VRAM)
        personajes_mostrar(40, 96, 0, 160, 96);      // x,y de Tarou, si mira a la izquierda, x,y de Bárbara
        personajes_animar();                         // una vez por frame
        personajes_ocultar();                        // al salir
      Tarou es el mismo arte del nivel (reposo de tarou_sprite.c, 4 cuadros,
      hoy marcadores hasta que llegue el de Pixellab) dibujado de forma estática
      como cuatro sprites de 32x32; Bárbara, el sprite de la samurái del pack
      hasta la Revisión 2.

    SE CONECTA CON
      Incluye:  <snes.h>.
      Lo usan:  pantallas/personajes.c, pantallas/pantalla_intro.c,
                pantallas/pantalla_titulo.c, sistema/main.c.

    HARDWARE QUE TOCA
      ninguno directamente (ver personajes.c).

    VARIABLES (globales y estáticas de este archivo)
      ninguna.

    → glosario: Sprite, OAM, Metasprite
---------------------------------------------------------------------------------*/
#ifndef PERSONAJES_H
#define PERSONAJES_H

#include <snes.h>

/* Copia a VRAM/CGRAM los 4 cuadros de reposo de Tarou y los de Bárbara. Llamar al
   entrar en la pantalla (pisa lo que hubiera en la OAM). */
void personajes_cargar(void);

/* Pone a Tarou en (xt, yt), mirando a la izquierda si tarou_mira_izquierda vale 1,
   y a Bárbara en (xb, yb); los hace visibles. Coordenadas: esquina superior
   izquierda de la celda de 64x64. */
void personajes_mostrar(u16 xt, u16 yt, u8 tarou_mira_izquierda, u16 xb, u16 yb);

/* Avanza la animación de reposo. Llamar una vez por frame mientras se ven. */
void personajes_animar(void);

/* Esconde a los dos. */
void personajes_ocultar(void);

#endif /* PERSONAJES_H */
