/*---------------------------------------------------------------------------------
    personajes.h — Interfaz de los sprites de Tarou y Bárbara de la secuencia de
    arranque (solo animación de reposo).

    PROPÓSITO
      Uso desde una pantalla:
        personajes_cargar();                    // al entrar (Mode 3/7 pisan la VRAM)
        personajes_mostrar(40, 150, 170, 150);  // posiciones x,y de cada uno
        personajes_animar();                    // una vez por frame
        personajes_ocultar();                   // al salir
      Tarou usa de momento el sprite provisional del lobo samurái (futuro
      enemigo); Bárbara, el de la samurái del pack.

    SE CONECTA CON
      Incluye:  <snes.h>.
      Lo usan:  pantallas/personajes.c, pantallas/pantalla_intro.c,
                pantallas/pantalla_titulo.c, sistema/main.c.

    HARDWARE QUE TOCA
      ninguno directamente (ver personajes.c).

    VARIABLES (globales y estáticas de este archivo)
      ninguna.

    → glosario: Sprite, OAM
---------------------------------------------------------------------------------*/
#ifndef PERSONAJES_H
#define PERSONAJES_H

#include <snes.h>

/* Copia los tiles y paletas de ambos personajes a la VRAM/CGRAM. Llamar una vez. */
void personajes_cargar(void);

/* Pone a Tarou en (xt, yt) y a Bárbara en (xb, yb) y los hace visibles.
   Las coordenadas son la esquina superior izquierda de la celda de 64x64. */
void personajes_mostrar(u16 xt, u16 yt, u16 xb, u16 yb);

/* Avanza la animación de reposo. Llamar una vez por frame mientras se ven. */
void personajes_animar(void);

/* Esconde a los dos. */
void personajes_ocultar(void);

#endif /* PERSONAJES_H */
