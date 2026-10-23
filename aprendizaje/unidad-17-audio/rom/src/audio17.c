/*---------------------------------------------------------------------------------
    audio17.c — Mini-ROM de la unidad 17 (EJERCICIO): música de fondo, un efecto
    con un botón, pausa/reanudar y volumen, todo desde C con PVSnesLib.

    ENUNCIADO
      Tal como está, la ROM compila, enseña los controles y el estado... y no
      suena. Completa los TODO 1..9 (en orden) con las funciones de sound.h que
      explica el README de la unidad, sección «La librería: PVSnesLib». Al
      terminar, en Mesen:
        - la música (overworld.it) suena sola al arrancar y da la vuelta;
        - A dispara el efecto del salto (mariojump.brr), aunque la música esté
          en pausa;
        - Start pausa la música y la reanuda DONDE SE QUEDÓ (no desde el
          principio) y la línea MUSIC cambia entre PLAYING y PAUSED;
        - arriba/abajo cambian VOLUME de 16 en 16 entre 0 y 255 sin dar la
          vuelta (255 + 16 en un u8 sería 15) y la música suena más fuerte o
          más flojo.
      La solución está en ../solucion/rom/src/audio17.c (mírala después).

    SE CONECTA CON
      Incluye:  <snes.h> (spc*, consola de texto, padsDown, KEY_*),
                res/soundbank.h (MOD_OVERWORLD: lo GENERA smconv al compilar).
      Lee de la ROM: SOUNDBANK__ (res/soundbank.asm), snesfont, snespal,
                saltobrr, saltobrr_fin (data.asm).
      Modelo:   tools/pvsneslib/snes-examples/systems/games/likemario/src/LikeMario.c
                (soundbank + efecto BRR) y snes-examples/audio/music2 (pausa).

    HARDWARE QUE TOCA
      SPC700 y su RAM de 64 KB, a través del driver snesmod y de los cuatro
      puertos $2140-$2143 (cuando completes los TODO). PPU: Mode 1 y la consola
      de texto en BG1. Mando 1 (padsDown, leído por la NMI del SDK).

    VARIABLES (globales de este archivo)
      volumen   u8   0..255, de 16 en 16   volumen del módulo (spcSetModuleVolume)
      pausada   u8   0 / 1                 1 = la música está en pausa
      pulsados  u16  bits KEY_*            botones que bajaron en este frame
      salto     brrsamples (8 bytes)       ficha del efecto que rellena spcSetSoundEntry

    → glosario: SNES / Super Nintendo, Soundbank / módulo .it, Mando (pad), VBlank
---------------------------------------------------------------------------------*/
#include <snes.h>                    /* Toda PVSnesLib: sound.h, textfont.h, input.h...  */
#include "res/soundbank.h"           /* MOD_OVERWORLD = 0: lo genera smconv.              */

extern char snesfont, snespal;       /* La fuente de la consola (data.asm).               */
extern char SOUNDBANK__;             /* Primer byte del soundbank (res/soundbank.asm).    */
extern char saltobrr, saltobrr_fin;  /* Principio y fin de la muestra BRR (data.asm).     */

#define VOLUMEN_INICIAL 128          /* La mitad del máximo (255).                        */
#define PASO_VOLUMEN 16              /* Cuánto sube o baja cada pulsación.                */
#define REGION_SONIDO 39             /* 39 bloques de 256 bytes (~10 KB) de RAM del SPC   */
                                     /* para los efectos BRR (como en LikeMario).         */

brrsamples salto;                    /* Ficha del efecto: tono, volumen, dirección...     */
u8 volumen;                          /* Volumen actual de la música (0..255).             */
u8 pausada;                          /* 0 = sonando, 1 = en pausa.                         */
u16 pulsados;                        /* Botones recién pulsados en este frame.            */

/*--- dibujar_estado: las dos líneas que cambian (música y volumen) ------------------*/
static void dibujar_estado(void)
{
    if (pausada)                                     /* Sin ?: con cadenas en printf-  */
    {                                                /* like: tcc816 se cuelga (§6.6). */
        consoleDrawText(4, 16, "MUSIC:  PAUSED ");   /* Espacio final: borra la "G".   */
    }
    else
    {
        consoleDrawText(4, 16, "MUSIC:  PLAYING");
    }
    consoleDrawText(4, 17, "VOLUME: %d  ", (u16)volumen); /* (u16): un u8 en "..." */
                                                     /* sale con basura (README). */
}

int main(void)                                       /* La librería exige int main.     */
{
    /* TODO 1: arranca el SPC700 (copia el driver a su RAM). Va lo PRIMERO: tarda
               y desactiva interrupciones mientras dura.                             */

    /*--- La consola de texto en BG1 (igual que los ejemplos de audio del SDK) -------*/
    consoleSetTextMapPtr(0x6800);                    /* Mapa del texto en VRAM $6800.   */
    consoleSetTextGfxPtr(0x3000);                    /* Tiles de la fuente en $3000.    */
    consoleSetTextOffset(0x0100);                    /* Los tiles empiezan en el 256.   */
    consoleInitText(0, 2 * 2, &snesfont, &snespal);  /* Paleta 0: 2 colores x 2 bytes. */
    {
        setPaletteColor(0, RGB5(0, 0, 0));           /* El color 0 de la fuente es el   */
    }                                                /* magenta clave del .bmp y el 0 de */
                                                     /* la CGRAM es el fondo: negro.     */
    bgSetGfxPtr(0, 0x2000);                          /* BG1: base de tiles.             */
    bgSetMapPtr(0, 0x6800, SC_32x32);                /* BG1: el mapa de la consola.     */
    setMode(BG_MODE1, 0);                            /* Mode 1.                         */
    bgSetDisable(1);                                 /* Solo BG1 visible.               */
    bgSetDisable(2);

    /* TODO 2: di al driver dónde empieza el soundbank (la etiqueta SOUNDBANK__) y
               reserva REGION_SONIDO bloques de RAM del SPC para los efectos BRR.
               Ojo al orden: reservar la región PARA la música, así que va antes de
               cargarla.                                                             */

    /* TODO 3: carga el módulo MOD_OVERWORLD (de res/soundbank.h) en el SPC700.     */

    /* TODO 4: registra el efecto: volumen 15, panorámica 8 (centro), tono 6
               (6 x 2 kHz = 12 kHz, la frecuencia del .wav), la longitud
               (&saltobrr_fin - &saltobrr), la dirección &saltobrr y la ficha &salto. */

    /*--- El texto fijo (en inglés) --------------------------------------------------*/
    consoleDrawText(4, 4, "UNIT 17 - AUDIO FROM C");
    consoleDrawText(4, 8, "A      JUMP SOUND");
    consoleDrawText(4, 9, "START  PAUSE / RESUME");
    consoleDrawText(4, 10, "UP     VOLUME +");
    consoleDrawText(4, 11, "DOWN   VOLUME -");
    setScreenOn();                                   /* Encender la pantalla.           */

    volumen = VOLUMEN_INICIAL;                       /* Las globales no empiezan en 0:  */
    pausada = 0;                                     /* se inicializan aquí (§6.6).     */
    /* TODO 5: toca la música desde la posición 0 y fija su volumen a `volumen`.     */
    dibujar_estado();

    while (1)                                        /* El bucle del juego: 60 vueltas  */
    {                                                /* por segundo.                    */
        pulsados = padsDown(0);                      /* Solo lo que bajó este frame.    */

        if (pulsados & KEY_A)
        {
            /* TODO 6: dispara el efecto. Pista: el índice 0 es el ÚLTIMO cargado.  */
        }

        if (pulsados & KEY_START)
        {
            /* TODO 7: si `pausada`, reanuda la música y pon pausada = 0; si no,
                       ponla en pausa (guardando la posición) y pausada = 1.         */
            dibujar_estado();
        }

        if (pulsados & KEY_UP)
        {
            /* TODO 8a: sube `volumen` PASO_VOLUMEN sin pasar de 255 (compara ANTES
                        de sumar: un u8 da la vuelta) y aplícalo a la música.        */
            dibujar_estado();
        }
        if (pulsados & KEY_DOWN)
        {
            /* TODO 8b: baja `volumen` PASO_VOLUMEN sin bajar de 0 y aplícalo.       */
            dibujar_estado();
        }

        /* TODO 9: entrega al SPC700 los mensajes encolados en este frame (sin esta
                   llamada, spcPlay, la pausa y los efectos se quedan en la cola).   */
        WaitForVBlank();                             /* Un frame.                       */
    }
    return 0;                                        /* Nunca se llega aquí.            */
}
