/*---------------------------------------------------------------------------------
    personajes.c — Sprites de Tarou y Bárbara para la secuencia de arranque.

    PROPÓSITO
      Cargar en VRAM de una vez (carga ESTÁTICA, sin motor dinámico) los cuadros
      de reposo de los dos personajes, mostrarlos, animarlos y esconderlos.
      - Tarou usa la tira res/personajes/tarou/tarou_reposo_estatico.png:
        SIEMPRE 2 cuadros de 96x64, elegidos del reposo real por
        preparar_personaje.py (tenga el reposo del nivel los cuadros que
        tenga), convertida por gfx4snes -s 32 -T en piezas de 32x32 más una
        tabla metasprite por cuadro. Los 2 cuadros (2 x 96 tiles = 6 KB) se
        copian a VRAM 0x0000 y cada cuadro se dibuja con un oamSet de 32x32
        por pieza siguiendo su tabla. Un cuadro tiene HASTA 6 piezas (gfx4snes
        -T omite las transparentes): las entradas de la OAM que sobran se
        esconden (juego/metasprite.c cuenta las piezas).
      - Bárbara sigue siendo el sprite de 64x64 del pack (res/personajes/
        barbara_samurai, gfx4snes -s 64), con sus tiles a continuación.
      LÍMITE: la OAM direcciona 512 tiles de sprites (16 KB) en DOS tablas de
      256. Un sprite de 64x64 NO puede cruzar de una tabla a la otra: al bajar
      de fila la PPU suma 16 al número de tile y da la vuelta dentro de su
      tabla (verificado en Mesen el 2026-10-19: con Bárbara en el tile 192, la
      mitad de abajo de sus cuadros 0 y 1 mostraba tiles de Tarou). Por eso
      Bárbara ocupa la segunda tabla entera (tiles 256..511) y Tarou la
      primera: 3 cuadros de 96 tiles no caben en 256, 2 sí (192, con 64 libres).
      Por eso la tira estática son exactamente 2.

      Numeración de tiles (verificada el 2026-10-16):
      - gfx4snes -s 32 empaqueta los bloques de 32x32 en filas de 128 px, cuatro
        por fila: el bloque n empieza en el tile (n / 4) * 64 + (n % 4) * 4. Los
        números de bloque de cada cuadro los da la tabla metasprite (dtile).
      - gfx4snes -s 64 empaqueta dos bloques de 64x64 por fila de 128 px: el
        bloque n empieza en el tile (n / 2) * 128 + (n % 2) * 8, sumando el
        tile base de Bárbara (256: el principio de la segunda tabla).

    SE CONECTA CON
      Incluye:  pantallas/personajes.h, juego/metasprite.h (metasprite_piezas) y
                tarou_reposo_estatico_meta.inc (lo genera gfx4snes -T; llega por
                la ruta -Ires/personajes/tarou del Makefile): define las tablas
                tarou_reposo_estatico_metasprites[4] y se incluye SOLO aquí.
      Lee de la ROM: tarou_reposo_estatico_til/_til_end y tarou_pal,
                barbara_samurai_til/_til_end/_pal (data.asm).
      Lo usan:  pantallas/pantalla_intro.c (escena 3), pantallas/pantalla_titulo.c,
                sistema/main.c (personajes_ocultar al arrancar).

    HARDWARE QUE TOCA
      OAM: oamInit, oamSet, oamSetEx, oamSetXY (la librería la copia a la PPU en
      cada VBlank). VRAM: dmaCopyVram (tiles) y oamInitGfxAttr (tamaño 32/64).
      CGRAM: dmaCopyCGram (paletas de sprites 0 y 1, colores 128..159).

    VARIABLES (globales y estáticas de este archivo)
      x_tarou, y_tarou      u16  píxeles de pantalla  esquina superior izquierda de la celda de Tarou
      x_barbara, y_barbara  u16  píxeles de pantalla  ídem Bárbara
      tarou_izquierda       u8   0 / 1                1 = Tarou mira a la izquierda (espejo)
      cuadro_tarou          u8   0..CUADROS_TAROU-1   cuadro de reposo de Tarou
      cuadro_barbara        u8   0..CUADROS_BARBARA-1 cuadro de reposo de Bárbara
      frames_cuadro         u8   0..FRAMES_POR_CUADRO frames desde el último cambio de cuadro
      visibles              u8   0 / 1                1 = se están mostrando
      tabla                 const t_metasprite*       tabla del cuadro de Tarou en curso
      piezas                u8   0..6                 cuántas piezas tiene ese cuadro
      i                     u8                        índice de pieza

    → glosario: Sprite, OAM, VRAM, Paleta, Metasprite, DMA / HDMA
---------------------------------------------------------------------------------*/
#include "pantallas/personajes.h"
#include "juego/metasprite.h"

/*--- Tablas metasprite de la tira estática: tarou_reposo_estatico_metasprites[4] --*/
#include "tarou_reposo_estatico_meta.inc"

/*--- Datos en ROM (pegados por data.asm) ----------------------------------------*/
extern char tarou_reposo_estatico_til, tarou_reposo_estatico_til_end; /* piezas 32x32 */
extern char tarou_pal;                                  /* paleta de Tarou (16 colores) */
extern char barbara_samurai_til, barbara_samurai_til_end; /* tiles de la samurái      */
extern char barbara_samurai_pal;                        /* paleta de la samurái        */

/*--- Constantes -----------------------------------------------------------------*/
#define OAM_TAROU 0                    /* Entradas 0..5 de la OAM (una por pieza).    */
#define OAM_BARBARA 6                  /* Entrada 6 → oamSet quiere id*4 = 24.        */
#define PIEZAS_TAROU METASPRITE_PIEZAS_MAX /* 96x64 = HASTA 6 piezas de 32x32.        */
#define CUADROS_TAROU 2                /* La tira estática trae SIEMPRE 2 (cabecera). */
#define BYTES_POR_CUADRO 3072          /* 96 tiles x 32 bytes.                        */
#define CUADROS_BARBARA 4              /* Cuadros de reposo (de los 5 del pack).      */
#define TILE_BASE_BARBARA 256          /* Segunda tabla de tiles: ver LÍMITE arriba.  */
#define VRAM_BARBARA 0x1000            /* Tile 256 x 16 palabras por tile (4bpp).     */
#define FRAMES_POR_CUADRO 8            /* Cambia de cuadro cada 8 frames (7,5 c/s).   */
#define PALETA_TAROU 0                 /* Paleta de sprites 0 (CGRAM 128..143).       */
#define PALETA_BARBARA 1               /* Paleta de sprites 1 (CGRAM 144..159).       */
#define VRAM_SPRITES 0x0000            /* Base de los tiles de sprites en VRAM.       */
#define PRIORIDAD_ALTA 3               /* Por delante de todos los fondos.            */

/*--- Estado ---------------------------------------------------------------------*/
static u16 x_tarou, y_tarou, x_barbara, y_barbara;     /* Posiciones actuales.     */
static u8 tarou_izquierda;                             /* 1 = espejo.              */
static u8 cuadro_tarou, cuadro_barbara;                /* Cuadro de animación.     */
static u8 frames_cuadro;                               /* Frames desde el cambio.  */
static u8 visibles;                                    /* 1 = se están mostrando.  */
static const t_metasprite *tabla;                      /* Tabla del cuadro actual. */
static u8 piezas;                                      /* Piezas de ese cuadro.    */
static u8 i;

/*--- tile_del_bloque32: bloque de 32x32 número n (de la tira de Tarou) → tile -----*/
static u16 tile_del_bloque32(u16 n)
{
    return (u16)((n / 4) * 64 + (n % 4) * 4);  /* Ver numeración en la cabecera.  */
}

/*--- tile_del_bloque64: bloque de 64x64 número n (de la tira de Bárbara) → tile ---*/
static u16 tile_del_bloque64(u8 n)
{
    return (u16)(TILE_BASE_BARBARA + (n / 2) * 128 + (n % 2) * 8);
}

/*--- esconder: manda una entrada de la OAM fuera de la pantalla -------------------
    OJO: OBJ_HIDE por sí solo manda el sprite a y = 240, pero la coordenada
    vertical de la OAM da la vuelta cada 256 píxeles: un sprite de 64 de alto en
    y = 240 asoma 48 filas por ARRIBA de la pantalla. Solución: hacerlo pequeño
    (32x32), ponerlo en y = 224 (justo debajo del borde, sin dar la vuelta) y en
    x = 256 (bit alto de x = fuera de pantalla por la derecha). Al volver a
    mostrar, dibujar() repone tamaño y posición. */
static void esconder(u8 entrada)
{
    oamSetEx((u16)entrada * 4, OBJ_SMALL, OBJ_HIDE);
    oamSetXY((u16)entrada * 4, 256, 224);
}

/*--- personajes_cargar ------------------------------------------------------------
    1) oamInit: borra la OAM (los 128 sprites quedan escondidos).
    2) oamInitGfxAttr: tiles de sprites desde VRAM 0x0000 y tamaños "pequeño"
       32x32 (las piezas de Tarou) y "grande" 64x64 (Bárbara): OBJ_SIZE32_L64.
    3) DMA de los 2 cuadros de la tira estática de Tarou (6 KB) a 0x0000 y de
       los de Bárbara a 0x1000 (tile 256, la segunda tabla); cada paleta a su
       hueco de la CGRAM. La VRAM se direcciona en palabras de 2 bytes. */
void personajes_cargar(void)
{
    u16 tam_tarou = (u16)(&tarou_reposo_estatico_til_end - &tarou_reposo_estatico_til); /* bytes */
    u16 tam_barbara = (u16)(&barbara_samurai_til_end - &barbara_samurai_til);   /* bytes */
    if (tam_tarou > CUADROS_TAROU * BYTES_POR_CUADRO)
    {
        tam_tarou = CUADROS_TAROU * BYTES_POR_CUADRO;   /* Red de seguridad: solo 6 KB. */
    }

    oamInit();
    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE32_L64);
    dmaCopyVram((u8 *)&tarou_reposo_estatico_til, VRAM_SPRITES, tam_tarou);
    dmaCopyCGram((u8 *)&tarou_pal, 128 + 16 * PALETA_TAROU, 16 * 2);
    dmaCopyVram((u8 *)&barbara_samurai_til, VRAM_BARBARA, tam_barbara);  /* Tile 256.   */
    dmaCopyCGram((u8 *)&barbara_samurai_pal, 128 + 16 * PALETA_BARBARA, 16 * 2);

    personajes_ocultar();                            /* Escondidos hasta que se pidan.      */
}

/*--- dibujar: escribe las siete entradas de la OAM con el cuadro actual ----------*/
static void dibujar(void)
{
    u16 dx;
    /* Tarou: una pieza de 32x32 por entrada de la tabla metasprite del cuadro
       (mirando a la derecha). En espejo, cada pieza cambia de lado (dx 0 <-> 64,
       la columna del medio se queda) y se voltea. Las entradas de la OAM que
       este cuadro no usa se esconden. */
    tabla = tarou_reposo_estatico_metasprites[cuadro_tarou];
    piezas = metasprite_piezas(tabla);               /* 0..4: para en el terminador.        */
    for (i = 0; i < piezas; i++)
    {
        dx = tarou_izquierda ? (u16)(METASPRITE_ESPEJO_DX - tabla[i].dx) : tabla[i].dx;
        oamSet((OAM_TAROU + i) * 4, (u16)(x_tarou + dx), (u16)(y_tarou + tabla[i].dy),
               PRIORIDAD_ALTA, tarou_izquierda, 0, tile_del_bloque32(tabla[i].dtile), PALETA_TAROU);
        oamSetEx((OAM_TAROU + i) * 4, OBJ_SMALL, OBJ_SHOW);   /* 32x32, visible.       */
    }
    for (; i < PIEZAS_TAROU; i++)
    {
        esconder(OAM_TAROU + i);                     /* Pieza transparente: sin sprite.     */
    }

    /* Bárbara: un sprite de 64x64. La samurái del pack mira a la izquierda; sin
       espejo mira hacia Tarou, que queda a su izquierda. */
    oamSet(OAM_BARBARA * 4, x_barbara, y_barbara, PRIORIDAD_ALTA, 0, 0,
           tile_del_bloque64(cuadro_barbara), PALETA_BARBARA);
    oamSetEx(OAM_BARBARA * 4, OBJ_LARGE, OBJ_SHOW);           /* 64x64, visible.       */
}

void personajes_mostrar(u16 xt, u16 yt, u8 tarou_mira_izquierda, u16 xb, u16 yb)
{
    x_tarou = xt;
    y_tarou = yt;
    tarou_izquierda = tarou_mira_izquierda;
    x_barbara = xb;
    y_barbara = yb;
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
    cuadro_tarou++;                      /* Siguiente cuadro; al llegar al final...   */
    if (cuadro_tarou >= CUADROS_TAROU)
    {
        cuadro_tarou = 0;                /* ...vuelve al primero (bucle).             */
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
    for (i = 0; i < PIEZAS_TAROU; i++)
    {
        esconder(OAM_TAROU + i);
    }
    esconder(OAM_BARBARA);
    visibles = 0;
}
