/*---------------------------------------------------------------------------------
    traza_23_salto.c — Traza de la unidad 23: cuánto tiempo y a qué altura está Tarou en el
    aire en un salto (con el código REAL de mundo.c). Sirve para elegir COMBATE_ESQUIVE.

    PROPÓSITO
      Salta una vez, sin otra tecla, e imprime por cada cuadro z (12.4), la altura en
      píxeles (mundo_altura) y si ese cuadro esquivaría un tajo con el umbral de la
      parte B del ejercicio (12 px). Al final, el resumen: cuadros en el aire, altura
      máxima y cuántos cuadros están por encima del umbral.

    SE CONECTA CON
      Incluye:  <stdio.h>, <string.h>, juego/mundo.h.
      Compilar (desde esta carpeta, en una sola línea):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o traza_23.exe traza_23_salto.c
            ../../src/juego/mundo.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES: c (Cuerpo), m (Mando), n (cuadro), aire, maxima, esquiva (contadores).
---------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "juego/mundo.h"

#define UMBRAL 12                                    /* El COMBATE_ESQUIVE de la parte B.   */

int main(void)
{
    Cuerpo c;
    Mando m;
    int n, aire = 0, maxima = 0, esquiva = 0;

    mundo_iniciar(&c, 400, 200);
    memset(&m, 0, sizeof(m));
    m.saltar_pulsado = 1;                            /* B recién pulsada en el cuadro 0.    */
    printf("cuadro      z  altura(px)  vz  esquiva?\n");
    for (n = 0; n < 60; n++)
    {
        mundo_actualizar(&c, &m);
        m.saltar_pulsado = 0;
        if (c.z == 0 && n > 0)
        {
            printf("%5d  %5d  %9d  %3d  (aterrizó)\n", n, c.z, 0, c.vz);
            break;
        }
        aire++;
        if ((int)mundo_altura(&c) > maxima)
        {
            maxima = (int)mundo_altura(&c);
        }
        if (mundo_altura(&c) >= UMBRAL)
        {
            esquiva++;
        }
        printf("%5d  %5d  %9d  %3d  %s\n", n, c.z, (int)mundo_altura(&c), c.vz,
               mundo_altura(&c) >= UMBRAL ? "SI" : "no");
    }
    printf("Cuadros en el aire: %d. Altura máxima: %d px. Cuadros con altura >= %d: %d.\n", aire,
           maxima, UMBRAL, esquiva);
    return 0;
}
