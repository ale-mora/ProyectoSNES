/*---------------------------------------------------------------------------------
    subida.h — Plan de copias de un cuadro en piezas de 16x16 y su reparto por VBlanks.

    PROPÓSITO
      Un cuadro en formato .p16 (scripts/piezas16.py) tiene sus n piezas en el
      orden de subida: los pares de filas completos (1 KB: 512 B la fila de
      arriba y 512 la de abajo) y, si el último par va a medias con q piezas,
      su fila de arriba (q·64 B) y la de abajo (q·64 B). En la VRAM la fila de
      abajo está 16 tiles = 256 palabras más allá. Así el plan de copias sale
      del número de piezas con desplazamientos, sin tablas ni multiplicaciones:
      la prueba de carga midió que planificar el cuadro de Tarou con
      metasprite_copias costaba hasta 120 líneas (docs/diseno-beat-em-up.md §9.1).
      Las copias son de 512 B como mucho, para que un tope de 1,5 KB por VBlank
      se llene entero (un cuadro de 3 KB, en 2 VBlanks).

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  pantallas/prueba_carga.c y tests/prueba_subida.c.

    HARDWARE QUE TOCA
      ninguno (lógica pura, probada en el PC): el que llama pasa las copias a la
      lista de DMA (juego/dma_lista.h).

    VARIABLES: ninguna (el estado va en cada struct Subida).
    → glosario: Pieza de 16×16, Par de filas, DMA / HDMA
---------------------------------------------------------------------------------*/
#ifndef SUBIDA_H
#define SUBIDA_H

#include "sistema/tipos.h"

#define SUBIDA_COPIAS_MAX 6          /* ≤ 24 piezas: 3 pares completos o 2 y medio.      */
#define SUBIDA_PIEZAS_MAX 24         /* 3 pares de filas: lo que cabe en un hueco.        */
#define SUBIDA_FILA_BYTES 512        /* Una fila de 16 tiles.                             */
#define SUBIDA_FILA_PALABRAS 256     /* La misma fila en palabras de la VRAM.             */

/* Una copia: 8 bytes (potencia de 2: plan[i] es un desplazamiento, sin tcc__mul). */
typedef struct
{
    u16 origen;                      /* Dirección del origen dentro de su banco.          */
    u16 vram;                        /* Dirección de la VRAM, en palabras.                */
    u16 bytes;                       /* Cuántos bytes.                                    */
    u16 libre;                       /* Relleno.                                          */
} Copia;

/* La subida de un cuadro: 64 bytes (potencia de 2, para tener varias en un array). */
typedef struct
{
    Copia plan[SUBIDA_COPIAS_MAX];   /* Las copias, en orden.                             */
    u8 n;                            /* Cuántas hay.                                      */
    u8 hechas;                       /* Cuántas se han tomado ya.                         */
    u8 banco;                        /* Banco de la ROM del origen.                       */
    u8 libre[13];                    /* Relleno hasta 64 bytes (48 + 3 + 13).             */
} Subida;

/* Prepara la subida de un cuadro de n_piezas cuyos tiles empiezan en `origen` (dentro
   del banco `banco`) hacia el bloque de la VRAM que empieza en `vram` (palabras). */
void subida_empezar(Subida *s, u8 n_piezas, u16 origen, u16 vram, u8 banco);

/* Las siguientes copias que caben en `tope` bytes: *desde apunta a la primera y
   *cuantas dice cuántas (0 si ya no queda nada). Si ninguna cabe, da la primera igual
   (para avanzar siempre). Las marca como hechas. Devuelve los bytes tomados. */
u16 subida_tomar(Subida *s, u16 tope, const Copia **desde, u8 *cuantas);

/* Bytes que faltan por tomar. */
u16 subida_restante(const Subida *s);

/* 1 si ya se tomaron todas las copias. */
u8 subida_terminada(const Subida *s);

/* Tile de la pieza k dentro del bloque del cuadro en la VRAM: (k/8)·32 + (k%8)·2. */
u16 subida_tile_pieza(u8 k);

#endif /* SUBIDA_H */
