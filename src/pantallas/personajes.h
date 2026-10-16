/*---------------------------------------------------------------------------------
    personajes.h — Los dos personajes de la demo como sprites: Tarou (lobo
    samurái) y Bárbara (mujer samurái). Solo animación de reposo.

    Uso desde una pantalla:
      personajes_mostrar(40, 150, 170, 150);   // al entrar: posiciones x,y de cada uno
      personajes_animar();                     // una vez por frame
      personajes_ocultar();                    // al salir
    La carga en VRAM se hace UNA sola vez desde main.c con personajes_cargar().
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
