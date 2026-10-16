/*---------------------------------------------------------------------------------
    comun.h — Interfaz de las utilidades compartidas y reparto de la VRAM.

    PROPÓSITO
      Declarar las funciones de comun.c y fijar, en un solo sitio, qué zona de
      la VRAM usa cada cosa (sprites, tiles de fondos, fuente, mapas). Todas las
      pantallas usan estas constantes; así ninguna pisa a otra.

    SE CONECTA CON
      Incluye:  <snes.h>.
      Lo usan:  sistema/comun.c, sistema/main.c y todas las pantallas.

    HARDWARE QUE TOCA
      ninguno directamente; define direcciones de VRAM (en palabras de 2 bytes).

    VARIABLES (globales y estáticas de este archivo)
      ninguna; solo constantes VRAM_* (ver la tabla de abajo).

    → glosario: VRAM, Mode 1, Mode 3, Mode 7
---------------------------------------------------------------------------------*/
#ifndef COMUN_H
#define COMUN_H

#include <snes.h>

/*--- Reparto de la VRAM (64 KB = 0x8000 palabras de 2 bytes) -------------------
    Lo fija este archivo para que todas las pantallas usen las mismas zonas.
    → glosario: VRAM */
#define VRAM_SPRITES     0x0000   /* 0x0000-0x1FFF tiles de sprites (512 tiles, el máximo)      */
#define VRAM_FONDO_A     0x2000   /* 0x2000-0x3FFF tiles del fondo principal / capa cercana     */
#define VRAM_FONDO_B     0x4000   /* 0x4000-0x5FFF tiles de la capa lejana (título)             */
#define VRAM_FUENTE      0x6000   /* 0x6000-0x65FF los 96 tiles de la fuente                    */
#define VRAM_MAPA_TEXTO  0x6800   /* 0x6800-0x6BFF mapa de la capa de texto (32x32)             */
#define VRAM_MAPA_A      0x6C00   /* 0x6C00-0x73FF mapa de la capa cercana (64x32, para scroll) */
#define VRAM_MAPA_B      0x7400   /* 0x7400-0x77FF mapa del fondo principal / capa lejana        */
/* Mode 3 (intro): la imagen de 256 colores ocupa 0x0000-0x3FFF (hasta 512
   tiles de 64 bytes) y su mapa va en VRAM_MAPA_B. Mode 7 (devs): 0x0000-0x3FFF. */

/*--- Vídeo y fuente ---------------------------------------------------------------*/
/* Deja la consola en Mode 1 con SOLO la capa de texto (fondo 0) y la fuente de
   16 colores en la paleta 0. Es el estado "por defecto" que usan las pantallas
   de solo texto y el punto de partida de las demás. */
void video_texto_mode1(void);

/* Carga la fuente (4 bits por píxel) para que la dibuje el fondo `bg` con la
   paleta `paleta`. Usado en Mode 3 (texto en el fondo 1, paleta 7). */
void fuente_4bpp(u8 bg, u8 paleta);

/* Carga la fuente de 2 bits por píxel (4 colores) para el fondo `bg`: es la
   única que puede mostrar BG3 en Mode 1 (título). */
void fuente_2bpp(u8 bg);

/*--- Fondos -----------------------------------------------------------------------*/
/* Copia a VRAM los tiles, la paleta (16 colores en la paleta `paleta`) y el mapa
   de un fondo convertido por gfx4snes, y lo asigna al fondo `bg`.
   tam_mapa: SC_32x32 o SC_64x32 (para scroll horizontal). */
void fondo16_cargar(u8 bg, u8 *til, u8 *til_end, u8 *pal, u8 paleta,
                    u8 *map, u8 *map_end, u16 vram_til, u16 vram_map, u8 tam_mapa);

/* Igual pero para una imagen de 256 colores (Mode 3, fondo 0). Sobrescribe las
   256 entradas de la CGRAM: hay que volver a cargar después las paletas de
   texto y sprites que se necesiten. */
void fondo256_cargar(u8 *til, u8 *til_end, u8 *pal, u8 *map, u8 *map_end,
                     u16 vram_til, u16 vram_map);

/*--- Texto --------------------------------------------------------------------------*/
u8 longitud(char *texto);
void limpiar_texto(void);
void texto_centrado(u8 fila, char *texto);
u16 escribir_parcial(u8 fila, char *texto, u16 letras);

/*--- Fundidos (bloquean mientras duran) ----------------------------------------------*/
void fade_in(u8 frames);
void fade_out(u8 frames);

#endif /* COMUN_H */
