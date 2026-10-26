/*---------------------------------------------------------------------------------
    ejercicio_20_rejilla.c — SOLUCIÓN del ejercicio de PC de la unidad 20: una
    rejilla de colisión propia, en pequeño, con las cuatro piezas del juego.

    PROPÓSITO
      Reescribir desde cero, sin mirar src/juego/, las cuatro ideas de la
      colisión del tutorial:
        TODO 1  celda_en: de píxel a celda con desplazamientos (sin * ni /).
        TODO 2  se_tocan: la prueba AABB de dos cajas.
        TODO 3  mover_x: integrar x y retroceder píxel a píxel contra lo sólido.
        TODO 4  aterriza: plataforma de un solo sentido (solo cuenta si los pies
                CRUZAN su borde de arriba bajando).
      Cada parte imprime su resultado; la salida esperada está al final.

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdint.h>, <string.h> (memset).
      Modelo:   src/juego/colision.c (colision_en, colision_columna_solida),
                src/juego/partida.c (se_tocan), src/juego/tarou.c (mover_x,
                mover_y).
      Compilar: gcc -std=c99 -Wall -Wextra -o e20.exe ejercicio_20_rejilla.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      rejilla  u8[128*14]  1 byte por celda de 16 px, fila a fila (como el juego)

    SALIDA ESPERADA (exacta)
      1. celda(40,130)=2 celda(85,200)=1 celda(10,10)=0 celda(2048,0)=1 celda(10,300)=0
      2. solapan=1 rozan=0 dentro=1 lejos=0
      3. x=53 x=56 x=59 x=62 x=64
      4. cruza=128 no_cruza=-1 sin_plataforma=-1 desde_abajo=-1 suelo=208
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf.                                     */
#include <stdint.h>                      /* Enteros de tamaño fijo.                     */
#include <string.h>                      /* memset.                                     */

typedef uint8_t u8;                      /* Los mismos nombres que el juego.            */
typedef uint16_t u16;
typedef int16_t s16;

#define COLS 128                         /* Columnas: por eso el índice es fila << 7.   */
#define FILAS 14                         /* Filas: 224 px de alto.                      */
#define CELDA 16                         /* Lado de una celda.                          */
#define VACIO 0
#define SOLIDO 1
#define PLATAFORMA 2

static u8 rejilla[COLS * FILAS];         /* El escenario.                               */

/* TODO 1: lo que hay en el píxel (x, y). A los lados del nivel (x >= 2048, y la x
   "negativa", que en u16 es enorme) hay pared; por debajo (y >= 224), vacío. */
static u8 celda_en(u16 x, u16 y)
{
    if (x >= COLS * CELDA)
    {
        return SOLIDO;                   /* Fuera por los lados: pared.                 */
    }
    if (y >= FILAS * CELDA)
    {
        return VACIO;                    /* Por debajo: se cae.                         */
    }
    return rejilla[((y >> 4) << 7) | (x >> 4)];   /* fila * 128 + columna.              */
}

/* TODO 2: 1 si las cajas a y b se solapan (AABB): cada una empieza antes de que
   acabe la otra, en x Y en y. Tocarse borde con borde NO es solapar. */
static u8 se_tocan(s16 ax, s16 ay, s16 aw, s16 ah, s16 bx, s16 by, s16 bw, s16 bh)
{
    return (u8)(ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah);
}

/* TODO 3: mueve una caja (x, y, ancho, alto) vx píxeles en x. Si su borde
   delantero entra en algo SÓLIDO, retrocede de píxel en píxel hasta no tocarlo.
   Devuelve la x nueva. */
static s16 mover_x(s16 x, s16 vx, s16 y, s16 ancho, s16 alto)
{
    s16 borde, fila;
    u8 choca;
    x = (s16)(x + vx);                   /* Integrar.                                   */
    do
    {
        borde = (s16)(vx > 0 ? x + ancho - 1 : x);   /* El borde que va delante.        */
        choca = 0;
        for (fila = y; fila < y + alto; fila++)      /* Toda la columna del borde.      */
        {
            if (celda_en((u16)borde, (u16)fila) == SOLIDO)
            {
                choca = 1;
            }
        }
        if (choca)
        {
            x = (s16)(vx > 0 ? x - 1 : x + 1);       /* Un píxel hacia atrás.           */
        }
    } while (choca);
    return x;
}

/* TODO 4: los pies bajan de pies_antes a pies_ahora (pies = la fila justo debajo
   de la caja). Devuelve la fila del primer borde de celda que CRUZAN, en o bajo
   pies_antes, en el que la celda de la columna x sostiene (sólido o plataforma);
   -1 si no cruzan ninguno. Primer borde: (pies_antes + 15) & ~15. */
static s16 aterriza(s16 pies_antes, s16 pies_ahora, u16 x)
{
    s16 fila;
    u8 c;
    for (fila = (s16)((pies_antes + 15) & ~15); fila <= pies_ahora; fila += CELDA)
    {
        c = celda_en(x, (u16)fila);
        if (c == SOLIDO || c == PLATAFORMA)
        {
            return fila;                 /* Los pies se quedan justo encima del borde.  */
        }
    }
    return -1;
}

int main(void)
{
    s16 x = 50;                          /* Parte 3: la caja empieza aquí.              */
    u8 i;

    memset(rejilla, VACIO, sizeof(rejilla));
    memset(rejilla + 13 * COLS, SOLIDO, COLS);           /* Suelo: fila 13 (y 208).     */
    memset(rejilla + 8 * COLS + 2, PLATAFORMA, 2);       /* Plataforma: x 32..63, y 128. */
    for (i = 10; i < 13; i++)
    {
        rejilla[i * COLS + 5] = SOLIDO;                  /* Muro: x 80..95, y 160..207. */
    }

    printf("1. celda(40,130)=%u celda(85,200)=%u celda(10,10)=%u celda(2048,0)=%u celda(10,300)=%u\n",
           celda_en(40, 130), celda_en(85, 200), celda_en(10, 10), celda_en(2048, 0), celda_en(10, 300));
    printf("2. solapan=%u rozan=%u dentro=%u lejos=%u\n",
           se_tocan(0, 0, 10, 10, 5, 5, 10, 10),         /* Se pisan 5x5 px.            */
           se_tocan(0, 0, 10, 10, 10, 0, 10, 10),        /* Borde con borde.            */
           se_tocan(0, 0, 40, 40, 10, 10, 5, 5),         /* Una dentro de otra.         */
           se_tocan(0, 0, 10, 10, 50, 50, 10, 10));
    printf("3.");
    for (i = 0; i < 5; i++)
    {
        x = mover_x(x, 3, 170, 16, 16);  /* Caja de 16x16 a 3 px por frame.             */
        printf(" x=%d", x);
    }
    printf("\n");
    printf("4. cruza=%d no_cruza=%d sin_plataforma=%d desde_abajo=%d suelo=%d\n",
           aterriza(120, 131, 40),       /* Baja de 120 a 131 sobre la plataforma.      */
           aterriza(120, 126, 40),       /* No llega a su borde (128).                  */
           aterriza(120, 131, 70),       /* En x 70 no hay plataforma.                  */
           aterriza(129, 140, 40),       /* Ya estaba por debajo del borde: la atraviesa. */
           aterriza(200, 215, 40));      /* El suelo, sólido.                           */
    return 0;
}
