#include <stdio.h>

/*
 * EJERCICIO 2: Funciones y modularidad
 *
 * Objetivo: tomar la logica que ya escribiste en el Ejercicio 1 (decidir
 * si Tarouh es cachorro/adulto/viejo segun su edad) y convertirla en una
 * FUNCION reutilizable, en vez de tenerla pegada adentro de un bucle en
 * main(). Esta es la idea central de "modularidad": sacar un pedazo de
 * logica que hace una sola cosa clara, ponerle un nombre, y poder
 * llamarlo desde donde haga falta sin repetir el codigo.
 *
 * Reglas (las mismas del Ejercicio 1):
 *   - edad < 3         -> "cachorro"
 *   - edad >= 3 y < 8   -> "adulto"
 *   - edad >= 8         -> "viejo"
 *
 * La salida esperada tiene que ser IDENTICA a la del ejercicio_01_logica.c
 * ya corregido.
 */

// TODO 1: define aca, ANTES de main(), una funcion:
//
void imprimirEtapaDeVida(int edad)
{
    if (edad <= 2)
    {
        printf("Tarouh es Cachorro y tiene %d anio. \n", edad);
    }
    else if (edad < 8)
    {
        printf("Tarouh es Adulto y tiene %d anios. \n", edad);
    }
    else
    {
        printf("Tarouh es Viejito y tiene %d anios. \n", edad);
    }
};

// que haga EXACTAMENTE lo mismo que ya escribiste en el ejercicio
// anterior: el if / else if / else con los tres printf, usando el
// parametro "edad" en vez de la variable que usabas antes.
// Pista: es "void" porque la funcion no calcula ni devuelve un numero,
// solo imprime un mensaje (efecto), como "saludarTarouh" en el demo.

int main(void)
{
    // TODO 2: escribi un bucle for que haga que una variable "edad" vaya
    //         de 0 a 10 (igual que en el ejercicio anterior), pero esta
    //         vez, en vez de repetir el if/else if/else adentro del
    //         bucle, simplemente LLAMA a la funcion que definiste en el
    //         TODO 1, pasandole "edad" como argumento.

    for (int edad = 0; edad <= 10; edad++)
    // Los argumentos de la funcion van entre parentesis, y se separan con comas si hay mas de uno.
    // En este caso solo hay uno: "edad". int al principio de la funcion indica que devuelve un entero,
    // pero como es void, no devuelve nada. El void está en la definición de la función
    // imprimirEtapaDeVida(), no en la llamada.
    {
        imprimirEtapaDeVida(edad);
    }

    return 0;
}
