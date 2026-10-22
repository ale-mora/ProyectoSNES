/*---------------------------------------------------------------------------------
    demo_02_semaforo.c — Demo de la unidad 02: una máquina de estados con enum
    y switch (un semáforo con botón para peatones).

    PROPÓSITO
      Enseña la forma de todas las máquinas de estados del juego con el
      ejemplo más simple posible: un enum con los estados, una variable que
      guarda el estado actual, un contador de frames en ese estado, una función
      _iniciar (una vez) y una _actualizar (una vez por frame) con un switch
      que decide la transición. Hay dos clases de transición: por tiempo (el
      verde dura 6 frames) y por una entrada (el botón de peatones acorta el
      verde). Imprime cada frame y marca los cambios.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t, uint16_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/sistema/secuencia.c (pantalla_actual, frames_en_pantalla,
                secuencia_cambiar y el switch de secuencia_actualizar).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_02.exe demo_02_semaforo.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      luz_actual       u8   LUZ_VERDE..LUZ_ROJA   estado activo de la máquina
      frames_en_luz    u16  0..                    frames que lleva la luz actual
      boton_pulsado    u8   0 / 1                  el peatón pidió paso (se recuerda)

    → conceptos (README de la unidad 02): estado, transición, enum, switch,
      máquina de estados, patrón _iniciar/_actualizar
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Enteros de tamaño fijo.                           */

typedef uint8_t u8;                  /* Los mismos nombres de tipo que el juego.          */
typedef uint16_t u16;

/*--- Los estados: un nombre por cada valor posible ---------------------------------*/
enum
{
    LUZ_VERDE = 0,                   /* Pasan los coches.                                 */
    LUZ_AMBAR,                       /* 1: va a cambiar.                                  */
    LUZ_ROJA,                        /* 2: pasan los peatones.                            */
    LUZ_CANTIDAD                     /* 3: no es un estado, cuenta cuántos hay.           */
};

/* Un nombre por estado, en el orden del enum: el estado sirve de índice. */
static const char *const NOMBRES[LUZ_CANTIDAD] = {"VERDE", "AMBAR", "ROJA"};

#define FRAMES_VERDE 6               /* Duración de cada luz, en frames.                  */
#define FRAMES_VERDE_MINIMO 2        /* Con el botón, el verde dura al menos esto.        */
#define FRAMES_AMBAR 2
#define FRAMES_ROJA 4
#define FRAMES_DEMO 20               /* Cuántos frames se simulan.                        */
#define FRAME_BOTON 15               /* En este frame un peatón pulsa el botón.           */

/*--- El estado de la máquina (en el juego: pantalla_actual, frames_en_pantalla) ------*/
static u8 luz_actual;                /* Qué luz está encendida.                           */
static u16 frames_en_luz;            /* Cuánto lleva encendida.                           */
static u8 boton_pulsado;             /* El peatón ya pidió paso.                          */

/* La única forma de cambiar de estado: anota el nuevo y reinicia el reloj.
   Es lo que hace secuencia_cambiar() en src/sistema/secuencia.c. */
static void cambiar_luz(u8 nueva)
{
    luz_actual = nueva;              /* El estado nuevo.                                  */
    frames_en_luz = 0;               /* Su tiempo empieza de cero.                        */
}

/* _iniciar: una vez, al principio. Deja la máquina en un estado conocido (las
   static de la SNES NO empiezan a 0: hay que inicializarlas siempre). */
static void semaforo_iniciar(void)
{
    boton_pulsado = 0;               /* Nadie ha pedido paso.                             */
    cambiar_luz(LUZ_VERDE);          /* Se empieza en verde.                              */
}

/* _actualizar: una vez por frame. Solo se ejecuta el código del estado actual. */
static void semaforo_actualizar(u8 boton)
{
    frames_en_luz++;                 /* Un frame más en esta luz.                         */
    if (boton)
    {
        boton_pulsado = 1;           /* Se recuerda aunque ahora no se pueda atender.     */
    }

    switch (luz_actual)              /* Salta al case del estado actual.                  */
    {
        case LUZ_VERDE:
            /* Dos transiciones posibles: por tiempo, o por el botón (si ya pasó el mínimo). */
            if (frames_en_luz >= FRAMES_VERDE
                || (boton_pulsado && frames_en_luz >= FRAMES_VERDE_MINIMO))
            {
                cambiar_luz(LUZ_AMBAR);
            }
            break;                   /* Sin break, C seguiría con el case de abajo.       */
        case LUZ_AMBAR:
            if (frames_en_luz >= FRAMES_AMBAR)
            {
                cambiar_luz(LUZ_ROJA);
            }
            break;
        case LUZ_ROJA:
            if (frames_en_luz >= FRAMES_ROJA)
            {
                boton_pulsado = 0;   /* El peatón ya cruzó.                               */
                cambiar_luz(LUZ_VERDE);
            }
            break;
        default:                     /* Valor imposible: defensa, como en secuencia.c.    */
            break;
    }
}

int main(void)                       /* El programa empieza aquí.                         */
{
    u8 f;                            /* Número de frame.                                  */
    u8 antes;                        /* Luz del frame anterior, para marcar los cambios.  */

    semaforo_iniciar();              /* Una vez.                                          */
    printf("inicio:    %s\n", NOMBRES[luz_actual]);
    for (f = 1; f <= FRAMES_DEMO; f++)
    {
        antes = luz_actual;          /* Para saber si este frame cambia.                  */
        semaforo_actualizar((u8)(f == FRAME_BOTON));   /* Una vez por frame.              */
        printf("frame %2u:  %-5s (lleva %u)", (unsigned)f, NOMBRES[luz_actual],
               (unsigned)frames_en_luz);
        if (f == FRAME_BOTON)
        {
            printf("  [boton]");     /* Aquí el peatón pulsó.                             */
        }
        if (luz_actual != antes)
        {
            printf("  <- cambia");   /* Hubo transición.                                  */
        }
        printf("\n");
    }
    return 0;                        /* Terminó bien.                                     */
}
