/*---------------------------------------------------------------------------------
    Tarou: A Dog's Journey — punto de entrada del programa.

    main.c hace tres cosas y nada más:
      1. Configura la pantalla y la consola de texto de PVSnesLib.
      2. Arranca la secuencia de pantallas (secuencia.c).
      3. Entra en el bucle principal: un frame por vuelta, sincronizado con VBlank.
    Toda la lógica de cada pantalla vive en su propio archivo pantalla_xxx.c.
    → glosario: VBlank, Consola de texto, Mode 1
---------------------------------------------------------------------------------*/
#include <snes.h>        /* Librería PVSnesLib (funciones de vídeo, mando, texto...). */
#include "sistema/secuencia.h"   /* Nuestra máquina de estados: secuencia_iniciar/actualizar. */
#include "sistema/comun.h"       /* video_texto_mode1() y el reparto de la VRAM.             */
#include "pantallas/personajes.h"  /* personajes_ocultar() al arrancar.                        */

int main(void)
{
    /*--- 1. Vídeo y texto ---------------------------------------------------------
        Estado base: Mode 1 con la capa de texto y nuestra fuente Ark Pixel. El
        reparto de la VRAM (qué va en cada dirección) está en comun.h. Cada
        pantalla carga después sus propios fondos y sprites. */
    video_texto_mode1();
    oamInit();                    /* Limpia la OAM (arranca con basura).                 */
    personajes_ocultar();         /* Y esconde de verdad los dos sprites grandes.        */

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
