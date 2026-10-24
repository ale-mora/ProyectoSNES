/*---------------------------------------------------------------------------------
    audio.h — Música y efectos de sonido (sesión de audio).

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
#ifndef AUDIO_H
#define AUDIO_H

#include "sistema/tipos.h"

/* Contrato previsto (sesión de audio):
void audio_iniciar(void);                    -- spcBoot + soundbank
void audio_musica(u8 id);                    -- spcLoad + spcPlay
void audio_efecto(u8 id);                    -- spcEffect
void audio_procesar(void);                   -- spcProcess, una vez por frame */

/* Única función real del stub: no hace nada. Se quitará al implementar el módulo. */
void audio_pendiente(void);

#endif /* AUDIO_H */
