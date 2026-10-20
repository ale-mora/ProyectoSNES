/*---------------------------------------------------------------------------------
    pantalla_proximamente.c — Pantalla (8): "LEVEL 2 / COMING SOON".

    PROPÓSITO
      Al llegar a la salida del tutorial, el nivel 1 pasa aquí: el sitio del
      siguiente nivel, que aún no existe. Muestra dos líneas de texto unos
      segundos y vuelve al título. Cuando exista el nivel 2, la salida del
      nivel 1 apuntará a él y esta pantalla dejará de usarse.
      (Decisión C1 de docs/diseno-tutorial-movimiento.md.)

    SE CONECTA CON
      Incluye:  sistema/pantallas.h (DURACION_PROXIMAMENTE, FRAMES_FADE),
                sistema/secuencia.h (frames_en_pantalla, secuencia_cambiar),
                sistema/comun.h (video_texto_mode1, texto_centrado, fundidos).
      Lo usan:  sistema/secuencia.c; pantallas/pantalla_nivel1.c (salir).

    HARDWARE QUE TOCA
      PPU por comun.c: Mode 1 con la capa de texto, fundidos de brillo.

    VARIABLES: ninguna propia (cuenta con frames_en_pantalla).
    → glosario: Pantalla, Frame
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"

void pantalla_proximamente_iniciar(void)
{
    video_texto_mode1();                             /* Mode 1, solo la capa de texto.      */
    limpiar_texto();
    texto_centrado(12, "LEVEL 2");                   /* Texto en pantalla: en inglés.       */
    texto_centrado(14, "COMING SOON");
    fade_in(FRAMES_FADE);
}

void pantalla_proximamente_actualizar(void)
{
    if (frames_en_pantalla >= DURACION_PROXIMAMENTE) /* Pasado el tiempo, al título.        */
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_TITULO);
    }
}
