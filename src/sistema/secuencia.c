/*---------------------------------------------------------------------------------
    secuencia.c — La máquina de estados que encadena las pantallas.

    PROPÓSITO
      Una variable dice EN QUÉ PANTALLA estamos. En cada frame, un switch ejecuta
      solo el código de esa pantalla. Cuando una pantalla termina, pide cambiar a
      la siguiente y aquí se hace el cambio. Hay tres momentos por pantalla:
      _iniciar (una vez, al entrar), _actualizar (cada frame, antes del VBlank) y
      _vblank (cada frame, justo después del VBlank; solo para las pantallas que
      copian datos a la VRAM, como el nivel).

    SE CONECTA CON
      Incluye:  sistema/secuencia.h (su propia interfaz y, a través de ella,
                sistema/pantallas.h con el enum Pantalla y los prototipos).
      Lo usan:  sistema/main.c (llama a las tres funciones del bucle); cada
                pantalla_xxx.c llama a secuencia_cambiar() y lee
                frames_en_pantalla.

    HARDWARE QUE TOCA
      ninguno: es lógica pura; el hardware lo tocan las pantallas.

    VARIABLES (globales y estáticas de este archivo)
      pantalla_actual     Pantalla  0..CANTIDAD_PANTALLAS-1  estado activo de la máquina
      frames_en_pantalla  u16       0..65535 frames          tiempo que lleva la pantalla activa

    → glosario: Máquina de estados, switch/case, enum, Frame
---------------------------------------------------------------------------------*/
#include "sistema/secuencia.h"   /* Nuestro propio header: declara lo que este archivo implementa. */

/*--- Estado de la máquina -------------------------------------------------------*/
Pantalla pantalla_actual;      /* En qué pantalla estamos ahora (un valor del enum Pantalla). */
u16 frames_en_pantalla;        /* Frames transcurridos desde que entramos en esa pantalla.
                                  u16 = entero sin signo de 16 bits (0..65535). → glosario: u8/u16 */

/*--- secuencia_cambiar ----------------------------------------------------------
    Entra en la pantalla `nueva`: la anota como actual, reinicia el contador y
    llama a la función _iniciar de esa pantalla para que dibuje y prepare todo. */
void secuencia_cambiar(Pantalla nueva)
{
    pantalla_actual = nueva;   /* Guardamos cuál es la pantalla activa a partir de ahora.  */
    frames_en_pantalla = 0;    /* El tiempo de la nueva pantalla empieza a contar de cero. */

    /* switch: mira el valor de pantalla_actual y salta al "case" que coincide.
       Cada case llama a la función de inicio correspondiente y "break" sale
       del switch (sin break, C seguiría ejecutando el case de abajo). */
    switch (pantalla_actual)
    {
        case PANTALLA_NINTENDO:     pantalla_nintendo_iniciar();     break;
        case PANTALLA_PUBLISHER:    pantalla_publisher_iniciar();    break;
        case PANTALLA_DEVS:         pantalla_devs_iniciar();         break;
        case PANTALLA_DEDICATORIA:  pantalla_dedicatoria_iniciar();  break;
        case PANTALLA_INTRO:        pantalla_intro_iniciar();        break;
        case PANTALLA_TITULO:       pantalla_titulo_iniciar();       break;
        case PANTALLA_NIVEL1:       pantalla_nivel1_iniciar();       break;
        default:                    break;   /* Valor desconocido: no hacemos nada (defensa). */
    }
}

/*--- secuencia_iniciar ----------------------------------------------------------
    Arranque de la secuencia. Hoy es simplemente "cambiar a la primera pantalla";
    existe como función aparte para que main.c se lea claro y para poder añadir
    aquí inicializaciones globales más adelante (música, sprites, etc.). */
void secuencia_iniciar(Pantalla primera)
{
    secuencia_cambiar(primera);   /* Reutilizamos el cambio de pantalla normal. */
}

/*--- secuencia_actualizar -------------------------------------------------------
    Un frame de vida de la pantalla actual. main.c la llama una vez por vuelta
    del bucle principal, justo antes de WaitForVBlank(). */
void secuencia_actualizar(void)
{
    /* Mismo patrón que arriba, pero llamando a la función _actualizar. */
    switch (pantalla_actual)
    {
        case PANTALLA_NINTENDO:     pantalla_nintendo_actualizar();     break;
        case PANTALLA_PUBLISHER:    pantalla_publisher_actualizar();    break;
        case PANTALLA_DEVS:         pantalla_devs_actualizar();         break;
        case PANTALLA_DEDICATORIA:  pantalla_dedicatoria_actualizar();  break;
        case PANTALLA_INTRO:        pantalla_intro_actualizar();        break;
        case PANTALLA_TITULO:       pantalla_titulo_actualizar();       break;
        case PANTALLA_NIVEL1:       pantalla_nivel1_actualizar();       break;
        default:                    break;
    }

    /* Un frame más en esta pantalla. Si la pantalla acaba de cambiar dentro del
       switch, el contador ya estaba en 0 y aquí pasa a 1: irrelevante. */
    frames_en_pantalla++;
}

/*--- secuencia_vblank -----------------------------------------------------------
    Tercer momento de cada pantalla: main.c la llama justo después de
    WaitForVBlank(), cuando la PPU ha terminado de dibujar y la VRAM se puede
    escribir sin romper la imagen. Solo el nivel 1 lo necesita: su _vblank copia
    a la VRAM los tiles del sprite dinámico (oamVramQueueUpdate). */
void secuencia_vblank(void)
{
    switch (pantalla_actual)
    {
        case PANTALLA_NIVEL1:       pantalla_nivel1_vblank();       break;
        default:                    break;   /* Las demás pantallas no necesitan _vblank. */
    }
}
