/*---------------------------------------------------------------------------------
    pantalla_proximamente.c — Pantalla (7): "LEVEL 1 - COMING SOON". Es el enchufe
    donde en el futuro arrancará el gameplay. Hoy: espera Start y vuelve al título.

    Cuando exista el Nivel 1, esta pantalla se reemplaza por la carga del nivel
    (Etapa 2/3 del plan); el resto de la secuencia no cambia.
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"

void pantalla_proximamente_iniciar(void)
{
    video_texto_mode1();                             /* Solo texto, Mode 1.                 */
    limpiar_texto();
    texto_centrado(12, "LEVEL 1 - COMING SOON");     /* Aviso de que aún no hay juego.      */
    texto_centrado(16, "PRESS START TO RETURN");     /* Cómo volver al título.              */
    fade_in(FRAMES_FADE);
}

void pantalla_proximamente_actualizar(void)
{
    if (padsDown(0) & KEY_START)                     /* Start recién pulsado...             */
    {
        fade_out(FRAMES_FADE);                       /* ...fundido a negro...               */
        secuencia_cambiar(PANTALLA_TITULO);          /* ...y de vuelta al título.           */
    }
}
