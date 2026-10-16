/*---------------------------------------------------------------------------------
    pantalla_intro.c — Pantalla (5): intro en tres escenas con texto letra a letra.

    PROPÓSITO
      Cada escena muestra un fondo y un párrafo corto que aparece letra por
      letra, 8 segundos por escena; sale hacia el título con efecto mosaico.
      Escenas 1 y 2: MODE 3, imagen de 256 colores en el fondo 0 (256x120 arriba;
      abajo, banda transparente) y el texto en el fondo 1 con la paleta 7 (los
      colores 112..127, que las imágenes dejan libres). Escena 3: MODE 1, imagen
      de 16 colores en el fondo 1 (paleta 1), texto en el fondo 0 y los sprites
      de Tarou y Bárbara delante.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h,
                pantallas/personajes.h (sprites de la escena 3).
      Lee de la ROM: escena1_*, escena2_*, escena3_* (data.asm).
      Lo usan:  sistema/secuencia.c.

    HARDWARE QUE TOCA
      PPU: Mode 3 y Mode 1 (setMode), fondos y texto por comun.c, mosaico
      (setMosaicEffect, REG_MOSAIC), brillo. VRAM/CGRAM por comun.c y
      personajes.c (Mode 3 pisa las 256 entradas de color y la VRAM de sprites).
      Mando: padsDown() si PERMITE_SALTO.

    VARIABLES (globales y estáticas de este archivo)
      escena         u8   0..2             escena actual
      letras         u16  0..~200          letras del párrafo ya mostradas
      restante       u16  0..~100          letras que sobran para la línea siguiente (encadenado)
      inicio_escena  u16  frames           frame en que empezó la escena actual

    → glosario: Mode 1, Mode 3, Mosaico, Sprite, Paleta, Consola de texto
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "pantallas/personajes.h"

extern char escena1_til, escena1_til_end, escena1_pal, escena1_map, escena1_map_end;
extern char escena2_til, escena2_til_end, escena2_pal, escena2_map, escena2_map_end;
extern char escena3_til, escena3_til_end, escena3_pal, escena3_map, escena3_map_end;

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u8 escena;         /* Escena actual: 0, 1 o 2.                               */
static u16 letras;        /* Letras del párrafo ya mostradas.                       */
static u16 restante;      /* Letras que sobran para la línea siguiente (encadenado).*/
static u16 inicio_escena; /* Frame en que empezó la escena actual.                  */

/*--- Escena en Mode 3: imagen de 256 colores + texto en el fondo 1 ---------------*/
static void escena_256(u8 *til, u8 *til_end, u8 *pal, u8 *map, u8 *map_end)
{
    setMode(BG_MODE3, 0); /* Fondo 0 = 256 colores, fondo 1 = 16.*/
    fondo256_cargar(til, til_end, pal, map, map_end, 0x0000, VRAM_MAPA_B);
    fuente_4bpp(1, 7); /* Texto en el fondo 1, paleta 7. Va
                          DESPUÉS de la imagen porque esta
                          escribe las 256 entradas de color.  */
    bgSetEnable(1);
    bgSetDisable(2);
}

/*--- Dibuja el marco de una escena (sin el texto, que crece frame a frame) ------*/
static void preparar_escena(void)
{
    switch (escena)
    {
    case 0: /* El pueblo (Spring Forest).          */
        escena_256(&escena1_til, &escena1_til_end, &escena1_pal, &escena1_map, &escena1_map_end);
        break;
    case 1: /* El viento (Autumn Forest).          */
        escena_256(&escena2_til, &escena2_til_end, &escena2_pal, &escena2_map, &escena2_map_end);
        break;
    case 2: /* El camino: Mode 1 + sprites.        */
        video_texto_mode1();
        personajes_cargar(); /* Mode 3 pisó la VRAM de los sprites. */
        fondo16_cargar(1, &escena3_til, &escena3_til_end, &escena3_pal, 1,
                       &escena3_map, &escena3_map_end, VRAM_FONDO_A, VRAM_MAPA_B, SC_32x32);
        /* Pies sobre el borde inferior de la imagen (fila 128): celda 64..128. */
        personajes_mostrar(48, 64, 0, 150, 64);  /* Tarou mira a la derecha.         */
        break;
    default:
        break;
    }
    limpiar_texto();
    letras = 0;                         /* El párrafo empieza vacío.           */
    inicio_escena = frames_en_pantalla; /* Para medir los 8 s de esta escena.  */
}

/*--- Escribe el párrafo de la escena actual con `letras` letras visibles --------*/
static void escribir_parrafo(void)
{
    switch (escena)
    {
    case 0:
        restante = escribir_parcial(18, "IN A QUIET VILLAGE BY THE HILLS", letras);
        restante = escribir_parcial(20, "LIVED A PUPPY NAMED TAROU", restante);
        restante = escribir_parcial(22, "AND HIS BEST FRIEND, BáRBARA.", restante);
        break;
    case 1:
        restante = escribir_parcial(18, "ONE MORNING, A STRANGE WIND", letras);
        restante = escribir_parcial(20, "SWEPT THROUGH THE VALLEY", restante);
        restante = escribir_parcial(22, "AND TOOK SOMETHING PRECIOUS.", restante);
        break;
    case 2:
        restante = escribir_parcial(19, "TOGETHER, THEY SET OUT", letras);
        restante = escribir_parcial(21, "ON A JOURNEY BEYOND THE HILLS...", restante);
        break;
    default:
        break;
    }
}

void pantalla_intro_iniciar(void)
{
    escena = 0;
    preparar_escena();
    fade_in(FRAMES_FADE);
}

void pantalla_intro_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))
    {
        fade_out(FRAMES_FADE);
        personajes_ocultar();
        secuencia_cambiar(PANTALLA_TITULO);
        return;
    }

    if ((frames_en_pantalla % FRAMES_POR_LETRA) == 0) /* Máquina de escribir.             */
    {
        letras++;
        escribir_parrafo();
    }

    personajes_animar(); /* Solo hace algo en la escena 3.      */

    if ((frames_en_pantalla - inicio_escena) >= DURACION_ESCENA_INTRO)
    {
        if (escena < 2)
        {
            fade_out(FRAMES_FADE);
            escena++;
            preparar_escena(); /* Cambia de modo/imagen en negro.     */
            fade_in(FRAMES_FADE);
        }
        else
        {
            personajes_ocultar();
            /* Mosaico sobre las dos capas visibles (texto = fondo 0, imagen = fondo 1).
               MOSAIC_OUT agranda los bloques de 0 a 15 (la doc del SDK está
               invertida; verificado en videos.asm). */
            setMosaicEffect(MOSAIC_OUT, MOSAIC_BG1 | MOSAIC_BG2);
            fade_out(FRAMES_FADE);
            REG_MOSAIC = 0; /* Apagar el mosaico ya en negro.      */
            secuencia_cambiar(PANTALLA_TITULO);
        }
    }
}
