/*---------------------------------------------------------------------------------
    pantalla_nintendo.c — Pantalla (1): logo "Nintendo" y "LICENSED BY NINTENDO".

    PROPÓSITO
      Muda, como en los juegos de la época, y es la primera que usa el MOTOR DE
      ESCENAS (src/escena/): en vez de fade_in, esperar y fade_out (que paran
      el programa mientras duran), la pantalla carga el logo y deja que una
      tabla de eventos (escena/escena_nintendo.c) mueva el brillo, el mosaico
      y un destello de luz que cruza el logo. Dura lo mismo que antes del
      motor (244 frames), para no correr el resto del arranque.
      El logo (res/nintendo/logo_nintendo.png, 256x224, 16 colores) va en el
      fondo 1 con la paleta 1; el texto en el fondo 0 (consola), por encima, y
      sin efectos.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h,
                escena/escena_nintendo.h, escena/efectos.h.
      Lee de la ROM: nintendo_til/_til_end/_pal/_map/_map_end (data.asm).
      Lo usan:  sistema/secuencia.c (llama a _iniciar, _actualizar y _vblank).

    HARDWARE QUE TOCA
      PPU y VRAM a través de comun.c (modo, fondo, texto). Brillo, mosaico,
      ventana, matemática de color y HDMA a través de escena/efectos.c. Mando:
      padsDown() solo si PERMITE_SALTO está activo.

    VARIABLES (globales y estáticas de este archivo)
      escena  Escena  el estado del motor (pistas, próximo evento, reloj)

    → glosario: BG / capas, Paleta, Fade / brillo, Mando, Motor de escenas
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "escena/escena_nintendo.h"
#include "escena/efectos.h"

/* Etiquetas de data.asm: dónde empiezan (y terminan) los tiles, la paleta y el mapa. */
extern char nintendo_til, nintendo_til_end, nintendo_pal, nintendo_map, nintendo_map_end;

/* El rectángulo rojo del logo en la pantalla (medido en logo_nintendo.png): el
   destello solo existe ahí dentro. */
#define LOGO_ARRIBA 64
#define LOGO_ALTO 59                                 /* Líneas 64..122.                     */
#define LOGO_X_MIN 40
#define LOGO_X_MAX 215
#define DESTELLO_ANCHO 12                            /* Ancho de la banda de luz, en px.    */
#define FONDO_LOGO 0x02                              /* Bit del fondo 1 (el del logo).      */

static Escena escena;

void pantalla_nintendo_iniciar(void)
{
    video_texto_mode1();                             /* Mode 1, texto en el fondo 0.        */
    fondo16_cargar(1, &nintendo_til, &nintendo_til_end, &nintendo_pal, 1,
                   &nintendo_map, &nintendo_map_end, VRAM_FONDO_A, VRAM_MAPA_B, SC_32x32);
    limpiar_texto();
    texto_centrado(17, "LICENSED BY NINTENDO");      /* Debajo del logo (fila 17 = y 136). */

    efectos_preparar(FONDO_LOGO);                    /* Mosaico y luz, solo en el logo.     */
    efectos_destello(LOGO_ARRIBA, LOGO_ALTO, LOGO_X_MIN, LOGO_X_MAX, DESTELLO_ANCHO);
    escena_empezar(&escena, ESCENA_NINTENDO);        /* Brillo 0: la escena lo sube.        */
}

/* Apaga los efectos ANTES de que la pantalla siguiente se prepare. */
static void salir(void)
{
    efectos_terminar();
    secuencia_cambiar(PANTALLA_PUBLISHER);
}

void pantalla_nintendo_actualizar(void)
{
    if (PERMITE_SALTO && (padsDown(0) & KEY_START))  /* Salto opcional (desactivado).       */
    {
        efectos_terminar();                          /* Sin luz ni mosaico durante el fundido. */
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

void pantalla_nintendo_vblank(void)
{
    efectos_aplicar(&escena);                        /* Los valores, a los registros.       */
}
