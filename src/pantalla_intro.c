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
#include "personajes.h"  /* Tarouh y Bárbara aparecen en la escena 3. */

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
                /* Los dos personajes, de pie sobre la línea de "suelo" (fila 17
                   de texto = píxel 136; la celda mide 64, así que y = 136 - 64 = 72). */
                personajes_mostrar(48, 72, 150, 72);
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

    personajes_animar();                             /* Solo hace algo si se ven (escena 3).*/

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
            personajes_ocultar();                    /* El título los vuelve a colocar.     */
            /* Mosaico: la capa de texto se vuelve bloques cada vez más grandes
               (MOSAIC_BG1 es la primera capa, la que usa la consola).
               OJO: la documentación del SDK está invertida respecto a su código
               (verificado en videos.asm el 2026-09-24): MOSAIC_OUT hace CRECER el
               mosaico de 0 a 15 y lo deja al máximo; MOSAIC_IN lo encoge de 15 a 0.
               Ambas bloquean unos 48 frames. */
            setMosaicEffect(MOSAIC_OUT, MOSAIC_BG1);
            fade_out(FRAMES_FADE);
            /* Ya en negro, apagamos el mosaico de golpe escribiendo 0 en su
               registro de la PPU (tamaño 0, ninguna capa): así el título sale
               nítido sin esperar otra animación. → glosario: Mosaico */
            REG_MOSAIC = 0;
            secuencia_cambiar(PANTALLA_TITULO);
        }
    }
}
