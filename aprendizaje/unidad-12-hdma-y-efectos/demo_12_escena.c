/*---------------------------------------------------------------------------------
    demo_12_escena.c — Demo de la unidad 12: el motor de escenas del juego,
    ejecutado en el PC frame a frame.

    PROPÓSITO
      Enlaza el secuenciador REAL del juego (src/escena/escena.c, lógica pura) con
      una tabla pequeña y escribe, frame a frame, el valor de dos pistas en punto
      fijo 8.8 (el número crudo y su parte entera, la que llega a los registros).
      Se ve en cuatro pasos numerados:
        1. el paso de cada RAMPA, calculado por el COMPILADOR en la macro;
        2. una rampa de brillo 0 → 15 en 16 frames y otra de mosaico 15 → 0 en
           6 que empiezan en el mismo frame, y un FIJAR;
        3. que el último frame de cada rampa pone el valor EXACTO;
        4. por qué RAMPA no multiplica por 256: en 16 bits, 255 * 256 no cabe.

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdint.h>, escena/escena.h (con -I../../src).
      Enlaza:   ../../src/escena/escena.c (el motor del juego, sin cambios).
      Modelo:   src/escena/escena_nintendo.c (una tabla real), tests/prueba_escena.c.
      Compilar (desde esta carpeta):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_12.exe demo_12_escena.c ../../src/escena/escena.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC; -DPRUEBAS_PC hace que sistema/tipos.h saque
      u8/u16/s16 de <stdint.h>).

    VARIABLES
      DEMO   const EventoEscena[5]  la tabla de la demo
      e      Escena                 el estado del motor (pistas, reloj, próximo evento)
    → conceptos (README de la unidad 12): motor de escenas, pista, punto fijo 8.8
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* printf.                                            */
#include <stdint.h>                  /* int16_t para imitar el int de 16 bits de tcc816.   */
#include "escena/escena.h"           /* RAMPA, FIJAR, FIN_ESCENA, Escena, escena_avanzar.  */

/* La tabla: se lee como un guion. Va ordenada por frame. */
static const EventoEscena DEMO[] = {
    FIJAR(0, PISTA_MOSAICO, 15),            /* Frame 0: bloques de 16x16 de golpe.         */
    RAMPA(0, 16, PISTA_BRILLO, 0, 15),      /* Frames 0..15: el brillo sube de 0 a 15.     */
    RAMPA(0, 6, PISTA_MOSAICO, 15, 0),      /* Frames 0..5: el mosaico se enfoca.          */
    RAMPA(20, 3, PISTA_BRILLO, 15, 0),      /* Frames 20..22: fundido de salida rápido.    */
    FIN_ESCENA(24),                         /* La escena termina al llegar al frame 24.    */
};

int main(void)
{
    Escena e;                                        /* El motor en marcha.                */
    const EventoEscena *ev;                          /* Para recorrer la tabla.            */
    int16_t a16;                                     /* Una cuenta hecha en 16 bits.       */

    /* 1. Los pasos ya vienen calculados en la tabla (8.8: 256 = 1 por frame). */
    for (ev = DEMO; ev->pista != ESCENA_FIN; ev++)
    {
        printf("1. evento: frame %2u, %2u frames, pista %u, hasta %2u, paso %5d (%.3f por frame)\n",
               (unsigned)ev->cuadro, (unsigned)ev->frames, (unsigned)ev->pista,
               (unsigned)ev->hasta, ev->paso, ev->paso / 256.0);
    }

    /* 2 y 3. Frame a frame: valor crudo 8.8 y la parte entera (>> 8). */
    escena_empezar(&e, DEMO);                        /* Todas las pistas a 0, reloj a 0.   */
    printf("2. frame | brillo 8.8  entero | mosaico 8.8  entero\n");
    while (!e.terminada)
    {
        unsigned c = e.cuadro;                       /* El frame que se va a calcular.     */
        escena_avanzar(&e);                          /* Lo que hace pantalla_nintendo.c.   */
        if (e.terminada)
        {
            printf("2. frame %2u: FIN_ESCENA (los valores se quedan como estaban)\n", c);
            break;
        }
        printf("2. %5u | %10u  %6u | %11u  %6u%s\n", c,
               (unsigned)e.pistas[PISTA_BRILLO].valor, (unsigned)ESCENA_VALOR(&e, PISTA_BRILLO),
               (unsigned)e.pistas[PISTA_MOSAICO].valor, (unsigned)ESCENA_VALOR(&e, PISTA_MOSAICO),
               (c == 15 || c == 5 || c == 22) ? "   <- ultimo frame de una rampa: exacto" : "");
    }

    /* 4. El int de tcc816 mide 16 bits: 255 * 256 = 65280 no cabe en -32768..32767. */
    a16 = (int16_t)(255 * 256);                      /* Lo que daría tcc816 (en gcc, int es de 32). */
    printf("4. 255 * 256 en 16 bits = %d (deberia ser 65280)\n", a16);
    a16 = (int16_t)(255 * 128);                      /* Lo que hace RAMPA: * 128 primero...  */
    printf("4. 255 * 128 en 16 bits = %d (cabe); / 56 frames = %d; * 2 = %d (paso de 0 a 255 en 56)\n",
           a16, a16 / 56, (a16 / 56) * 2);
    return 0;
}
