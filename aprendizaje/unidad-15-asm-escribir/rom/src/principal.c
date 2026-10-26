/*---------------------------------------------------------------------------------
    principal.c — Mini-ROM de la unidad 15: C llama a una rutina en ensamblador.

    PROPÓSITO
      Dos franjas de la pantalla se rellenan con un carácter repetido: la de
      arriba ('#') con la rutina en C (relleno_c.c) y la de abajo ('=') con la
      rutina en ensamblador (relleno.asm). Las dos se llaman dos veces, una por
      fila de 32 celdas, con los mismos argumentos de tamaño, para que el profiler
      de Mesen las compare llamada a llamada. Debajo, la consola escribe cuántas
      veces se llamó a la rutina en ensamblador (una variable que vive en el .asm).
      Además, al arrancar se vacía un búfer de 256 bytes dos veces: con el bucle
      en C (limpiar_c) y con memset (librería, en ensamblador). En pantalla no se
      ve; es para el profiler y para leer el .asm.
      CAPAS: el texto va en BG1 (la consola del SDK, mapa en 0x6800) y las franjas
      en BG2 (mapa propio en 0x6000, mismos tiles de la fuente). Así la consola,
      que en cada VBlank con texto nuevo copia su mapa ENTERO a la VRAM, no borra
      las franjas.

    SE CONECTA CON
      Incluye:  <snes.h>, <string.h> (memset), relleno.h.
      Usa:      relleno_vram (relleno.asm), relleno_vram_c y limpiar_c (relleno_c.c).

    HARDWARE QUE TOCA
      VRAM: fuente en 0x3000, mapa de texto en 0x6800, mapa de las franjas en
      0x6000 (filas 10-11 y 14-15). PPU: Mode 1, BG1 y BG2 encendidos.

    VARIABLES
      bufer  u8[256]  búfer de la WRAM que vacían limpiar_c y memset

    → glosario: VRAM, Tilemap, Consola de texto, Forced blank
---------------------------------------------------------------------------------*/
#include <snes.h>
#include <string.h>
#include "relleno.h"

#define VRAM_FUENTE 0x3000          /* Tiles de la fuente (los de la consola).           */
#define VRAM_TEXTO 0x6800           /* Mapa de BG1: la consola de texto.                 */
#define VRAM_FRANJAS 0x6000         /* Mapa de BG2: las franjas que rellenamos.          */
#define TILE(c) ((u16)((c) - 32))   /* La fuente empieza en el espacio (ASCII 32).       */
#define FILA(f) ((u16)((f) << 5))   /* Palabra donde empieza la fila f (32 por fila).    */

static u8 bufer[256];               /* Lo que vacían limpiar_c y memset.                 */

int main(void)                      /* tcc816 exige `int main`; el resto, sin int.       */
{
    relleno_llamadas = 0;           /* La WRAM arranca con basura: a 0 a mano (unidad 13). */
    dmaClearVram();               /* Toda la VRAM a 0: el mapa de BG2 queda en espacios. */
    consoleInitDefaultText(0);      /* La fuente de la librería, paleta 0.               */
    bgSetGfxPtr(0, VRAM_FUENTE);    /* BG1 (número 0): los tiles de la fuente...         */
    bgSetMapPtr(0, VRAM_TEXTO, SC_32x32);   /* ...y el mapa de la consola.               */
    bgSetGfxPtr(1, VRAM_FUENTE);    /* BG2 (número 1): los MISMOS tiles...               */
    bgSetMapPtr(1, VRAM_FRANJAS, SC_32x32); /* ...con su propio mapa.                    */
    setMode(BG_MODE1, 0);           /* Mode 1: BG1 y BG2 de 16 colores.                  */
    bgSetDisable(2);                /* BG3 no se usa.                                    */

    consoleDrawText(2, 2, "UNIT 15: ASM CALLED FROM C");
    consoleDrawText(2, 9, "FILLED BY C:");
    consoleDrawText(2, 13, "FILLED BY ASM:");
    setScreenOn();                  /* Brillo 15 (el SDK lo apunta en mirrorINIDISP).   */
    WaitForVBlank();                /* La consola copia su texto en este VBlank: después  */
                                    /* no queda nada pendiente que toque VMADD a medias. */

    relleno_vram_c(VRAM_FRANJAS + FILA(10), TILE('#'), 32);   /* Fila 10, en C.        */
    relleno_vram_c(VRAM_FRANJAS + FILA(11), TILE('#'), 32);   /* Fila 11, en C.        */
    relleno_vram(VRAM_FRANJAS + FILA(14), TILE('='), 32);     /* Fila 14, en asm.      */
    relleno_vram(VRAM_FRANJAS + FILA(15), TILE('='), 32);     /* Fila 15, en asm.      */

    limpiar_c(bufer, sizeof(bufer));    /* 256 bytes a 0 con el bucle en C...            */
    memset(bufer, 0, sizeof(bufer));    /* ...y con memset (ensamblador de la librería). */

    consoleDrawText(2, 18, "ASM CALLS: %d", relleno_llamadas);   /* Debe decir 2.       */

    while (1)
    {
        WaitForVBlank();            /* Nada más que hacer: esperar.                      */
    }
    return 0;                       /* Nunca se llega aquí; C exige que main devuelva.  */
}
