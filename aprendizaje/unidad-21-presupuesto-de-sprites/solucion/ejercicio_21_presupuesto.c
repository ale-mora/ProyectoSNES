/*---------------------------------------------------------------------------------
    ejercicio_21_presupuesto.c — SOLUCIÓN del ejercicio de PC de la unidad 21: el
    presupuesto de sprites en pequeño, con datos inventados pero reglas reales.

    PROPÓSITO
      Reescribir desde cero, sin mirar src/juego/subida.c, las cuatro cuentas que
      manda la ROM de prueba de carga:
        TODO 1  tile_pieza: el tile de la pieza k de un cuadro dentro de su hueco
                (pares de filas de 8 piezas), solo con desplazamientos.
        TODO 2  plan_subida: de n piezas a la lista de copias (pares completos de
                2 x 512 B y el último par a medias, 2 x q*64 B).
        TODO 3  vblanks: cuántos VBlanks tarda un plan con un tope de bytes por
                VBlank (siempre avanza: como mínimo una copia).
        TODO 4  max_ranuras: ranuras de 8 px por línea de una escena inventada
                (cada pieza de 16x16 son 2 ranuras en cada una de sus 16 líneas).
        TODO 5  ninjas_que_caben: cuántos ninjas a la misma y caben en 34 ranuras,
                con y sin sombras.

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdint.h>.
      Modelo:   src/juego/subida.c (subida_empezar, subida_tomar, subida_tile_pieza)
                y scripts/preparar_prueba_carga.py (ranuras_por_linea).
      Compilar: gcc -std=c99 -Wall -Wextra -o e21.exe ejercicio_21_presupuesto.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      TAROU, NINJA, SOMBRA   Personaje   los tres «dibujos» inventados de la escena

    SALIDA ESPERADA (exacta)
      1. tiles: 0 14 32 64 78
      2. plan de 21 piezas: 6 copias
           copia 0: origen 0 vram 0 bytes 512
           copia 1: origen 512 vram 256 bytes 512
           copia 2: origen 1024 vram 512 bytes 512
           copia 3: origen 1536 vram 768 bytes 512
           copia 4: origen 2048 vram 1024 bytes 320
           copia 5: origen 2368 vram 1280 bytes 320
      3. vblanks: tope1536=2 tope65535=1 tope1024=3 tope100=6
      4. ranuras: Tarou=10 Tarou+2ninjas=22 Tarou+2ninjas+sombras=34 Tarou+3ninjas+sombras=44
      5. caben: sin sombras=4 con sombras=2
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf.                                     */
#include <stdint.h>                      /* Enteros de tamaño fijo.                     */

typedef uint8_t u8;                      /* Los mismos nombres que el juego.            */
typedef uint16_t u16;

#define RANURAS_MAX 34                   /* Trozos de 8 px de sprite por línea (PPU).   */
#define LINEAS 224                       /* Líneas visibles.                            */

/*--- TODO 1: el tile de la pieza k ----------------------------------------------------
    Las piezas van de 8 en 8 por «par de filas» (32 tiles). Dentro del par, la pieza
    avanza 2 tiles. Fórmula: (k / 8) * 32 + (k % 8) * 2, pero SIN / ni * ni %. */
static u16 tile_pieza(u8 k)
{
    return (u16)((((u16)k >> 3) << 5) | ((u16)(k & 7) << 1));   /* Par * 32 + sitio * 2. */
}

/*--- TODO 2: el plan de copias de un cuadro -------------------------------------------
    Los tiles del cuadro están en el origen en el ORDEN DE SUBIDA: los pares completos
    seguidos (la fila de arriba de 512 B y la de abajo de 512 B) y el último par, si
    tiene q piezas, como su fila de arriba (q * 64 B) y luego la de abajo (q * 64 B).
    En la VRAM la fila de abajo de un par está 256 palabras más allá. Origen y vram
    empiezan en 0. Devuelve cuántas copias hay. */
typedef struct
{
    u16 origen;                          /* Desplazamiento en el origen (bytes).        */
    u16 vram;                            /* Desplazamiento en la VRAM (palabras).       */
    u16 bytes;                           /* Cuántos bytes.                              */
} Copia;

static u8 plan_subida(u8 n_piezas, Copia *plan)
{
    u8 n = 0;                            /* Copias apuntadas.                           */
    u16 origen = 0, vram = 0, medio;

    while (n_piezas >= 8)                /* Un par completo: dos filas de 512 B.        */
    {
        plan[n].origen = origen;         /* Fila de arriba.                             */
        plan[n].vram = vram;
        plan[n].bytes = 512;
        n++;
        plan[n].origen = (u16)(origen + 512);   /* Fila de abajo: 256 palabras más allá. */
        plan[n].vram = (u16)(vram + 256);
        plan[n].bytes = 512;
        n++;
        origen = (u16)(origen + 1024);   /* El par siguiente: 1 KB más allá en el origen... */
        vram = (u16)(vram + 512);        /* ...y 512 palabras más allá en la VRAM.      */
        n_piezas = (u8)(n_piezas - 8);
    }
    if (n_piezas != 0)                   /* El último par, a medias.                    */
    {
        medio = (u16)((u16)n_piezas << 6);   /* q * 64 bytes por fila.                  */
        plan[n].origen = origen;
        plan[n].vram = vram;
        plan[n].bytes = medio;
        n++;
        plan[n].origen = (u16)(origen + medio);
        plan[n].vram = (u16)(vram + 256);
        plan[n].bytes = medio;
        n++;
    }
    return n;
}

/*--- TODO 3: cuántos VBlanks ------------------------------------------------------------
    Cada VBlank toma copias, en orden, mientras la suma de bytes no pase del tope. Si no
    cabe ni una, toma una igual (si no, no avanzaría nunca). */
static u8 vblanks(const Copia *plan, u8 n, u16 tope)
{
    u8 hechas = 0, veces = 0, k;
    u16 total;

    while (hechas < n)
    {
        total = 0;
        k = 0;
        while (hechas < n && (k == 0 || (u16)(total + plan[hechas].bytes) <= tope))
        {
            total = (u16)(total + plan[hechas].bytes);   /* Esta copia entra en el VBlank. */
            hechas++;
            k++;
        }
        veces++;                         /* Un VBlank más.                              */
    }
    return veces;
}

/*--- TODO 4: ranuras por línea de una escena inventada ----------------------------------
    Un Personaje es una pila de filas de 16 px (la de arriba primero): piezas[f] es
    cuántas piezas de 16x16 tiene la fila f. Cada pieza ocupa 2 ranuras en cada una de
    las 16 líneas de su fila. Devuelve el máximo de ranuras en una sola línea. */
typedef struct
{
    int y0;                              /* Línea del borde de arriba.                  */
    u8 filas;                            /* Filas de 16 px.                             */
    u8 piezas[6];                        /* Piezas de 16x16 de cada fila.               */
} Personaje;

static const Personaje TAROU = {112, 4, {2, 4, 5, 5, 0, 0}};     /* 64 px de alto.      */
static const Personaje NINJA = {80, 6, {2, 3, 3, 3, 3, 3}};      /* 96 px; pies en 175. */
static const Personaje SOMBRA = {166, 1, {2, 0, 0, 0, 0, 0}};    /* 16 px: bajo los pies. */

static unsigned max_ranuras(const Personaje *lista, int n)
{
    unsigned ran[LINEAS] = {0};          /* Ranuras de cada línea.                      */
    unsigned maximo = 0;
    int i, f, l;

    for (i = 0; i < n; i++)
    {
        for (f = 0; f < lista[i].filas; f++)
        {
            for (l = lista[i].y0 + 16 * f; l < lista[i].y0 + 16 * f + 16; l++)
            {
                if (l >= 0 && l < LINEAS)
                {
                    ran[l] += 2u * lista[i].piezas[f];   /* 2 ranuras por pieza.        */
                }
            }
        }
    }
    for (l = 0; l < LINEAS; l++)
    {
        if (ran[l] > maximo)
        {
            maximo = ran[l];
        }
    }
    return maximo;
}

/* Escena: Tarou + n ninjas a la misma y (+ la sombra de cada uno y la de Tarou). */
static unsigned escena(int ninjas, int sombras)
{
    Personaje lista[24];                /* 2 + 2 * 8 = 18 como mucho.              */
    int n = 0, i;

    lista[n++] = TAROU;
    if (sombras)
    {
        lista[n++] = SOMBRA;             /* Bajo Tarou.                                 */
    }
    for (i = 0; i < ninjas; i++)
    {
        lista[n++] = NINJA;
        if (sombras)
        {
            lista[n++] = SOMBRA;         /* Bajo cada ninja (misma y: misma sombra).    */
        }
    }
    return max_ranuras(lista, n);
}

/*--- TODO 5: cuántos caben ----------------------------------------------------------------
    El mayor número de ninjas (0..8) con el que la escena no pasa de RANURAS_MAX. */
static int ninjas_que_caben(int sombras)
{
    int n, mejor = 0;

    for (n = 0; n <= 8; n++)
    {
        if (escena(n, sombras) <= RANURAS_MAX)
        {
            mejor = n;
        }
    }
    return mejor;
}

int main(void)
{
    Copia plan[8];
    u8 n, k;

    printf("1. tiles: %u %u %u %u %u\n", tile_pieza(0), tile_pieza(7), tile_pieza(8), tile_pieza(16), tile_pieza(23));

    n = plan_subida(21, plan);
    printf("2. plan de 21 piezas: %u copias\n", n);
    for (k = 0; k < n; k++)
    {
        printf("     copia %u: origen %u vram %u bytes %u\n", k, plan[k].origen, plan[k].vram, plan[k].bytes);
    }

    printf("3. vblanks: tope1536=%u tope65535=%u tope1024=%u tope100=%u\n", vblanks(plan, n, 1536),
           vblanks(plan, n, 65535), vblanks(plan, n, 1024), vblanks(plan, n, 100));

    printf("4. ranuras: Tarou=%u Tarou+2ninjas=%u Tarou+2ninjas+sombras=%u Tarou+3ninjas+sombras=%u\n", escena(0, 0),
           escena(2, 0), escena(2, 1), escena(3, 1));

    printf("5. caben: sin sombras=%d con sombras=%d\n", ninjas_que_caben(0), ninjas_que_caben(1));
    return 0;
}
