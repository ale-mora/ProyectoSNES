/*---------------------------------------------------------------------------------
    pantalla_dedicatoria.c — Pantalla (4): mensaje personal sobre negro, letra por
    letra (máquina de escribir), con fundidos lentos y música suave. 7 segundos.

    ESTADO: el texto definitivo lo entrega el usuario; mientras, el marcador
    "For ..." . La música llega con el soundbank (tarea 5 del plan).
---------------------------------------------------------------------------------*/
#include "pantallas.h"
#include "secuencia.h"
#include "comun.h"

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u16 letras_mostradas;   /* Cuántas letras del mensaje ya se ven.                  */

void pantalla_dedicatoria_iniciar(void)
{
    limpiar_texto();
    letras_mostradas = 0;                            /* Empezamos sin ninguna letra.        */
    fade_in(FRAMES_FADE_LENTO);                      /* Fundido lento (~1 s) desde negro.   */
}

void pantalla_dedicatoria_actualizar(void)
{
    /* Máquina de escribir: cada FRAMES_POR_LETRA frames mostramos una letra más.
       El operador % (módulo) da el resto de la división: vale 0 exactamente
       cada FRAMES_POR_LETRA frames. */
    if ((frames_en_pantalla % FRAMES_POR_LETRA) == 0)
    {
        letras_mostradas++;                          /* Una letra más...                    */
        /* ...y redibujamos la línea con esas letras. MARCADOR: reemplazar por el
           texto real de la dedicatoria (en inglés) cuando el usuario lo entregue. */
        escribir_parcial(13, "For ...", letras_mostradas);
    }

    if (frames_en_pantalla >= DURACION_DEDICATORIA)  /* Fin por tiempo (7 s).               */
    {
        fade_out(FRAMES_FADE_LENTO);                 /* Fundido lento a negro.              */
        secuencia_cambiar(PANTALLA_INTRO);
    }
}
