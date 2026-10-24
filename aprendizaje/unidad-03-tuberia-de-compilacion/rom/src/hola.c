/*---------------------------------------------------------------------------------
    hola.c — Mini-ROM de la unidad 03: tres líneas de texto con la consola del SDK.

    PROPÓSITO
      El programa de SNES más pequeño que enseña algo: prepara la consola de
      texto de PVSnesLib, pone la PPU en Mode 1 con un solo fondo, escribe tres
      líneas y se queda esperando VBlanks para siempre. No hace nada más a
      propósito: la unidad trata de CÓMO se convierte este .c en una ROM, no de
      lo que hace. Parte de tools/pvsneslib/snes-examples/hello_world.

    SE CONECTA CON
      Incluye:  <snes.h> (PVSnesLib entera), textos.h (los textos y la fila).
      Lo usan:  nadie: el código de arranque del SDK (crt0_snes) salta a main().

    HARDWARE QUE TOCA
      VRAM (la fuente y el mapa de texto, por consoleInitDefaultText y la copia
      de la consola en el VBlank), CGRAM (la paleta de la fuente), PPU (BGMODE
      con setMode, TM con bgSetDisable, INIDISP con setScreenOn).

    VARIABLES
      ninguna.

    → glosario: Consola de texto, Mode 1, VBlank / NMI, BG / capas
---------------------------------------------------------------------------------*/
#include <snes.h>                    /* Toda la librería: vídeo, texto, mando, DMA...      */
#include "textos.h"                  /* TEXTO_SALUDO, TEXTO_UNIDAD, TEXTO_PIE, FILA_SALUDO. */

/* main devuelve int porque C lo exige: es el ÚNICO int permitido en el código de
   la SNES (las reglas de tcc816 de CLAUDE.md prohíben int en todo lo demás). */
int main(void)
{
    /* 1. La consola de texto: fuente de la librería en la VRAM 0x3000, mapa de
          texto en 0x6800 (los valores por defecto del ejemplo) y paleta 0. */
    consoleSetTextMapPtr(0x6800);    /* Dónde escribe la consola el mapa de texto.        */
    consoleSetTextGfxPtr(0x3000);    /* Dónde copia la consola los tiles de la fuente.    */
    consoleInitDefaultText(0);       /* Copia la fuente por defecto y su paleta (la 0).   */

    /* 2. El fondo 0 (BG1) muestra ese mapa con esos tiles. */
    bgSetGfxPtr(0, 0x3000);          /* Tiles del fondo 0: los de la fuente.              */
    bgSetMapPtr(0, 0x6800, SC_32x32);/* Mapa del fondo 0: el de la consola, 32x32 celdas. */

    /* 3. Mode 1 (dos fondos de 16 colores y uno de 4) y solo el fondo 0 encendido. */
    setMode(BG_MODE1, 0);            /* BGMODE = 1.                                       */
    bgSetDisable(1);                 /* Fondo 1 apagado (tendría basura).                 */
    bgSetDisable(2);                 /* Fondo 2 apagado.                                  */

    /* 4. El texto: consoleDrawText escribe en un búfer en RAM y marca que hay
          cambios; la copia a la VRAM la hace la rutina del VBlank del SDK. */
    consoleDrawText(10, FILA_SALUDO, TEXTO_SALUDO);       /* Columna 10, fila 10.        */
    consoleDrawText(4, FILA_SALUDO + 3, TEXTO_UNIDAD);    /* Tres filas más abajo.        */
    consoleDrawText(6, FILA_SALUDO + 6, TEXTO_PIE);       /* Y otras tres.                */

    /* 5. Encender la pantalla (el arranque del SDK la dejó apagada). */
    setScreenOn();                   /* Espera un VBlank y escribe brillo 15 en INIDISP.  */

    /* 6. Una consola no termina nunca: esperar VBlanks para siempre. */
    while (1)
    {
        WaitForVBlank();             /* Duerme hasta la interrupción del VBlank (unidad 04). */
    }
    return 0;                        /* Nunca se llega aquí; C exige que main devuelva.    */
}
