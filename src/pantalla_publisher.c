/*---------------------------------------------------------------------------------
    pantalla_publisher.c — Pantalla (2): logo del publisher, 3 segundos, con jingle.
    Provisional: se usará el logo de Capcom del ejemplo del SDK
    (tools/pvsneslib/snes-examples/thirdparty/logo/snes-logo-capcom/).

    ESTADO: marcador de texto. La tarea 3 del plan trae la imagen y el primer
    módulo .it (soundbank), y con él spcBoot/spcLoad/spcPlay.
---------------------------------------------------------------------------------*/
#include "pantallas.h"
#include "secuencia.h"
#include "comun.h"

void pantalla_publisher_iniciar(void)
{
    limpiar_texto();
    texto_centrado(12, "[ PUBLISHER LOGO ]");        /* MARCADOR del logo (Capcom).         */
    texto_centrado(15, "+ JINGLE");                  /* Recordatorio: aquí sonará el jingle.*/
    fade_in(FRAMES_FADE);
}

void pantalla_publisher_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))  /* Salto opcional (desactivado).       */
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_DEVS);
        return;
    }

    if (frames_en_pantalla >= DURACION_PUBLISHER)    /* Fin por tiempo (3 s).               */
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_DEVS);
    }
}
