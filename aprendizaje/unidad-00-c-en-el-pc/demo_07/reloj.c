/*---------------------------------------------------------------------------------
    demo_07/reloj.c — Implementación del reloj de la demo 7: el cómo.

    PROPÓSITO
      Define la variable global que reloj.h anunció con extern, guarda su
      estado interno en variables static de archivo (nadie de fuera puede
      tocarlas) y usa una función static auxiliar. Es la forma de
      src/sistema/secuencia.c (define frames_en_pantalla) y de
      src/juego/tarou.c (cambiar_estado, acelerar... son static).

    SE CONECTA CON
      Incluye:  reloj.h.
      Lo usan:  demo_07/main.c, a través de reloj.h.
      Compilar: junto con main.c (ver demo_07/main.c).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      frames_totales      u16  global          definida aquí; main.c la lee por extern
      frames_del_segundo  u8   static, 0..59   frames dentro del segundo actual
      segundos            u8   static, 0..255  segundos completos

    → conceptos (README de la unidad 00): definición, static de archivo, función static
---------------------------------------------------------------------------------*/
#include "reloj.h"                   /* Su propio .h: así el compilador comprueba que las
                                        firmas de aquí coinciden con los prototipos.      */

u16 frames_totales;                  /* DEFINICIÓN: aquí vive la variable (sin static).   */

static u8 frames_del_segundo;        /* static de archivo: solo se ve en reloj.c.         */
static u8 segundos;                  /* Ídem.                                             */

/* Función static: una pieza interna; desde main.c no se puede llamar. */
static void pasar_segundo(void)
{
    frames_del_segundo = 0;          /* Empieza el segundo siguiente...                   */
    segundos++;                      /* ...y se cuenta el que acaba de terminar.          */
}

void reloj_iniciar(void)
{
    frames_totales = 0;              /* Todo a cero A MANO: en la SNES las variables      */
    frames_del_segundo = 0;          /* no empiezan a 0 (en el PC sí, pero el proyecto    */
    segundos = 0;                    /* no cuenta con ello).                              */
}

void reloj_avanzar(void)
{
    frames_totales++;                /* Un frame más en total.                            */
    frames_del_segundo++;            /* Y dentro del segundo.                             */
    if (frames_del_segundo == FPS)   /* ¿Se completó un segundo?                          */
    {
        pasar_segundo();             /* La función static se usa desde dentro.            */
    }
}

u8 reloj_segundos(void)
{
    return segundos;                 /* Leer una static desde fuera: solo con una función. */
}
