/*---------------------------------------------------------------------------------
    pantalla_publisher.c — Pantalla (2): logo del publisher, 3 segundos.
    Provisional: logo de Capcom del ejemplo del SDK, ampliado x2 sobre negro
    (res/publisher/logo_publisher.png, 16 colores, paleta 1, fondo 1).
    El jingle llega con el soundbank (tarea de audio).
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"

extern char publisher_til, publisher_til_end, publisher_pal, publisher_map, publisher_map_end;

void pantalla_publisher_iniciar(void)
{
    video_texto_mode1();
    fondo16_cargar(1, &publisher_til, &publisher_til_end, &publisher_pal, 1,
                   &publisher_map, &publisher_map_end, VRAM_FONDO_A, VRAM_MAPA_B, SC_32x32);
    limpiar_texto();                                 /* Sin texto: solo el logo.            */
    fade_in(FRAMES_FADE);
}

void pantalla_publisher_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_DEVS);
        return;
    }

    if (frames_en_pantalla >= DURACION_PUBLISHER)
    {
        fade_out(FRAMES_FADE);
        secuencia_cambiar(PANTALLA_DEVS);
    }
}
