#include <stdio.h>

// Funcion que RECIBE un entero (numero) y DEVUELVE otro entero (el doble).
// "int" antes del nombre es el tipo de lo que devuelve.
int duplicar(int numero) {
    return numero * 2;
}

// Funcion "void": no devuelve ningun valor, solo tiene un efecto (acá,
// imprimir un mensaje). Por eso no lleva "return valor;" al final.
void saludarTarouh(int edad) {
    printf("Hola, soy Tarouh y tengo %d anios.\n", edad);
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
    saludarTarouh(5);

    return 0;
}
