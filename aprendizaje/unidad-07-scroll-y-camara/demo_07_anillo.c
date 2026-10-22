/*---------------------------------------------------------------------------------
    demo_07_anillo.c — Demo de PC de la unidad 07: la cámara, el parallax y el
    mapa-anillo, con los números del nivel 1.

    PROPÓSITO
      Simula a Tarou caminando por el nivel de 2048 px e imprime, para algunas
      posiciones, lo que el juego calcula en cada frame:
        - la cámara (camara_para: x + 48 - 128, recortada a 0..1792);
        - el scroll de las dos capas (1:1 y la mitad) y dónde cae dentro de su
          mapa de 512 px (lo que la PPU usa de verdad: los 9 bits bajos);
        - la columna del borde izquierdo y las dos columnas que fondo_nivel.c
          apunta al cambiar de columna (col + 33 y col - 1), con su sitio en el
          anillo de 64x32 (nivel_columna_vram).
      Y, al principio, la trampa de restar en u16 cerca del borde izquierdo.

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdint.h>.
      Lo usan:  nadie: programa independiente de PC.
      Modelo:   src/juego/camara.c, src/juego/nivel.c, src/juego/fondo_nivel.c
                (fondo_nivel_actualizar), src/pantallas/pantalla_nivel1.c (paso 3).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_07.exe demo_07_anillo.c

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      xs[]   u16  posiciones de la celda de Tarou que se imprimen
    → conceptos (README de la unidad 07): cámara, recorte, parallax, mapa-anillo
---------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

typedef uint16_t u16;                /* Los nombres del juego.                            */
typedef int16_t s16;

#define ANCHO_NIVEL 2048             /* src/juego/nivel.h                                 */
#define ANCHO_PANTALLA 256
#define ANCHO_CELDA 96
#define COLUMNAS_NIVEL 256

/* camara_para de src/juego/camara.c, tal cual. */
static u16 camara_para(u16 x_pix)
{
    s16 camara = (s16)((s16)x_pix + (ANCHO_CELDA / 2) - (ANCHO_PANTALLA / 2));
    if (camara < 0)
    {
        camara = 0;
    }
    if (camara > ANCHO_NIVEL - ANCHO_PANTALLA)
    {
        camara = ANCHO_NIVEL - ANCHO_PANTALLA;
    }
    return (u16)camara;
}

/* nivel_columna_vram de src/juego/nivel.c, tal cual. */
static u16 nivel_columna_vram(u16 col)
{
    return (u16)(((col & 32) << 5) | (col & 31));
}

/* Imprime una columna del nivel y su sitio en el anillo (o "fuera"). */
static void columna(const char *nombre, u16 col)
{
    if (col >= COLUMNAS_NIVEL)       /* fondo_nivel.c no apunta las de fuera del nivel.  */
    {
        printf("  %s %5s -> fuera", nombre, col == 0xFFFF ? "-1" : "256+");
        return;
    }
    printf("  %s %3u -> +0x%03X", nombre, col, nivel_columna_vram(col));
}

int main(void)
{
    static const u16 xs[] = {0, 80, 81, 200, 432, 433, 944, 1500, 1872, 1952};

    /*--- 1. La trampa del u16 ---------------------------------------------------------*/
    u16 mal = (u16)(10 + 48 - 128);  /* Lo que daría la cuenta en u16.                  */
    printf("1. x = 10: en s16, 10 + 48 - 128 = %d; en u16 = %u (por eso camara_para usa s16)\n",
           10 + 48 - 128, mal);

    /*--- 2. La tabla ------------------------------------------------------------------*/
    printf("2.    x  camara  cerca(&511)  lejos=cam/2(&511)  col | columnas que entran (anillo)\n");
    for (unsigned k = 0; k < sizeof xs / sizeof xs[0]; k++)
    {
        u16 cam = camara_para(xs[k]);
        u16 col = (u16)(cam >> 3);               /* Columna de tiles del borde izquierdo. */
        printf("   %4u   %4u   %4u          %4u  (%3u)       %3u |", xs[k], cam, cam & 511,
               cam >> 1, (cam >> 1) & 511, col);
        columna("der", (u16)(col + 33));
        columna(" izq", (u16)(col - 1));
        printf("\n");
    }

    /*--- 3. El anillo da la vuelta cada 64 columnas (512 px) --------------------------*/
    printf("3. Columnas que comparten sitio en el anillo:");
    for (u16 c = 4; c < COLUMNAS_NIVEL; c += 64)
    {
        printf(" %u", c);
    }
    printf(" -> todas en +0x%03X\n", nivel_columna_vram(4));
    printf("   Pagina 2 (columnas 32..63): la 40 va a +0x%03X = 0x400 + 8\n", nivel_columna_vram(40));
    return 0;
}
