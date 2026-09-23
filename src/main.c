/*---------------------------------------------------------------------------------
    Tarouh: A Dog's Journey — punto de entrada del programa.

    main.c hace tres cosas y nada más:
      1. Configura la pantalla y la consola de texto de PVSnesLib.
      2. Arranca la secuencia de pantallas (secuencia.c).
      3. Entra en el bucle principal: un frame por vuelta, sincronizado con VBlank.
    Toda la lógica de cada pantalla vive en su propio archivo pantalla_xxx.c.
    → glosario: VBlank, Consola de texto, Mode 1
---------------------------------------------------------------------------------*/
#include <snes.h>        /* Librería PVSnesLib (funciones de vídeo, mando, texto...). */
#include "secuencia.h"   /* Nuestra máquina de estados: secuencia_iniciar/actualizar. */

int main(void)
{
    /*--- 1. Vídeo y texto ---------------------------------------------------------
        consoleInitDefaultText(0): carga la fuente embebida de PVSnesLib en la
        VRAM y la asocia a la paleta 0. Es la fuente provisional del proyecto. */
    consoleInitDefaultText(0);

    /* La capa de texto es el fondo 0 (BG1 en la jerga de la PPU, numerado desde 0
       en PVSnesLib). Le decimos dónde están sus tiles (0x3000) y su mapa (0x6800)
       dentro de la VRAM, en hexadecimal. SC_32x32 = mapa de 32x32 celdas. */
    bgSetGfxPtr(0, 0x3000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    /* Mode 1: dos fondos de 16 colores y uno de 4. Por ahora solo usamos el fondo
       0, así que apagamos el 1 y el 2 para que no muestren basura de la VRAM. */
    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    /* Encendemos la pantalla y de inmediato bajamos el brillo a 0 (negro): así la
       primera pantalla puede hacer su fade in desde negro sin un destello. */
    setScreenOn();
    setBrightness(0);

    /*--- 2. Arranque de la secuencia ----------------------------------------------
        Entra en la primera pantalla (Nintendo): dibuja su contenido y hace fade in. */
    secuencia_iniciar(PANTALLA_NINTENDO);

    /*--- 3. Bucle principal -------------------------------------------------------
        while (1) = "para siempre": una consola no termina su programa. En cada
        vuelta: actualizamos la pantalla activa y esperamos al VBlank. Esa espera
        es lo que fija el ritmo a 60 vueltas por segundo y lo que hace que el
        mando se lea y el texto se copie a la VRAM. */
    while (1)
    {
        secuencia_actualizar();   /* Un frame de la pantalla actual.                  */
        WaitForVBlank();          /* Esperar al final del dibujo de este frame.       */
    }

    return 0;                     /* Nunca se llega aquí; C exige que main devuelva.  */
}
