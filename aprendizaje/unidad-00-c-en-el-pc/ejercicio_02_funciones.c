/*---------------------------------------------------------------------------------
    ejercicio_02_funciones.c — Ejercicio 2 de la unidad 00: funciones.

    PROPÓSITO
      Sacar la lógica del ejercicio 1 (decidir si Tarou es cachorro, adulto o
      viejo según su edad) a una FUNCIÓN con nombre, y llamarla desde el bucle
      de main. Es la idea de «modularidad»: un trozo de código que hace una
      sola cosa clara, con nombre, que se puede llamar desde donde haga falta
      sin repetirlo. tarou.c está hecho entero así.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o ejercicio_02.exe ejercicio_02_funciones.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC, no en la SNES).

    VARIABLES
      edad     int    0..10 (años del juego)    el parámetro de la función y el contador del for

    → conceptos (README de la unidad 00): función, parámetro, void
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */

/*
 * EJERCICIO 2: funciones y modularidad
 *
 * Reglas (las mismas del ejercicio 1):
 *   - edad < 3           -> "cachorro"
 *   - edad >= 3 y < 8    -> "adulto"
 *   - edad >= 8          -> "viejo"
 *
 * SALIDA ESPERADA (exacta, idéntica a la del ejercicio 1):
 *   Anio 0: Tarou es cachorro
 *   Anio 1: Tarou es cachorro
 *   Anio 2: Tarou es cachorro
 *   Anio 3: Tarou es adulto
 *   Anio 4: Tarou es adulto
 *   Anio 5: Tarou es adulto
 *   Anio 6: Tarou es adulto
 *   Anio 7: Tarou es adulto
 *   Anio 8: Tarou es viejo
 *   Anio 9: Tarou es viejo
 *   Anio 10: Tarou es viejo
 */

/* TODO 1: define aquí, ANTES de main(), una función

       void imprimirEtapaDeVida(int edad)

   que haga EXACTAMENTE lo mismo que el TODO 3 del ejercicio anterior: el
   if / else if / else con los tres printf, usando el parámetro "edad".
   Pista: es "void" porque no calcula ni devuelve un número; solo imprime un
   mensaje (un efecto), como saludarTarou en demo_02_funciones.c. */

int main(void)                       /* El programa empieza aquí.                         */
{
    /* TODO 2: escribe un for que haga que una variable "edad" vaya de 0 a 10
       (como en el ejercicio anterior), pero esta vez, en vez de repetir el
       if / else if / else dentro del bucle, LLAMA a la función del TODO 1
       pasándole "edad" como argumento. */

    return 0;                        /* Terminó bien.                                     */
}
