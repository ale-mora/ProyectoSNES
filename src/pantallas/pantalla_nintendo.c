/*---------------------------------------------------------------------------------
    pantalla_nintendo.c — Pantalla (1): logo "Nintendo" y "LICENSED BY NINTENDO".

    PROPÓSITO
      Muda, 3 segundos, fade in y fade out, como en los juegos de la época. El
      logo (res/nintendo/logo_nintendo.png, 256x224, 16 colores) va en el fondo 1
      con la paleta 1; el texto en el fondo 0 (consola), por encima.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h.
      Lee de la ROM: nintendo_til/_til_end/_pal/_map/_map_end (data.asm).
      Lo usan:  sistema/secuencia.c (llama a _iniciar y _actualizar).

    HARDWARE QUE TOCA
      PPU y VRAM a través de comun.c (modo, fondo, texto, brillo). Mando:
      padsDown() solo si PERMITE_SALTO está activo.

    VARIABLES (globales y estáticas de este archivo)
      ninguna.

    → glosario: BG / capas, Paleta, Fade / brillo, Mando
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"

/* Etiquetas de data.asm: dónde empiezan (y terminan) los tiles, la paleta y el mapa. */
extern char nintendo_til, nintendo_til_end, nintendo_pal, nintendo_map, nintendo_map_end;

void pantalla_nintendo_iniciar(void)
{
    video_texto_mode1();                             /* Mode 1, texto en el fondo 0.        */
    fondo16_cargar(1, &nintendo_til, &nintendo_til_end, &nintendo_pal, 1,
                   &nintendo_map, &nintendo_map_end, VRAM_FONDO_A, VRAM_MAPA_B, SC_32x32);
    limpiar_texto();
    texto_centrado(17, "LICENSED BY NINTENDO");      /* Debajo del logo (fila 17 = y 136). */
    fade_in(FRAMES_FADE);
}

void pantalla_nintendo_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))  /* Salto opcional (desactivado).       */
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_PUBLISHER);
        return;
    }

    if (frames_en_pantalla >= DURACION_NINTENDO)     /* Fin por tiempo (3 s).               */
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_PUBLISHER);
    }
}
