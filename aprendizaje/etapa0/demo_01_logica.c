#include <stdio.h>

int main(void) {
    int vidas = 3;

    printf("Tarou empieza con %d vidas.\n", vidas);

    for (int obstaculo = 1; obstaculo <= 5; obstaculo++) {
        if (vidas > 0) {
            vidas = vidas - 1;
            printf("Obstaculo %d: Tarou choco. Vidas restantes: %d\n", obstaculo, vidas);
        } else {
            printf("Obstaculo %d: Tarou ya no tiene vidas. Fin del intento.\n", obstaculo);
        }
    }

    if (vidas > 0) {
        printf("Tarou termino el nivel con %d vidas de sobra.\n", vidas);
    } else {
        printf("Game Over.\n");
    }

    return 0;
}
