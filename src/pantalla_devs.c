/*---------------------------------------------------------------------------------
    pantalla_devs.c — Pantalla (3): logo del estudio (provisional, estilo Konami)
    entrando con zoom y rotación en Mode 7, y debajo "ALEJANDRO MORA" con roles.
    4 segundos: 2 s de animación Mode 7 + 2 s quieto. Entra desde negro, sale
    con fade.

    ESTADO: marcador de texto. La tarea 4 del plan cambia a Mode 7 con la imagen
    de 256 colores (res/devs/logo_devs.png) y vuelve a Mode 1 al salir.
    → glosario: Mode 7
---------------------------------------------------------------------------------*/
#include "pantallas.h"
#include "secuencia.h"
#include "comun.h"

void pantalla_devs_iniciar(void)
{
    limpiar_texto();
    texto_centrado(7,  "[ STUDIO LOGO ]");           /* MARCADOR: imagen Mode 7 con zoom.   */
    texto_centrado(9,  "MODE 7 ZOOM + ROTATION");    /* Recordatorio del efecto previsto.   */
    texto_centrado(15, "ALEJANDRO MORA");            /* Nombre del desarrollador.           */
    texto_centrado(17, "PROGRAMMING - ART - MUSIC"); /* Roles (hoy todos la misma persona). */
    setBrightness(15);                               /* Aparece de golpe (sin fade): así
                                                        arrancará el zoom desde negro.      */
}

void pantalla_devs_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))  /* Salto opcional (desactivado).       */
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_DEDICATORIA);
        return;
    }

    if (frames_en_pantalla >= DURACION_DEVS)         /* Fin por tiempo (4 s).               */
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_DEDICATORIA);
    }
}
