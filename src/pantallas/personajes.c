/*---------------------------------------------------------------------------------
    personajes.c — Sprites de Tarou y Bárbara para la secuencia de arranque.

    PROPÓSITO
      Cargar en VRAM de una vez (carga ESTÁTICA, sin motor dinámico) los cuadros
      de reposo de los dos personajes, mostrarlos, animarlos y esconderlos.
      - Tarou usa el mismo arte que el nivel (res/personajes/tarou/tarou_reposo,
        convertido por gfx4snes -s 32 -T: piezas de 32x32 más una tabla
        metasprite). Aquí no se usa el motor dinámico: los 4 primeros cuadros
        (4 x 64 tiles = 8 KB) se copian a VRAM 0x0000 y cada cuadro se dibuja
        con cuatro oamSet de 32x32 siguiendo su tabla (tarou_sprite_tabla).
      - Bárbara sigue siendo el sprite de 64x64 del pack (res/personajes/
        barbara_samurai, gfx4snes -s 64), con sus tiles a continuación.
      LÍMITE: la OAM direcciona 512 tiles de sprites (16 KB). Tarou 4 cuadros
      (256 tiles) + Bárbara 4 cuadros (256 tiles) = 512 justos; por eso aquí se
      usan como mucho 4 cuadros de reposo aunque el arte traiga más.

      Numeración de tiles (verificada el 2026-10-16):
      - gfx4snes -s 32 empaqueta los bloques de 32x32 en filas de 128 px, cuatro
        por fila: el bloque n empieza en el tile (n / 4) * 64 + (n % 4) * 4. Los
        números de bloque de cada cuadro los da la tabla metasprite (dtile).
      - gfx4snes -s 64 empaqueta dos bloques de 64x64 por fila de 128 px: el
        bloque n empieza en el tile (n / 2) * 128 + (n % 2) * 8, sumando el
        tile base de Bárbara (256, detrás de los 8 KB de Tarou).

    SE CONECTA CON
      Incluye:  pantallas/personajes.h, juego/tarou_sprite.h (tarou_sprite_tabla,
                tarou_sprite_tiles), juego/animaciones.h (ESTADO_PARADO).
      Lee de la ROM: tarou_reposo_til/_til_end y tarou_pal (vía tarou_sprite.c y
                data.asm), barbara_samurai_til/_til_end/_pal (data.asm).
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
      i                     u8                        índice de pieza

    → glosario: Sprite, OAM, VRAM, Paleta, Metasprite, DMA / HDMA
---------------------------------------------------------------------------------*/
#include "pantallas/personajes.h"
#include "juego/tarou_sprite.h"
#include "juego/animaciones.h"

/*--- Datos en ROM (pegados por data.asm) ----------------------------------------*/
extern char tarou_reposo_til, tarou_reposo_til_end;    /* piezas de 32x32 del reposo   */
extern char tarou_pal;                                  /* paleta de Tarou (16 colores) */
extern char barbara_samurai_til, barbara_samurai_til_end; /* tiles de la samurái      */
extern char barbara_samurai_pal;                        /* paleta de la samurái        */

/*--- Constantes -----------------------------------------------------------------*/
#define OAM_TAROU 0                    /* Entradas 0..3 de la OAM (una por pieza).    */
#define OAM_BARBARA 4                  /* Entrada 4 → oamSet quiere id*4 = 16.        */
#define PIEZAS_TAROU 4                 /* 64x64 = 4 piezas de 32x32.                  */
#define CUADROS_TAROU 4                /* Cuadros de reposo que caben (ver cabecera). */
#define BYTES_POR_CUADRO 2048          /* 64 tiles x 32 bytes.                        */
#define CUADROS_BARBARA 4              /* Cuadros de reposo (de los 5 del pack).      */
#define TILE_BASE_BARBARA 256          /* 8 KB de Tarou / 32 bytes por tile.          */
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

/*--- personajes_cargar ------------------------------------------------------------
    1) oamInit: borra la OAM (los 128 sprites quedan escondidos).
    2) oamInitGfxAttr: tiles de sprites desde VRAM 0x0000 y tamaños "pequeño"
       32x32 (las piezas de Tarou) y "grande" 64x64 (Bárbara): OBJ_SIZE32_L64.
    3) DMA de los 4 primeros cuadros de reposo de Tarou (8 KB) a 0x0000 y de los
       de Bárbara a continuación; cada paleta a su hueco de la CGRAM. La VRAM se
       direcciona en palabras de 2 bytes: los bytes se dividen entre 2. */
void personajes_cargar(void)
{
    u16 tam_tarou = (u16)(&tarou_reposo_til_end - &tarou_reposo_til);           /* bytes */
    u16 tam_barbara = (u16)(&barbara_samurai_til_end - &barbara_samurai_til);   /* bytes */
    if (tam_tarou > CUADROS_TAROU * BYTES_POR_CUADRO)
    {
        tam_tarou = CUADROS_TAROU * BYTES_POR_CUADRO;   /* Con más cuadros, solo 4.   */
    }

    oamInit();
    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE32_L64);
    dmaCopyVram((u8 *)&tarou_reposo_til, VRAM_SPRITES, tam_tarou);
    dmaCopyCGram((u8 *)&tarou_pal, 128 + 16 * PALETA_TAROU, 16 * 2);
    dmaCopyVram((u8 *)&barbara_samurai_til, VRAM_SPRITES + tam_tarou / 2, tam_barbara);
    dmaCopyCGram((u8 *)&barbara_samurai_pal, 128 + 16 * PALETA_BARBARA, 16 * 2);

    personajes_ocultar();                            /* Escondidos hasta que se pidan.      */
}

/*--- dibujar: escribe las cinco entradas de la OAM con el cuadro actual ----------*/
static void dibujar(void)
{
    u16 dx;
    /* Tarou: cuatro piezas de 32x32 según la tabla metasprite del cuadro (mirando a
       la derecha). En espejo, cada pieza cambia de lado (dx 0 <-> 32) y se voltea. */
    tabla = tarou_sprite_tabla(ESTADO_PARADO, cuadro_tarou);
    for (i = 0; i < PIEZAS_TAROU; i++)
    {
        dx = tarou_izquierda ? (u16)(32 - tabla[i].dx) : tabla[i].dx;
        oamSet((OAM_TAROU + i) * 4, (u16)(x_tarou + dx), (u16)(y_tarou + tabla[i].dy),
               PRIORIDAD_ALTA, tarou_izquierda, 0, tile_del_bloque32(tabla[i].dtile), PALETA_TAROU);
        oamSetEx((OAM_TAROU + i) * 4, OBJ_SMALL, OBJ_SHOW);   /* 32x32, visible.       */
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
    /* OJO: OBJ_HIDE por sí solo manda el sprite a y = 240, pero la coordenada
       vertical de la OAM da la vuelta cada 256 píxeles: un sprite de 64 de alto
       en y = 240 asoma 48 filas por ARRIBA de la pantalla. Solución: hacerlo
       pequeño (32x32), ponerlo en y = 224 (justo debajo del borde, sin dar la
       vuelta) y en x = 256 (bit alto de x = fuera de pantalla por la derecha).
       Al volver a mostrar, dibujar() repone tamaño y posición. */
    for (i = 0; i < PIEZAS_TAROU; i++)
    {
        oamSetEx((OAM_TAROU + i) * 4, OBJ_SMALL, OBJ_HIDE);
        oamSetXY((OAM_TAROU + i) * 4, 256, 224);
    }
    oamSetEx(OAM_BARBARA * 4, OBJ_SMALL, OBJ_HIDE);
    oamSetXY(OAM_BARBARA * 4, 256, 224);
    visibles = 0;
}
