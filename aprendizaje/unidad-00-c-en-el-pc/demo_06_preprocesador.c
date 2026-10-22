/*---------------------------------------------------------------------------------
    demo_06_preprocesador.c — Demo 6 de la unidad 00: el preprocesador.

    PROPÓSITO
      Enseña, en cinco pasos numerados en la salida, lo que pasa ANTES de
      compilar: 1) #define de constantes, también hechas de otras; 2) macros
      con parámetros y las dos trampas de los paréntesis (en cada parámetro y
      alrededor de todo), con un caso real de una macro del SDK; 3) #include
      de una cabecera dos veces, protegida por su guarda; 4) #ifdef / #ifndef /
      #else / #endif para elegir código según lo que se define con -D (como
      PRUEBAS_PC y GUION_PRUEBA en el juego); 5) un valor por defecto que -D
      puede cambiar.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), demo_06_caja.h (dos veces, a propósito).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/sistema/tipos.h:33 (#ifdef PRUEBAS_PC), src/juego/mando.c
                (#ifdef GUION_PRUEBA), src/sistema/pantallas.h (SEGUNDOS(s)),
                tools/pvsneslib/pvsneslib/include/snes/sprite.h:60 (OBJ_PRIO).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_06.exe demo_06_preprocesador.c
                gcc -std=c99 -Wall -Wextra -DGUION_PRUEBA -DVIDAS_INICIALES=5 -o demo_06d.exe demo_06_preprocesador.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      cuerpo    struct Caja   -     una variable del tipo de demo_06_caja.h
      prio      u8            2     prioridad de sprite para el paso 2

    → conceptos (README de la unidad 00): preprocesador, #define, macro,
      #include, guarda de inclusión, #ifdef, -D
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include "demo_06_caja.h"            /* Primera vez: se pega entero.                      */
#include "demo_06_caja.h"            /* Segunda vez: la guarda lo deja vacío.             */

/*--- 1. Constantes. Una constante puede usar otras: se pega texto, en cadena --------*/
#define FPS 60                       /* Frames por segundo (NTSC).                        */
#define ANCHO_PANTALLA 256           /* Píxeles.                                          */
#define ANCHO_NIVEL 2048             /* El tutorial mide 2048 px.                         */
#define PANTALLAS_NIVEL (ANCHO_NIVEL / ANCHO_PANTALLA)   /* 8: la calcula el compilador. */

/*--- 2. Macros con parámetros ---------------------------------------------------------*/
#define SEGUNDOS(s) ((s) * FPS)      /* Como pantallas.h: SEGUNDOS(3) -> ((3) * 60).      */
#define CUADRADO(n) ((n) * (n))      /* Bien: paréntesis en cada n y alrededor de todo.   */
#define CUADRADO_MAL(n) n * n        /* Sin paréntesis en los parámetros.                 */
#define SUMA(a, b) ((a) + (b))       /* Bien.                                             */
#define SUMA_MAL(a, b) (a) + (b)     /* Sin paréntesis alrededor de todo.                 */
#define OBJ_PRIO(prio) (prio << 4)   /* Copiada tal cual del SDK (sprite.h:60).           */

/*--- 4 y 5. Lo que se elige al compilar -----------------------------------------------*/
#ifndef VIDAS_INICIALES              /* Si nadie la definió con -D...                     */
#define VIDAS_INICIALES 3            /* ...vale 3. Con -DVIDAS_INICIALES=5 vale 5.        */
#endif

typedef uint8_t u8;                  /* Como src/sistema/tipos.h (stdint.h llega por el .h). */

int main(void)
{
    /* 1. Constantes: el compilador ve 2048 / 256, dos números, y lo resuelve él. */
    printf("1. El nivel mide %d px = %d pantallas de %d px\n",
           ANCHO_NIVEL, PANTALLAS_NIVEL, ANCHO_PANTALLA);
    printf("1. SEGUNDOS(3) = %d frames\n", SEGUNDOS(3));

    /* 2. Macros: el preprocesador pega el texto del argumento, sin calcularlo. */
    printf("2. CUADRADO(2 + 1)     = %d\n", CUADRADO(2 + 1));      /* ((2 + 1) * (2 + 1)) = 9. */
    printf("2. CUADRADO_MAL(2 + 1) = %d\n", CUADRADO_MAL(2 + 1));  /* 2 + 1 * 2 + 1 = 5.       */
    printf("2. SUMA(1, 2) * 10     = %d\n", SUMA(1, 2) * 10);      /* ((1) + (2)) * 10 = 30.   */
    printf("2. SUMA_MAL(1, 2) * 10 = %d\n", SUMA_MAL(1, 2) * 10);  /* (1) + (2) * 10 = 21.     */
    u8 prio = 2;                                                    /* Prioridad de un sprite.  */
    printf("2. OBJ_PRIO(prio)      = %d\n", OBJ_PRIO(prio));        /* (prio << 4) = 32: bien.  */
    printf("2. OBJ_PRIO(prio & 3)  = %d\n", OBJ_PRIO(prio & 3));    /* (prio & 3 << 4) = 0!     */
    printf("2. OBJ_PRIO((prio & 3)) = %d\n", OBJ_PRIO((prio & 3))); /* Paréntesis al llamar: 32.*/

    /* 3. El .h se incluyó dos veces y compila: la guarda hizo su trabajo. */
    struct Caja cuerpo = {24, 14, 52, 50};   /* La caja del cuerpo de Tarou de pie.      */
    printf("3. Caja de demo_06_caja.h: %dx%d (ancho maximo %d)\n",
           cuerpo.ancho, cuerpo.alto, CAJA_MAX_ANCHO);

    /* 4. #ifdef: este código se compila o no según lo que diga la línea de gcc. */
#ifdef GUION_PRUEBA
    printf("4. GUION_PRUEBA definido: el mando lo da el guion\n");
#else
    printf("4. GUION_PRUEBA sin definir: el mando lo da el jugador\n");
#endif
#ifdef PRUEBAS_PC
    printf("4. PRUEBAS_PC definido (como en tests/Makefile)\n");
#else
    printf("4. PRUEBAS_PC sin definir\n");
#endif

    /* 5. Un valor por defecto que -D puede cambiar. */
    printf("5. VIDAS_INICIALES = %d\n", VIDAS_INICIALES);

    return 0;                        /* Terminó bien.                                     */
}
