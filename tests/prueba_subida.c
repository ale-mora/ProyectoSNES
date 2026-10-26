/*---------------------------------------------------------------------------------
    prueba_subida.c — Pruebas de juego/subida.c.

    PROPÓSITO
      Fijar el plan de copias de un cuadro en piezas de 16x16 (formato .p16,
      docs/diseno-beat-em-up.md §8 v3) y su reparto entre VBlanks:
        - un par completo son 2 copias de 512 B (fila de arriba y de abajo,
          256 palabras más abajo en la VRAM);
        - el último par con q piezas son 2 copias de q·64 B;
        - el total es siempre n·128 bytes;
        - subida_tomar da las copias en orden sin pasarse del tope (salvo que no
          se haya tomado ninguna: entonces la primera, para avanzar siempre).

    SE CONECTA CON
      Incluye:  minitest.h, juego/subida.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_subida).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Pieza de 16×16, Par de filas, DMA / HDMA
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/subida.h"

void pruebas_subida(void)
{
    Subida s;
    const Copia *c;
    u8 n;
    u16 b;

    /* 10 piezas: un par completo (2 x 512) y 2 piezas a medias (2 x 128). */
    subida_empezar(&s, 10, 0x8000, 0x1000, 0x85);
    COMPROBAR(s.n == 4);
    COMPROBAR(s.plan[0].origen == 0x8000 && s.plan[0].vram == 0x1000 && s.plan[0].bytes == 512);
    COMPROBAR(s.plan[1].origen == 0x8200 && s.plan[1].vram == 0x1100 && s.plan[1].bytes == 512);
    COMPROBAR(s.plan[2].origen == 0x8400 && s.plan[2].vram == 0x1200 && s.plan[2].bytes == 128);
    COMPROBAR(s.plan[3].origen == 0x8480 && s.plan[3].vram == 0x1300 && s.plan[3].bytes == 128);
    COMPROBAR(subida_restante(&s) == 10 * 128);
    COMPROBAR(s.banco == 0x85);

    /* Reparto con tope de 1536: primer VBlank 512+512 (la siguiente de 128 cabe: 1152). */
    b = subida_tomar(&s, 1536, &c, &n);
    COMPROBAR(n == 4 && b == 1280);
    COMPROBAR(c == &s.plan[0]);
    COMPROBAR(subida_terminada(&s));

    /* 24 piezas (3 pares): 6 copias de 512; con tope de 1536, 3 + 3 (dos VBlanks). */
    subida_empezar(&s, 24, 0, 0, 0);
    COMPROBAR(s.n == 6);
    COMPROBAR(s.plan[5].vram == 0x0500 && s.plan[5].bytes == 512);
    b = subida_tomar(&s, 1536, &c, &n);
    COMPROBAR(n == 3 && b == 1536 && !subida_terminada(&s));
    COMPROBAR(subida_restante(&s) == 1536);
    b = subida_tomar(&s, 1536, &c, &n);
    COMPROBAR(n == 3 && b == 1536 && c == &s.plan[3] && subida_terminada(&s));

    /* Sin nada tomado, la primera copia sale aunque pase del tope (para avanzar). */
    subida_empezar(&s, 8, 0, 0, 0);
    b = subida_tomar(&s, 100, &c, &n);
    COMPROBAR(n == 1 && b == 512);

    /* Un cuadro vacío: sin copias, terminado desde el principio. */
    subida_empezar(&s, 0, 0, 0, 0);
    COMPROBAR(s.n == 0 && subida_terminada(&s) && subida_restante(&s) == 0);
    b = subida_tomar(&s, 1536, &c, &n);
    COMPROBAR(n == 0 && b == 0);

    /* El tile de la pieza k dentro del bloque: k/8*32 + k%8*2. */
    COMPROBAR(subida_tile_pieza(0) == 0);
    COMPROBAR(subida_tile_pieza(7) == 14);
    COMPROBAR(subida_tile_pieza(8) == 32);
    COMPROBAR(subida_tile_pieza(21) == 74);
}
