/*---------------------------------------------------------------------------------
    pantalla_devs.c — Pantalla (3): logo del estudio y créditos, 4 segundos.

    PROPÓSITO
      Logo propio del estudio (res/devs/logo_devs.png, 16 colores, paleta 1,
      fondo 1) y, debajo, el autor y los roles con la consola de texto. Entra de
      golpe (como los logos de la época) y sale con fade. Nota de diseño: la
      versión con zoom y rotación en Mode 7 se descartó (2026-09-24) porque no
      quedaba bien; el logo fijo es más limpio.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h.
      Lee de la ROM: devs_til/_til_end/_pal/_map/_map_end (data.asm).
      Lo usan:  sistema/secuencia.c.

    HARDWARE QUE TOCA
      PPU y VRAM a través de comun.c; brillo directo (setBrightness(15)).
      Mando: padsDown() si PERMITE_SALTO.

    VARIABLES (globales y estáticas de este archivo)
      ninguna.

    → glosario: BG / capas, Consola de texto, Fade / brillo, Mode 7
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"

extern char devs_til, devs_til_end, devs_pal, devs_map, devs_map_end;

void pantalla_devs_iniciar(void)
{
    video_texto_mode1();
    fondo16_cargar(1, &devs_til, &devs_til_end, &devs_pal, 1,
                   &devs_map, &devs_map_end, VRAM_FONDO_A, VRAM_MAPA_B, SC_32x32);
    limpiar_texto();
    texto_centrado(18, "ALEJANDRO MORA");
    texto_centrado(20, "PROGRAMMING - ART - MUSIC");
    setBrightness(15);                               /* Aparece de golpe.                   */
}

void pantalla_devs_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))
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
