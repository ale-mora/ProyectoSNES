/*---------------------------------------------------------------------------------
    pantalla_nintendo.c — Pantalla (1): logo "Nintendo" y "LICENSED BY NINTENDO".
    Muda, 3 segundos, fade in y fade out (como en los juegos de la época).

    ESTADO: marcador de texto. Cuando llegue res/nintendo/logo_nintendo.png, el
    texto "Nintendo" se reemplaza por la imagen cargada en BG1 (tarea 2 del plan).
---------------------------------------------------------------------------------*/
#include "pantallas.h"   /* Tiempos, enum Pantalla y prototipos de esta pantalla. */
#include "secuencia.h"   /* frames_en_pantalla y secuencia_cambiar().            */
#include "comun.h"       /* limpiar_texto, texto_centrado, fade_in/out.          */

/*--- iniciar: se ejecuta UNA vez al entrar --------------------------------------*/
void pantalla_nintendo_iniciar(void)
{
    limpiar_texto();                                 /* Pantalla de texto en blanco.       */
    texto_centrado(11, "Nintendo");                  /* MARCADOR: aquí irá el logo rojo.   */
    texto_centrado(14, "LICENSED BY NINTENDO");      /* Texto de licencia debajo del logo. */
    fade_in(FRAMES_FADE);                            /* Aparece desde negro (~0,5 s).      */
}

/*--- actualizar: se ejecuta UNA vez por frame -----------------------------------*/
void pantalla_nintendo_actualizar(void)
{
    /* Salto opcional (desactivado por diseño): si PERMITE_SALTO es 1 y se pulsa
       Start, terminamos antes. padsDown(0) = botones recién pulsados del mando 1;
       "& KEY_START" pregunta solo por el bit de Start. → glosario: Mando */
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))
    {
        fade_out(FRAMES_FADE);                       /* A negro...                          */
        secuencia_cambiar(PANTALLA_PUBLISHER);       /* ...y a la siguiente pantalla.       */
        return;                                      /* Nada más que hacer este frame.      */
    }

    /* Fin por tiempo: cuando el contador llega a la duración, salimos. */
    if (frames_en_pantalla >= DURACION_NINTENDO)
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_PUBLISHER);
    }
}
