/*---------------------------------------------------------------------------------
    fondo.c — Mini-ROM de la unidad 06 (ESQUELETO del ejercicio:
    completa los TODO 1..4; la solución está en ../../solucion/fondo.c): un fondo de 16
    colores cargado con bgInitTileSet + bgInitMapSet, y cambios en marcha de la
    paleta (CGRAM) y del mapa (VRAM).

    PROPÓSITO
      Cargar las montañas del título (res/montanas.png = res/titulo/fondo_lejos.png
      del juego) en el fondo 1 y una línea de texto en el fondo 0, y después,
      contando frames:
        a los 2 s  cambiar dos colores: el color de fondo (CGRAM 0: el "cielo",
                   porque el índice 0 de las montañas es transparente) y el color
                   7 de la paleta 2 (el cuerpo de las montañas);
        a los 4 s  copiar las 4 filas del mapa donde está la palabra TAROU a las
                   filas 10..13 (el cielo): el mismo dibujo dos veces sin gastar ni
                   un tile más de VRAM;
        a los 6 s  reescribir esa copia en espejo: invertir el orden de las 32
                   celdas de cada fila Y voltear cada tile (bit 14 de la entrada).
      Tal como está, compila y muestra el fondo y el texto; los cambios de los
      TODO no ocurren hasta que se escriben (enunciado en ejercicio_06.md).

    SE CONECTA CON
      Incluye:  <snes.h> (PVSnesLib).
      Lee de la ROM: fuente_til/_til_end/_pal y montanas_til/_til_end/_pal/_map/
                _map_end (data.asm).
      Modelo:   src/sistema/comun.c (fondo16_cargar, fuente_4bpp) y
                src/juego/hud_debug.c (escribir entradas del tilemap a mano).
      Ejemplo del SDK: snes-examples/graphics/Backgrounds/Mode1.

    HARDWARE QUE TOCA
      PPU: modo (setMode → BGMODE $2105), base de tiles y de mapa de cada fondo
      (bgSetGfxPtr/bgSetMapPtr → BG12NBA $210B, BGnSC $2107/$2108), capas
      encendidas (bgSetEnable/Disable → TM $212C), pantalla (setScreenOn → INIDISP).
      VRAM: tiles y mapas por DMA (bgInitTileSet, bgInitMapSet, dmaCopyVram →
      VMAIN/VMADD $2115-$2117, canal 0 de DMA). CGRAM: paletas (bgInitTileSet,
      consoleInitText) y dos colores sueltos (setPaletteColor → CGADD $2121,
      CGDATA $2122).

    VARIABLES (estáticas de este archivo)
      frames  u16              0..65535   frames desde que se encendió la pantalla
      fila    u16[32]          entradas   una fila del mapa preparada en la WRAM
      mapa    const u16 *      ROM        el mapa de las montañas, tal como está en la ROM
      i       u8               0..31      columna dentro de la fila
      f       u8               0..3       fila dentro del bloque que se copia
    → glosario: Tile, Tilemap, Paleta, BG / capas, Mode 1, VRAM, CGRAM, DMA / HDMA,
      Volteo de tile, Consola de texto
---------------------------------------------------------------------------------*/
#include <snes.h>

/*--- Datos en la ROM (data.asm) -------------------------------------------------*/
extern char fuente_til, fuente_til_end, fuente_pal;          /* Fuente 4bpp del juego.  */
extern char montanas_til, montanas_til_end, montanas_pal;    /* Tiles y paleta.         */
extern char montanas_map, montanas_map_end;                  /* Mapa de 32x28 entradas. */

/*--- Reparto de la VRAM: el mismo que src/sistema/comun.h (en palabras) -------------*/
#define VRAM_FONDO_B 0x4000          /* Tiles de las montañas (como la capa lejana).      */
#define VRAM_FUENTE 0x6000           /* Los 96 tiles de la fuente.                        */
#define VRAM_MAPA_TEXTO 0x6800       /* Mapa 32x32 de la capa de texto (fondo 0).         */
#define VRAM_MAPA_B 0x7400           /* Mapa 32x32 de las montañas (fondo 1).             */

/*--- Paletas y momentos -----------------------------------------------------------*/
#define PALETA_MONTANAS 2            /* gfx4snes -e 2: CGRAM 32..47, como en el título.   */
#define COLOR_MONTANA 7              /* El color más usado: el cuerpo azul verdoso.       */
#define FRAMES_COLOR 120             /* 2 s a 60 frames por segundo.                      */
#define FRAMES_COPIA 240             /* 4 s.                                              */
#define FRAMES_ESPEJO 360            /* 6 s.                                              */
#define FILA_LOGO 2                  /* La palabra TAROU ocupa las filas 2..5 del mapa... */
#define FILAS_LOGO 4                 /* ...cuatro filas de 8 px.                          */
#define FILA_CIELO 10                /* Destino de la copia: filas 10..13, cielo vacío.   */

/*--- Estado -------------------------------------------------------------------------*/
static u16 frames;                   /* Frames desde el encendido.                        */
static u16 fila[32];                 /* Una fila del mapa, preparada en la WRAM.          */
static const u16 *mapa;              /* El mapa en la ROM, visto como palabras de 16 bits.*/
static u8 i, f;                      /* Columna y fila de los bucles.                     */

int main(void)
{
    /*--- 1. Texto en el fondo 0: la fuente del juego, paleta 0 (colores 0 y 1) -------*/
    bgSetGfxPtr(0, VRAM_FUENTE);                     /* Tiles del fondo 0 = la fuente.     */
    bgSetMapPtr(0, VRAM_MAPA_TEXTO, SC_32x32);       /* Mapa del fondo 0 = mapa de texto.  */
    consoleSetTextGfxPtr(VRAM_FUENTE);               /* Dónde copia la consola la fuente.  */
    consoleSetTextMapPtr(VRAM_MAPA_TEXTO);           /* Dónde escribe el texto.            */
    consoleSetTextOffset(0);                         /* El tile 0 de la fuente es el 0.    */
    consoleInitText(0, 2 * 2, (u8 *)&fuente_til, (u8 *)&fuente_pal); /* 2 colores x 2 B.  */

    /*--- 2. Las montañas en el fondo 1: tiles + paleta, y después el mapa ------------*/
    bgInitTileSet(1, (u8 *)&montanas_til, (u8 *)&montanas_pal, PALETA_MONTANAS,
                  (u16)(&montanas_til_end - &montanas_til), 16 * 2, BG_16COLORS, VRAM_FONDO_B);
    bgInitMapSet(1, (u8 *)&montanas_map, (u16)(&montanas_map_end - &montanas_map),
                 SC_32x32, VRAM_MAPA_B);             /* 1792 bytes: 32x28 entradas.        */

    /*--- 3. Mode 1, fondos 0 y 1 encendidos, el 2 apagado -----------------------------*/
    setMode(BG_MODE1, 0);                            /* 16 + 16 + 4 colores.               */
    bgSetEnable(0);                                  /* Texto.                             */
    bgSetEnable(1);                                  /* Montañas.                          */
    bgSetDisable(2);                                 /* El tercer fondo no se usa.         */

    consoleDrawText(1, 22, "UNIT 06 - TILES AND PALETTES");
    consoleDrawText(1, 23, "BG TILES: %d", (u16)((&montanas_til_end - &montanas_til) >> 5));
    consoleDrawText(1, 25, "2 S: SKY AND MOUNTAIN COLORS");
    consoleDrawText(1, 26, "4 S: LOGO TILES COPIED");
    consoleDrawText(1, 27, "6 S: COPY MIRRORED (H FLIP)");

    mapa = (const u16 *)&montanas_map;               /* El mapa se lee DIRECTAMENTE de ROM. */
    frames = 0;
    setScreenOn();                                   /* Hasta aquí, pantalla apagada.      */

    while (1)
    {
        WaitForVBlank();                             /* A partir de aquí estamos en el VBlank: */
        frames++;                                    /* se puede tocar CGRAM y VRAM.        */

        if (frames == FRAMES_COLOR)
        {
            /* TODO 1: poner el color 0 de la CGRAM (el color de fondo: se ve donde
               todas las capas son transparentes) en azul noche, RGB5(2, 3, 10).
               Pista: setPaletteColor (video.h). */

            /* TODO 2: poner el color COLOR_MONTANA de la paleta PALETA_MONTANAS en
               naranja, RGB8(232, 120, 40). ¿Qué número de la CGRAM es? */
        }

        if (frames == FRAMES_COPIA)
        {
            /* TODO 3: copiar las FILAS_LOGO filas del mapa que empiezan en FILA_LOGO
               a las filas que empiezan en FILA_CIELO. Leer las entradas de `mapa`
               (la ROM) a `fila` (la WRAM) y copiar cada fila a la VRAM con
               dmaCopyVram. La entrada (columna c, fila r) de un mapa de 32x32 está
               en la palabra r * 32 + c (escríbelo con << 5). */
        }

        if (frames == FRAMES_ESPEJO)
        {
            /* TODO 4: repetir la copia del TODO 3 en espejo horizontal: la columna i
               se toma de la columna 31 - i, y cada entrada lleva el bit de volteo
               horizontal cambiado (BG_TIL_FLIPX, background.h). ¿Por qué ^ y no |? */
        }
    }
    return 0;                                        /* Nunca se llega: C lo exige.        */
}
