/*---------------------------------------------------------------------------------
    comun.c — Utilidades compartidas por las pantallas.

    Regla de PVSnesLib que se nota aquí: las variables se declaran al principio
    de cada función (C clásico) y preferimos variables globales a locales, porque
    el compilador de la SNES (tcc816) genera código más rápido con ellas.
---------------------------------------------------------------------------------*/
#include "comun.h"

/*--- Las dos fuentes viven en la ROM (data.asm) -------------------------------------*/
extern char fuente_consola_til, fuente_consola_pal;      /* 16 colores (4bpp) */
extern char fuente_2bpp_til, fuente_2bpp_pal;            /* 4 colores  (2bpp) */

/*--- Variables de trabajo (globales al archivo) ---------------------------------*/
static char texto_parcial[33];  /* Búfer para el trozo de texto que se muestra en la
                                   máquina de escribir: 32 letras (ancho de pantalla)
                                   + 1 para el 0 final. "static" = solo se ve en este
                                   archivo. → glosario: static */
static u8 i;                    /* Índice para bucles. */
static u8 largo;                /* Largo del texto que estamos procesando. */
static u8 columna;              /* Columna (x) donde empieza un texto centrado. */

/*=== VÍDEO Y FUENTE ==============================================================*/

/*--- fuente_4bpp ------------------------------------------------------------------
    La consola de texto de PVSnesLib escribe su mapa en VRAM_MAPA_TEXTO y sus
    tiles en VRAM_FUENTE; qué fondo los MUESTRA lo decidimos nosotros apuntando
    ese fondo a esas mismas direcciones. El "offset" 0 significa que el tile 0
    de la fuente es el tile 0 del fondo (su base de tiles ES la fuente). */
void fuente_4bpp(u8 bg, u8 paleta)
{
    bgSetGfxPtr(bg, VRAM_FUENTE);                    /* Tiles del fondo = la fuente.        */
    bgSetMapPtr(bg, VRAM_MAPA_TEXTO, SC_32x32);      /* Mapa del fondo = mapa de texto.     */
    consoleSetTextGfxPtr(VRAM_FUENTE);               /* Dónde copia la consola la fuente.   */
    consoleSetTextMapPtr(VRAM_MAPA_TEXTO);           /* Dónde escribe la consola el mapa.   */
    consoleSetTextOffset(0);
    /* Paleta `paleta`, 2 colores (2 * 2 bytes): el 0 transparente y el 1 blanco. */
    consoleInitText(paleta, 2 * 2, &fuente_consola_til, &fuente_consola_pal);
}

void fuente_2bpp(u8 bg)
{
    bgSetGfxPtr(bg, VRAM_FUENTE);
    bgSetMapPtr(bg, VRAM_MAPA_TEXTO, SC_32x32);
    consoleSetTextGfxPtr(VRAM_FUENTE);
    consoleSetTextMapPtr(VRAM_MAPA_TEXTO);
    consoleSetTextOffset(0);
    consoleInitText(0, 2 * 2, &fuente_2bpp_til, &fuente_2bpp_pal);   /* Paleta 0 (colores 0..3). */
}

/*--- video_texto_mode1 --------------------------------------------------------------
    Estado base: Mode 1, texto en el fondo 0, los otros dos fondos apagados. Se
    llama al entrar en cada pantalla de solo texto, y también al volver de
    Mode 7 o Mode 3, que cambian el modo y pisan las paletas. */
void video_texto_mode1(void)
{
    setMode(BG_MODE1, 0);                            /* Mode 1: 16 + 16 + 4 colores.        */
    fuente_4bpp(0, 0);                               /* Texto en el fondo 0, paleta 0.      */
    bgSetEnable(0);
    bgSetDisable(1);
    bgSetDisable(2);
}

/*=== FONDOS ======================================================================*/

/*--- fondo16_cargar -----------------------------------------------------------------
    bgInitTileSet copia los tiles a VRAM y la paleta a la CGRAM (16 colores * 2
    bytes en la paleta `paleta`, es decir, a partir del color paleta*16).
    bgInitMapSet copia el mapa (qué tile va en cada celda). Los tamaños se pasan
    en bytes: restando las etiquetas de fin e inicio que pone data.asm. */
void fondo16_cargar(u8 bg, u8 *til, u8 *til_end, u8 *pal, u8 paleta,
                    u8 *map, u8 *map_end, u16 vram_til, u16 vram_map, u8 tam_mapa)
{
    bgInitTileSet(bg, til, pal, paleta, (u16)(til_end - til), 16 * 2, BG_16COLORS, vram_til);
    bgInitMapSet(bg, map, (u16)(map_end - map), tam_mapa, vram_map);
    bgSetEnable(bg);
}

void fondo256_cargar(u8 *til, u8 *til_end, u8 *pal, u8 *map, u8 *map_end,
                     u16 vram_til, u16 vram_map)
{
    bgInitTileSet(0, til, pal, 0, (u16)(til_end - til), 256 * 2, BG_256COLORS, vram_til);
    bgInitMapSet(0, map, (u16)(map_end - map), SC_32x32, vram_map);
    bgSetEnable(0);
}

/*=== TEXTO =======================================================================*/

/*--- longitud -------------------------------------------------------------------
    En C una cadena es una fila de letras terminada en un byte 0. Contamos letras
    hasta encontrar ese 0. */
u8 longitud(char *texto)
{
    u8 n = 0;                    /* Contador de letras, empieza en cero.                   */
    while (texto[n] != 0)        /* Mientras la letra en la posición n NO sea el 0 final...*/
    {
        n++;                     /* ...avanzamos una posición (n = n + 1).                 */
    }
    return n;                    /* n es ahora la cantidad de letras.                      */
}

/*--- limpiar_texto --------------------------------------------------------------
    La capa de texto tiene 32 columnas x 28 filas. Escribir 32 espacios en cada
    fila la deja en blanco. → glosario: Consola de texto */
void limpiar_texto(void)
{
    for (i = 0; i < 28; i++)
    {
        consoleDrawText(0, i, "                                ");  /* 32 espacios.       */
    }
}

/*--- texto_centrado -------------------------------------------------------------*/
void texto_centrado(u8 fila, char *texto)
{
    largo = longitud(texto);
    columna = (32 - largo) / 2;
    consoleDrawText(columna, fila, texto);
}

/*--- escribir_parcial -----------------------------------------------------------
    Efecto máquina de escribir para textos de varias líneas (ver comun.h). */
u16 escribir_parcial(u8 fila, char *texto, u16 letras)
{
    largo = longitud(texto);
    columna = (32 - largo) / 2;

    if (letras >= largo)
    {
        consoleDrawText(columna, fila, texto);
        return letras - largo;
    }

    for (i = 0; i < letras; i++)
    {
        texto_parcial[i] = texto[i];
    }
    texto_parcial[letras] = 0;
    consoleDrawText(columna, fila, texto_parcial);
    return 0;
}

/*=== FUNDIDOS ====================================================================
    setFadeEffectEx anima el brillo de la PPU (0 = negro, 15 = normal) y BLOQUEA
    el programa hasta terminar. El parámetro son los frames que espera ENTRE
    CADA UNO de los 16 pasos de brillo (no el total): con 2, el fundido dura
    2 * 16 = 32 frames. → glosario: Fade / brillo */
void fade_in(u8 frames)
{
    setFadeEffectEx(FADE_IN, frames);
}

void fade_out(u8 frames)
{
    setFadeEffectEx(FADE_OUT, frames);
}
