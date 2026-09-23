/*---------------------------------------------------------------------------------
    pantalla_titulo.c — Pantalla (6): título del juego. Espera a que el jugador
    pulse Start. "PRESS START" parpadea. Si nadie toca nada durante 15 s, vuelve
    a la intro (modo "attract", como las máquinas de la época en las tiendas).

    ESTADO: marcador de texto. La tarea 7 del plan añade el logo generado por
    script (BG3), dos capas con parallax (BG1/BG2), cielo con degradado HDMA,
    el sprite de Tarouh y la música en bucle.
    → glosario: Mando, HDMA, BG / capas
---------------------------------------------------------------------------------*/
#include "pantallas.h"
#include "secuencia.h"
#include "comun.h"

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u16 frames_sin_input;   /* Frames seguidos sin tocar ningún botón (para el attract). */
static u8  frames_parpadeo;    /* Cuenta frames hasta el próximo cambio de PRESS START.    */
static u8  texto_visible;      /* 1 = PRESS START se ve, 0 = está oculto.                  */

void pantalla_titulo_iniciar(void)
{
    limpiar_texto();
    texto_centrado(5,  "TAROUH");                    /* MARCADOR del logo del título.       */
    texto_centrado(7,  "A DOG'S JOURNEY");
    texto_centrado(12, "[ PARALLAX + HDMA SKY ]");   /* Recordatorio de los efectos.        */
    texto_centrado(14, "[ TAROUH SPRITE ]");
    texto_centrado(26, "(C) 2026 STUDIO NAME");      /* MARCADOR: nombre del estudio.       */

    frames_sin_input = 0;                            /* Nadie ha tocado nada todavía.       */
    frames_parpadeo = 0;
    texto_visible = 1;                               /* Empezamos con el texto visible...   */
    texto_centrado(20, "PRESS START");               /* ...y lo dibujamos.                  */

    fade_in(FRAMES_FADE);
}

void pantalla_titulo_actualizar(void)
{
    /*--- 1. Start: pasamos a "coming soon" ---------------------------------------
        padsDown(0) tiene un bit por botón RECIÉN pulsado en este frame; "&"
        deja solo el bit de Start, y el if es verdadero si ese bit está a 1. */
    if (padsDown(0) & KEY_START)
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_PROXIMAMENTE);
        return;
    }

    /*--- 2. Attract: 15 s sin ningún botón → volvemos a la intro ---------------
        padsCurrent(0) es distinto de 0 si CUALQUIER botón está pulsado. */
    if (padsCurrent(0) != 0)
    {
        frames_sin_input = 0;                        /* Alguien tocó algo: reiniciamos.     */
    }
    else
    {
        frames_sin_input++;                          /* Un frame más de silencio.           */
    }

    if (frames_sin_input >= ATTRACT_SIN_INPUT)
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_INTRO);
        return;
    }

    /*--- 3. Parpadeo de PRESS START ---------------------------------------------
        Cada FRAMES_PARPADEO frames (0,5 s) alternamos entre mostrar el texto y
        taparlo con espacios de la misma longitud. */
    frames_parpadeo++;
    if (frames_parpadeo >= FRAMES_PARPADEO)
    {
        frames_parpadeo = 0;                         /* Reiniciamos la cuenta...            */
        texto_visible = !texto_visible;              /* ...e invertimos: 1→0 o 0→1.         */
        if (texto_visible)
        {
            texto_centrado(20, "PRESS START");
        }
        else
        {
            texto_centrado(20, "           ");       /* 11 espacios = mismo ancho.          */
        }
    }
}
