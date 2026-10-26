/*---------------------------------------------------------------------------------
    demo_09_copias.c — Demo de la unidad 09: el plan de copias de un cuadro de
    Tarou, calculado por el código REAL del juego (src/juego/metasprite.c).

    PROPÓSITO
      Enseña, sin SNES, las cuentas del motor de sprites propio del juego:
        1. qué piezas tiene un cuadro de 96x64 (tabla de gfx4snes -T) y cómo
           queda su espejo (dx -> 64 - dx, bit 0x40);
        2. qué trozos de la tira hay que copiar a la VRAM para subirlo
           (metasprite_copias): origen en la tira, destino en la VRAM, bytes;
        3. cómo se reparten esas copias en dos VBlanks de 1536 bytes como
           mucho (lo que hace apuntar_copias en src/juego/tarou_sprite.c);
        4. cuántas líneas del VBlank cuesta eso a ~170 bytes por línea
           (redondeado al entero más cercano; solo los datos, sin la
           preparación de cada DMA).
      Las tablas son copias exactas de cuadros reales (res/personajes/tarou/
      tarou_correr_meta.inc y tarou_dormido_meta.inc, generadas con make).

    SE CONECTA CON
      Incluye:  <stdio.h>, juego/metasprite.h (el del juego, con -I../../src).
      Enlaza:   ../../src/juego/metasprite.c (lógica pura: compila en el PC).
      Compilar (desde esta carpeta):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_09.exe demo_09_copias.c ../../src/juego/metasprite.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      CORRER0, CORRER1, DORMIDO0  t_metasprite[]   tres cuadros reales
      copias                      CopiaBloque[16]  el plan de copias que se imprime
      espejo                      t_metasprite[7]  el cuadro espejado

    → conceptos (README de la unidad 09): metasprite, plan de copias, doble búfer,
      presupuesto del VBlank
---------------------------------------------------------------------------------*/
#include <stdio.h>                         /* printf.                                   */
#include "juego/metasprite.h"              /* t_metasprite, CopiaBloque y las funciones. */

#define PROPS 0x30                         /* OBJ_PAL(0) | OBJ_PRIO(3): lo que pone -P 3. */
#define BYTES_POR_VBLANK 1536              /* El tope de src/juego/tarou_sprite.c.      */
#define BYTES_POR_LINEA 170                /* DMA: 1364 ciclos maestros por línea / 8.  */
#define BASE_ROM 0x8000                    /* Dirección (inventada) de la tira en su banco.*/

/* Cuadro 0 de correr (8 cuadros: la tira mide 24 bloques de ancho, así que la fila de
   abajo empieza en el bloque 24). Copiado de tarou_correr_meta.inc. */
static const t_metasprite CORRER0[] = {
    {0, 0, 0, PROPS, 0}, {32, 0, 1, PROPS, 0}, {64, 0, 2, PROPS, 0},
    {0, 32, 24, PROPS, 0}, {32, 32, 25, PROPS, 0}, {64, 32, 26, PROPS, 0},
    {METASPRITE_FIN, 0, 0, 0, 0}};
/* Cuadro 1 de correr: bloques 3..5 y 27..29 (el 3 y el 27 cierran una fila de 4). */
static const t_metasprite CORRER1[] = {
    {0, 0, 3, PROPS, 0}, {32, 0, 4, PROPS, 0}, {64, 0, 5, PROPS, 0},
    {0, 32, 27, PROPS, 0}, {32, 32, 28, PROPS, 0}, {64, 32, 29, PROPS, 0},
    {METASPRITE_FIN, 0, 0, 0, 0}};
/* Cuadro 0 de dormido (9 cuadros: 27 bloques de ancho): solo 4 piezas, porque
   gfx4snes -T omite las de arriba a la izquierda y a la derecha (transparentes). */
static const t_metasprite DORMIDO0[] = {
    {32, 0, 1, PROPS, 0},
    {0, 32, 27, PROPS, 0}, {32, 32, 28, PROPS, 0}, {64, 32, 29, PROPS, 0},
    {METASPRITE_FIN, 0, 0, 0, 0}};

static CopiaBloque copias[METASPRITE_COPIAS_MAX];   /* El plan que se imprime.         */
static t_metasprite espejo[METASPRITE_PIEZAS_MAX + 1]; /* El cuadro mirando a la izq.  */

/* Imprime las piezas de una tabla (hasta el terminador). */
static void piezas(const char *nombre, const t_metasprite *t)
{
    unsigned n = metasprite_piezas(t);     /* 0..6: para en dx = 0xFF80.              */
    printf("%s: %u piezas\n", nombre, n);
    for (unsigned i = 0; i < n; i++)       /* Una línea por pieza.                    */
    {
        printf("  pieza %u: dx=%2u dy=%2u bloque=%2u atributo=0x%02X\n",
               i, t[i].dx, t[i].dy, t[i].dtile, t[i].props);
    }
}

/* Calcula e imprime el plan de copias y su reparto en VBlanks. */
static void plan(const char *nombre, const t_metasprite *t)
{
    unsigned n = metasprite_copias(t, BASE_ROM, 0x0000, copias);  /* Celda 0 de la VRAM. */
    unsigned total = 0, vblank = 1, en_este = 0;
    printf("%s: %u copias\n", nombre, n);
    for (unsigned i = 0; i < n; i++)
    {
        /* La misma regla que apuntar_copias(): si esta copia no cabe en el tope de
           este VBlank (y ya hay algo apuntado), pasa al VBlank siguiente. */
        if (en_este != 0 && en_este + copias[i].bytes > BYTES_POR_VBLANK)
        {
            printf("    -- VBlank %u: %u bytes, unas %u lineas de DMA\n",
                   vblank, en_este, (en_este + BYTES_POR_LINEA / 2) / BYTES_POR_LINEA);
            vblank++;                      /* Lo que queda, al VBlank siguiente.        */
            en_este = 0;
        }
        printf("  copia %2u: tira+%5u -> VRAM 0x%04X  %3u bytes\n",
               i, (unsigned)(copias[i].origen - BASE_ROM), copias[i].vram, copias[i].bytes);
        en_este += copias[i].bytes;        /* Lo que lleva este VBlank.                 */
        total += copias[i].bytes;          /* Lo que lleva el cuadro.                   */
    }
    printf("    -- VBlank %u: %u bytes, unas %u lineas de DMA\n",
           vblank, en_este, (en_este + BYTES_POR_LINEA / 2) / BYTES_POR_LINEA);
    printf("  total %u bytes = %u lineas de DMA (redondeado) si fuera de una vez (de las 37 del VBlank)\n\n",
           total, (total + BYTES_POR_LINEA / 2) / BYTES_POR_LINEA);
}

int main(void)
{
    /* 1. Las piezas y el espejo. */
    printf("== 1. Piezas de un cuadro y su espejo ==\n");
    piezas("correr, cuadro 0", CORRER0);
    metasprite_espejar(CORRER0, espejo);   /* dx -> 64 - dx, atributo | 0x40.          */
    piezas("correr, cuadro 0 ESPEJADO", espejo);
    piezas("dormido, cuadro 0", DORMIDO0);
    printf("\n");

    /* 2-4. El plan de copias y su reparto en dos VBlanks. */
    printf("== 2. Plan de copias (metasprite_copias) y reparto en VBlanks ==\n");
    plan("correr, cuadro 0", CORRER0);     /* Bloques seguidos: 8 copias de 384.       */
    plan("correr, cuadro 1", CORRER1);     /* Bloques partidos: 16 copias.             */
    plan("dormido, cuadro 0", DORMIDO0);   /* 4 piezas: menos bytes.                   */
    return 0;
}
