#include <stdio.h>

/*
 * EJERCICIO 3: Tipos de datos y arrays
 *
 * Objetivo: declarar un array, recorrerlo con un for, acumular un total,
 * y usar un if adentro del bucle para contar cuantos elementos cumplen
 * una condicion.
 *
 * Datos: vidas perdidas por Tarouh en cada uno de los primeros 5 niveles:
 *   Nivel 1: 2 vidas perdidas
 *   Nivel 2: 0 vidas perdidas
 *   Nivel 3: 1 vida perdida
 *   Nivel 4: 0 vidas perdidas
 *   Nivel 5: 3 vidas perdidas
 *
 * Salida esperada:
 *   Nivel 1: 2 vidas perdidas
 *   Nivel 2: 0 vidas perdidas
 *   Nivel 3: 1 vidas perdidas
 *   Nivel 4: 0 vidas perdidas
 *   Nivel 5: 3 vidas perdidas
 *   Total de vidas perdidas: 6
 *   Niveles perfectos (0 vidas perdidas): 2
 */

int main(void) {
    // TODO 1: declara un array de 5 enteros llamado "vidasPerdidas" con
    //         los valores de arriba, en orden: {2, 0, 1, 0, 3}

    // TODO 2: declara dos variables enteras, inicializadas en 0:
    //         "total" y "nivelesPerfectos" (para ir acumulando adentro
    //         del bucle)

    // TODO 3: escribi un for que recorra el array de indice 0 a 4 y,
    //         en cada vuelta:
    //           - imprima "Nivel %d: %d vidas perdidas" -- OJO: el
    //             numero de nivel es el INDICE + 1 (los niveles
    //             empiezan en 1, los indices en 0)
    //           - sume vidasPerdidas[i] a "total"
    //           - si vidasPerdidas[i] es igual a 0, sume 1 a
    //             "nivelesPerfectos"

    // TODO 4: despues de que termine el for, imprimi:
    //         "Total de vidas perdidas: %d"
    //         "Niveles perfectos (0 vidas perdidas): %d"

    return 0;
}
