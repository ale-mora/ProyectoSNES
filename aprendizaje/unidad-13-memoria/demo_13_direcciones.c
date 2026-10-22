/*---------------------------------------------------------------------------------
    demo_13_direcciones.c — Demo de la unidad 13: qué memoria hay detrás de una
    dirección de 24 bits del 65816.

    PROPÓSITO
      Toma etiquetas reales del .sym del juego (copiadas aquí: las de una
      compilación normal del 2026-10-22, rama demo-secuencia-arranque) y, para
      cada una, separa el BANCO (8 bits altos) de la dirección dentro del banco
      (16 bits bajos), dice qué memoria es (WRAM, ROM rápida, ROM lenta) y, si es
      ROM, en qué byte del archivo .sfc está (mapa LoROM). Al final enseña por
      qué 128 bancos numerados desde $00 (SlowROM) pisarían la WRAM.

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdint.h>.
      Modelo:   ProyectoSNES.sym (no$snes: "00BBAAAA etiqueta"), data.asm,
                docs/hardware-snes-por-modulo.md §6 y §7.
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_13.exe demo_13_direcciones.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      ETIQUETAS  const Etiqueta[]  nombre y dirección de 24 bits de cada etiqueta
    → conceptos (README de la unidad 13): banco, LoROM, FastROM, WRAM, etiqueta
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* printf.                                           */
#include <stdint.h>                  /* uint32_t: una dirección de 24 bits cabe en 32.    */

typedef struct
{
    const char *nombre;              /* La etiqueta, como sale en el .sym.                */
    uint32_t dir;                    /* Su dirección de 24 bits: banco y 16 bits.         */
} Etiqueta;

/* Copiadas de ProyectoSNES.sym (compilación normal del 2026-10-22). Cambian con
   cada compilación que mueva algo: búscalas siempre en TU .sym. */
static const Etiqueta ETIQUETAS[] = {
    {"main", 0x8ED147},                                   /* código C (.text)             */
    {"tarou_actualizar", 0x8BFCE0},                       /* código C                     */
    {"nivel1_cerca_col", 0x81B3BA},                       /* data.asm, .incbin            */
    {"nivel1_cerca_col_banco", 0x80FFB1},                 /* data.asm, .db :etiqueta      */
    {"nivel1_colision", 0x86F900},                        /* data.asm, .incbin            */
    {"tarou_reposo_til", 0x828000},                       /* data.asm: empieza un banco   */
    {"ESCENA_NINTENDO", 0x7F01FA},                        /* const global de C            */
    {"ANIMACIONES", 0x7F0150},                            /* const global de C            */
    {"..._controles (static const)", 0x0ED5B2},           /* static const de C            */
    {"..._tarou (static Tarou)", 0x7E2631},               /* variable static de C         */
    {"frames_en_pantalla", 0x7E26E9},                     /* variable global de C         */
};

#define CANTIDAD (sizeof(ETIQUETAS) / sizeof(ETIQUETAS[0]))   /* Cuántas hay.             */

int main(void)
{
    unsigned i;                      /* Índice de la etiqueta.                            */

    printf("1. etiqueta                        banco  dir    memoria         byte del .sfc\n");
    for (i = 0; i < CANTIDAD; i++)
    {
        uint32_t banco = (ETIQUETAS[i].dir >> 16) & 0xFF;   /* Los 8 bits altos.          */
        uint32_t dir = ETIQUETAS[i].dir & 0xFFFF;           /* Los 16 bits bajos.         */
        const char *que;                                    /* Qué memoria es.            */
        int es_rom = 0;                                     /* 1 si está en el cartucho.  */

        if (banco == 0x7E || banco == 0x7F)
        {
            que = "WRAM";                                   /* 128 KB: $7E0000-$7FFFFF.   */
        }
        else if (dir >= 0x8000 && banco >= 0x80)
        {
            que = "ROM rapida";                             /* FastROM: bancos $80-$FF.   */
            es_rom = 1;
        }
        else if (dir >= 0x8000)
        {
            que = "ROM lenta";                              /* La misma ROM vista en $00-$7D. */
            es_rom = 1;
        }
        else
        {
            que = "registros/WRAM";                         /* $0000-$7FFF de los bancos bajos. */
        }

        if (es_rom)
        {
            /* LoROM: cada banco ve 32 KB de la ROM en su mitad alta. El banco n
               (sin el bit 7, que solo dice "rápido") es el trozo n del archivo. */
            uint32_t fichero = (banco & 0x7F) * 0x8000 + (dir & 0x7FFF);
            printf("1. %-32s $%02X   $%04X  %-15s 0x%06X\n", ETIQUETAS[i].nombre,
                   (unsigned)banco, (unsigned)dir, que, (unsigned)fichero);
        }
        else
        {
            printf("1. %-32s $%02X   $%04X  %-15s -\n", ETIQUETAS[i].nombre,
                   (unsigned)banco, (unsigned)dir, que);
        }
    }

    /* 2. 128 bancos de 32 KB = 4 MB. Numerados desde $00 llegan a $7F... */
    printf("2. SlowROM: 128 bancos = $00..$%02X (los dos ultimos son $7E y $7F: la WRAM)\n", 0x00 + 127);
    printf("2. FastROM: 128 bancos = $%02X..$%02X (ninguno choca con la WRAM)\n", 0x80, 0x80 + 127);
    printf("2. 128 x 32768 = %u bytes\n", 128u * 32768u);
    return 0;
}
