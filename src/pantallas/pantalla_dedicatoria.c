/*---------------------------------------------------------------------------------
    pantalla_dedicatoria.c — Pantalla (4): mensaje personal sobre negro, letra por
    letra, con fundidos lentos. 7 segundos.

    PROPÓSITO
      Efecto máquina de escribir sobre fondo negro con el texto definitivo
      "A la memoria de Tarou". La música suave llegará con el soundbank (sesión
      de audio).

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h.
      Lo usan:  sistema/secuencia.c.

    HARDWARE QUE TOCA
      PPU a través de comun.c (Mode 1, consola de texto, brillo).

    VARIABLES (globales y estáticas de este archivo)
      letras_mostradas  u16  0..DURACION_DEDICATORIA/FRAMES_POR_LETRA  letras del mensaje ya visibles

    → glosario: Consola de texto, Fade / brillo, Frame
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u16 letras_mostradas; /* Cuántas letras del mensaje ya se ven.                  */

void pantalla_dedicatoria_iniciar(void)
{
    video_texto_mode1(); /* Volvemos de Mode 7: Mode 1 y fuente.*/
    limpiar_texto();
    letras_mostradas = 0;       /* Empezamos sin ninguna letra.        */
    fade_in(FRAMES_FADE_LENTO); /* Fundido lento (~1 s) desde negro.   */
}

void pantalla_dedicatoria_actualizar(void)
{
    /* Máquina de escribir: cada FRAMES_POR_LETRA frames mostramos una letra más.
       El operador % (módulo) da el resto de la división: vale 0 exactamente
       cada FRAMES_POR_LETRA frames. */
    if ((frames_en_pantalla % FRAMES_POR_LETRA) == 0)
    {
        letras_mostradas++; /* Una letra más...                    */
        /* ...y redibujamos la línea con esas letras. Texto definitivo de la
           dedicatoria, tal como lo escribió el autor (decisión del 2026-09-24). */
        escribir_parcial(13, "A la memoria de Tarou", letras_mostradas);
    }

    if (frames_en_pantalla >= DURACION_DEDICATORIA) /* Fin por tiempo (7 s).               */
    {
        fade_out(FRAMES_FADE_LENTO); /* Fundido lento a negro.              */
        secuencia_cambiar(PANTALLA_INTRO);
    }
}
