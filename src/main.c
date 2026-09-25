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
#include "personajes.h"  /* Sprites de Tarouh y Bárbara (carga única en VRAM).       */

/* La fuente vive en la ROM, pegada por data.asm con esas dos etiquetas. "extern
   char" le dice al compilador: "existe, está en otro archivo, aquí solo la uso".
   Lo que se pasa a la librería es su dirección (&fuente_consola_til). → glosario: extern */
extern char fuente_consola_til, fuente_consola_pal;

int main(void)
{
    /*--- 1. Vídeo y texto ---------------------------------------------------------
        Reparto de la VRAM (64 KB = 0x8000 palabras de 2 bytes; direcciones en
        hexadecimal). → glosario: VRAM
            0x0000-0x1FFF  tiles de sprites (Tarouh 8 KB + Bárbara 8 KB = 512 tiles, el máximo)
            0x2000-0x4FFF  tiles de fondos (reservado para el título)
            0x5000-0x5FFF  base de tiles del fondo de texto
            0x6000-0x65FF  tiles de la fuente (96 x 32 bytes)
            0x6800-0x6BFF  mapa del fondo de texto (32x32 celdas x 2 bytes)
        El "offset" 0x0100 es el número del primer tile de la fuente contado
        desde la base del fondo de texto: (0x6000 - 0x5000) / 16 palabras por
        tile = 0x100. */
    consoleSetTextMapPtr(0x6800);
    consoleSetTextGfxPtr(0x6000);
    consoleSetTextOffset(0x0100);

    /* Cargamos NUESTRA fuente (Ark Pixel, mayúsculas): paleta 0, tamaño de la
       paleta en bytes (2 colores * 2 bytes, porque cada color de la SNES ocupa
       2 bytes: el 0 transparente y el 1 el trazo), tiles y paleta. */
    consoleInitText(0, 2 * 2, &fuente_consola_til, &fuente_consola_pal);

    /* La capa de texto es el fondo 0 (BG1 en la jerga de la PPU, numerado desde 0
       en PVSnesLib). Base de tiles en 0x5000 y mapa en 0x6800, como arriba.
       SC_32x32 = mapa de 32x32 celdas. */
    bgSetGfxPtr(0, 0x5000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    /* Sprites de los dos personajes: se copian a la VRAM una sola vez, aquí. Las
       pantallas que los usan solo los muestran, animan y esconden. */
    personajes_cargar();

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
