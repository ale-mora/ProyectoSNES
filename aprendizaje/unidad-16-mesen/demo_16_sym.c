/*---------------------------------------------------------------------------------
    demo_16_sym.c — Demo de la unidad 16: buscar etiquetas en el .sym para
    poner puntos de ruptura en Mesen.

    PROPÓSITO
      Mesen pone breakpoints y salta a direcciones; el .sym dice dónde está cada
      función y cada variable. Este programa busca en un .sym todas las
      etiquetas que CONTIENEN un texto y las escribe como las pide Mesen
      ($BBAAAA) y separadas ($BB:AAAA). Con un tercer argumento le suma un
      desplazamiento: así se saca la dirección de un CAMPO de una struct
      (tarou + 11 = tarou.en_suelo) para un breakpoint de escritura.
      Uso:  demo_16.exe <rom.sym> <texto> [desplazamiento]

    SE CONECTA CON
      Incluye:  <stdio.h>, <string.h>, <stdlib.h>.
      Lee:      un .sym (formato no$snes: "00BBAAAA etiqueta"), por ejemplo
                E:\ProyectoSNES_trabajo\roms\u16_guion_hud.sym.
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_16.exe demo_16_sym.c

    HARDWARE QUE TOCA
      ninguno (PC).

    VARIABLES
      texto  const char *     lo que se busca dentro del nombre
      suma   unsigned long    desplazamiento a sumar (0 si no se da)
    → conceptos (README de la unidad 16): breakpoint, etiqueta del .sym
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* fopen, fgets, sscanf, printf.                     */
#include <string.h>                  /* strstr.                                           */
#include <stdlib.h>                  /* strtoul.                                          */

int main(int argc, char **argv)
{
    FILE *f;                         /* El .sym.                                          */
    char linea[512];                 /* Una línea.                                        */
    char nombre[400];                /* La etiqueta de la línea.                          */
    unsigned long valor;             /* Su dirección de 24 bits.                          */
    unsigned long suma = 0;          /* Desplazamiento pedido.                            */
    unsigned encontradas = 0;        /* Cuántas coinciden.                                */

    if (argc < 3)
    {
        printf("uso: %s <rom.sym> <texto> [desplazamiento]\n", argv[0]);
        return 1;
    }
    if (argc >= 4)
    {
        suma = strtoul(argv[3], NULL, 0);            /* Admite 11 o 0x0B.                 */
    }
    f = fopen(argv[1], "r");
    if (f == NULL)
    {
        printf("no puedo abrir %s\n", argv[1]);
        return 1;
    }
    while (fgets(linea, sizeof(linea), f) != NULL)
    {
        /* Solo las líneas "8 cifras hex + nombre" que contengan el texto. Las
           etiquetas __local_N y las de línea (L.src...) sobran: se saltan. */
        if (sscanf(linea, "%8lx %399s", &valor, nombre) == 2 && strstr(nombre, argv[2]) != NULL &&
            strstr(nombre, "__local") == NULL && strstr(nombre, "_L.src") == NULL)
        {
            unsigned long d = (valor + suma) & 0xFFFFFF;  /* La dirección + el campo.    */
            if (suma != 0)
            {
                printf("$%06lX  ($%02lX:%04lX)  %s + %lu\n", d, d >> 16, d & 0xFFFF, nombre, suma);
            }
            else
            {
                printf("$%06lX  ($%02lX:%04lX)  %s\n", d, d >> 16, d & 0xFFFF, nombre);
            }
            encontradas++;
        }
    }
    fclose(f);
    printf("%u etiquetas\n", encontradas);
    return 0;
}
