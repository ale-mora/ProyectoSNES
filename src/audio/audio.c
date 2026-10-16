/*---------------------------------------------------------------------------------
    audio.c — Música y efectos de sonido (sesión de audio).

    PROPÓSITO
      PENDIENTE: sesión de audio — arrancar el chip de sonido, cargar el soundbank (módulos .it convertidos por smconv), reproducir música por pantalla y efectos por evento (salto, espada).
      Este archivo es un STUB: compila, reserva el sitio del subsistema y documenta
      el contrato previsto. No contiene lógica; la única función real,
      audio_pendiente(), existe para que el enlazador vea el objeto.

    SE CONECTA CON
      Incluirá: <snes.h> (spcBoot, spcLoad, spcPlay, spcProcess, spcEffect) y soundbank.h generado.
      Lo usarán: sistema/main.c (audio_iniciar y audio_procesar por frame), cada pantalla (música) y pantalla_nivel1 (efectos).

    HARDWARE QUE TOCA
      SPC700: el procesador de sonido y sus 64 KB de RAM, por el driver de PVSnesLib.

    VARIABLES (globales y estáticas de este archivo)
      ninguna todavía.

    → glosario: Stub, Soundbank / módulo .it, SNES / Super Nintendo
---------------------------------------------------------------------------------*/
#include "audio/audio.h"

void audio_pendiente(void)
{
    /* PENDIENTE: sesión de audio. Ver el contrato previsto en audio.h. */
}
