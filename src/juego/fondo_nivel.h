/*---------------------------------------------------------------------------------
    fondo_nivel.h — La capa cercana del nivel 1 dibujada por columnas.

    PROPÓSITO
      El nivel mide 2048 px (256 columnas de tiles de 8 px) y el mapa de la
      VRAM, 64x32 tiles: solo 512 px. No cabe entero, así que el mapa de la
      VRAM se usa como un ANILLO: cada vez que la cámara entra en una columna
      nueva, se escribe la columna que está a punto de entrar por el borde,
      encima de una que ya salió por el otro. Es el "streaming de columnas" de
      casi todos los juegos de plataformas de la SNES. Las copias se APUNTAN
      durante el frame en la lista de DMA (juego/dma_lista.h) y se disparan
      en el VBlank con todo lo demás.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Usa:      juego/nivel.h (nivel_columna_vram, COLUMNAS_NIVEL),
                juego/dma_lista.h, sistema/comun.h (VRAM_FONDO_A, VRAM_MAPA_A)
                y los datos de data.asm (nivel1_cerca_til/_pal/_col y
                nivel1_cerca_col_banco).
      Lo usan:  pantallas/pantalla_nivel1.c (cargar al entrar, actualizar cada
                frame después de dibujar a Tarou, rellenar tras reaparecer).

    HARDWARE QUE TOCA
      VRAM (tiles y mapa del fondo 0) y CGRAM (paleta 1) por el SDK al cargar;
      el mapa por columnas, por la lista de DMA (dma_ejecutar.asm, con VMAIN en
      modo "avanzar 32 palabras": bajar una fila del mapa).

    VARIABLES: ninguna (ver fondo_nivel.c).
    → glosario: Streaming de columnas, Tilemap, DMA / HDMA, VRAM
---------------------------------------------------------------------------------*/
#ifndef FONDO_NIVEL_H
#define FONDO_NIVEL_H

#include "sistema/tipos.h"

/* Tiles y paleta de la capa cercana a la VRAM, mapa en VRAM_MAPA_A (64x32) y las 64
   columnas alrededor de la cámara. Con la pantalla apagada (al entrar): copia 4 KB,
   más de lo que cabe en un VBlank. */
void fondo_nivel_cargar(u16 camara);

/* Vuelve a escribir las 64 columnas alrededor de la cámara (sin tocar tiles ni
   paleta), con la pantalla apagada: tras un salto de la cámara (reaparecer). Deja
   la lista de DMA vacía. */
void fondo_nivel_rellenar(u16 camara);

/* Apunta en la lista de DMA las columnas que hacen falta este frame: si la cámara
   entró en una columna nueva, la que entra por la derecha y la que entra por la
   izquierda; y, si se acaba de quitar el parche, sus columnas (las que estén a la
   vista). Una vez por frame, DESPUÉS de tarou_sprite_dibujar (Tarou primero en
   la lista) y después de dma_lista_vaciar. A 2 px por cuadro como mucho, la
   cámara nunca se salta una columna. */
void fondo_nivel_actualizar(u16 camara);

/* PARCHE: las PARCHE_COLUMNAS columnas del nivel desde `col` se toman de otro sitio
   del mapa de la ROM, desde la columna `fuente` (el tronco: el mapa no se puede
   cambiar, así que el script dibuja aparte las columnas CON el tronco). Llamar antes
   de fondo_nivel_cargar. */
void fondo_nivel_parche(u16 col, u16 fuente);

/* Quita el parche (el tronco se rompió): sus columnas vuelven a ser las del nivel
   y se reescriben en el siguiente fondo_nivel_actualizar. */
void fondo_nivel_quitar_parche(void);

#define PARCHE_COLUMNAS 4      /* El tronco: 32 px = 4 columnas de tiles.                */

#endif /* FONDO_NIVEL_H */
