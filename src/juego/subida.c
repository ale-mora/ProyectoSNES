/*---------------------------------------------------------------------------------
    subida.c — Plan de copias de un cuadro .p16 y su reparto (ver subida.h).

    PROPÓSITO
      subida_empezar recorre los pares de filas del cuadro: por cada par
      completo, dos copias de 512 B (la fila de arriba y, 256 palabras más
      abajo en la VRAM, la de abajo); por el último par a medias con q piezas,
      dos copias de q·64 B. En el origen todo va seguido (orden de subida del
      .p16). Solo sumas y desplazamientos: nada de multiplicar.
      subida_tomar entrega copias en orden mientras quepan en el tope.

    SE CONECTA CON
      Incluye:  juego/subida.h.
      Lo usan:  pantallas/prueba_carga.c y tests/prueba_subida.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo en la struct Subida del que llama).
    → glosario: Pieza de 16×16, Par de filas
---------------------------------------------------------------------------------*/
#include "juego/subida.h"

/*--- apuntar: añade una copia al plan --------------------------------------------*/
static void apuntar(Subida *s, u16 origen, u16 vram, u16 bytes)
{
    Copia *c = &s->plan[s->n];                       /* plan[n]: 8 bytes, un desplazamiento. */

    c->origen = origen;
    c->vram = vram;
    c->bytes = bytes;
    s->n++;
}

void subida_empezar(Subida *s, u8 n_piezas, u16 origen, u16 vram, u8 banco)
{
    u8 resto;
    u16 medio;

    s->n = 0;
    s->hechas = 0;
    s->banco = banco;
    if (n_piezas > SUBIDA_PIEZAS_MAX)
    {
        n_piezas = SUBIDA_PIEZAS_MAX;                /* No cabe más en un hueco.            */
    }
    resto = n_piezas;
    while (resto >= 8)                               /* Pares completos: 2 filas de 512 B.  */
    {
        apuntar(s, origen, vram, SUBIDA_FILA_BYTES);
        apuntar(s, (u16)(origen + SUBIDA_FILA_BYTES), (u16)(vram + SUBIDA_FILA_PALABRAS),
                SUBIDA_FILA_BYTES);
        origen = (u16)(origen + 2 * SUBIDA_FILA_BYTES);
        vram = (u16)(vram + 2 * SUBIDA_FILA_PALABRAS);
        resto = (u8)(resto - 8);
    }
    if (resto != 0)                                  /* El último par, a medias: q·64 B.    */
    {
        medio = (u16)((u16)resto << 6);
        apuntar(s, origen, vram, medio);
        apuntar(s, (u16)(origen + medio), (u16)(vram + SUBIDA_FILA_PALABRAS), medio);
    }
}

u16 subida_tomar(Subida *s, u16 tope, const Copia **desde, u8 *cuantas)
{
    const Copia *c = &s->plan[s->hechas];
    u16 total = 0;
    u8 k = 0;

    *desde = c;
    while (s->hechas < s->n)
    {
        if (k != 0 && (u16)(total + c->bytes) > tope)
        {
            break;                                   /* Lo que queda, al VBlank siguiente.  */
        }
        total = (u16)(total + c->bytes);
        s->hechas++;
        k++;
        c++;
    }
    *cuantas = k;
    return total;
}

u16 subida_restante(const Subida *s)
{
    const Copia *c = &s->plan[s->hechas];
    u16 total = 0;
    u8 i;

    for (i = s->hechas; i < s->n; i++)
    {
        total = (u16)(total + c->bytes);
        c++;
    }
    return total;
}

u8 subida_terminada(const Subida *s)
{
    return (u8)(s->hechas >= s->n);
}

u16 subida_tile_pieza(u8 k)
{
    return (u16)((((u16)k >> 3) << 5) | ((u16)(k & 7) << 1));
}
