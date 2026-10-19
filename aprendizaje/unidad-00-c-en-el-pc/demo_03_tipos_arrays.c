#include <stdio.h>

int main(void) {
    // Un array de 5 enteros: el puntaje que Tarou consigue en cada nivel.
    int puntajePorNivel[5] = {120, 340, 90, 500, 275};

    // Los indices de un array van de 0 a (tamanio - 1). Este array tiene
    // 5 elementos, entonces los indices validos son 0, 1, 2, 3 y 4.
    printf("Puntaje del nivel 1 (indice 0): %d\n", puntajePorNivel[0]);
    printf("Puntaje del nivel 3 (indice 2): %d\n", puntajePorNivel[2]);

    // Recorrer todo el array con un for, sumando el puntaje total.
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total = total + puntajePorNivel[i];
    }
    printf("Puntaje total: %d\n", total);

    // "unsigned char" es un tipo mas chico que "int": ocupa 1 solo byte
    // (guarda numeros de 0 a 255). Es el mismo tipo que en PVSnesLib vas
    // a ver llamado "u8" -- se usa mucho en SNES para ahorrar memoria.
    unsigned char vidas = 3;
    printf("Vidas (guardadas en 1 byte): %d\n", vidas);

    return 0;
}
