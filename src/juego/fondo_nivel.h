/*---------------------------------------------------------------------------------
    fondo_nivel.h — La capa cercana del nivel 1 dibujada por columnas.

    PROPÓSITO
      El nivel mide 2048 px (256 columnas de tiles de 8 px) y el mapa de la
      VRAM, 64x32 tiles: solo 512 px. No cabe entero, así que el mapa de la
      VRAM se usa como un ANILLO: cada vez que la cámara avanza, se escribe la
      columna que está a punto de entrar por el borde, encima de una que ya
      salió por el otro. Es el "streaming de columnas" de casi todos los juegos
      de plataformas de la SNES.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Usa:      juego/nivel.h (nivel_columna_vram, COLUMNAS_NIVEL),
                sistema/comun.h (VRAM_FONDO_A, VRAM_MAPA_A) y los datos de
                data.asm (nivel1_cerca_til/_pal/_col y nivel1_cerca_col_banco).
      Lo usan:  pantallas/pantalla_nivel1.c (cargar al entrar, columnas en cada
                VBlank, rellenar tras reaparecer).

    HARDWARE QUE TOCA
      VRAM (tiles y mapa del fondo 0), CGRAM (paleta 1) y el canal 0 de DMA
      con los registros a mano: VMAIN $2115 en modo "avanzar 32 palabras"
      (bajar una fila del mapa) para escribir una COLUMNA de una sola vez.

    VARIABLES: ninguna (ver fondo_nivel.c).
    → glosario: Streaming de columnas, Tilemap, DMA, VRAM
---------------------------------------------------------------------------------*/
#ifndef FONDO_NIVEL_H
#define FONDO_NIVEL_H

#include "sistema/tipos.h"

/* Tiles y paleta de la capa cercana a la VRAM, mapa en VRAM_MAPA_A (64x32) y las 64
   columnas alrededor de la cámara. Con la pantalla apagada (al entrar o tras un
   fundido a negro): copia 4 KB, más de lo que cabe en un VBlank. */
void fondo_nivel_cargar(u16 camara);

/* Vuelve a escribir las 64 columnas alrededor de la cámara (sin tocar tiles ni
   paleta), con la pantalla apagada: tras un salto de la cámara (reaparecer). */
void fondo_nivel_rellenar(u16 camara);

/* Escribe las dos columnas de los bordes: la que entra por la derecha y la que
   entra por la izquierda. Llamar en cada VBlank (pantalla_nivel1_vblank). No guarda
   estado: con la cámara a 2 px por cuadro como mucho, nunca se salta una columna. */
void fondo_nivel_vblank(u16 camara);

#endif /* FONDO_NIVEL_H */
