/*---------------------------------------------------------------------------------
    personajes.c — Sprites de Tarou y Bárbara para la secuencia de arranque.

    PROPÓSITO
      Cargar en VRAM los cuadros de reposo de los dos personajes, mostrarlos,
      animarlos y esconderlos. Tarou usa de momento el lobo samurái del pack
      (res/personajes/lobo_samurai.png), que en el juego será un enemigo; el
      arte real de Tarou (Pixellab) entra en la Tarea 5.1 del plan.

      Cómo están guardados los gráficos: cada personaje es una tira de cuadros
      de 64x64 convertida por gfx4snes con bloques de 64 (-s 64). gfx4snes
      empaqueta los bloques en filas de 128 píxeles de ancho (16 tiles), dos
      bloques por fila, así que el cuadro n empieza en el tile
      (n / 2) * 128 + (n % 2) * 8 (misma cuenta que el ejemplo AnimatedSprite
      del SDK). LÍMITE DURO: la OAM direcciona como máximo 512 tiles de sprites;
      cada bloque de 64x64 son 64 tiles, así que caben 8 bloques: Tarou usa los
      bloques 0..3 y Bárbara los 4..7, uno detrás de otro desde VRAM 0x0000.
      Con más cuadros los tiles "dan la vuelta" y se ve otro sprite.

    SE CONECTA CON
      Incluye:  pantallas/personajes.h.
      Lee de la ROM: lobo_samurai_til/_til_end/_pal y barbara_samurai_til/
                _til_end/_pal (data.asm).
      Lo usan:  pantallas/pantalla_intro.c (escena 3), pantallas/pantalla_titulo.c,
                sistema/main.c (personajes_ocultar al arrancar).

    HARDWARE QUE TOCA
      OAM: oamInit, oamSet, oamSetEx, oamSetXY (la librería la copia a la PPU en
      cada VBlank). VRAM: oamInitGfxSet y dmaCopyVram (tiles). CGRAM:
      dmaCopyCGram (paletas de sprites 0 y 1, colores 128..159).

    VARIABLES (globales y estáticas de este archivo)
      x_tarou, y_tarou      u16  píxeles de pantalla  esquina superior izquierda de la celda de Tarou
      x_barbara, y_barbara  u16  píxeles de pantalla  ídem Bárbara
      cuadro_tarou          u8   0..CUADROS_TAROU-1   cuadro de animación de Tarou
      cuadro_barbara        u8   0..CUADROS_BARBARA-1 cuadro de animación de Bárbara
      frames_cuadro         u8   0..FRAMES_POR_CUADRO frames desde el último cambio de cuadro
      visibles              u8   0 / 1                1 = se están mostrando
      tile                  u16  0..511               número de tile calculado para la OAM

    → glosario: Sprite, OAM, VRAM, Paleta, DMA / HDMA
---------------------------------------------------------------------------------*/
#include "pantallas/personajes.h"

/*--- Datos en ROM (pegados por data.asm) ----------------------------------------*/
extern char lobo_samurai_til, lobo_samurai_til_end;      /* tiles del lobo             */
extern char lobo_samurai_pal;                            /* paleta del lobo (16 colores)*/
extern char barbara_samurai_til, barbara_samurai_til_end; /* tiles de la samurái      */
extern char barbara_samurai_pal;                        /* paleta de la samurái        */

/*--- Constantes -----------------------------------------------------------------*/
#define OAM_TAROU   0                 /* Entrada 0 de la OAM. oamSet quiere id*4.   */
#define OAM_BARBARA  4                 /* Entrada 1 de la OAM → 1*4 = 4.             */
#define CUADROS_TAROU   4             /* Cuadros de reposo (de los 6 del pack).     */
#define CUADROS_BARBARA  4             /* Cuadros de reposo (de los 5 del pack).     */
#define BLOQUE_BARBARA   CUADROS_TAROU /* Primer bloque de Bárbara en la VRAM (4).  */
#define FRAMES_POR_CUADRO 8            /* Cambia de cuadro cada 8 frames (7,5 c/s).  */
#define PALETA_TAROU   0              /* Paleta de sprites 0 (CGRAM 128..143).      */
#define PALETA_BARBARA  1              /* Paleta de sprites 1 (CGRAM 144..159).      */
#define VRAM_SPRITES    0x0000         /* Base de los tiles de sprites en VRAM.      */
#define PRIORIDAD_ALTA  3              /* Por delante de todos los fondos.           */

/*--- Estado ---------------------------------------------------------------------*/
static u16 x_tarou, y_tarou, x_barbara, y_barbara;   /* Posiciones actuales.     */
static u8  cuadro_tarou, cuadro_barbara;              /* Cuadro de animación.     */
static u8  frames_cuadro;                              /* Frames desde el cambio.  */
static u8  visibles;                                   /* 1 = se están mostrando.  */
static u16 tile;                                       /* Cálculo temporal.        */

/*--- tile_del_bloque: bloque de 64x64 número n → número de tile en la OAM -------*/
static u16 tile_del_bloque(u8 n)
{
    return (u16)((n / 2) * 128 + (n % 2) * 8);   /* Ver explicación en la cabecera. */
}

/*--- personajes_cargar ------------------------------------------------------------
    1) oamInit: borra la OAM (los 128 sprites quedan escondidos).
    2) oamInitGfxSet: copia los tiles del lobo a VRAM 0x0000, su paleta a la
       paleta de sprites 0, y fija el tamaño de sprites: "grandes" de 64x64 y
       "pequeños" de 32x32 (OBJ_SIZE32_L64). Usamos siempre el grande.
    3) La samurái va justo detrás del lobo en la VRAM. oamInitGfxSet solo sabe
       cargar un juego de tiles, así que la copiamos "a mano" con DMA: sus tiles
       a la dirección donde terminan los del lobo, y su paleta a la paleta de
       sprites 1 (los colores 128 + 16 = 144 en adelante de la CGRAM). */
void personajes_cargar(void)
{
    u16 tam_tarou  = (u16)(&lobo_samurai_til_end - &lobo_samurai_til);          /* bytes */
    u16 tam_barbara = (u16)(&barbara_samurai_til_end - &barbara_samurai_til);  /* bytes */

    oamInit();
    oamInitGfxSet(&lobo_samurai_til, tam_tarou, &lobo_samurai_pal, 16 * 2,
                  PALETA_TAROU, VRAM_SPRITES, OBJ_SIZE32_L64);
    personajes_ocultar();                            /* Escondidos hasta que se pidan.      */

    /* La VRAM se direcciona en "palabras" de 2 bytes: dividimos los bytes por 2. */
    dmaCopyVram(&barbara_samurai_til, VRAM_SPRITES + tam_tarou / 2, tam_barbara);
    dmaCopyCGram(&barbara_samurai_pal, 128 + 16 * PALETA_BARBARA, 16 * 2);

    visibles = 0;
}

/*--- dibujar: escribe las dos entradas de la OAM con el cuadro actual -----------*/
static void dibujar(void)
{
    /* El lobo del pack mira a la izquierda; con hflip = 1 la PPU lo dibuja en
       espejo y mira a la derecha, hacia Bárbara. Gratis: no gasta VRAM. */
    tile = tile_del_bloque(cuadro_tarou);
    oamSet(OAM_TAROU, x_tarou, y_tarou, PRIORIDAD_ALTA, 1, 0, tile, PALETA_TAROU);
    oamSetEx(OAM_TAROU, OBJ_LARGE, OBJ_SHOW);        /* 64x64, visible.          */

    tile = tile_del_bloque(BLOQUE_BARBARA + cuadro_barbara);
    oamSet(OAM_BARBARA, x_barbara, y_barbara, PRIORIDAD_ALTA, 0, 0, tile, PALETA_BARBARA);
    oamSetEx(OAM_BARBARA, OBJ_LARGE, OBJ_SHOW);
}

void personajes_mostrar(u16 xt, u16 yt, u16 xb, u16 yb)
{
    x_tarou = xt;  y_tarou = yt;
    x_barbara = xb; y_barbara = yb;
    cuadro_tarou = 0;
    cuadro_barbara = 0;
    frames_cuadro = 0;
    visibles = 1;
    dibujar();
}

void personajes_animar(void)
{
    if (!visibles)                       /* Si están escondidos no hay nada que animar.*/
    {
        return;
    }
    frames_cuadro++;
    if (frames_cuadro < FRAMES_POR_CUADRO)
    {
        return;                          /* Todavía no toca cambiar de cuadro.        */
    }
    frames_cuadro = 0;
    cuadro_tarou++;                     /* Siguiente cuadro; al llegar al final...   */
    if (cuadro_tarou >= CUADROS_TAROU)
    {
        cuadro_tarou = 0;               /* ...vuelve al primero (bucle).             */
    }
    cuadro_barbara++;
    if (cuadro_barbara >= CUADROS_BARBARA)
    {
        cuadro_barbara = 0;
    }
    dibujar();
}

void personajes_ocultar(void)
{
    /* OJO: OBJ_HIDE por sí solo manda el sprite a y = 240, pero la coordenada
       vertical de la OAM da la vuelta cada 256 píxeles: un sprite de 64 de alto
       en y = 240 asoma 48 filas por ARRIBA de la pantalla (se veían rectángulos
       negros). Solución: hacerlo pequeño (32x32), ponerlo en y = 224 (justo
       debajo del borde, sin dar la vuelta) y en x = 256 (bit alto de x = fuera
       de pantalla por la derecha). Al volver a mostrar, personajes_mostrar()
       repone tamaño y posición. */
    oamSetEx(OAM_TAROU, OBJ_SMALL, OBJ_HIDE);
    oamSetXY(OAM_TAROU, 256, 224);
    oamSetEx(OAM_BARBARA, OBJ_SMALL, OBJ_HIDE);
    oamSetXY(OAM_BARBARA, 256, 224);
    visibles = 0;
}
