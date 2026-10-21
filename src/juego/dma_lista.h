/*---------------------------------------------------------------------------------
    dma_lista.h — Lista de copias a la VRAM que se preparan en el frame y se
    disparan de un tirón en el VBlank.

    PROPÓSITO
      La VRAM solo se puede escribir en el VBlank (37 líneas) y, para no salirse
      de él, lo que se haga ahí tiene que ser lo mínimo. Durante el frame, cada
      módulo que necesite copiar algo a la VRAM (los tiles del cuadro de Tarou,
      las columnas del fondo) lo APUNTA en esta lista con dma_lista_anadir();
      en el VBlank, dma_lista_ejecutar() (en ensamblador, dma_ejecutar.asm)
      recorre la lista disparando un DMA por entrada. Así el VBlank cuesta lo
      que cuesta el DMA y casi nada más.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: dma_lista.c (vaciar, añadir) y dma_ejecutar.asm (ejecutar y la
                memoria de la lista).
      Lo usan:  juego/tarou_sprite.c, juego/fondo_nivel.c,
                pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      ninguno aquí (dma_ejecutar.asm: canal 0 de DMA, VMAIN, VMADD).

    VARIABLES (viven en dma_ejecutar.asm, en la WRAM)
      dma_lista_n       u8          cuántas copias hay (0..DMA_LISTA_MAX)
      dma_lista_items   DmaItem[32] las copias, en orden de ejecución

    → glosario: DMA / HDMA, VRAM, VBlank
---------------------------------------------------------------------------------*/
#ifndef DMA_LISTA_H
#define DMA_LISTA_H

#include "sistema/tipos.h"

#define DMA_LISTA_MAX 32             /* Tarou (hasta 16) + columnas (hasta 6) + margen.   */
#define DMA_VMAIN_TILES 0x80         /* Tras cada palabra, la dirección de VRAM avanza 1. */
#define DMA_VMAIN_COLUMNA 0x81       /* Avanza 32: baja una fila del mapa de 32 de ancho. */

/* Una copia: 8 bytes, tal como los lee dma_ejecutar.asm (no cambiar el orden). */
typedef struct
{
    u16 origen;                      /* Dirección del origen dentro de su banco (16 bits). */
    u16 vram;                        /* Dirección de la VRAM, en palabras.                */
    u16 bytes;                       /* Cuántos bytes.                                    */
    u8 banco;                        /* Banco de la ROM del origen.                       */
    u8 vmain;                        /* DMA_VMAIN_TILES o DMA_VMAIN_COLUMNA.              */
} DmaItem;

extern u8 dma_lista_n;
extern DmaItem dma_lista_items[DMA_LISTA_MAX];

/* Deja la lista vacía. Al empezar cada frame. */
void dma_lista_vaciar(void);

/* Apunta una copia. Si la lista está llena, no hace nada (y el dibujo se queda
   como estaba: mejor que escribir fuera de la lista). */
void dma_lista_anadir(u16 origen, u16 vram, u16 bytes, u8 banco, u8 vmain);

/* Dispara todas las copias de la lista, en orden (dma_ejecutar.asm). Solo en el
   VBlank o con la pantalla apagada. NO vacía la lista: eso lo hace el frame. */
void dma_lista_ejecutar(void);

#endif /* DMA_LISTA_H */
