/*---------------------------------------------------------------------------------
    secuencia.c — La máquina de estados de la secuencia de arranque.

    Idea: una variable dice EN QUÉ PANTALLA estamos. En cada frame, un switch
    ejecuta solo el código de esa pantalla. Cuando una pantalla termina, pide
    cambiar a la siguiente y aquí se hace el cambio.
    → glosario: Máquina de estados, switch/case, enum
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
        case PANTALLA_PROXIMAMENTE: pantalla_proximamente_iniciar(); break;
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
        case PANTALLA_PROXIMAMENTE: pantalla_proximamente_actualizar(); break;
        default:                    break;
    }

    /* Un frame más en esta pantalla. Si la pantalla acaba de cambiar dentro del
       switch, el contador ya estaba en 0 y aquí pasa a 1: irrelevante. */
    frames_en_pantalla++;
}
