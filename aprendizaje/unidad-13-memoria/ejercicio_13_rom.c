/*---------------------------------------------------------------------------------
    ejercicio_13_rom.c — Ejercicio 13, parte A (ESQUELETO: completa los TODO
    1..4; la solución está en solucion/ejercicio_13_rom.c): buscar una etiqueta
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

    /* TODO 1: buscar la etiqueta argv[3] en el .sym argv[1]. Cada línea útil es
       "00BBAAAA nombre" (8 cifras hexadecimales, un espacio, la etiqueta); las que
       empiezan por ';' son comentarios. Abre con fopen(..., "r"), lee con fgets y
       separa con sscanf(linea, "%8lx %399s", &valor, nombre). Si la encuentras,
       guarda valor en dir24 y pon encontrada = 1. Si no, imprime
       "<etiqueta>: no esta en <sym>" y devuelve 1. */

    /* TODO 2: separar el banco (bits 16..23 de dir24) y la dirección dentro del
       banco (bits 0..15). */
    banco = 0;
    dir = 0;

    /* TODO 3: si el banco es $7E o $7F, o dir < $8000, imprimir
       "<etiqueta>: $BB:AAAA no esta en la ROM (WRAM o registros)" y salir con 0.
       Si es ROM, calcular el byte del .sfc (LoROM: 32 KB por banco, en la mitad
       alta; el bit 7 del banco solo dice "rápido") e imprimir
       "<etiqueta>: $BB:AAAA -> byte 0xNNNNNN del .sfc". */
    fichero = 0;

    /* TODO 4: con 5 argumentos, leer el archivo de origen argv[4] entero (fread a
       `origen`, n_origen bytes), leer n_origen bytes del .sfc desde `fichero`
       (fseek + fread a `rom`) y compararlos: imprimir
       "<etiqueta>: los N bytes coinciden con <archivo>" o
       "<etiqueta>: distinto en el byte K". Con 4 argumentos, leer 1 byte e
       imprimir "<etiqueta>: primer byte = $XX". */
    (void)f; (void)linea; (void)nombre; (void)valor; (void)dir24; (void)encontrada;
    (void)n_origen; (void)n_rom; (void)k; (void)banco; (void)dir; (void)fichero;
    (void)rom; (void)origen;
    return 0;
}
