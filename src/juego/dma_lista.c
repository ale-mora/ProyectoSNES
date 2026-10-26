/*---------------------------------------------------------------------------------
    dma_lista.c — La parte en C de la lista de copias (ver dma_lista.h).

    PROPÓSITO
      Vaciar la lista y apuntar copias en ella. Corre DURANTE el frame, donde
      hay tiempo de sobra; lo que corre en el VBlank es dma_lista_ejecutar(),
      en dma_ejecutar.asm.

    SE CONECTA CON
      Incluye:  juego/dma_lista.h.
      Escribe:  dma_lista_n y dma_lista_items (en la WRAM, definidas en
                dma_ejecutar.asm).
      Lo usan:  juego/tarou_sprite.c, juego/fondo_nivel.c,
                pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna propia.
    → glosario: DMA / HDMA
---------------------------------------------------------------------------------*/
#include "juego/dma_lista.h"

void dma_lista_vaciar(void)
{
    dma_lista_n = 0;                                 /* Las entradas viejas ya no cuentan.  */
}

void dma_lista_anadir(u16 origen, u16 vram, u16 bytes, u8 banco, u8 vmain)
{
    DmaItem *d;
    if (dma_lista_n >= DMA_LISTA_MAX)
    {
        return;                                      /* Llena: esta copia se pierde.        */
    }
    d = &dma_lista_items[dma_lista_n];               /* 8 bytes por entrada: n << 3.        */
    d->origen = origen;
    d->vram = vram;
    d->bytes = bytes;
    d->banco = banco;
    d->vmain = vmain;
    dma_lista_n++;
}
