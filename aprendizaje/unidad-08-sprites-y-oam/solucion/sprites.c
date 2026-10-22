/*---------------------------------------------------------------------------------
    sprites.c — Mini-ROM de la unidad 08 (SOLUCIÓN del ejercicio): dos sprites
    de 64x64 en la OAM, cada uno con su paleta, uno animado y movido con la
    cruceta.

    PROPÓSITO
      - Bárbara (res/barbara.png, 4 cuadros de 64x64): entrada 0 de la OAM,
        tiles 0..255 (la primera tabla), paleta de sprites 0. Cambia de cuadro
        cada 8 frames y se mueve 1 px por frame con la cruceta.
      - El lobo samurái (res/lobo.png, sus 2 primeros cuadros): entrada 1,
        desde el tile TILE_LOBO (256: el principio de la segunda tabla), paleta
        de sprites 1, mirando hacia Bárbara (volteo horizontal).
      - Experimentos del ejercicio, sin tocar el código: compilar con
        CFLAGS="-DTILE_LOBO=192" (el lobo cruza de la tabla 0 a la 1 y se
        corrompe) o con CFLAGS="-DLOBO_Y=240" (un sprite de 64 de alto en
        y = 240 asoma por ARRIBA).
      Copia este archivo sobre rom/src/sprites.c para compilar la solución.

    SE CONECTA CON
      Incluye:  <snes.h> (PVSnesLib).
      Lee de la ROM: fuente_til/_pal, barbara_til/_til_end/_pal, lobo_til/_pal (data.asm).
      Modelo:   src/pantallas/personajes.c (tile_del_bloque64, esconder, el bloque
                LÍMITE de su cabecera).
      Ejemplo del SDK: snes-examples/graphics/Sprites/SimpleSprite.

    HARDWARE QUE TOCA
      OAM: oamInit, oamSet, oamSetEx, oamSetXY (escriben un búfer en la WRAM que
      la NMI del SDK copia a la OAM en cada VBlank: OAMADD/OAMDATA $2102-$2104).
      PPU: OBSEL $2101 (oamInitGfxSet: base de tiles de sprites y tamaños
      32x32 / 64x64). VRAM y CGRAM por DMA (oamInitGfxSet, dmaCopyVram,
      dmaCopyCGram). Mando: padsCurrent (lo lee la NMI).

    VARIABLES (estáticas de este archivo)
      x, y          u16  píxeles de pantalla   esquina superior izquierda de Bárbara
      cuadro        u8   0..3                  cuadro de Bárbara
      cuadro_lobo   u8   0..1                  cuadro del lobo
      espera        u8   0..FRAMES_POR_CUADRO  frames desde el último cambio de cuadro
      teclas        u16  bits                  botones mantenidos
    → glosario: Sprite, OAM, Paleta, VRAM, CGRAM
---------------------------------------------------------------------------------*/
#include <snes.h>

/*--- Datos en la ROM (data.asm) -------------------------------------------------*/
extern char fuente_til, fuente_pal;
extern char barbara_til, barbara_til_end, barbara_pal;
extern char lobo_til, lobo_pal;

/*--- VRAM (palabras) y el texto ---------------------------------------------------*/
#define VRAM_SPRITES 0x0000          /* Tiles de sprites: 512 tiles = 0x0000..0x1FFF.     */
#define VRAM_FUENTE 0x6000           /* Fuente del texto (fondo 0).                       */
#define VRAM_MAPA_TEXTO 0x6800       /* Mapa del texto.                                   */

/*--- Los dos sprites --------------------------------------------------------------*/
#define OAM_BARBARA 0                /* Entrada 0 de la OAM → id 0 * 4.                   */
#define OAM_LOBO 1                   /* Entrada 1 → id 4.                                 */
#define PALETA_BARBARA 0             /* Paleta de sprites 0 = CGRAM 128..143.             */
#define PALETA_LOBO 1                /* Paleta de sprites 1 = CGRAM 144..159.             */
#define PRIORIDAD 3                  /* Delante de todos los fondos.                      */
#ifndef TILE_LOBO
#define TILE_LOBO 256                /* Primer tile del lobo: la segunda tabla entera.    */
#endif
#ifndef LOBO_Y
#define LOBO_Y 112                   /* Fila de pantalla del lobo.                        */
#endif
#define LOBO_X 32                    /* Columna de pantalla del lobo.                     */
#define BYTES_LOBO 4096              /* Sus 2 primeros cuadros: la primera fila de 128 px
                                        de la tira empaquetada = 128 tiles x 32 bytes.    */
#define CUADROS_BARBARA 4
#define CUADROS_LOBO 2
#define FRAMES_POR_CUADRO 8          /* 60 / 8 = 7,5 cuadros por segundo.                 */

/*--- Estado -------------------------------------------------------------------------*/
static u16 x, y, teclas;
static u8 cuadro, cuadro_lobo, espera;

/*--- tile_del_bloque64: bloque de 64x64 número n de una tira de gfx4snes -s 64 ------
    gfx4snes coloca los bloques de 64x64 de dos en dos en filas de 128 px (16 tiles
    de ancho). El bloque n empieza en la fila n / 2 de bloques (cada una son 8 filas
    de tiles x 16 = 128 tiles) y en la columna n % 2 (8 tiles): (n / 2) * 128 +
    (n % 2) * 8. Con desplazamientos y máscaras, sin dividir: la misma cuenta que
    tile_del_bloque64 de src/pantallas/personajes.c. */
static u16 tile_del_bloque64(u16 n)
{
    return (u16)(((n >> 1) << 7) | ((n & 1) << 3));
}

/*--- esconder: una entrada de la OAM fuera de la pantalla de verdad ------------------
    Como src/pantallas/personajes.c: tamaño pequeño y (256, 224). x = 256 tiene el
    bit 8 a 1 (fuera por la derecha); y = 224 es la primera fila bajo la pantalla y
    un sprite de 32 de alto ahí no llega a dar la vuelta (224 + 32 = 256). */
static void esconder(u8 entrada)
{
    oamSetEx((u16)entrada * 4, OBJ_SMALL, OBJ_HIDE);
    oamSetXY((u16)entrada * 4, 256, 224);
}

/*--- dibujar: las dos entradas de la OAM con su cuadro y su posición -------------*/
static void dibujar(void)
{
    /* oamSet(id, x, y, prioridad, volteoH, volteoV, primer tile, paleta) */
    oamSet(OAM_BARBARA * 4, x, y, PRIORIDAD, 0, 0, tile_del_bloque64(cuadro), PALETA_BARBARA);
    oamSetEx(OAM_BARBARA * 4, OBJ_LARGE, OBJ_SHOW);  /* Grande = 64x64 con OBJ_SIZE32_L64. */

    /* TODO 2 (resuelto): el lobo, volteado para mirar a Bárbara, con su paleta. */
    oamSet(OAM_LOBO * 4, LOBO_X, LOBO_Y, PRIORIDAD, 1, 0,
           TILE_LOBO + tile_del_bloque64(cuadro_lobo), PALETA_LOBO);
    oamSetEx(OAM_LOBO * 4, OBJ_LARGE, OBJ_SHOW);
}

int main(void)
{
    /*--- 1. Texto en el fondo 0 ------------------------------------------------------*/
    bgSetGfxPtr(0, VRAM_FUENTE);
    bgSetMapPtr(0, VRAM_MAPA_TEXTO, SC_32x32);
    consoleSetTextGfxPtr(VRAM_FUENTE);
    consoleSetTextMapPtr(VRAM_MAPA_TEXTO);
    consoleSetTextOffset(0);
    consoleInitText(0, 2 * 2, (u8 *)&fuente_til, (u8 *)&fuente_pal);
    setPaletteColor(0, RGB5(6, 10, 14));             /* Color de fondo: gris azulado.      */

    /*--- 2. Sprites: OAM limpia, tiles de Bárbara a 0x0000 y su paleta a la 0 --------
        OBJ_SIZE32_L64: "pequeño" = 32x32, "grande" = 64x64. Los 8 KB de Bárbara son
        los tiles 0..255: la primera tabla entera. */
    oamInit();
    oamInitGfxSet((u8 *)&barbara_til, (u16)(&barbara_til_end - &barbara_til),
                  (u8 *)&barbara_pal, 16 * 2, PALETA_BARBARA, VRAM_SPRITES, OBJ_SIZE32_L64);

    /* TODO 1 (resuelto): los 2 primeros cuadros del lobo desde el tile TILE_LOBO
       (16 palabras por tile de 4bpp) y su paleta a la paleta de sprites 1. */
    dmaCopyVram((u8 *)&lobo_til, VRAM_SPRITES + TILE_LOBO * 16, BYTES_LOBO);
    dmaCopyCGram((u8 *)&lobo_pal, 128 + 16 * PALETA_LOBO, 16 * 2);

    esconder(OAM_LOBO);                              /* Hasta que se dibuje.               */

    /*--- 3. Mode 1, solo el texto de fondos; los sprites no dependen de bgSetEnable --*/
    setMode(BG_MODE1, 0);
    bgSetEnable(0);
    bgSetDisable(1);
    bgSetDisable(2);
    consoleDrawText(1, 1, "UNIT 08 - SPRITES AND OAM");
    consoleDrawText(1, 2, "DPAD: MOVE THE SAMURAI");
    consoleDrawText(1, 3, "WOLF: TILE %d  Y %d", TILE_LOBO, LOBO_Y);

    x = 160;
    y = 112;
    cuadro = 0;
    cuadro_lobo = 0;
    espera = 0;
    dibujar();
    setScreenOn();

    while (1)
    {
        /*--- Cruceta: 1 px por frame, dentro de la pantalla ------------------------*/
        teclas = padsCurrent(0);
        if ((teclas & KEY_RIGHT) && x < 256 - 64)
        {
            x++;
        }
        if ((teclas & KEY_LEFT) && x > 0)
        {
            x--;
        }
        if ((teclas & KEY_DOWN) && y < 224 - 64)
        {
            y++;
        }
        if ((teclas & KEY_UP) && y > 0)
        {
            y--;
        }

        /*--- Animación: un cuadro cada FRAMES_POR_CUADRO frames, en bucle -----------*/
        espera++;
        if (espera >= FRAMES_POR_CUADRO)
        {
            espera = 0;
            cuadro++;
            if (cuadro >= CUADROS_BARBARA)
            {
                cuadro = 0;
            }
            cuadro_lobo ^= 1;                        /* 0, 1, 0, 1...                      */
        }

        dibujar();                                   /* Al búfer de la OAM...              */
        WaitForVBlank();                             /* ...que la NMI copia a la PPU.      */
    }
    return 0;
}
