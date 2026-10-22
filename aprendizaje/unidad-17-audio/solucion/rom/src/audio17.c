/*---------------------------------------------------------------------------------
    audio17.c — Mini-ROM de la unidad 17 (SOLUCIÓN): música de fondo, un efecto
    con un botón, pausa/reanudar y volumen, todo desde C con PVSnesLib.

    PROPÓSITO
      Arranca el driver de sonido en el SPC700, carga el módulo overworld.it
      del soundbank y lo reproduce en bucle. A dispara el efecto del salto
      (mariojump.brr), Start pausa y reanuda la música en el mismo punto y
      arriba/abajo suben y bajan su volumen. La pantalla dice qué hace cada
      botón y el estado actual (texto en inglés, regla del proyecto).

    SE CONECTA CON
      Incluye:  <snes.h> (spc*, consola de texto, padsDown, KEY_*),
                res/soundbank.h (MOD_OVERWORLD: lo GENERA smconv al compilar).
      Lee de la ROM: SOUNDBANK__ (res/soundbank.asm), snesfont, snespal,
                saltobrr, saltobrr_fin (data.asm).
      Modelo:   tools/pvsneslib/snes-examples/systems/games/likemario/src/LikeMario.c
                (soundbank + efecto BRR) y snes-examples/audio/music2 (pausa).
      Lo usará: el juego, cuando src/audio/audio.c deje de ser un stub.

    HARDWARE QUE TOCA
      SPC700 y su RAM de 64 KB, a través del driver snesmod y de los cuatro
      puertos $2140-$2143 (spcBoot, spcLoad, spcProcess...). PPU: Mode 1 y la
      consola de texto en BG1. Mando 1 (padsDown, leído por la NMI del SDK).

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
    consoleDrawText(4, 17, "VOLUME: %d  ", volumen); /* Espacios: borran cifras viejas. */
}

int main(void)                                       /* La librería exige int main.     */
{
    /*--- 1. Arrancar el SPC700: copia el driver snesmod a su RAM (tarda un poco) ----*/
    spcBoot();

    /*--- 2. La consola de texto en BG1 (igual que los ejemplos de audio del SDK) ----*/
    consoleSetTextMapPtr(0x6800);                    /* Mapa del texto en VRAM $6800.   */
    consoleSetTextGfxPtr(0x3000);                    /* Tiles de la fuente en $3000.    */
    consoleSetTextOffset(0x0100);                    /* Los tiles empiezan en el 256.   */
    consoleInitText(0, 2 * 2, &snesfont, &snespal);  /* Paleta 0: 2 colores x 2 bytes. */
    bgSetGfxPtr(0, 0x2000);                          /* BG1: base de tiles.             */
    bgSetMapPtr(0, 0x6800, SC_32x32);                /* BG1: el mapa de la consola.     */
    setMode(BG_MODE1, 0);                            /* Mode 1.                         */
    bgSetDisable(1);                                 /* Solo BG1 visible.               */
    bgSetDisable(2);

    /*--- 3. El soundbank, la región de efectos, el módulo y el efecto ---------------*/
    spcSetBank(&SOUNDBANK__);                        /* Dónde empieza el soundbank.     */
    spcAllocateSoundRegion(REGION_SONIDO);           /* RAM del SPC para efectos BRR.   */
                                                     /* (Para la música: va ANTES.)     */
    spcLoad(MOD_OVERWORLD);                          /* Copia el módulo 0 al SPC700.    */
    spcSetSoundEntry(15, 8, 6,                       /* Volumen 15, centro (8), tono 6  */
                     &saltobrr_fin - &saltobrr,      /* (6 x 2 kHz = 12 kHz), longitud, */
                     &saltobrr, &salto);             /* dirección y ficha de salida.    */

    /*--- 4. El texto fijo (en inglés) -----------------------------------------------*/
    consoleDrawText(4, 4, "UNIT 17 - AUDIO FROM C");
    consoleDrawText(4, 8, "A      JUMP SOUND");
    consoleDrawText(4, 9, "START  PAUSE / RESUME");
    consoleDrawText(4, 10, "UP     VOLUME +");
    consoleDrawText(4, 11, "DOWN   VOLUME -");
    setScreenOn();                                   /* Encender la pantalla.           */

    /*--- 5. Música de fondo desde el principio --------------------------------------*/
    volumen = VOLUMEN_INICIAL;                       /* Las globales no empiezan en 0:  */
    pausada = 0;                                     /* se inicializan aquí (§6.6).     */
    spcPlay(0);                                      /* Encola "tocar desde la pos. 0". */
    spcSetModuleVolume(volumen);                     /* Encola el volumen.              */
    dibujar_estado();

    while (1)                                        /* El bucle del juego: 60 vueltas  */
    {                                                /* por segundo.                    */
        pulsados = padsDown(0);                      /* Solo lo que bajó este frame.    */

        /*--- 6. A: el efecto del salto ----------------------------------------------*/
        if (pulsados & KEY_A)
        {
            spcPlaySound(0);                         /* 0 = el ÚLTIMO efecto cargado.   */
        }

        /*--- 7. Start: pausa / reanudar ---------------------------------------------*/
        if (pulsados & KEY_START)
        {
            if (pausada)
            {
                spcResumeMusic();                    /* Sigue donde se quedó.           */
                pausada = 0;
            }
            else
            {
                spcPauseMusic();                     /* Para y guarda la posición.      */
                pausada = 1;
            }
            dibujar_estado();
        }

        /*--- 8. Arriba / abajo: volumen, recortado a 0..255 -------------------------*/
        if (pulsados & KEY_UP)
        {
            if (volumen <= 255 - PASO_VOLUMEN)       /* ¿Cabe otro paso sin pasar de    */
            {                                        /* 255? (un u8 daría la vuelta).   */
                volumen = (u8)(volumen + PASO_VOLUMEN);
            }
            else
            {
                volumen = 255;                       /* El máximo.                      */
            }
            spcSetModuleVolume(volumen);
            dibujar_estado();
        }
        if (pulsados & KEY_DOWN)
        {
            if (volumen >= PASO_VOLUMEN)             /* ¿Cabe un paso sin bajar de 0?   */
            {
                volumen = (u8)(volumen - PASO_VOLUMEN);
            }
            else
            {
                volumen = 0;                         /* Silencio.                       */
            }
            spcSetModuleVolume(volumen);
            dibujar_estado();
        }

        /*--- 9. Entregar los mensajes al SPC700 y esperar al VBlank -----------------*/
        spcProcess();                                /* SIN esto no suena nada nuevo.   */
        WaitForVBlank();                             /* Un frame.                       */
    }
    return 0;                                        /* Nunca se llega aquí.            */
}
