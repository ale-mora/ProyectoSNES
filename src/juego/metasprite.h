/*---------------------------------------------------------------------------------
    metasprite.h — Recorrer y espejar tablas metasprite de gfx4snes -T (lógica pura).

    PROPÓSITO
      Una tabla metasprite describe un cuadro como una lista de piezas (dx, dy,
      bloque, atributos) que termina en una entrada con dx = metasprite_end.
      OJO: gfx4snes -T OMITE las piezas totalmente transparentes, así que un
      cuadro de 96x64 (3x2 piezas) puede tener de 0 a 6 piezas (un Tarou
      dormido solo ocupa la fila de abajo): nadie debe suponer "siempre 6".
      Este módulo da las dos operaciones que el juego necesita sin tocar el
      hardware: contar las piezas y construir la copia espejada (mirar a la
      izquierda) con el terminador en su sitio. Se compila también en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h (en la ROM trae <snes.h>, que define t_metasprite;
                en el PC, con PRUEBAS_PC, la struct idéntica se define aquí).
      Lo usan:  juego/tarou_sprite.c (espejo del cuadro de Tarou),
                pantallas/personajes.c (dibujo estático de intro y título),
                tests/prueba_metasprite.c.

    HARDWARE QUE TOCA
      ninguno. Las reglas vienen de la OAM: cada pieza es un sprite de 32x32 y
      el bit 0x40 del atributo ("vhoopppc") es el espejo horizontal.

    VARIABLES (constantes)
      METASPRITE_FIN         u16  0xFF80  dx del terminador: metasprite_end (-128) visto como u16
      METASPRITE_PIEZAS_MAX  u8   6       piezas de 32x32 que caben en un cuadro de 96x64
      METASPRITE_HFLIP       u8   0x40    bit h del atributo: espejo horizontal
      METASPRITE_ESPEJO_DX   u8   64      96 - 32: la pieza que estaba en dx pasa a 64 - dx

    → glosario: Metasprite, Sprite dinámico, OAM, Dirección
---------------------------------------------------------------------------------*/
#ifndef METASPRITE_H
#define METASPRITE_H

#include "sistema/tipos.h"

#ifdef PRUEBAS_PC
/* La misma struct que <snes.h> (snes/sprite.h) en la SNES: 8 bytes, alineada a 16 bits. */
typedef struct
{
    u16 dx, dy;      /* Desplazamiento de la pieza dentro del cuadro, en píxeles.        */
    u16 dtile;       /* Número de bloque de 32x32 dentro de la tira (lo numera gfx4snes). */
    u8 props;        /* Atributos: paleta, prioridad y espejos.                           */
    u8 unused;       /* Relleno para que la struct ocupe 8 bytes.                         */
} t_metasprite;
#endif

#define METASPRITE_FIN 0xFF80
#define METASPRITE_PIEZAS_MAX 6     /* Celda de 96x64 = 3 columnas x 2 filas de 32x32.     */
#define METASPRITE_HFLIP 0x40
#define METASPRITE_ESPEJO_DX 64     /* Ancho de la celda menos una pieza: 96 - 32.         */

/* Cuántas piezas tiene `tabla` antes del terminador: 0..6 (6 también si la tabla
   no termina, para no leer más allá de un cuadro). */
u8 metasprite_piezas(const t_metasprite *tabla);

/* Copia `tabla` en `espejo` mirando a la izquierda: dx pasa a 64 - dx, el atributo
   gana METASPRITE_HFLIP y el terminador queda justo detrás de la última pieza.
   `espejo` debe tener sitio para METASPRITE_PIEZAS_MAX + 1 entradas. Devuelve
   cuántas piezas copió. */
u8 metasprite_espejar(const t_metasprite *tabla, t_metasprite *espejo);

/*--- Subir un cuadro a la VRAM: el plan de copias -----------------------------------
    La tira (.pic de gfx4snes -s 32) guarda los bloques de 32x32 de 4 en 4 por fila
    de 128 px: el bloque n está en la fila n/4, columna n%4; cada fila de bloques
    son 4 filas de tiles de 512 bytes (16 tiles). En la VRAM los sprites tienen la
    misma anchura de 16 tiles, así que una fila de tiles de un bloque (128 bytes)
    se copia de un tirón, y la de varios bloques seguidos en la misma fila de la
    tira, también. El cuadro vivo va en la VRAM como una celda de 3x2 bloques:
    el bloque de la columna c y la fila f de la celda ocupa el tile f*64 + c*4. */
#define METASPRITE_COPIAS_MAX 16    /* 2 filas de celda x 2 tramos x 4 filas de tiles.      */
#define METASPRITE_BYTES_FILA_BLOQUE 128   /* Una fila de tiles de un bloque: 4 tiles x 32 B. */

typedef struct
{
    u16 origen;      /* Desplazamiento desde el comienzo de la tira (bytes).             */
    u16 vram;        /* Dirección de VRAM, en palabras.                                   */
    u16 bytes;       /* Cuántos bytes seguidos.                                           */
    u16 libre;       /* Relleno: con 8 bytes, copias[i] es un desplazamiento (i << 3);   */
} CopiaBloque;       /* con 6, tcc816 multiplicaba con tcc__mul en cada acceso.          */

/* Rellena `salida` con las copias que suben las piezas de `tabla` (mirando a la
   derecha) a la celda de la VRAM que empieza en `base_vram` (en palabras), tomando
   los bloques desde `base_rom` (dirección de la tira dentro de su banco). Las
   piezas seguidas de una misma fila de la celda con bloques seguidos en una misma
   fila de la tira van en un solo tramo. Devuelve cuántas copias hay (0..16). */
u8 metasprite_copias(const t_metasprite *tabla, u16 base_rom, u16 base_vram, CopiaBloque *salida);

#endif /* METASPRITE_H */
