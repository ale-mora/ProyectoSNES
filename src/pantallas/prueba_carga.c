/*---------------------------------------------------------------------------------
    prueba_carga.c — ROM de prueba de carga del beat 'em up (ver prueba_carga.h).

    PROPÓSITO
      Una escena fija con la mayor carga que tendrá el juego, para medirla, ya con
      la capa de sprites del beat 'em up (docs/diseno-beat-em-up.md §8 v3):
        - Tarou andando, en piezas de 16x16 (tarou_caminar.p16), cambiando de
          cuadro cada 2 frames: su subida (≤ 1,5 KB por VBlank en trozos de 512 B,
          juego/subida.c) va en todos los VBlanks, el peor caso. Doble búfer:
          pares de filas 0-2 y 3-5. Se pasea de lado a lado, así que pasa por
          delante y por detrás de los ninjas (entra en el orden por profundidad).
        - Hasta 3 ninjas provisionales (el samurái humano del pack recortado a la
          celda de 64x96, ninja64_*.bin) en sus huecos de la VRAM (pares 6-8, 9-11
          y 12-14) con UN solo búfer: el cuadro nuevo sube entero en un VBlank y
          la OAM pasa a él en ese mismo frame; como mucho `subidas` por VBlank.
        - Sombras (par 15): óvalos negros en la paleta 4 con MEDIA SUMA de color
          sobre BG1+BG2; van al final del orden (detrás de todo).
        - Todo a la OAM con UNA llamada: sprites_dibujar() (juego/sprites_oam.asm),
          con entradas FIJAS: cada frame solo se tocan Tarou y su sombra; las piezas
          de un ninja, solo cuando cambia de cuadro.
      Las configuraciones (CONFIGS) se recorren solas, 360 frames cada una; el HUD
      enseña la configuración y, para cada una, el MÁXIMO de lo que tarda la CPU
      (CPU: líneas desde el VBlank en que empezó el frame, tras las subidas, tras
      rellenar la tabla de entidades y tras la OAM; 262 o más = el frame no cupo), de la línea del VBlank al acabar las
      copias (V; el límite es 37), los frames perdidos (PERD) y lo que más esperó
      un ninja su turno de subida (ESP). La predicción de ranuras por línea la
      imprime scripts/preparar_prueba_carga.py.

    SE CONECTA CON
      Incluye:  <snes.h>, pantallas/prueba_carga.h, sistema/comun.h, juego/nivel.h,
                juego/animaciones.h, juego/fondo_nivel.h, juego/dma_lista.h,
                juego/hud_debug.h, juego/sprites.h, juego/subida.h, juego/p16.h,
                nivel1_datos.h (el tronco) y tarou_anim.h.
      Lee de la ROM: tarou_caminar_p16, tarou_p16_bancos, tarou_pal, ninja64_andar,
                ninja64_ataque, ninja_bancos, ninja_pal, sombra_til, sombra_piezas,
                sombra_banco (data.asm) y los fondos del nivel 1.
      Lo usan:  sistema/main.c (con -DPRUEBA_CARGA).

    HARDWARE QUE TOCA
      PPU: Mode 1, fondos del nivel 1 (comun.c, fondo_nivel.c), BG3 del HUD.
      Matemática de color: TS $212D (BG1+BG2 a la pantalla secundaria), CGWSEL
      $2130 = 0x02 (el segundo operando es la secundaria), CGADSUB $2131 = 0x50
      (sumar a la mitad, solo los sprites de las paletas 4..7). OAM: el búfer del
      SDK por juego/sprites_oam.asm (piezas de 16x16; OBSEL 16/32). VRAM: los 512
      tiles de sprites por la lista de DMA. CGRAM: paletas de sprites 0..4.
      HDMA: el cielo (canal 6). Contadores del haz ($2137/$213D).

    VARIABLES (estáticas de este archivo)
      cfg, n_cfg       Config, u8   la configuración en curso
      tarou_x          u16          x de la celda de Tarou en pantalla; tarou_vx su paso
      t_cuadro, t_muestra, t_bufer, t_subiendo  u8  cuadro que toca / el que enseña la
                                    OAM / en qué búfer está / el que se sube (0xFF: nada)
      t_sub            Subida       la subida en marcha de Tarou
      ex, pies         u16[3]       x del cuerpo y y de los pies de cada ninja
      quiere, muestra  u8[3]        cuadro que toca / el que enseña (0xFF: ninguno)
      reloj, espera    u8[3]        frames en el cuadro / frames esperando turno
      e_sub            Subida       subida de un ninja (se toma entera en un VBlank)
      cpu_a, cpu_b, v_ini, v_fin  u16  medidas del frame; *_max y perdidos: de la config
    → glosario: Prueba de carga, Ranura por línea, Orden por profundidad, OAM, VBlank
---------------------------------------------------------------------------------*/
#ifdef PRUEBA_CARGA

#include <snes.h>
#include "pantallas/prueba_carga.h"
#include "sistema/comun.h"
#include "juego/nivel.h"
#include "juego/animaciones.h"
#include "juego/fondo_nivel.h"
#include "juego/dma_lista.h"
#include "juego/hud_debug.h"
#include "juego/sprites.h"
#include "juego/subida.h"
#include "juego/p16.h"
#include "nivel1_datos.h"
#include "tarou_anim.h"

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];
extern char tarou_caminar_p16, tarou_pal;
extern u8 tarou_p16_bancos[];        /* Banco de cada tira .p16 de Tarou (enum Estado).   */
extern char ninja64_andar, ninja64_ataque, ninja_pal, sombra_til, sombra_piezas;
extern u8 ninja_bancos[];            /* Andar, ataque.                                    */
extern u8 sombra_banco;
extern u16 diag_linea[3];            /* pantalla_nivel1.c: los usa también el HUD.        */

/*--- Configuraciones: 8 bytes cada una (CONFIGS[n] es un desplazamiento, no tcc__mul) ---*/
typedef struct
{
    u8 n;                            /* Cuántos ninjas (0..3).                            */
    u8 ataque;                       /* 0 = solo andar (8 cuadros); 1 = andar + ataque (15). */
    u8 reparto;                      /* 0 = todos los pies en la misma y; 1 = repartidos. */
    u8 sombras;                      /* 1 = una sombra bajo cada uno (y bajo Tarou).      */
    u8 subidas;                      /* Ninjas que pueden subir cuadro en un VBlank.      */
    u8 libre[3];
} Config;

static const Config CONFIGS[] = {
    {0, 0, 0, 0, 1, {0, 0, 0}},      /* 0: Tarou solo (la base).                          */
    {1, 1, 0, 1, 1, {0, 0, 0}},      /* 1: 1 ninja andando y atacando, sombras.           */
    {2, 1, 0, 1, 1, {0, 0, 0}},      /* 2: 2 ninjas, misma y, sombras.                    */
    {3, 1, 0, 1, 1, {0, 0, 0}},      /* 3: 3 ninjas, misma y, sombras.                    */
    {3, 1, 1, 1, 1, {0, 0, 0}},      /* 4: 3 ninjas repartidos, sombras.                  */
    {3, 0, 1, 1, 1, {0, 0, 0}},      /* 5: 3 repartidos, solo andando, sombras.           */
    {3, 1, 1, 0, 1, {0, 0, 0}},      /* 6: 3 repartidos, sin sombras.                     */
    {3, 1, 1, 1, 2, {0, 0, 0}},      /* 7: como la 4, con 2 subidas por VBlank (el techo). */
};
#define CONFIGS_N 8
#define FRAMES_POR_CONFIG 360        /* 6 s: tres capturas de Mesen cada 2 s.             */
#define NINJAS_MAX 3
#define NINJA_ANDAR 8                /* Cuadros de cada tira del ninja.                   */
#define NINJA_CUADROS 15             /* Andar (8) + atacar (7).                           */
#define NINJA_ALTO 96
#define NINJA_ANCHO 64
#define NINJA_CENTRO 32              /* x del cuerpo dentro de la celda.                  */

#define REG_SLHV (*(volatile u8 *)0x2137)
#define REG_OPVCT (*(volatile u8 *)0x213D)

#define TILE_TAROU_A 0               /* Pares 0-2.                                        */
#define TILE_TAROU_B 96              /* Pares 3-5.                                        */
#define TILE_SOMBRA 480              /* Par 15: lo compartido.                            */
#define PALETA_SOMBRA 4              /* Paletas 4..7: las que entran en la matemática de color.*/
#define PRIORIDAD 3
#define TOPE_TAROU 1536              /* Bytes de Tarou por VBlank (3 copias de 512).      */
#define FRAMES_TAROU 2               /* Tarou cambia de cuadro cada 2 frames.             */
#define FRAMES_NINJA 4               /* Cada ninja quiere cambiar de cuadro cada 4.       */
#define TAROU_X_MIN 8
#define TAROU_X_MAX 152
#define SOMBRA_ALTO 10               /* La sombra, 10 px por encima de los pies.          */
#define SIN_CUADRO 0xFF
#define VRAM_TILE(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))  /* 16 palabras por tile.   */

static const u8 PALETAS[NINJAS_MAX] = {1, 2, 3};     /* Paleta de sprites de cada ninja. */
static const u16 HUECOS[NINJAS_MAX] = {192, 288, 384};  /* Primer tile de cada hueco.   */

/*--- Estado ----------------------------------------------------------------------------*/
static Config cfg;
static u8 n_cfg;
static u16 frames_cfg, vueltas_cfg, vb_antes, vb_frame;
static u16 tarou_x;
static s16 tarou_vx;
static u8 reloj_tarou, t_cuadro, t_muestra, t_bufer, t_subiendo;
static Subida t_sub;
static u16 ex[NINJAS_MAX], pies[NINJAS_MAX];
static u8 quiere[NINJAS_MAX], muestra[NINJAS_MAX], reloj[NINJAS_MAX], espera[NINJAS_MAX];
static Subida e_sub;
static u8 turno, fila_hud, oam_usadas;
static u8 e_sombras;                 /* Entrada de la primera sombra.                     */
static u16 bytes_frame, cpu_a, cpu_b, cpu_c, v_ini, v_fin;
static u16 v_max, espera_max, perdidos, bytes_max, cpu_max;

static void preparar_entidades(void);
static void poner_cuadro(u8 e, const u8 *tira, u8 f, u8 banco);

/*--- leer_linea / desde_vblank / medir_cpu: como en pantalla_nivel1.c ----------------*/
static u16 leer_linea(void)
{
    u8 bajo, alto;
    bajo = REG_SLHV;                                 /* Congela los contadores del haz.     */
    bajo = REG_OPVCT;                                /* Bits 0..7 de la línea.              */
    alto = (u8)(REG_OPVCT & 1);                      /* Bit 8.                              */
    return (u16)(((u16)alto << 8) | bajo);
}

static u16 desde_vblank(u16 linea)
{
    if (linea >= 225)
    {
        return (u16)(linea - 225);                   /* 225 → 0 ... 261 → 36.               */
    }
    return (u16)(linea + 37);                        /* 0 → 37: ya se dibuja.               */
}

/* Líneas desde el VBlank en que empezó el frame; +262 si llegó otro (frame perdido). */
static u16 medir_cpu(void)
{
    u16 l = desde_vblank(leer_linea());
    if (snes_vblank_count != vb_frame)
    {
        l = (u16)(l + 262);
    }
    return l;
}

/*--- empezar_config: prepara la configuración n -------------------------------------*/
static void empezar_config(u8 n)
{
    u8 k;

    n_cfg = n;
    cfg = CONFIGS[n];
    for (k = 0; k < NINJAS_MAX; k++)
    {
        ex[k] = (u16)(112 + (k << 5));               /* 112, 144, 176: apiñados.            */
        pies[k] = (u16)(SUELO_Y + (cfg.reparto ? ((u16)k * 12) : 0));
        quiere[k] = (u8)(k * 5);                     /* Cada uno en un punto del ciclo.     */
        muestra[k] = SIN_CUADRO;                     /* Nada en la VRAM todavía.            */
        reloj[k] = k;
        espera[k] = 0;
    }
    turno = 0;
    frames_cfg = vueltas_cfg = 0;
    v_max = espera_max = perdidos = bytes_max = cpu_max = 0;
    preparar_entidades();
}

/*--- subir_tarou: sigue su subida o empieza la del cuadro nuevo ----------------------*/
static void subir_tarou(void)
{
    P16Cuadro c;
    const Copia *copias;
    u8 n;

    if (t_subiendo == SIN_CUADRO && t_cuadro != t_muestra)
    {
        p16_cuadro((const u8 *)&tarou_caminar_p16, t_cuadro, &c);
        subida_empezar(&t_sub, c.n, (u16)((u16)&tarou_caminar_p16 + c.tiles),
                       VRAM_TILE(t_bufer ? TILE_TAROU_A : TILE_TAROU_B),   /* El búfer oculto. */
                       tarou_p16_bancos[ESTADO_CAMINANDO]);
        t_subiendo = t_cuadro;
    }
    if (t_subiendo != SIN_CUADRO)
    {
        bytes_frame = (u16)(bytes_frame + subida_tomar(&t_sub, TOPE_TAROU, &copias, &n));
        dma_lista_copias(copias, n, t_sub.banco);   /* En ensamblador: dma_copias.asm. */
        if (subida_terminada(&t_sub))
        {
            t_muestra = t_subiendo;                  /* Tras este VBlank está entero.       */
            t_bufer ^= 1;
            sprites_ent[0].atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(0) |
                                        (tarou_vx < 0 ? SPRITE_HFLIP : 0));  /* Hacia donde anda. */
            poner_cuadro(0, (const u8 *)&tarou_caminar_p16, t_muestra, tarou_p16_bancos[ESTADO_CAMINANDO]);
            t_subiendo = SIN_CUADRO;
        }
    }
}

/*--- subir_ninjas: como mucho cfg.subidas ninjas suben su cuadro ENTERO ---------------*/
static void subir_ninjas(void)
{
    P16Cuadro c;
    const Copia *copias;
    const u8 *tira;
    u8 i, k, f, cupo, n, banco;

    cupo = cfg.subidas;
    k = turno;
    for (i = 0; i < cfg.n; i++)
    {
        if (quiere[k] != muestra[k])
        {
            if (cupo != 0)
            {
                f = quiere[k];
                if (f < NINJA_ANDAR)
                {
                    tira = (const u8 *)&ninja64_andar;
                    banco = ninja_bancos[0];
                }
                else
                {
                    tira = (const u8 *)&ninja64_ataque;
                    banco = ninja_bancos[1];
                    f = (u8)(f - NINJA_ANDAR);
                }
                p16_cuadro(tira, f, &c);
                subida_empezar(&e_sub, c.n, (u16)((u16)tira + c.tiles), VRAM_TILE(HUECOS[k]), banco);
                bytes_frame = (u16)(bytes_frame + subida_tomar(&e_sub, 0xFFFF, &copias, &n));
                dma_lista_copias(copias, n, banco);
                muestra[k] = quiere[k];
                poner_cuadro((u8)(1 + k), tira, f, banco);  /* Su entrada, al cuadro nuevo. */
                espera[k] = 0;
                cupo--;
            }
            else
            {
                espera[k]++;                         /* Sin turno: sigue con el viejo.      */
                if (espera[k] > espera_max)
                {
                    espera_max = espera[k];
                }
            }
        }
        k++;
        if (k >= cfg.n)
        {
            k = 0;
        }
    }
    turno++;
    if (turno >= cfg.n)
    {
        turno = 0;
    }
}

/*--- poner_cuadro: la entrada `e` enseña el cuadro `f` de la tira (solo al cambiar) ---
    La tabla de piezas está en el banco de la tira: hay que guardarlo con ella (sin él,
    sprites_oam.asm leía los dx, dy de otro banco y el ninja salía a trozos). */
static void poner_cuadro(u8 e, const u8 *tira, u8 f, u8 banco)
{
    P16Cuadro c;
    SpriteEnt *s = sprites_ent + e;                  /* 16 bytes por entrada: sin tcc__mul. */

    p16_cuadro(tira, f, &c);
    s->piezas = (u16)((u16)tira + ((s->atrib & SPRITE_HFLIP) ? c.espejo : c.piezas));
    s->banco = banco;
    s->n = c.n;
}

/*--- preparar_entidades: las entradas FIJAS de esta configuración ---------------------
    0 = Tarou; 1..n = los ninjas; detrás, las sombras (Tarou y cada ninja). Lo que no
    cambia (tile, atributos, banco, la posición de los ninjas, que están quietos) se
    escribe aquí una vez; n = 0 hasta que su primer cuadro esté en la VRAM. */
static void preparar_entidades(void)
{
    SpriteEnt *s;
    u8 k, e;

    e_sombras = (u8)(1 + cfg.n);                     /* Primera sombra.                     */
    sprites_usar((u8)(cfg.sombras ? (e_sombras + 1 + cfg.n) : e_sombras));
    s = sprites_ent;                                 /* Tarou.                              */
    s->n = 0;
    s->banco = tarou_p16_bancos[ESTADO_CAMINANDO];
    s->y0 = (u16)(SUELO_Y - ALTO_CELDA + 2);         /* 2 px hundido, como en el nivel.    */
    sprites_y[0] = SUELO_Y;
    for (k = 0; k < cfg.n; k++)                      /* Los ninjas, mirando a la izquierda. */
    {
        e = (u8)(1 + k);
        s = sprites_ent + e;
        s->n = 0;
        s->x0 = (u16)(ex[k] - NINJA_CENTRO);         /* Borde izquierdo de la celda.        */
        s->y0 = (u16)(pies[k] - NINJA_ALTO);
        s->tile = HUECOS[k];
        s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETAS[k]) | SPRITE_HFLIP);
        s->borde = sprites_borde(s->x0, s->y0, NINJA_ANCHO, NINJA_ALTO);
        sprites_y[e] = pies[k];
    }
    if (cfg.sombras)                                 /* Las sombras: y = 0, detrás de todo. */
    {
        for (k = 0; k <= cfg.n; k++)
        {
            e = (u8)(e_sombras + k);
            s = sprites_ent + e;
            s->x0 = k ? (u16)(ex[k - 1] - 16) : 0;   /* La de Tarou se mueve con él.        */
            s->y0 = (u16)((k ? pies[k - 1] : SUELO_Y) - SOMBRA_ALTO);
            s->piezas = (u16)&sombra_piezas;
            s->banco = sombra_banco;
            s->n = 2;
            s->tile = TILE_SOMBRA;
            s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETA_SOMBRA));
            s->borde = sprites_borde(s->x0, s->y0, 32, 16);
            sprites_y[e] = 0;
        }
    }
}

/*--- mover_entidades: lo que cambia cada frame (Tarou y su sombra) -------------------*/
static void mover_entidades(void)
{
    SpriteEnt *s = sprites_ent;                      /* Tarou.                              */

    s->x0 = tarou_x;                                 /* Borde izquierdo de la celda.        */
    s->borde = sprites_borde(s->x0, s->y0, ANCHO_CELDA, ALTO_CELDA);
    s->tile = t_bufer ? TILE_TAROU_B : TILE_TAROU_A;
    if (cfg.sombras)
    {
        s = sprites_ent + e_sombras;                 /* Su sombra, bajo su centro.          */
        s->x0 = (u16)(tarou_x + ANCHO_CELDA / 2 - 16);
        s->borde = sprites_borde(s->x0, s->y0, 32, 16);
    }
    sprites_ordenar();                               /* Por la y de los pies.               */
}

/*--- hud: una fila por frame (8 filas) ---------------------------------------------*/
static void hud(void)
{
    u16 *p = hud_debug_empezar(fila_hud);

    switch (fila_hud)
    {
        case 0:                                      /* CFG 4  N3                           */
            p = hud_debug_texto(p, "CFG ");
            p = hud_debug_numero(p, n_cfg);
            p = hud_debug_texto(p, "  N");
            hud_debug_numero(p, cfg.n);
            break;
        case 1:                                      /* SOMB 1 REP 1 SUB 1 ATQ 1            */
            p = hud_debug_texto(p, "SOMB ");
            p = hud_debug_numero(p, cfg.sombras);
            p = hud_debug_texto(p, " REP ");
            p = hud_debug_numero(p, cfg.reparto);
            p = hud_debug_texto(p, " SUB ");
            p = hud_debug_numero(p, cfg.subidas);
            p = hud_debug_texto(p, " ATQ ");
            hud_debug_numero(p, cfg.ataque);
            break;
        case 2:                                      /* OAM 70  ENT 8                       */
            p = hud_debug_texto(p, "OAM ");
            p = hud_debug_numero(p, oam_usadas);
            p = hud_debug_texto(p, "  ENT ");
            hud_debug_numero(p, sprites_n);
            break;
        case 3:                                      /* CPU 60 80 90  MAX 120               */
            p = hud_debug_texto(p, "CPU ");
            p = hud_debug_numero(p, (s16)cpu_a);
            p = hud_debug_texto(p, " ");
            p = hud_debug_numero(p, (s16)cpu_c);
            p = hud_debug_texto(p, " ");
            p = hud_debug_numero(p, (s16)cpu_b);
            p = hud_debug_texto(p, "  MAX ");
            hud_debug_numero(p, (s16)cpu_max);
            break;
        case 4:                                      /* V5 30  MAX 33                       */
            p = hud_debug_texto(p, "V");
            p = hud_debug_numero(p, (s16)v_ini);
            p = hud_debug_texto(p, " ");
            p = hud_debug_numero(p, (s16)v_fin);
            p = hud_debug_texto(p, "  MAX ");
            hud_debug_numero(p, (s16)v_max);
            break;
        case 5:                                      /* PERD 0  ESP 2                       */
            p = hud_debug_texto(p, "PERD ");
            p = hud_debug_numero(p, (s16)perdidos);
            p = hud_debug_texto(p, "  ESP ");
            hud_debug_numero(p, (s16)espera_max);
            break;
        case 6:                                      /* BYTES 2048  MAX 4096                */
            p = hud_debug_texto(p, "BYTES ");
            p = hud_debug_numero(p, (s16)bytes_frame);
            p = hud_debug_texto(p, "  MAX ");
            hud_debug_numero(p, (s16)bytes_max);
            break;
        default:                                     /* T 120                               */
            p = hud_debug_texto(p, "T ");
            hud_debug_numero(p, (s16)frames_cfg);
            break;
    }
    hud_debug_enviar(fila_hud);
    fila_hud = (u8)((fila_hud + 1) & 7);
}

/*--- frame: un frame de la escena de prueba -----------------------------------------*/
static void frame(void)
{
    u8 k;

    vb_frame = snes_vblank_count;
    dma_lista_vaciar();
    bytes_frame = 0;

    /* Tarou: anda de lado a lado y cambia de cuadro cada FRAMES_TAROU frames. */
    tarou_x = (u16)(tarou_x + tarou_vx);
    if (tarou_x <= TAROU_X_MIN || tarou_x >= TAROU_X_MAX)
    {
        tarou_vx = (s16)(-tarou_vx);                 /* Media vuelta en los bordes.         */
    }
    reloj_tarou++;
    if (reloj_tarou >= FRAMES_TAROU)
    {
        reloj_tarou = 0;
        t_cuadro++;
        if (t_cuadro >= TAROU_CAMINAR_CUADROS)
        {
            t_cuadro = 0;
        }
    }
    subir_tarou();                                   /* Primero: sus copias van delante.    */

    /* Ninjas: avanzan su cuadro y suben por turnos. */
    for (k = 0; k < cfg.n; k++)
    {
        reloj[k]++;
        if (reloj[k] >= FRAMES_NINJA)
        {
            reloj[k] = 0;
            quiere[k]++;
            if (quiere[k] >= (cfg.ataque ? NINJA_CUADROS : NINJA_ANDAR))
            {
                quiere[k] = 0;
            }
        }
    }
    subir_ninjas();
    if (bytes_frame > bytes_max)
    {
        bytes_max = bytes_frame;
    }
    cpu_a = medir_cpu();                             /* Medida: las subidas.                */

    mover_entidades();
    cpu_c = medir_cpu();                             /* Medida: + la tabla de entidades (C). */
    oam_usadas = sprites_dibujar();                  /* UNA llamada: toda la OAM.           */
    cpu_b = medir_cpu();                             /* Medida: + la OAM (ensamblador).     */
    if (cpu_b > cpu_max)
    {
        cpu_max = cpu_b;
    }
    diag_linea[1] = cpu_b;
    hud();                                           /* Su coste, fuera de CPU (en PERD sí). */
}

void prueba_carga_ejecutar(void)
{
    /*--- Carga, con la pantalla apagada (main la deja así) --------------------------*/
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);
    fondo_nivel_parche(NIVEL1_TRONCO_COL, NIVEL1_PARCHE_COL);
    fondo_nivel_cargar(0);                           /* La capa cercana del nivel 1, quieta. */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    bgSetScroll(0, 0, 0);
    bgSetScroll(1, 0, 0);
    fuente_2bpp(2);
    hud_debug_iniciar();
    hud_debug_alternar();                            /* El HUD, encendido desde el principio. */

    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE16_L32);    /* Piezas pequeñas de 16x16.           */
    sprites_iniciar();                               /* Las 128 entradas, escondidas.       */
    dmaCopyCGram((u8 *)&tarou_pal, 128, 32);         /* Paleta de sprites 0: Tarou.         */
    dmaCopyCGram((u8 *)&ninja_pal, 128 + 16, 32 * 4);   /* Paletas 1..4: ninjas y sombra.   */
    dmaCopyVram((u8 *)&sombra_til, VRAM_TILE(TILE_SOMBRA), 128);            /* Fila de arriba. */
    dmaCopyVram((u8 *)&sombra_til + 128, VRAM_TILE(TILE_SOMBRA + 16), 128); /* Y la de abajo.  */

    /* Sombras: media suma con la pantalla secundaria (BG1 + BG2), solo sprites 4..7. */
    REG_TS = 0x03;                                   /* BG1 y BG2 también en la secundaria. */
    REG_CGWSEL = 0x02;                               /* Operando: la pantalla secundaria.   */
    REG_CGADSUB = 0x50;                              /* Sumar a la mitad (0x40) los sprites (0x10). */

    tarou_x = 40;
    tarou_vx = 1;
    reloj_tarou = 0;
    t_cuadro = 0;
    t_muestra = SIN_CUADRO;
    t_subiendo = SIN_CUADRO;
    t_bufer = 1;                                     /* El primer cuadro va al búfer A.     */
    fila_hud = 0;
    oam_usadas = 0;
    empezar_config(0);

    setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradcieloList);
    setBrightness(15);
    vb_antes = snes_vblank_count;

    /*--- El bucle: un frame, esperar al VBlank, copias, medidas --------------------*/
    while (1)
    {
        frame();
        WaitForVBlank();
        v_ini = desde_vblank(leer_linea());
        dma_lista_ejecutar();                        /* Tarou, ninjas y la fila del HUD.    */
        v_fin = desde_vblank(leer_linea());
        if (v_fin > v_max)
        {
            v_max = v_fin;
        }
        frames_cfg = (u16)(frames_cfg + (u16)(snes_vblank_count - vb_antes));
        vb_antes = snes_vblank_count;
        vueltas_cfg++;
        perdidos = (u16)(frames_cfg - vueltas_cfg);
        if (frames_cfg >= FRAMES_POR_CONFIG)
        {
            empezar_config((u8)((n_cfg + 1 >= CONFIGS_N) ? 0 : n_cfg + 1));
        }
    }
}

#endif /* PRUEBA_CARGA */
