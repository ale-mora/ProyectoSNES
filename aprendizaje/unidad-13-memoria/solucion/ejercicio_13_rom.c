/*---------------------------------------------------------------------------------
    ejercicio_13_rom.c — SOLUCIÓN del ejercicio 13, parte A: buscar una etiqueta
    en el .sym, convertir su dirección de 24 bits en un byte del .sfc y comparar
    lo que hay ahí con el archivo de origen.

    PROPÓSITO
      Demostrar sin emulador que data.asm hace lo que dice: que los 1792 bytes de
      res/nivel1/colision.bin están DENTRO de la ROM en la dirección que el .sym
      da a nivel1_colision, y que `.db :nivel1_cerca_col` guardó el banco
      correcto. Uso:
        ejercicio_13.exe <rom.sym> <rom.sfc> <etiqueta> [archivo_de_origen]
      Sin archivo, imprime el primer byte que hay en esa dirección.

    SE CONECTA CON
      Incluye:  <stdio.h>, <string.h>.
      Lee:      el .sym y el .sfc de una compilación del juego (tu clon) y, si se
                da, el archivo de origen (por ejemplo res/nivel1/colision.bin).
      Compilar: gcc -std=c99 -Wall -Wextra -o ejercicio_13.exe ejercicio_13_rom.c

    HARDWARE QUE TOCA
      ninguno (PC).

    VARIABLES
      buscada  const char *     la etiqueta pedida
      dir24    unsigned long    su dirección de 24 bits según el .sym
      banco    unsigned         los 8 bits altos
      dir      unsigned         los 16 bits bajos
      fichero  unsigned long    el byte del .sfc donde empieza
    → conceptos (README de la unidad 13): .sym, banco, LoROM, .incbin
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* fopen, fgets, sscanf, fread, printf.              */
#include <string.h>                  /* strcmp.                                           */

#define MAX_COMPARAR 65536           /* Lo más que compara (un asset no pasa de 32 KB).   */

static unsigned char rom[MAX_COMPARAR];      /* Bytes leídos del .sfc.                    */
static unsigned char origen[MAX_COMPARAR];   /* Bytes del archivo de origen.              */

int main(int argc, char **argv)
{
    FILE *f;                         /* El archivo que se esté leyendo.                   */
    char linea[512];                 /* Una línea del .sym.                               */
    char nombre[400];                /* La etiqueta de esa línea.                         */
    unsigned long valor;             /* El número de esa línea.                           */
    unsigned long dir24 = 0;         /* La dirección de la etiqueta buscada.              */
    int encontrada = 0;              /* 1 cuando aparece.                                 */
    unsigned banco, dir;             /* Banco y dirección dentro del banco.               */
    unsigned long fichero;           /* Byte del .sfc.                                    */
    size_t n_origen, n_rom, k;       /* Tamaños y contador.                               */

    if (argc < 4)
    {
        printf("uso: %s <rom.sym> <rom.sfc> <etiqueta> [archivo_de_origen]\n", argv[0]);
        return 1;
    }

    /* TODO 1: buscar la etiqueta en el .sym. Cada línea útil es "00BBAAAA nombre"
       (8 cifras hexadecimales, un espacio, la etiqueta); las que empiezan por ';'
       son comentarios. */
    f = fopen(argv[1], "r");
    if (f == NULL)
    {
        printf("no puedo abrir %s\n", argv[1]);
        return 1;
    }
    while (fgets(linea, sizeof(linea), f) != NULL)
    {
        if (sscanf(linea, "%8lx %399s", &valor, nombre) == 2 && strcmp(nombre, argv[3]) == 0)
        {
            dir24 = valor;           /* La primera que coincida.                          */
            encontrada = 1;
            break;
        }
    }
    fclose(f);
    if (!encontrada)
    {
        printf("%s: no esta en %s\n", argv[3], argv[1]);
        return 1;
    }

    /* TODO 2: separar el banco (bits 16..23) de la dirección dentro del banco. */
    banco = (unsigned)((dir24 >> 16) & 0xFF);
    dir = (unsigned)(dir24 & 0xFFFF);

    /* TODO 3: si no es ROM (banco $7E/$7F, o dirección por debajo de $8000),
       decirlo y salir; si es ROM, el byte del archivo en LoROM. */
    if (banco == 0x7E || banco == 0x7F || dir < 0x8000)
    {
        printf("%s: $%02X:%04X no esta en la ROM (WRAM o registros)\n", argv[3], banco, dir);
        return 0;
    }
    fichero = (unsigned long)(banco & 0x7F) * 0x8000UL + (dir & 0x7FFFUL);
    printf("%s: $%02X:%04X -> byte 0x%06lX del .sfc\n", argv[3], banco, dir, fichero);

    /* TODO 4: leer de la ROM y comparar con el archivo de origen (o enseñar el
       primer byte si no se dio archivo). */
    n_origen = 1;                    /* Sin archivo: un byte.                             */
    if (argc >= 5)
    {
        f = fopen(argv[4], "rb");
        if (f == NULL)
        {
            printf("no puedo abrir %s\n", argv[4]);
            return 1;
        }
        n_origen = fread(origen, 1, MAX_COMPARAR, f);
        fclose(f);
    }
    f = fopen(argv[2], "rb");
    if (f == NULL || fseek(f, (long)fichero, SEEK_SET) != 0)
    {
        printf("no puedo leer %s\n", argv[2]);
        return 1;
    }
    n_rom = fread(rom, 1, n_origen, f);
    fclose(f);
    if (argc < 5)
    {
        printf("%s: primer byte = $%02X\n", argv[3], rom[0]);
        return 0;
    }
    for (k = 0; k < n_origen && k < n_rom && rom[k] == origen[k]; k++)
    {
    }
    if (k == n_origen)
    {
        printf("%s: los %lu bytes coinciden con %s\n", argv[3], (unsigned long)n_origen, argv[4]);
    }
    else
    {
        printf("%s: distinto en el byte %lu\n", argv[3], (unsigned long)k);
    }
    return 0;
}
