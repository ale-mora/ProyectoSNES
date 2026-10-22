/*---------------------------------------------------------------------------------
    demo_07/reloj.h — Interfaz del reloj de la demo 7: lo que otros archivos
    pueden usar.

    PROPÓSITO
      Un .h dice QUÉ ofrece un módulo, sin el cómo: una constante, una
      variable global anunciada con extern y los prototipos de tres
      funciones. Es la misma forma que src/sistema/secuencia.h (extern u16
      frames_en_pantalla; y las firmas de secuencia_*).

    SE CONECTA CON
      Incluye:  tipos.h.
      Lo usan:  demo_07/reloj.c (la implementación), demo_07/main.c (lo usa).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (declaradas aquí, definidas en reloj.c)
      frames_totales  u16  0..65535  frames desde reloj_iniciar

    → conceptos (README de la unidad 00): .h, prototipo, extern, guarda de inclusión
---------------------------------------------------------------------------------*/
#ifndef RELOJ_H                      /* Guarda de inclusión.                              */
#define RELOJ_H

#include "tipos.h"                   /* Este .h usa u8 y u16: incluye lo que necesita.    */

#define FPS 60                       /* Frames por segundo (NTSC).                        */

/* Declaración: "existe una u16 llamada así, en otro archivo". No reserva memoria. */
extern u16 frames_totales;

void reloj_iniciar(void);            /* Prototipo: pone el reloj a cero.                  */
void reloj_avanzar(void);            /* Prototipo: cuenta un frame.                       */
u8 reloj_segundos(void);             /* Prototipo: segundos completos desde el inicio.    */

#endif /* RELOJ_H */
