/*---------------------------------------------------------------------------------
    demo_21_presupuesto.c — Demo de la unidad 21: el presupuesto de sprites del beat
    'em up, calculado en el PC con el CÓDIGO REAL y los ARCHIVOS REALES del juego.

    PROPÓSITO
      Repite en el PC las cuentas que la ROM de prueba de carga (-DPRUEBA_CARGA)
      mide en Mesen, para ver de dónde salen las cifras:
        A. Cada cuadro del ninja (y de Tarou) en formato .p16: cuántas piezas de
           16x16, cuántos bytes y cuántos VBlanks tarda en subir (juego/p16.c y
           juego/subida.c, los de la SNES).
        B. La VRAM de sprites: 16 pares de filas (512 tiles) y quién ocupa cada uno.
        C. Las ranuras de 8 px por línea de cada configuración (el límite es 34), con
           la misma cuenta que scripts/preparar_prueba_carga.py (debe dar lo mismo).
        D. El orden por profundidad (juego/profundidad.c) y QUIÉN pierde trozos
           cuando una línea se pasa de 34 (modelo, ver abajo).
        E. El VBlank: cuántas líneas gasta una subida (modelo lineal ajustado a
           las medidas de la spec) y qué pasa si se apunta una copia de más.
        F. La lista de DMA llena (DMA_LISTA_MAX = 32): la copia de más se pierde
           sin avisar (juego/dma_lista.c, el real).
      Lo MEDIDO por este programa: piezas, bytes, copias, VBlanks, ranuras por línea
      (cuentas sobre los archivos reales). Lo que es MODELO está marcado «MODELO»:
      las líneas de VBlank salen de dos puntos medidos en Mesen por la spec
      (docs/diseno-beat-em-up.md §9.1), no de este programa.

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdlib.h>, <string.h>, juego/p16.h, juego/subida.h,
                juego/profundidad.h, juego/dma_lista.h, sistema/tipos.h.
      Lee:      res/prueba_carga/ninja64_andar.bin y ninja64_ataque.bin (versionados) y
                res/personajes/tarou/tarou_caminar.p16 (GENERADO por make; si falta,
                se salta Tarou y lo dice).
      Compilar (desde aprendizaje/unidad-21-presupuesto-de-sprites/, en el clon):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_21.exe
            demo_21_presupuesto.c ../../src/juego/p16.c ../../src/juego/subida.c
            ../../src/juego/profundidad.c ../../src/juego/dma_lista.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      tiras        Tira[3]     las tiras .p16 leídas del disco
      lista        (dma_lista_n / dma_lista_items) la lista de DMA real; en la SNES
                   las define dma_ejecutar.asm, aquí las define este archivo

    → glosario: Ranura por línea, Pieza de 16×16, Par de filas, Orden por profundidad,
      Prueba de carga, DMA / HDMA, VBlank
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf, fopen.                              */
#include <stdlib.h>                      /* exit.                                       */
#include <string.h>                      /* memset.                                     */
#include "juego/p16.h"                   /* p16_cuadro: el índice de una tira.          */
#include "juego/subida.h"                /* subida_empezar / subida_tomar.              */
#include "juego/profundidad.h"           /* profundidad_ordenar.                        */
#include "juego/dma_lista.h"             /* dma_lista_anadir y su tope de 32.           */

/* En la SNES las define dma_ejecutar.asm (WRAM); en el PC, aquí. */
u8 dma_lista_n;
DmaItem dma_lista_items[DMA_LISTA_MAX];

#define RANURAS_MAX 34                   /* Trozos de 8 px de sprite por línea (PPU).   */
#define SPRITES_LINEA_MAX 32             /* Sprites por línea (PPU).                    */
#define LINEAS 224                       /* Líneas visibles.                            */
#define SUELO_Y 176                      /* nivel.h: pies de Tarou en el carril base.   */
#define VBLANK_LINEAS 37                 /* Líneas del VBlank (el SDK gasta 5).         */
#define TOPE_TAROU 1536                  /* prueba_carga.c: bytes de Tarou por VBlank.  */

/*--- Una tira .p16 cargada en memoria -------------------------------------------------*/
typedef struct
{
    const char *nombre;                  /* Para imprimir.                              */
    u8 datos[32768];                     /* Un banco (una tira cabe en 32 KB).          */
    unsigned tam;                        /* Bytes leídos (0 = no se pudo abrir).        */
    u8 cuadros;                          /* Cuántos cuadros (lo deduce del índice).     */
} Tira;

static Tira tiras[3];                    /* 0 ninja andar, 1 ninja ataque, 2 Tarou.     */

/* Lee un archivo entero a la tira. Si no existe, deja tam = 0. */
static void cargar(Tira *t, const char *nombre, const char *ruta)
{
    FILE *f = fopen(ruta, "rb");         /* Binario: los .p16 no son texto.             */
    P16Cuadro c;                         /* El registro del cuadro 0.                   */

    t->nombre = nombre;
    t->tam = 0;
    t->cuadros = 0;
    if (f == NULL)
    {
        return;
    }
    t->tam = (unsigned)fread(t->datos, 1, sizeof t->datos, f);
    fclose(f);
    p16_cuadro(t->datos, 0, &c);         /* La tabla de piezas del cuadro 0 empieza      */
    t->cuadros = (u8)(c.piezas / P16_REGISTRO);  /* justo tras el índice: n cuadros * 8. */
}

/* dy de la pieza k del cuadro f de la tira (la tabla guarda pares dx, dy). */
static unsigned dy_pieza(const Tira *t, u8 f, u8 k)
{
    P16Cuadro c;
    p16_cuadro(t->datos, f, &c);
    return t->datos[c.piezas + 2u * k + 1u];
}

/* El cuadro con más piezas en una misma fila de 16 px (como peor_cuadro del script). */
static u8 peor_cuadro(const Tira *t)
{
    u8 f, k, mejor = 0;
    unsigned fila, cuenta, max_mejor = 0;
    P16Cuadro c;

    for (f = 0; f < t->cuadros; f++)
    {
        p16_cuadro(t->datos, f, &c);
        for (fila = 0; fila < 96; fila += 16)
        {
            cuenta = 0;
            for (k = 0; k < c.n; k++)
            {
                if (dy_pieza(t, f, k) == fila)
                {
                    cuenta++;
                }
            }
            if (cuenta > max_mejor)
            {
                max_mejor = cuenta;
                mejor = f;
            }
        }
    }
    return mejor;
}

/* Piezas del cuadro f en su fila de 16 px más cargada (x2 = ranuras por línea). */
static unsigned max_en_fila(const Tira *t, u8 f)
{
    u8 k;
    unsigned fila, cuenta, mejor = 0;
    P16Cuadro c;

    p16_cuadro(t->datos, f, &c);
    for (fila = 0; fila < 96; fila += 16)
    {
        cuenta = 0;
        for (k = 0; k < c.n; k++)
        {
            if (dy_pieza(t, f, k) == fila)
            {
                cuenta++;
            }
        }
        if (cuenta > mejor)
        {
            mejor = cuenta;
        }
    }
    return mejor;
}

/*--- A. Piezas, bytes y VBlanks por cuadro --------------------------------------------*/
static void parte_a(void)
{
    int i;
    u8 f;
    Subida s;
    const Copia *c;
    u8 n;
    unsigned vb_tope, max_piezas, max_bytes;

    printf("A. CUADROS .p16: piezas, bytes y VBlanks de subida\n");
    for (i = 0; i < 3; i++)
    {
        const Tira *t = &tiras[i];
        P16Cuadro q;

        if (t->tam == 0)
        {
            printf("   %s: no encontrada (la genera make); se salta.\n", t->nombre);
            continue;
        }
        max_piezas = 0;
        max_bytes = 0;
        printf("   %s (%u cuadros, %u bytes en el archivo)\n", t->nombre, t->cuadros, t->tam);
        printf("     cuadro piezas bytes copias VBlanks(tope %d) VBlanks(sin tope)\n", TOPE_TAROU);
        for (f = 0; f < t->cuadros; f++)
        {
            p16_cuadro(t->datos, f, &q);
            subida_empezar(&s, q.n, 0x8000, 0, 0);
            vb_tope = 0;
            while (!subida_terminada(&s))    /* Un VBlank por vuelta, con el tope de Tarou. */
            {
                subida_tomar(&s, TOPE_TAROU, &c, &n);
                vb_tope++;
            }
            if (q.n > max_piezas)
            {
                max_piezas = q.n;
            }
            if ((unsigned)q.n * 128u > max_bytes)
            {
                max_bytes = (unsigned)q.n * 128u;
            }
            subida_empezar(&s, q.n, 0x8000, 0, 0);
            printf("     %5u %6u %5u %6u %9u %17u\n", f, q.n, q.n * 128u, s.n, vb_tope, 1u);
        }
        printf("     máximo: %u piezas = %u bytes = %u tiles\n", max_piezas, max_bytes, max_piezas * 4);
        {
            unsigned fila_max = 0, g;
            for (g = 0; g < t->cuadros; g++)
            {
                if (max_en_fila(t, (u8)g) > fila_max)
                {
                    fila_max = max_en_fila(t, (u8)g);
                }
            }
            printf("     fila de 16 px más cargada: %u piezas = %u ranuras de 8 px en esas líneas\n", fila_max, fila_max * 2);
        }
    }
}

/*--- B. El reparto de la VRAM de sprites ----------------------------------------------*/
static void parte_b(void)
{
    static const char *QUIEN[16] = {
        "protagonista A", "protagonista A", "protagonista A",
        "protagonista B", "protagonista B", "protagonista B",
        "enemigo 1", "enemigo 1", "enemigo 1",
        "enemigo 2", "enemigo 2", "enemigo 2",
        "enemigo 3", "enemigo 3", "enemigo 3",
        "compartido (sombras...)"};
    int par;
    u8 k;

    printf("\nB. VRAM DE SPRITES: 16 pares de filas x 32 tiles = %d tiles (%d KB)\n", 16 * 32, 16 * 32 * 32 / 1024);
    for (par = 0; par < 16; par += 3)
    {
        int hasta = (par + 2 < 16) ? par + 2 : 15;
        printf("   pares %2d-%2d: tiles %3d-%3d  %s\n", par, hasta, par * 32, (hasta + 1) * 32 - 1, QUIEN[par]);
    }
    printf("   Un hueco de 3 pares guarda 24 piezas (8 por par). Tile de la pieza k: (k/8)*32 + (k%%8)*2\n");
    printf("   El hueco del enemigo 1 empieza en el tile 192: sus piezas cuyo tile pasa de 255\n");
    printf("   llevan el bit 8 de tile (el sprite entero NO lo lleva):");
    for (k = 0; k < 24; k++)
    {
        if (192 + subida_tile_pieza(k) >= 256)
        {
            printf(" %u", k);
        }
    }
    printf("\n");
}

/*--- Una entidad para contar ranuras --------------------------------------------------*/
typedef struct
{
    const char *nombre;
    int y0;                              /* Línea del borde de arriba de la celda.      */
    u8 n;                                /* Piezas.                                     */
    u8 dy[24];                           /* dy de cada pieza.                           */
    u16 y_pies;                          /* La clave del orden por profundidad.         */
} Ent;

/* Entidad con las piezas de un cuadro de una tira. */
static Ent de_cuadro(const Tira *t, u8 f, const char *nombre, int y0, u16 y_pies)
{
    Ent e;
    P16Cuadro c;
    u8 k;

    memset(&e, 0, sizeof e);
    p16_cuadro(t->datos, f, &c);
    e.nombre = nombre;
    e.y0 = y0;
    e.n = c.n;
    e.y_pies = y_pies;
    for (k = 0; k < c.n; k++)
    {
        e.dy[k] = (u8)dy_pieza(t, f, k);
    }
    return e;
}

static Ent sombra_ent(const char *nombre, int y0)
{
    Ent e;
    memset(&e, 0, sizeof e);
    e.nombre = nombre;
    e.y0 = y0;
    e.n = 2;                             /* Dos piezas de 16x16 (sombra_piezas), dy = 0. */
    e.y_pies = 0;                        /* y = 0: detrás de todo.                      */
    return e;
}

/* Ranuras y sprites por línea de una lista de entidades; devuelve el máximo y rellena
   cuántas líneas pasan de RANURAS_MAX y cuál es la primera y la última. */
static unsigned contar(const Ent *e, int n, unsigned *max_spr, int *primera, int *ultima, int *pasadas)
{
    unsigned ran[LINEAS], spr[LINEAS], maximo = 0;
    int i, l, k;

    memset(ran, 0, sizeof ran);
    memset(spr, 0, sizeof spr);
    for (i = 0; i < n; i++)
    {
        for (k = 0; k < e[i].n; k++)
        {
            for (l = e[i].y0 + e[i].dy[k]; l < e[i].y0 + e[i].dy[k] + 16; l++)
            {
                if (l >= 0 && l < LINEAS)
                {
                    ran[l] += 2;         /* 16 px = 2 ranuras de 8 px.                  */
                    spr[l] += 1;
                }
            }
        }
    }
    *max_spr = 0;
    *primera = -1;
    *ultima = -1;
    *pasadas = 0;
    for (l = 0; l < LINEAS; l++)
    {
        if (ran[l] > maximo)
        {
            maximo = ran[l];
        }
        if (spr[l] > *max_spr)
        {
            *max_spr = spr[l];
        }
        if (ran[l] > RANURAS_MAX)
        {
            if (*primera < 0)
            {
                *primera = l;
            }
            *ultima = l;
            (*pasadas)++;
        }
    }
    return maximo;
}

/* Las seis configuraciones del script (nombre, desplazamientos y de los ninjas, sombras). */
typedef struct
{
    const char *nombre;
    int n;
    int desp[3];
    int sombras;
} ConfigPred;

static const ConfigPred CONFIGS_PRED[6] = {
    {"Tarou solo", 0, {0, 0, 0}, 0},
    {"Tarou + 1, sombras", 1, {0, 0, 0}, 1},
    {"Tarou + 2, sombras", 2, {0, 0, 0}, 1},
    {"Tarou + 3 misma y, sombras", 3, {0, 0, 0}, 1},
    {"Tarou + 3 repartidos, sombras", 3, {0, 12, 24}, 1},
    {"Tarou + 3 repartidos, sin sombras", 3, {0, 12, 24}, 0},
};

/*--- C. Ranuras por línea: la predicción del script, en C ------------------------------*/
static void parte_c(void)
{
    static const char *ROTULO[2] = {"andando: peor cuadro de andar", "atacando: peor cuadro de ataque"};
    int tira, c, i, primera, ultima, pasadas;
    unsigned max_spr, maxr;
    Ent e[8];
    int ne;
    u8 fn, ft;

    printf("\nC. RANURAS POR LINEA (limite %d ranuras de 8 px y %d sprites), como preparar_prueba_carga.py\n",
           RANURAS_MAX, SPRITES_LINEA_MAX);
    if (tiras[2].tam == 0)
    {
        printf("   Falta la tira de Tarou: no se puede predecir.\n");
        return;
    }
    ft = peor_cuadro(&tiras[2]);
    for (tira = 0; tira < 2; tira++)
    {
        fn = peor_cuadro(&tiras[tira]);
        {
            P16Cuadro q;
            p16_cuadro(tiras[tira].datos, fn, &q);
            printf("   %s (%u piezas)\n", ROTULO[tira], q.n);
        }
        for (c = 0; c < 6; c++)
        {
            ne = 0;
            e[ne++] = de_cuadro(&tiras[2], ft, "Tarou", SUELO_Y - 64, SUELO_Y);
            if (CONFIGS_PRED[c].sombras)
            {
                e[ne++] = sombra_ent("sombra Tarou", SUELO_Y - 8);
            }
            for (i = 0; i < CONFIGS_PRED[c].n; i++)
            {
                e[ne++] = de_cuadro(&tiras[tira], fn, "ninja", SUELO_Y + CONFIGS_PRED[c].desp[i] - 96,
                                    (u16)(SUELO_Y + CONFIGS_PRED[c].desp[i]));
                if (CONFIGS_PRED[c].sombras)
                {
                    e[ne++] = sombra_ent("sombra ninja", SUELO_Y + CONFIGS_PRED[c].desp[i] - 8);
                }
            }
            maxr = contar(e, ne, &max_spr, &primera, &ultima, &pasadas);
            printf("     %-34s max %2u ranuras, %2u sprites; lineas > %d: ", CONFIGS_PRED[c].nombre, maxr,
                   max_spr, RANURAS_MAX);
            if (pasadas == 0)
            {
                printf("ninguna\n");
            }
            else
            {
                printf("%d..%d (%d)\n", primera, ultima, pasadas);
            }
        }
    }
}

/*--- D. Orden por profundidad y quién pierde trozos -----------------------------------*/
static void parte_d(void)
{
    Ent e[8];
    u16 y[8];
    u8 orden[8];
    int ne = 0, i, l, k, o, perdidas[8], total;
    u8 ft, fn;
    unsigned ran;
    int oam_ent[LINEAS][40];             /* Por línea: entidad de cada pieza que la cruza. */
    int cuenta[LINEAS];

    printf("\nD. ORDEN POR PROFUNDIDAD Y QUIEN PIERDE (3 ninjas a la misma y, atacando, con sombras)\n");
    if (tiras[2].tam == 0)
    {
        printf("   Falta la tira de Tarou.\n");
        return;
    }
    ft = peor_cuadro(&tiras[2]);
    fn = peor_cuadro(&tiras[1]);
    /* Como preparar_entidades() de prueba_carga.c: 0 Tarou, 1..3 ninjas, luego sombras. */
    e[ne++] = de_cuadro(&tiras[2], ft, "Tarou", SUELO_Y - 64 + 2, SUELO_Y);
    for (i = 0; i < 3; i++)
    {
        e[ne++] = de_cuadro(&tiras[1], fn, "ninja", SUELO_Y - 96, SUELO_Y);   /* misma y */
    }
    e[ne++] = sombra_ent("sombra Tarou", SUELO_Y - 10);
    for (i = 0; i < 3; i++)
    {
        e[ne++] = sombra_ent("sombra ninja", SUELO_Y - 10);
    }
    for (i = 0; i < ne; i++)
    {
        y[i] = e[i].y_pies;              /* sprites_y: las sombras llevan 0.            */
        orden[i] = (u8)i;                /* sprites_usar: orden de partida 0..n-1.      */
    }
    profundidad_ordenar(orden, y, (u8)ne);   /* El código REAL de la SNES.             */
    printf("   orden de dibujo (primero = entrada más baja de la OAM = DELANTE):");
    for (i = 0; i < ne; i++)
    {
        printf(" %s%d", e[orden[i]].nombre, orden[i]);
    }
    printf("\n   (con la misma y gana el orden de partida: Tarou, entidad 0, queda DELANTE de todos)\n");

    /* Reparto de las entradas de la OAM por línea, en el orden de dibujo. */
    memset(cuenta, 0, sizeof cuenta);
    memset(perdidas, 0, sizeof perdidas);
    for (o = 0; o < ne; o++)
    {
        const Ent *q = &e[orden[o]];
        for (k = 0; k < q->n; k++)
        {
            for (l = q->y0 + q->dy[k]; l < q->y0 + q->dy[k] + 16; l++)
            {
                if (l >= 0 && l < LINEAS && cuenta[l] < 40)
                {
                    oam_ent[l][cuenta[l]++] = orden[o];   /* Crece con el índice de OAM. */
                }
            }
        }
    }
    /* MODELO (spec §8 y §9.1, Mesen): con más de 34 ranuras la PPU se queda con los
       sprites de índice ALTO y descarta los de índice más BAJO (los de delante). */
    total = 0;
    for (l = 0; l < LINEAS; l++)
    {
        ran = (unsigned)cuenta[l] * 2;
        for (i = 0; ran > RANURAS_MAX && i < cuenta[l]; i++, ran -= 2)
        {
            perdidas[oam_ent[l][i]]++;   /* Cae la pieza de índice más bajo de la línea.*/
            total++;
        }
    }
    printf("   líneas-pieza perdidas (cada una = 16 px de una pieza que no se dibuja en esa línea):\n");
    for (i = 0; i < ne; i++)
    {
        if (perdidas[i] != 0)
        {
            printf("     %s %d: %d\n", e[i].nombre, i, perdidas[i]);
        }
    }
    printf("   total %d (el resto de las entidades: 0)\n", total);
}

/*--- E. VBlank: modelo de líneas -------------------------------------------------------*/
/* MODELO: la spec (§9.1, Mesen) midió que con el medio cuadro de Tarou (1,5 KB) el
   VBlank llega a la línea 17..19, y que una subida de enemigo de 2 KB acaba en la 32
   y una de 3 KB en la 39: unas 7 líneas por KB. Recta por esos puntos:
   línea ~= 7,5 + 7 * (KB subidos en total). Es una cota gruesa (±3 líneas). */
static double linea_vblank(unsigned bytes)
{
    return 7.5 + 7.0 * (double)bytes / 1024.0;
}

static void parte_e(void)
{
    unsigned i, bytes_ninja = 0, bytes_tarou = TOPE_TAROU, hud = 64;
    P16Cuadro q;
    u8 f;
    double v;

    printf("\nE. VBLANK (MODELO lineal: linea = 7,5 + 7 * KB; medidas de la spec: Tarou solo 17-19,\n");
    printf("   1 ninja 34, 2 ninjas 36, con dos subidas de ninja por VBlank 43-50)\n");
    for (f = 0; f < tiras[0].cuadros; f++)
    {
        p16_cuadro(tiras[0].datos, f, &q);
        if ((unsigned)q.n * 128u > bytes_ninja)
        {
            bytes_ninja = (unsigned)q.n * 128u;
        }
    }
    for (f = 0; f < tiras[1].cuadros; f++)
    {
        p16_cuadro(tiras[1].datos, f, &q);
        if ((unsigned)q.n * 128u > bytes_ninja)
        {
            bytes_ninja = (unsigned)q.n * 128u;
        }
    }
    printf("   peor cuadro de ninja: %u bytes\n", bytes_ninja);
    v = linea_vblank(bytes_tarou);
    printf("   Tarou solo (%u B)                         -> linea %.1f\n", bytes_tarou, v);
    v = linea_vblank(bytes_tarou + bytes_ninja);
    printf("   Tarou + 1 subida de ninja (%u B)         -> linea %.1f\n", bytes_tarou + bytes_ninja, v);
    v = linea_vblank(bytes_tarou + bytes_ninja + hud);
    printf("   ... y una fila de HUD (+%u B)              -> linea %.1f\n", hud, v);
    v = linea_vblank(bytes_tarou + bytes_ninja + 2 * hud);
    printf("   ... y OTRA fila de tilemap (+%u B más)     -> linea %.1f  (%s)\n", hud, v,
           v > VBLANK_LINEAS ? "PASA DE 37: la ultima copia se pierde" : "cabe");
    v = linea_vblank(bytes_tarou + 2 * bytes_ninja);
    printf("   Tarou + 2 subidas de ninja (%u B)        -> linea %.1f  (%s)\n", bytes_tarou + 2 * bytes_ninja, v,
           v > VBLANK_LINEAS ? "NO CABE" : "cabe");
    /* Qué copia cae fuera, en el orden en que prueba_carga.c apunta la lista:
       Tarou, ninja, fila de HUD, fila de HUD extra. */
    printf("   En qué línea acaba cada copia de la lista (orden: Tarou, ninja, HUD, HUD extra):\n");
    {
        unsigned acumulado = 0;
        unsigned tam[4];
        const char *quien[4] = {"Tarou", "ninja", "HUD", "HUD extra"};
        tam[0] = bytes_tarou;
        tam[1] = bytes_ninja;
        tam[2] = hud;
        tam[3] = hud;
        for (i = 0; i < 4; i++)
        {
            acumulado += tam[i];
            v = linea_vblank(acumulado);
            printf("     %-10s %5u B  acaba en la linea %5.1f %s\n", quien[i], tam[i], v,
                   v > VBLANK_LINEAS ? "<- fuera del VBlank: no se dibuja" : "");
        }
    }
    /* Cuánto sobra en el peor frame (Tarou + el peor cuadro de ninja), según el modelo. */
    v = ((double)VBLANK_LINEAS - 7.5) / 7.0 * 1024.0 - (double)(bytes_tarou + bytes_ninja);
    printf("   margen del peor frame (Tarou + peor ninja) hasta la linea %d: ~%.0f bytes (MODELO, +-3 lineas = +-440 B)\n",
           VBLANK_LINEAS, v);
}

/*--- F. La lista de DMA llena ----------------------------------------------------------*/
static void parte_f(void)
{
    int i;

    printf("\nF. LISTA DE DMA LLENA (DMA_LISTA_MAX = %d): dma_lista_anadir.c, el real\n", DMA_LISTA_MAX);
    dma_lista_vaciar();
    for (i = 0; i < DMA_LISTA_MAX + 3; i++)
    {
        dma_lista_anadir((u16)(0x8000 + i), (u16)(0x1000 + 32 * i), 64, 0x7E, DMA_VMAIN_TILES);
    }
    printf("   apuntadas %d copias, en la lista quedan %u. Las 3 últimas se perdieron, sin error ni aviso.\n",
           DMA_LISTA_MAX + 3, (unsigned)dma_lista_n);
    printf("   Peor caso de prueba_carga.c: Tarou 3 + ninja 6 + fila de HUD 1 = 10 entradas (con 2\n");
    printf("   subidas de ninja, 16): NO es esta limitación la que cuenta; es el VBlank (parte E).\n");
}

int main(void)
{
    cargar(&tiras[0], "ninja64_andar.bin", "../../res/prueba_carga/ninja64_andar.bin");
    cargar(&tiras[1], "ninja64_ataque.bin", "../../res/prueba_carga/ninja64_ataque.bin");
    cargar(&tiras[2], "tarou_caminar.p16", "../../res/personajes/tarou/tarou_caminar.p16");
    if (tiras[0].tam == 0 || tiras[1].tam == 0)
    {
        printf("No encuentro res/prueba_carga/*.bin: ejecuta esto desde aprendizaje/unidad-21-.../ del clon.\n");
        exit(1);
    }
    parte_a();
    parte_b();
    parte_c();
    parte_d();
    parte_e();
    parte_f();
    return 0;
}
