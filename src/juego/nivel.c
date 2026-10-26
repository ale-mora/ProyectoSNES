/*---------------------------------------------------------------------------------
    nivel.c — El suelo del nivel 1 (plano, por ahora).

    PROPÓSITO
      Implementa nivel_columna_vram(): dónde va cada columna del nivel en el
      mapa-anillo de la VRAM. (Hasta el tutorial también daba la altura del
      suelo, siempre la misma; ahora el escenario lo responde juego/colision.c.)

    SE CONECTA CON
      Incluye:  juego/nivel.h.
      Lo usan:  juego/fondo_nivel.c, tests/prueba_camara.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Nivel, Streaming de columnas
---------------------------------------------------------------------------------*/
#include "juego/nivel.h"


u16 nivel_columna_vram(u16 col)
{
    /* (col & 32) vale 0 o 32: la página. Por 32 (<< 5) da 0 o 0x400 palabras, el
       comienzo de la segunda página. (col & 31) es la posición dentro de la página.
       Sin multiplicar ni dividir: solo máscaras y desplazamientos. */
    return (u16)(((col & 32) << 5) | (col & 31));
}
