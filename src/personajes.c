/*---------------------------------------------------------------------------------
    personajes.c — Sprites de Tarouh (lobo) y Bárbara (samurái) para la demo.

    Cómo están guardados los gráficos:
      - Cada personaje es una tira de cuadros de 64x64 (res/personajes/*.png),
        convertida por gfx4snes con bloques de 64 (-s 64). gfx4snes empaqueta
        los bloques en filas de 128 píxeles de ancho (16 tiles), dos bloques de
        64x64 por fila. Por eso el cuadro número n empieza en el tile:
            (n / 2) * 128 + (n % 2) * 8
        (128 tiles por fila de bloques; 8 tiles de desplazamiento para el
        segundo bloque de la fila). Es la misma cuenta que hace el ejemplo
        AnimatedSprite del SDK con su tabla sprTiles.
      - LÍMITE DURO: la OAM direcciona como máximo 512 tiles de sprites (dos
        tablas de 256). Cada bloque de 64x64 son 64 tiles, así que caben 8
        bloques en total: Tarouh usa los bloques 0..3 (4 cuadros) y Bárbara los
        bloques 4..7 (4 cuadros), uno detrás de otro en la VRAM desde 0x0000
        (16 KB). Con más cuadros, los tiles "dan la vuelta" y se ve otro sprite
        (pasó en la primera prueba: dos lobos).
    → glosario: Sprite, OAM, VRAM, Paleta
---------------------------------------------------------------------------------*/
#include "personajes.h"

/*--- Datos en ROM (pegados por data.asm) ----------------------------------------*/
extern char tarouh_lobo_til, tarouh_lobo_til_end;      /* tiles del lobo             */
extern char tarouh_lobo_pal;                            /* paleta del lobo (16 colores)*/
extern char barbara_samurai_til, barbara_samurai_til_end; /* tiles de la samurái      */
extern char barbara_samurai_pal;                        /* paleta de la samurái        */

/*--- Constantes -----------------------------------------------------------------*/
#define OAM_TAROUH   0                 /* Entrada 0 de la OAM. oamSet quiere id*4.   */
#define OAM_BARBARA  4                 /* Entrada 1 de la OAM → 1*4 = 4.             */
#define CUADROS_TAROUH   4             /* Cuadros de reposo (de los 6 del pack).     */
#define CUADROS_BARBARA  4             /* Cuadros de reposo (de los 5 del pack).     */
#define BLOQUE_BARBARA   CUADROS_TAROUH /* Primer bloque de Bárbara en la VRAM (4).  */
#define FRAMES_POR_CUADRO 8            /* Cambia de cuadro cada 8 frames (7,5 c/s).  */
#define PALETA_TAROUH   0              /* Paleta de sprites 0 (CGRAM 128..143).      */
#define PALETA_BARBARA  1              /* Paleta de sprites 1 (CGRAM 144..159).      */
#define VRAM_SPRITES    0x0000         /* Base de los tiles de sprites en VRAM.      */
#define PRIORIDAD_ALTA  3              /* Por delante de todos los fondos.           */

/*--- Estado ---------------------------------------------------------------------*/
static u16 x_tarouh, y_tarouh, x_barbara, y_barbara;   /* Posiciones actuales.     */
static u8  cuadro_tarouh, cuadro_barbara;              /* Cuadro de animación.     */
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
    u16 tam_tarouh  = (u16)(&tarouh_lobo_til_end - &tarouh_lobo_til);          /* bytes */
    u16 tam_barbara = (u16)(&barbara_samurai_til_end - &barbara_samurai_til);  /* bytes */

    oamInit();
    oamInitGfxSet(&tarouh_lobo_til, tam_tarouh, &tarouh_lobo_pal, 16 * 2,
                  PALETA_TAROUH, VRAM_SPRITES, OBJ_SIZE32_L64);
    personajes_ocultar();                            /* Escondidos hasta que se pidan.      */

    /* La VRAM se direcciona en "palabras" de 2 bytes: dividimos los bytes por 2. */
    dmaCopyVram(&barbara_samurai_til, VRAM_SPRITES + tam_tarouh / 2, tam_barbara);
    dmaCopyCGram(&barbara_samurai_pal, 128 + 16 * PALETA_BARBARA, 16 * 2);

    visibles = 0;
}

/*--- dibujar: escribe las dos entradas de la OAM con el cuadro actual -----------*/
static void dibujar(void)
{
    /* El lobo del pack mira a la izquierda; con hflip = 1 la PPU lo dibuja en
       espejo y mira a la derecha, hacia Bárbara. Gratis: no gasta VRAM. */
    tile = tile_del_bloque(cuadro_tarouh);
    oamSet(OAM_TAROUH, x_tarouh, y_tarouh, PRIORIDAD_ALTA, 1, 0, tile, PALETA_TAROUH);
    oamSetEx(OAM_TAROUH, OBJ_LARGE, OBJ_SHOW);        /* 64x64, visible.          */

    tile = tile_del_bloque(BLOQUE_BARBARA + cuadro_barbara);
    oamSet(OAM_BARBARA, x_barbara, y_barbara, PRIORIDAD_ALTA, 0, 0, tile, PALETA_BARBARA);
    oamSetEx(OAM_BARBARA, OBJ_LARGE, OBJ_SHOW);
}

void personajes_mostrar(u16 xt, u16 yt, u16 xb, u16 yb)
{
    x_tarouh = xt;  y_tarouh = yt;
    x_barbara = xb; y_barbara = yb;
    cuadro_tarouh = 0;
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
    cuadro_tarouh++;                     /* Siguiente cuadro; al llegar al final...   */
    if (cuadro_tarouh >= CUADROS_TAROUH)
    {
        cuadro_tarouh = 0;               /* ...vuelve al primero (bucle).             */
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
    oamSetEx(OAM_TAROUH, OBJ_SMALL, OBJ_HIDE);
    oamSetXY(OAM_TAROUH, 256, 224);
    oamSetEx(OAM_BARBARA, OBJ_SMALL, OBJ_HIDE);
    oamSetXY(OAM_BARBARA, 256, 224);
    visibles = 0;
}
