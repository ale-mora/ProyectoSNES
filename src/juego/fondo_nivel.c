/*---------------------------------------------------------------------------------
    fondo_nivel.c — La capa cercana del nivel 1 dibujada por columnas (ver .h).

    PROPÓSITO
      Decidir, durante el frame, qué columnas del mapa del nivel hay que copiar
      a la VRAM, y apuntarlas en la lista de DMA. El mapa vive en la ROM
      ordenado COLUMNA A COLUMNA (scripts/mapa_a_columnas.py): las 32 entradas
      de una columna son 64 bytes seguidos, justo lo que un DMA sabe copiar.
      En la VRAM, en cambio, una columna son 32 entradas separadas 32 palabras
      (una fila del mapa): para eso la copia va con VMAIN en modo "avanzar 32
      tras cada palabra" (DMA_VMAIN_COLUMNA). Hasta el 2026-10-20 este archivo
      disparaba los DMA él mismo en el VBlank: preparar dos copias en C costaba
      9 líneas de las 37, y junto a los tiles de Tarou se salía del VBlank.

    SE CONECTA CON
      Incluye:  juego/fondo_nivel.h, juego/nivel.h, juego/dma_lista.h,
                sistema/comun.h.
      Datos:    nivel1_cerca_til/_til_end/_pal, nivel1_cerca_col y
                nivel1_cerca_col_banco (data.asm, .rodata_nivel1_cerca/_banco).
      Lo usa:   pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      VRAM y CGRAM por el SDK (bgInitTileSet, bgSetMapPtr) al cargar. Las
      columnas las copia dma_ejecutar.asm; aquí solo se calcula de dónde a dónde.

    VARIABLES (estáticas)
      parche_col     u16  primera columna del nivel con parche (el tronco)
      parche_fuente  u16  de qué columna del mapa se toman (0 = sin parche)
      reescribir     u8   1 = el parche se quitó: volver a copiar sus columnas
      ultima_col     u16  columna del borde izquierdo en el último frame (0xFFFF = ninguna)
    VARIABLES (constantes)
      BYTES_COLUMNA     64   32 filas x 2 bytes (una columna del mapa en la ROM)
      COLUMNAS_ANILLO   64   columnas del mapa de la VRAM (64x32)
      ANTES             15   columnas que se rellenan a la izquierda de la cámara
    → glosario: Streaming de columnas, DMA / HDMA, VRAM, Tilemap
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/fondo_nivel.h"
#include "juego/nivel.h"
#include "juego/dma_lista.h"
#include "sistema/comun.h"

extern char nivel1_cerca_til, nivel1_cerca_til_end, nivel1_cerca_pal; /* data.asm       */
extern char nivel1_cerca_col;        /* 260 columnas x 64 bytes, columna a columna.        */
extern u8 nivel1_cerca_col_banco;    /* El banco de la ROM donde está nivel1_cerca_col.    */

#define BYTES_COLUMNA 64             /* 32 filas x 2 bytes por entrada del mapa.          */
#define COLUMNAS_ANILLO 64           /* El mapa de la VRAM: 64 columnas (512 px).          */
#define ANTES 15                     /* Al rellenar: 15 a la izquierda, 49 a la derecha.  */
#define NINGUNA 0xFFFF               /* ultima_col: todavía no se ha apuntado ninguna.     */

static u16 parche_col;               /* Primera columna del nivel que se sustituye.        */
static u16 parche_fuente;            /* Columna del mapa de donde salen (0 = ninguna).     */
static u8 reescribir;                /* 1 = quitaron el parche: copiar otra vez esas columnas. */
static u16 ultima_col;               /* Columna del borde izquierdo en el frame anterior.  */
static u16 i;

void fondo_nivel_parche(u16 col, u16 fuente)
{
    parche_col = col;
    parche_fuente = fuente;
    reescribir = 0;
}

void fondo_nivel_quitar_parche(void)
{
    parche_fuente = 0;                               /* Desde ahora, las columnas de siempre. */
    reescribir = 1;                                  /* Y hay que redibujarlas.             */
}

/*--- apuntar_columna: la columna `col` del nivel, a su sitio del anillo ------------
    Con col fuera del nivel no hace nada (los bordes del nivel; también los
    "negativos", que en u16 dan la vuelta). Origen: la columna `fuente` del mapa por
    columnas (col << 6 = col x 64 bytes), en el banco que dice data.asm. */
static void apuntar_columna(u16 col)
{
    u16 fuente;
    if (col >= COLUMNAS_NIVEL)
    {
        return;
    }
    fuente = col;                                    /* Lo normal: la misma columna.        */
    if (parche_fuente != 0 && (u16)(col - parche_col) < PARCHE_COLUMNAS)
    {
        fuente = (u16)(parche_fuente + (col - parche_col)); /* Dentro del parche: con tronco. */
    }
    dma_lista_anadir((u16)((u16)&nivel1_cerca_col + (fuente << 6)),
                     (u16)(VRAM_MAPA_A + nivel_columna_vram(col)),
                     BYTES_COLUMNA, nivel1_cerca_col_banco, DMA_VMAIN_COLUMNA);
}

void fondo_nivel_rellenar(u16 camara)
{
    u16 primera;
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
    for (i = 0; i < COLUMNAS_ANILLO; i++)            /* Las 64 columnas del anillo, de una  */
    {                                                /* en una (la pantalla está apagada:   */
        dma_lista_vaciar();                          /* no hay prisa y la lista es corta).  */
        apuntar_columna(primera + i);
        dma_lista_ejecutar();
    }
    dma_lista_vaciar();                              /* La lista queda vacía.               */
    ultima_col = NINGUNA;                            /* El próximo frame apunta sus bordes. */
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

void fondo_nivel_actualizar(u16 camara)
{
    u16 col;
    col = camara >> 3;                               /* Columna del borde izquierdo.        */
    if (col != ultima_col)                           /* Entró en una columna nueva:         */
    {
        apuntar_columna(col + 33);                   /* la siguiente a la última visible... */
        apuntar_columna(col - 1);                    /* ...y la anterior a la primera.      */
        ultima_col = col;
    }
    if (reescribir)                                  /* El tronco se rompió: sus columnas,  */
    {                                                /* las de debajo, si están en el anillo. */
        reescribir = 0;
        for (i = 0; i < PARCHE_COLUMNAS; i++)
        {
            /* Solo las que están a la vista (de col - 1 a col + 33): una columna lejana
               caería en el sitio del anillo de otra que SÍ se ve y la estropearía. */
            if ((u16)(parche_col + i + 1 - col) <= 34)
            {
                apuntar_columna(parche_col + i);
            }
        }
    }
}
