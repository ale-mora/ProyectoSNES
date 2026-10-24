/*---------------------------------------------------------------------------------
    pantalla_publisher.c — Pantalla (2): logo del publisher, con el motor de escenas.

    PROPÓSITO
      Provisional: logo de Capcom del ejemplo del SDK, ampliado x2 sobre negro
      (res/publisher/logo_publisher.png, 16 colores, paleta 1, fondo 1). El
      jingle llegará con el soundbank (sesión de audio).
      Desde la unidad 12 (ampliación del ejercicio) no usa fade_in/fade_out
      (que bloquean): la tabla escena/escena_publisher.c mueve el brillo y el
      mosaico, y escena/efectos.c los lleva a la PPU en el VBlank. Dura lo
      mismo que antes (244 frames).

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h,
                escena/escena_publisher.h, escena/efectos.h.
      Lee de la ROM: publisher_til/_til_end/_pal/_map/_map_end (data.asm).
      Lo usan:  sistema/secuencia.c (llama a _iniciar, _actualizar y _vblank).

    HARDWARE QUE TOCA
      PPU y VRAM a través de comun.c. Brillo y mosaico a través de
      escena/efectos.c. Mando: padsDown() si PERMITE_SALTO.

    VARIABLES (globales y estáticas de este archivo)
      escena  Escena  el estado del motor (pistas, próximo evento, reloj)

    → glosario: BG / capas, Paleta, Fade / brillo, Mosaico, Motor de escenas
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "escena/escena_publisher.h"
#include "escena/efectos.h"

extern char publisher_til, publisher_til_end, publisher_pal, publisher_map, publisher_map_end;

#define FONDO_LOGO 0x02                              /* Bit del fondo 1 (el del logo).      */

static Escena escena;

void pantalla_publisher_iniciar(void)
{
    video_texto_mode1();
    fondo16_cargar(1, &publisher_til, &publisher_til_end, &publisher_pal, 1,
                   &publisher_map, &publisher_map_end, VRAM_FONDO_A, VRAM_MAPA_B, SC_32x32);
    limpiar_texto();                                 /* Sin texto: solo el logo.            */
    efectos_preparar(FONDO_LOGO);                    /* Mosaico solo en el logo; sin destello. */
    escena_empezar(&escena, ESCENA_PUBLISHER);       /* Brillo 0: la escena lo sube.        */
}

/* Apaga los efectos (el mosaico acaba en 15) ANTES de preparar la siguiente. */
static void salir(void)
{
    efectos_terminar();
    secuencia_cambiar(PANTALLA_DEVS);
}

void pantalla_publisher_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))  /* Salto opcional (desactivado).       */
    {
        efectos_terminar();                          /* Sin mosaico durante el fundido.     */
        fade_out(FRAMES_FADE);
        salir();
        return;
    }

    escena_avanzar(&escena);                         /* Los valores de este frame.          */
    if (escena.terminada)                            /* Llegó al FIN_ESCENA (frame 244),    */
    {                                                /* con el brillo ya en 0.              */
        salir();
    }
}

void pantalla_publisher_vblank(void)
{
    efectos_aplicar(&escena);                        /* Los valores, a los registros.       */
}
