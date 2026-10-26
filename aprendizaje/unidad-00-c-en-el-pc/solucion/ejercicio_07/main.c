/*---------------------------------------------------------------------------------
    ejercicio_07/main.c — Programa del ejercicio 7 de la unidad 00 (ya escrito: no lo cambies).

    PROPÓSITO
      Juega dos partidas con el marcador usando solo lo que ofrece
      marcador.h: pone el récord a 0 (es una global; en la SNES nada empieza
      a 0), suma puntos e imprime total y récord. La salida esperada está en
      ejercicio_07/marcador.c, que es donde están los TODO.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), marcador.h.
      Lo usan:  nadie: es el programa.
      Compilar (desde la carpeta de la unidad):
                gcc -std=c99 -Wall -Wextra -o ejercicio_07.exe ejercicio_07/main.c ejercicio_07/marcador.c
      Si olvidas ejercicio_07/marcador.c en esa línea, main.c compila pero el
      ENLAZADOR falla: «undefined reference to `marcador_iniciar'».

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      n   u8   0..255   lo que devuelve marcador_sumar (número de sumas)

    → conceptos (README de la unidad 00): varios archivos, extern, enlazar
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include "marcador.h"                /* Todo lo que este archivo sabe del marcador.       */

int main(void)
{
    u8 n;                            /* Número de sumas que devuelve marcador_sumar.      */

    marcador_record = 0;             /* Escribir una global de otro archivo, por extern.  */

    marcador_iniciar();              /* Partida 1.                                        */
    printf("Partida 1: total=%d record=%d\n", marcador_total(), marcador_record);
    n = marcador_sumar(100);
    printf("Suma %d: total=%d record=%d\n", n, marcador_total(), marcador_record);
    n = marcador_sumar(250);
    printf("Suma %d: total=%d record=%d\n", n, marcador_total(), marcador_record);

    marcador_iniciar();              /* Partida 2: el total vuelve a 0, el récord queda.  */
    printf("Partida 2: total=%d record=%d\n", marcador_total(), marcador_record);
    n = marcador_sumar(50);
    printf("Suma %d: total=%d record=%d\n", n, marcador_total(), marcador_record);
    n = marcador_sumar(400);
    printf("Suma %d: total=%d record=%d\n", n, marcador_total(), marcador_record);

    return 0;                        /* Terminó bien.                                     */
}
