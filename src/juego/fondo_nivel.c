/*---------------------------------------------------------------------------------
    fondo_nivel.c — La capa cercana del nivel 1 dibujada por columnas (ver .h).

    PROPÓSITO
      Copiar a la VRAM, con DMA, las columnas del mapa del nivel que la cámara
      va a mostrar. El mapa vive en la ROM ordenado COLUMNA A COLUMNA
      (scripts/mapa_a_columnas.py): las 32 entradas de una columna son 64
      bytes seguidos, justo lo que un DMA sabe copiar. En la VRAM, en cambio,
      una columna son 32 entradas separadas 32 palabras (una fila del mapa):
      para eso el registro VMAIN tiene el modo "avanzar 32 tras cada palabra".

    SE CONECTA CON
      Incluye:  juego/fondo_nivel.h, juego/nivel.h, sistema/comun.h.
      Datos:    nivel1_cerca_til/_til_end/_pal, nivel1_cerca_col y
                nivel1_cerca_col_banco (data.asm, .rodata_nivel1_cerca/_banco).
      Lo usa:   pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      $2115 VMAIN   0x81 = avanzar 32 palabras tras escribir el byte alto
                    (bajar una fila); se deja en 0x80 (avanzar 1), que es lo que
                    esperan las funciones del SDK.
      $2116 VMADD   dirección de la VRAM donde empieza la columna.
      $4300..$4305  canal 0 de DMA: modo 1 (dos registros, $2118 y $2119),
                    origen de 24 bits (dirección + banco) y 64 bytes.
      $420B MDMAEN  bit 0 = arrancar el canal 0. La CPU espera a que acabe.
      No usa el canal 6: es el del cielo (HDMA, pantalla_nivel1.c).

    VARIABLES (constantes)
      BYTES_COLUMNA     64   32 filas x 2 bytes (una columna del mapa en la ROM)
      COLUMNAS_ANILLO   64   columnas del mapa de la VRAM (64x32)
      ANTES             15   columnas que se rellenan a la izquierda de la cámara
    → glosario: Streaming de columnas, DMA, VRAM, Tilemap
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/fondo_nivel.h"
#include "juego/nivel.h"
#include "sistema/comun.h"

extern char nivel1_cerca_til, nivel1_cerca_til_end, nivel1_cerca_pal; /* data.asm       */
extern char nivel1_cerca_col;        /* 256 columnas x 64 bytes, columna a columna.        */
extern u8 nivel1_cerca_col_banco;    /* El banco de la ROM donde está nivel1_cerca_col.    */

#define BYTES_COLUMNA 64             /* 32 filas x 2 bytes por entrada del mapa.          */
#define COLUMNAS_ANILLO 64           /* El mapa de la VRAM: 64 columnas (512 px).          */
#define ANTES 15                     /* Al rellenar: 15 a la izquierda, 49 a la derecha.  */

/*--- copiar_columna: la columna `col` del nivel a su sitio del anillo ------------
    Con col fuera del nivel no hace nada (los bordes del nivel). */
static void copiar_columna(u16 col)
{
    if (col >= COLUMNAS_NIVEL)                       /* También "negativos": u16 da la vuelta. */
    {
        return;
    }
    REG_VMAIN = 0x81;                                /* Tras cada palabra, +32: baja una fila. */
    REG_VMADDLH = VRAM_MAPA_A + nivel_columna_vram(col); /* Primera fila de la columna.   */
    REG_DMAP0 = 0x01;                                /* Modo 1: bytes a $2118, $2119, ...      */
    REG_BBAD0 = 0x18;                                /* Destino: $2118 (dato de la VRAM).      */
    REG_A1T0LH = (u16)&nivel1_cerca_col + (col << 6); /* Origen: col * 64 bytes más allá.   */
    REG_A1B0 = nivel1_cerca_col_banco;               /* ...en el banco del mapa.               */
    REG_DAS0LH = BYTES_COLUMNA;                      /* 64 bytes = 32 entradas.                */
    REG_MDMAEN = 0x01;                               /* ¡Ya! (la CPU se para hasta que acabe). */
    REG_VMAIN = 0x80;                                /* Lo normal: avanzar 1 palabra.          */
}

void fondo_nivel_rellenar(u16 camara)
{
    u16 primera, i;
    primera = camara >> 3;                           /* Columna del borde izquierdo.        */
    if (primera > ANTES)                             /* Un poco de margen a la izquierda... */
    {
        primera -= ANTES;
    }
    else
    {
        primera = 0;                                 /* ...salvo al principio del nivel.    */
    }
    if (primera > COLUMNAS_NIVEL - COLUMNAS_ANILLO)  /* Y sin pasarse del final.            */
    {
        primera = COLUMNAS_NIVEL - COLUMNAS_ANILLO;
    }
    for (i = 0; i < COLUMNAS_ANILLO; i++)            /* Las 64 columnas del anillo.         */
    {
        copiar_columna(primera + i);
    }
}

void fondo_nivel_cargar(u16 camara)
{
    /* Tiles a VRAM_FONDO_A y la paleta en la 1 (los tiles del mapa la llevan: -e 1). */
    bgInitTileSet(0, (u8 *)&nivel1_cerca_til, (u8 *)&nivel1_cerca_pal, 1,
                  (u16)(&nivel1_cerca_til_end - &nivel1_cerca_til), 16 * 2, BG_16COLORS, VRAM_FONDO_A);
    bgSetMapPtr(0, VRAM_MAPA_A, SC_64x32);           /* Mapa de 64x32: el anillo.           */
    fondo_nivel_rellenar(camara);                    /* Lo que se ve y algo más.            */
    bgSetEnable(0);
}

void fondo_nivel_vblank(u16 camara)
{
    u16 col;
    col = camara >> 3;                               /* Columna del borde izquierdo.        */
    copiar_columna(col + 33);                        /* La siguiente a la última visible.   */
    copiar_columna(col - 1);                         /* La anterior a la primera (si col es
                                                        0, da 65535 y no se copia).         */
}
