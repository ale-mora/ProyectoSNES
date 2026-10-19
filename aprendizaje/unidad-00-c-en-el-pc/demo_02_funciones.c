/*---------------------------------------------------------------------------------
    demo_02_funciones.c — Demo 2 de la unidad 00: funciones.

    PROPÓSITO
      Dos funciones de ejemplo: una que recibe un número y devuelve otro
      (duplicar) y una "void" que no devuelve nada y solo tiene un efecto
      (saludarTarou). Es la base del ejercicio 2.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_02.exe demo_02_funciones.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      vidas         int   3     vidas de partida
      vidasDobles   int   6     lo que devuelve duplicar(vidas)

    → conceptos (README de la unidad 00): función, parámetro, void
---------------------------------------------------------------------------------*/
#include <stdio.h>

// Funcion que RECIBE un entero (numero) y DEVUELVE otro entero (el doble).
// "int" antes del nombre es el tipo de lo que devuelve.
int duplicar(int numero) {
    return numero * 2;
}

// Funcion "void": no devuelve ningun valor, solo tiene un efecto (acá,
// imprimir un mensaje). Por eso no lleva "return valor;" al final.
void saludarTarou(int edad) {
    printf("Hola, soy Tarou y tengo %d anios.\n", edad);
}

int main(void) {
    int vidas = 3;

    // Llamamos a "duplicar", le pasamos "vidas" como argumento, y
    // guardamos lo que devuelve en una variable nueva.
    int vidasDobles = duplicar(vidas);

    printf("Vidas originales: %d\n", vidas);
    printf("Vidas duplicadas (power-up): %d\n", vidasDobles);

    // Llamamos a una funcion void: no guardamos nada, porque no
    // devuelve nada. Solo nos interesa el efecto (que imprima).
    saludarTarou(5);

    return 0;
}
