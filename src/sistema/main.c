/*---------------------------------------------------------------------------------
    main.c — Punto de entrada del programa: prepara el vídeo, arranca la secuencia
    de pantallas y entra en el bucle de un frame por vuelta.

    PROPÓSITO
      main() hace tres cosas y nada más: (1) deja la PPU en Mode 1 con la consola
      de texto y la pantalla en negro, (2) arranca la máquina de estados en la
      pantalla de Nintendo y (3) repite para siempre: actualizar la pantalla
      activa, esperar al VBlank y hacer el trabajo que solo puede hacerse justo
      después del VBlank. Toda la lógica de cada pantalla vive en pantalla_xxx.c.

    SE CONECTA CON
      Incluye:  <snes.h> (PVSnesLib), sistema/secuencia.h (secuencia_iniciar,
                secuencia_actualizar, secuencia_vblank), sistema/comun.h
                (video_texto_mode1), pantallas/personajes.h (personajes_ocultar).
      Lo usan:  nadie. El enlazador salta aquí tras el código de arranque del SDK.

    HARDWARE QUE TOCA
      PPU: modo de vídeo y fuente (por comun.c), encendido (setScreenOn), brillo
      (setBrightness). OAM: limpieza inicial (oamInit). CPU: WaitForVBlank()
      duerme hasta la interrupción NMI que la PPU dispara al final de cada frame.

    VARIABLES (globales y estáticas de este archivo)
      ninguna.

    → glosario: VBlank, Consola de texto, Mode 1, Máquina de estados
---------------------------------------------------------------------------------*/
#include <snes.h>        /* Librería PVSnesLib (funciones de vídeo, mando, texto...). */
#include "sistema/secuencia.h"   /* Nuestra máquina de estados: secuencia_iniciar/actualizar. */
#include "sistema/comun.h"       /* video_texto_mode1() y el reparto de la VRAM.             */
#include "pantallas/personajes.h"  /* personajes_ocultar() al arrancar.                        */
#ifdef PRUEBA_CARGA
#include "pantallas/prueba_carga.h"  /* La ROM de prueba de carga (docs/diseno-beat-em-up.md §9). */
#endif

int main(void)
{
    /*--- 1. Vídeo y texto ---------------------------------------------------------
        Estado base: Mode 1 con la capa de texto y nuestra fuente Ark Pixel. El
        reparto de la VRAM (qué va en cada dirección) está en comun.h. Cada
        pantalla carga después sus propios fondos y sprites. */
    video_texto_mode1();
    oamInit();                    /* Limpia la OAM (arranca con basura).                 */
    personajes_ocultar();         /* Y esconde de verdad los dos sprites grandes.        */

    /* Encendemos la pantalla y de inmediato la dejamos en negro: setBrightness(0)
       escribe $80 en INIDISP, es decir, pantalla apagada (forced blank), no
       "brillo 0 encendida". Así la primera pantalla hace su fade in desde negro
       sin un destello (su fade in escribe $00..$0F y la vuelve a encender). */
    setScreenOn();
    setBrightness(0);

    /*--- 2. Arranque de la secuencia ----------------------------------------------
        Entra en la primera pantalla (Nintendo): dibuja su contenido y hace fade in. */
#if defined(PRUEBA_CARGA)
    prueba_carga_ejecutar();          /* ROM de prueba de carga (make con -DPRUEBA_CARGA):
                                         su propio bucle, no vuelve (pantallas/prueba_carga.h). */
#elif defined(GUION_FASE1)
    secuencia_iniciar(PANTALLA_FASE1);   /* ROM de prueba (make con -DGUION_FASE1): directo a la
                                            fase 1 del beat 'em up, que se juega sola (mando.c). */
#elif defined(GUION_PRUEBA)
    secuencia_iniciar(PANTALLA_NIVEL1);  /* ROM de prueba (make con -DGUION_PRUEBA): directo
                                            al nivel, que se juega solo (juego/guion.h).  */
#else
    secuencia_iniciar(PANTALLA_NINTENDO);
#endif

    /*--- 3. Bucle principal -------------------------------------------------------
        while (1) = "para siempre": una consola no termina su programa. En cada
        vuelta: actualizamos la pantalla activa y esperamos al VBlank. Esa espera
        es lo que fija el ritmo a 60 vueltas por segundo y lo que hace que el
        mando se lea y el texto se copie a la VRAM. */
    while (1)
    {
        secuencia_actualizar();   /* Un frame de la pantalla actual.                  */
        WaitForVBlank();          /* Esperar al final del dibujo de este frame.       */
        secuencia_vblank();       /* Lo que solo puede hacerse recién pasado el VBlank.*/
    }

    return 0;                     /* Nunca se llega aquí; C exige que main devuelva.  */
}
