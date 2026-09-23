/*---------------------------------------------------------------------------------
    pantalla_intro.c — Pantalla (5): intro en tres escenas. Cada escena muestra un
    fondo y un párrafo corto que aparece letra por letra. En la tercera aparece
    Tarouh cachorro (sprite). 8 segundos por escena. Sale hacia el título con
    efecto mosaico.

    ESTADO: marcador. Los fondos (BG1), la franja de texto en BG3 y el sprite
    llegan en la tarea 6 del plan. Los párrafos son un borrador en inglés que el
    usuario corregirá; no comprometen el guion definitivo.
    → glosario: Mosaico, Sprite
---------------------------------------------------------------------------------*/
#include "pantallas.h"
#include "secuencia.h"
#include "comun.h"

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u8  escena;               /* Escena actual: 0, 1 o 2.                               */
static u16 letras;               /* Letras del párrafo ya mostradas.                       */
static u16 restante;             /* Letras que sobran para la línea siguiente (encadenado).*/
static u16 inicio_escena;        /* Frame en que empezó la escena actual.                  */

/*--- Dibuja el marco de una escena (sin el texto, que crece frame a frame) ------*/
static void preparar_escena(void)
{
    limpiar_texto();
    switch (escena)                                  /* Rótulo del fondo que irá en BG1.    */
    {
        case 0: texto_centrado(3, "[ SCENE 1: THE VILLAGE ]"); break;
        case 1: texto_centrado(3, "[ SCENE 2: THE WIND ]");    break;
        case 2: texto_centrado(3, "[ SCENE 3: THE ROAD ]");
                texto_centrado(9, "[ TAROUH SPRITE ]");        /* MARCADOR del sprite.     */
                break;
        default: break;
    }
    letras = 0;                                      /* El párrafo empieza vacío.           */
    inicio_escena = frames_en_pantalla;              /* Para medir los 8 s de esta escena.  */
}

/*--- Escribe el párrafo de la escena actual con `letras` letras visibles --------
    Cada línea consume letras y pasa el sobrante a la siguiente. */
static void escribir_parrafo(void)
{
    switch (escena)
    {
        case 0:
            restante = escribir_parcial(18, "IN A QUIET VILLAGE BY THE HILLS", letras);
            restante = escribir_parcial(20, "LIVED A PUPPY NAMED TAROUH",      restante);
            restante = escribir_parcial(22, "AND HIS BEST FRIEND, BARBARA.",   restante);
            break;
        case 1:
            restante = escribir_parcial(18, "ONE MORNING, A STRANGE WIND",     letras);
            restante = escribir_parcial(20, "SWEPT THROUGH THE VALLEY",        restante);
            restante = escribir_parcial(22, "AND TOOK SOMETHING PRECIOUS.",    restante);
            break;
        case 2:
            restante = escribir_parcial(18, "TOGETHER, THEY SET OUT",          letras);
            restante = escribir_parcial(20, "ON A JOURNEY BEYOND THE HILLS...", restante);
            break;
        default: break;
    }
}

void pantalla_intro_iniciar(void)
{
    escena = 0;                                      /* Siempre arranca en la escena 1.     */
    preparar_escena();
    fade_in(FRAMES_FADE);
}

void pantalla_intro_actualizar(void)
{
    /* Salto opcional (desactivado por diseño). */
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_TITULO);
        return;
    }

    /* Máquina de escribir: una letra más cada FRAMES_POR_LETRA frames. */
    if ((frames_en_pantalla % FRAMES_POR_LETRA) == 0)
    {
        letras++;
        escribir_parrafo();
    }

    /* ¿Terminó el tiempo de esta escena? (frames desde que empezó la escena) */
    if ((frames_en_pantalla - inicio_escena) >= DURACION_ESCENA_INTRO)
    {
        if (escena < 2)                              /* Quedan escenas: pasamos a la otra.  */
        {
            fade_out(FRAMES_FADE);
            escena++;
            preparar_escena();
            fade_in(FRAMES_FADE);
        }
        else                                         /* Era la última: rumbo al título.     */
        {
            /* Mosaico: la capa de texto se vuelve bloques grandes (MOSAIC_BG1 es
               la primera capa, la que usa la consola). Después fundido a negro. */
            setMosaicEffect(MOSAIC_IN, MOSAIC_BG1);
            fade_out(FRAMES_FADE);
            setMosaicEffect(MOSAIC_OUT, MOSAIC_BG1); /* Deshacemos el mosaico ya en negro,
                                                        para que el título salga nítido.    */
            secuencia_cambiar(PANTALLA_TITULO);
        }
    }
}
