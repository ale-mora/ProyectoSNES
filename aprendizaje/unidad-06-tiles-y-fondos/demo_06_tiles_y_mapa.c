/*---------------------------------------------------------------------------------
    demo_06_tiles_y_mapa.c — Demo de PC de la unidad 06: cómo se guardan por dentro
    un tile, una entrada de tilemap y un color de la SNES.

    PROPÓSITO
      Antes de cargar nada en la VRAM, ver los bytes. Cuatro pasos numerados en
      la salida:
        1. cuánto ocupa un tile según sus bits por píxel (2, 4, 8 bpp);
        2. un tile de 4bpp dibujado con índices de color, codificado al formato
           planar de la SNES (32 bytes) y decodificado otra vez;
        3. la entrada de tilemap de 16 bits: tile, paleta, prioridad y volteos,
           armada con las mismas macros que background.h (BG_TIL_*) y la que usa
           el HUD del juego para una letra (src/juego/hud_debug.c, TILE(c));
        4. un color de 15 bits con RGB8 y RGB5 (video.h).

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t, uint16_t).
      Lo usan:  nadie: programa independiente de PC.
      Modelo:   tools/pvsneslib/pvsneslib/include/snes/background.h (BG_TIL_*),
                video.h (RGB8, RGB5), src/juego/hud_debug.c (TILE).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_06.exe demo_06_tiles_y_mapa.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC; imita el formato de la VRAM y la CGRAM).

    VARIABLES
      dibujo    const char[8][9]  '0'..'3'     el tile, un carácter por píxel
      tile      u8[32]            bytes        el tile codificado en 4bpp planar
      entrada   u16               0..0xFFFF    una celda del tilemap
    → conceptos (README de la unidad 06): tile, bpp, planar, tilemap, paleta
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* printf.                                           */
#include <stdint.h>                  /* Enteros de tamaño fijo.                           */

typedef uint8_t u8;                  /* Los nombres del juego (src/sistema/tipos.h).      */
typedef uint16_t u16;

/*--- Las macros de PVSnesLib, copiadas tal cual de background.h y video.h -----------*/
#define BG_TIL_PRIO (1 << 13)        /* Bit 13: prioridad (delante de las de prioridad 0).*/
#define BG_TIL_PAL(n) ((n) << 10)    /* Bits 10-12: paleta 0..7.                          */
#define BG_TIL_NUM(n) ((n) << 0)     /* Bits 0-9: número de tile 0..1023.                 */
#define BG_TIL_FLIPX (1 << 14)       /* Bit 14: volteo horizontal.                        */
#define BG_TIL_FLIPY (1 << 15)       /* Bit 15: volteo vertical.                          */
#define RGB8(r, g, b) (((r) >> 3) | (((g) >> 3) << 5) | (((b) >> 3) << 10))
#define RGB5(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

/* La macro del HUD del juego (src/juego/hud_debug.c:85): letra ASCII → entrada. */
#define TILE(c) ((u16)(((c) - 32) | 0x2000))

/* Un tile de 8x8 con 4 colores (índices 0..3): una flecha. El 0 es transparente. */
static const char dibujo[8][9] = {
    "00011000",
    "00122100",
    "01233210",
    "12333321",
    "00133100",
    "00133100",
    "00133100",
    "00111100",
};

/* Codifica un tile de 8x8 (índices 0..15) al formato 4bpp de la SNES: por cada fila,
   los bytes de los planos 0 y 1 van juntos en los bytes 0..15 (fila*2, fila*2+1) y
   los de los planos 2 y 3 en los bytes 16..31. El bit 7 de cada byte es el píxel de
   la izquierda. */
static void codificar_4bpp(const char d[8][9], u8 *tile)
{
    for (int i = 0; i < 32; i++)
    {
        tile[i] = 0;                                 /* Empezar con todos los bits a 0.   */
    }
    for (int fila = 0; fila < 8; fila++)
    {
        for (int x = 0; x < 8; x++)
        {
            int indice = d[fila][x] - '0';           /* El color de este píxel (0..15).   */
            u8 bit = (u8)(0x80 >> x);                /* Su bit dentro del byte de la fila.*/
            if (indice & 1) tile[fila * 2] |= bit;           /* Plano 0. */
            if (indice & 2) tile[fila * 2 + 1] |= bit;       /* Plano 1. */
            if (indice & 4) tile[16 + fila * 2] |= bit;      /* Plano 2. */
            if (indice & 8) tile[16 + fila * 2 + 1] |= bit;  /* Plano 3. */
        }
    }
}

/* Lo contrario: lee un píxel del tile juntando un bit de cada plano. */
static int pixel_4bpp(const u8 *tile, int fila, int x)
{
    u8 bit = (u8)(0x80 >> x);
    return ((tile[fila * 2] & bit) ? 1 : 0)          /* Plano 0 = bit 0 del índice.   */
         | ((tile[fila * 2 + 1] & bit) ? 2 : 0)      /* Plano 1 = bit 1.              */
         | ((tile[16 + fila * 2] & bit) ? 4 : 0)     /* Plano 2 = bit 2.              */
         | ((tile[16 + fila * 2 + 1] & bit) ? 8 : 0);/* Plano 3 = bit 3.              */
}

/* Imprime una entrada de tilemap separada en sus campos. */
static void decodificar(const char *nombre, u16 e)
{
    printf("  %-24s 0x%04X = tile %4u  paleta %u  prioridad %u  voltH %u  voltV %u\n",
           nombre, e, e & 0x3FF, (e >> 10) & 7, (e >> 13) & 1, (e >> 14) & 1, (e >> 15) & 1);
}

int main(void)
{
    u8 tile[32];

    /*--- 1. Tamaño de un tile según los bits por píxel -------------------------------*/
    printf("1. Un tile de 8x8 son 64 pixeles:\n");
    printf("   2bpp (4 colores):   64 * 2 / 8 = %d bytes\n", 64 * 2 / 8);
    printf("   4bpp (16 colores):  64 * 4 / 8 = %d bytes\n", 64 * 4 / 8);
    printf("   8bpp (256 colores): 64 * 8 / 8 = %d bytes\n", 64 * 8 / 8);
    printf("   Pantalla 256x224 = %d celdas; 4bpp sin repetir ninguna: %d bytes\n",
           32 * 28, 32 * 28 * 32);

    /*--- 2. Un tile de 4bpp, codificado y decodificado -------------------------------*/
    codificar_4bpp(dibujo, tile);
    printf("2. Bytes del tile (planos 0-1 | planos 2-3):\n   ");
    for (int i = 0; i < 32; i++)
    {
        printf("%02X%s", tile[i], i == 15 ? " | " : " ");
    }
    printf("\n   Decodificado otra vez (indice de color por pixel, . = 0):\n");
    for (int fila = 0; fila < 8; fila++)
    {
        printf("   ");
        for (int x = 0; x < 8; x++)
        {
            int p = pixel_4bpp(tile, fila, x);
            printf("%c", p == 0 ? '.' : '0' + p);
        }
        printf("\n");
    }

    /*--- 3. Entradas de tilemap ------------------------------------------------------*/
    printf("3. Entradas de tilemap (16 bits):\n");
    decodificar("tile 17, paleta 2", (u16)(BG_TIL_NUM(17) | BG_TIL_PAL(2)));
    decodificar("el mismo, voltH", (u16)(BG_TIL_NUM(17) | BG_TIL_PAL(2) | BG_TIL_FLIPX));
    decodificar("el mismo, voltH y voltV", (u16)(BG_TIL_NUM(17) | BG_TIL_PAL(2) | BG_TIL_FLIPX | BG_TIL_FLIPY));
    decodificar("HUD: TILE('A')", TILE('A'));
    decodificar("HUD: TILE('0')", TILE('0'));
    decodificar("montanas: 0x0800", 0x0800);

    /*--- 4. Colores de 15 bits -------------------------------------------------------*/
    printf("4. Colores (0bbbbbgggggrrrrr):\n");
    printf("   RGB8(232,120,40) = 0x%04X  (r %u, g %u, b %u de 0..31)\n", RGB8(232, 120, 40),
           RGB8(232, 120, 40) & 31, (RGB8(232, 120, 40) >> 5) & 31, (RGB8(232, 120, 40) >> 10) & 31);
    printf("   RGB5(2,3,10)     = 0x%04X\n", RGB5(2, 3, 10));
    printf("   blanco RGB5(31,31,31) = 0x%04X; una paleta de 16 = %d bytes\n",
           RGB5(31, 31, 31), 16 * 2);
    return 0;
}
