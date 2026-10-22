/*---------------------------------------------------------------------------------
    demo_06_expandir.c — Demo 6 (bis) de la unidad 00: ver lo que produce el preprocesador.

    PROPÓSITO
      Un archivo diminuto, sin #include, para pasarlo por el preprocesador y
      leer el resultado: gcc -E se detiene después de ese paso e imprime el C
      que de verdad recibe el compilador, con cada macro ya sustituida. Sin
      <stdio.h> la salida cabe en una pantalla (stdio.h son cientos de líneas).
      La salida real está en capturas/salida_demo_06_expandir_E.txt.

    SE CONECTA CON
      Incluye:  nada.
      Lo usan:  nadie.
      Ver:      gcc -E -P demo_06_expandir.c
                (-E: solo preprocesar; -P: sin las líneas de marca "# 1 ...").
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_06e.exe demo_06_expandir.c
                (compila y sale con código 0; no imprime nada).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      a, b, c, d   int   valores calculados con las macros (solo para verlos en -E)

    → conceptos (README de la unidad 00): preprocesador, macro, #ifdef
---------------------------------------------------------------------------------*/
#define FPS 60                       /* Constante.                                        */
#define SEGUNDOS(s) ((s) * FPS)      /* Macro que usa otra constante.                     */
#define CUADRADO(n) ((n) * (n))      /* Bien escrita.                                     */
#define CUADRADO_MAL(n) n * n        /* Mal escrita.                                      */

#ifdef GUION_PRUEBA                  /* Con -DGUION_PRUEBA, solo queda la primera línea.  */
#define ORIGEN 1                     /* 1 = guion.                                        */
#else
#define ORIGEN 0                     /* 0 = jugador.                                      */
#endif

int main(void)
{
    int a = SEGUNDOS(3);             /* Mira en -E: ((3) * 60).                           */
    int b = CUADRADO(2 + 1);         /* ((2 + 1) * (2 + 1)).                              */
    int c = CUADRADO_MAL(2 + 1);     /* 2 + 1 * 2 + 1.                                    */
    int d = ORIGEN;                  /* 0 o 1 según -D.                                   */
    return (a - 180) + (b - 9) + (c - 5) + d;   /* 0 si todo es como dice el comentario.  */
}
