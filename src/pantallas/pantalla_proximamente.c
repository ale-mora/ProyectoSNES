/*---------------------------------------------------------------------------------
    pantalla_proximamente.c — Pantalla (7): "LEVEL 1 - COMING SOON".

    PROPÓSITO
      Es el enchufe donde arrancará el gameplay: hoy espera Start y vuelve al
      título. En la Revisión 1 (docs/diseno-demo-jugable.md) se reemplaza por
      pantalla_nivel1.c; el resto de la secuencia no cambia.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h.
      Lo usan:  sistema/secuencia.c.

    HARDWARE QUE TOCA
      PPU a través de comun.c (Mode 1, texto, brillo). Mando: padsDown().

    VARIABLES (globales y estáticas de este archivo)
      ninguna.

    → glosario: Pantalla, Mando
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
