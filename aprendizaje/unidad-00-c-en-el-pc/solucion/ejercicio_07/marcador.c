/*---------------------------------------------------------------------------------
    solucion/ejercicio_07/marcador.c — Solución del ejercicio 7 de la unidad 00.

    PROPÓSITO
      El marcador completo, con las tres formas de static: una variable de
      archivo (total), una función de archivo (comprobar_record) y una
      variable local que conserva su valor entre llamadas (veces). Los demás
      archivos de esta carpeta son copias idénticas de los de ejercicio_07/,
      para que la solución compile sola.

    SE CONECTA CON
      Incluye:  marcador.h.
      Lo usan:  ejercicio_07/main.c, a través de marcador.h.
      Modelo:   src/sistema/secuencia.c (define la variable que su .h declara
                extern), src/juego/tarou.c (funciones static internas).
      Compilar (desde la carpeta de la unidad, los dos .c a la vez):
                gcc -std=c99 -Wall -Wextra -o solucion_07.exe solucion/ejercicio_07/main.c solucion/ejercicio_07/marcador.c

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      marcador_record  u16  global, definida aquí     el récord (main.c lo ve por extern)
      total            u16  static de archivo         total de la partida actual
      veces            u8   static local de sumar     sumas desde que arrancó el programa

    → conceptos (README de la unidad 00): definición, static, extern
---------------------------------------------------------------------------------*/
#include "marcador.h"                /* Su propio .h: el compilador comprueba las firmas. */

/*
 * SALIDA ESPERADA del programa completo (exacta):
 *   Partida 1: total=0 record=0
 *   Suma 1: total=100 record=100
 *   Suma 2: total=350 record=350
 *   Partida 2: total=0 record=350
 *   Suma 3: total=50 record=350
 *   Suma 4: total=450 record=450
 *
 * Fíjate en la línea «Suma 3»: el número de sumas NO vuelve a 1 al empezar la
 * partida 2. Eso es lo que hace una static local: nadie de fuera puede
 * ponerla a cero.
 */

u16 marcador_record;                 /* DEFINICIÓN de la variable que marcador.h anuncia.
                                        Sin ella, el enlazador diría «undefined reference
                                        to `marcador_record'». main.c la pone a 0.         */

/* TODO 1: variable de archivo: solo marcador.c la ve. */
static u16 total;                    /* Sin "= 0": se pone a 0 en marcador_iniciar.       */

/* TODO 2: función de archivo; va antes de marcador_sumar, que la llama. */
static void comprobar_record(void)
{
    if (total > marcador_record)     /* ¿Nuevo récord?                                    */
    {
        marcador_record = total;     /* Se guarda en la global.                           */
    }
}

void marcador_iniciar(void)
{
    total = 0;                       /* TODO 1: a mano, siempre. El récord no se toca.    */
}

/* TODO 3: la static local `veces` se crea una sola vez y recuerda su valor. */
u8 marcador_sumar(u16 puntos)
{
    static u8 veces = 0;             /* El "= 0" se aplica una vez, al arrancar.          */
    total = (u16)(total + puntos);   /* Acumula.                                          */
    comprobar_record();              /* La pieza interna.                                 */
    veces++;                         /* Una suma más...                                   */
    return veces;                    /* ...en todo el programa.                           */
}

/* TODO 4: leer la static desde fuera solo es posible a través de una función. */
u16 marcador_total(void)
{
    return total;
}
