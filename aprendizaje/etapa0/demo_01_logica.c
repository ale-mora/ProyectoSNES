#include <stdio.h>

int main(void) {
    int vidas = 3;

    printf("Tarouh empieza con %d vidas.\n", vidas);

    for (int obstaculo = 1; obstaculo <= 5; obstaculo++) {
        if (vidas > 0) {
            vidas = vidas - 1;
            printf("Obstaculo %d: Tarouh choco. Vidas restantes: %d\n", obstaculo, vidas);
        } else {
            printf("Obstaculo %d: Tarouh ya no tiene vidas. Fin del intento.\n", obstaculo);
        }
    }

    if (vidas > 0) {
        printf("Tarouh termino el nivel con %d vidas de sobra.\n", vidas);
    } else {
        printf("Game Over.\n");
    }

    return 0;
}
