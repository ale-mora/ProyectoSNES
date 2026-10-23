/*---------------------------------------------------------------------------------
    prueba_carga.c — ROM de prueba de carga del beat 'em up (ver prueba_carga.h).

    PROPÓSITO
      Una escena fija con la mayor carga que tendrá el juego, para medirla:
        - Tarou andando con su sprite real (6 piezas de 32x32), cambiando de
          cuadro cada 2 frames: su subida de medio cuadro (1,5 KB) va en TODOS
          los VBlanks, el peor caso.
        - N ninjas provisionales (el samurái humano del pack, ampliado ×3 o ×2:
          scripts/preparar_prueba_carga.py) en piezas de 16x16 recortadas, cada
          uno en su «hueco» de la VRAM con UN solo búfer: el cuadro nuevo sube en
          un único VBlank y la OAM pasa a él en ese mismo frame. Como mucho
          `subidas` enemigos suben cuadro por VBlank, por turnos.
        - Sombras: óvalos negros en la paleta de sprites 4 con MEDIA SUMA de
          color sobre BG1+BG2 (oscurecen el 50 % del suelo).
        - Orden por profundidad (juego/profundidad.c) en la OAM, y una
          configuración que invierte el orden en frames alternos (parpadeo en
          lugar de un agujero fijo cuando una línea se pasa de 34 ranuras).
      Tarou ocupa siempre las entradas 0..5 de la OAM: tapa a todos y no entra en
      el orden ni en la rotación (limitación aceptada de la prueba).
      Las 10 configuraciones (CONFIGS) se recorren solas, 360 frames cada una; el
      HUD enseña la configuración y, para cada una, el MÁXIMO de lo que tarda la
      CPU (CPU: líneas desde el VBlank en que empezó el frame, tras Tarou y las
      subidas y tras la OAM; 262 o más = el frame no cupo), de la línea del VBlank al acabar las copias (V;
      el límite es 37), los frames perdidos (PERD) y lo que más esperó un enemigo
      su turno de subida (ESP). Las predicciones de ranuras por línea las imprime
      el script: la captura de Mesen tiene que coincidir con ellas.

    SE CONECTA CON
      Incluye:  <snes.h>, pantallas/prueba_carga.h, sistema/comun.h,
                juego/tarou.h, juego/tarou_sprite.h, juego/fondo_nivel.h,
                juego/dma_lista.h, juego/hud_debug.h, juego/profundidad.h,
                juego/nivel.h, nivel1_datos.h (el tronco), tarou_anim.h y
                ninja_datos.h (generado por el script).
      Lee de la ROM: ninja3_*_til, ninja2_*_til, ninja_pal, sombra_til,
                ninja_bancos (data.asm) y los fondos del nivel 1.
      Lo usan:  sistema/main.c (con -DPRUEBA_CARGA).

    HARDWARE QUE TOCA
      PPU: Mode 1, fondos del nivel 1 (comun.c, fondo_nivel.c), BG3 del HUD.
      Matemática de color: TS $212D (BG1+BG2 a la pantalla secundaria), CGWSEL
      $2130 = 0x02 (el segundo operando es la secundaria), CGADSUB $2131 = 0x50
      (sumar a la mitad, solo los sprites de las paletas 4..7). OAM: el búfer del
      SDK por juego/oam_piezas.asm (piezas pequeñas de 16x16) y oamSet en Tarou (32x32). VRAM: tiles
      de sprites 256..511 para los ninjas y las columnas 12..15 de la celda 0 de
      Tarou (que su cuadro no usa) para la sombra. CGRAM: paletas de sprites 1..4.
      HDMA: el cielo (canal 6). Contadores del haz ($2137/$213D).

    VARIABLES (estáticas de este archivo)
      tarou            Tarou    el protagonista (solo dibujo: no hay lógica)
      cfg              Config   la configuración en curso (copia de CONFIGS[n_cfg])
      n_cfg            u8       0..CONFIGS_N-1
      ex, pies         u16[4]   x del cuerpo y y de los pies de cada ninja (pantalla)
      quiere, muestra  u8[4]    cuadro que toca / cuadro que enseña la OAM (0xFF: ninguno)
      reloj, espera    u8[4]    frames en el cuadro / frames esperando turno de subida
      orden            u8[4]    ninjas en orden de dibujo (juego/profundidad.c)
      base_hueco       u16[4]   primer tile de la VRAM del hueco de cada ninja
      lin_max, v_max, espera_max, perdidos  u16  máximos de la configuración
    → glosario: Prueba de carga, Ranura por línea, Orden por profundidad, OAM, VBlank
---------------------------------------------------------------------------------*/
#ifdef PRUEBA_CARGA

#include <snes.h>
#include "pantallas/prueba_carga.h"
#include "sistema/comun.h"
#include "sistema/puntofijo.h"
#include "juego/nivel.h"
#include "juego/tarou.h"
#include "juego/tarou_sprite.h"
#include "juego/fondo_nivel.h"
#include "juego/dma_lista.h"
#include "juego/hud_debug.h"
#include "juego/profundidad.h"
#include "juego/oam_piezas.h"
#include "nivel1_datos.h"
#include "tarou_anim.h"
#include "ninja_datos.h"

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];
extern char ninja3_andar_til, ninja3_ataque_til, ninja2_andar_til, ninja2_ataque_til;
extern char ninja_pal, sombra_til;
extern u8 ninja_bancos[];            /* ×3 andar, ×3 ataque, ×2 andar, ×2 ataque.         */
extern u16 diag_linea[3];            /* pantalla_nivel1.c: los usa también el HUD.        */

/*--- Configuraciones: 8 bytes cada una (CONFIGS[n] es un desplazamiento, no tcc__mul) ---*/
typedef struct
{
    u8 escala;                       /* 3 (93 px de alto) o 2 (62 px).                    */
    u8 n;                            /* Cuántos ninjas (0..4).                            */
    u8 pares;                        /* Pares de filas (1 KB) por hueco de la VRAM.       */
    u8 ataque;                       /* 0 = solo andar (8 cuadros); 1 = andar + ataque.   */
    u8 reparto;                      /* 0 = todos los pies en la misma y; 1 = repartidos. */
    u8 sombras;                      /* 1 = una sombra bajo cada uno (y bajo Tarou).      */
    u8 subidas;                      /* Ninjas que pueden subir cuadro en un VBlank.      */
    u8 rotar;                        /* 1 = invertir el orden de la OAM en frames impares.*/
} Config;

/* Los huecos caben en los 8 pares de filas de los tiles 256..511: con 2 pares, 4
   huecos; con 3 o 4, 2; con 5, 1. Un ninja sin hueco propio enseña el cuadro del
   que lo comparte (no sube nada). */
static const Config CONFIGS[] = {
    {2, 0, 2, 0, 0, 0, 1, 0},        /* 0: Tarou solo (la base).                          */
    {3, 1, 5, 1, 0, 1, 1, 0},        /* 1: ×3, 1 ninja andando y atacando, sombras.        */
    {3, 2, 4, 0, 0, 1, 1, 0},        /* 2: ×3, 2 ninjas andando, misma y, sombras.         */
    {2, 2, 3, 1, 0, 1, 1, 0},        /* 3: ×2, 2 ninjas andando y atacando, sombras.       */
    {2, 3, 2, 0, 0, 1, 1, 0},        /* 4: ×2, 3 ninjas, misma y, sombras.                 */
    {2, 3, 2, 0, 1, 1, 1, 0},        /* 5: ×2, 3 ninjas repartidos, sombras.               */
    {2, 4, 2, 0, 1, 1, 1, 0},        /* 6: ×2, 4 ninjas repartidos, sombras.               */
    {2, 3, 2, 0, 0, 1, 1, 1},        /* 7: como la 4, con el orden rotando (parpadeo).     */
    {2, 4, 2, 0, 1, 1, 2, 0},        /* 8: como la 6, con 2 subidas por VBlank (el techo). */
    {3, 2, 4, 0, 0, 1, 2, 0},        /* 9: como la 2, con 2 subidas por VBlank.            */
};
#define CONFIGS_N 10
#define FRAMES_POR_CONFIG 360        /* 6 s: tres capturas de Mesen cada 2 s.             */
#define NINJAS_MAX 4

#define REG_SLHV (*(volatile u8 *)0x2137)
#define REG_OPVCT (*(volatile u8 *)0x213D)

#define OAM_PRIMERA 6                /* Tarou usa las entradas 0..5.                      */
#define PRIORIDAD 3                  /* Delante de todos los fondos, como Tarou.          */
#define TILE_ENEMIGOS 256            /* Primer tile de los huecos (la 2.ª tabla de nombres).*/
#define TILE_SOMBRA 12               /* Columnas 12..15 de la celda 0 de Tarou: libres.   */
#define PALETA_SOMBRA 4              /* Paletas 4..7: las que entran en la matemática de color.*/
#define TAROU_X 8                    /* Celda de Tarou (96 px) en la pantalla.            */
#define FRAMES_TAROU 2               /* Tarou cambia de cuadro cada 2 frames.             */
#define FRAMES_NINJA 4               /* Cada ninja quiere cambiar de cuadro cada 4.       */
#define ESPEJO_X (NINJA_CENTRO_X + 1 - 16)  /* Mirando a la izquierda: x = ex + 25 - dx.  */
#define SOMBRA_ALTO 10               /* La sombra se dibuja 10 px por encima de los pies. */
#define SIN_CUADRO 0xFF
static const u8 PALETAS[NINJAS_MAX] = {1, 2, 3, 1};  /* Paleta de sprites de cada ninja. */

/*--- Estado ----------------------------------------------------------------------------*/
static Tarou tarou;
static Config cfg;
static u8 n_cfg;
static u16 frames_cfg;               /* VBlanks desde que empezó la configuración.        */
static u16 vueltas_cfg;              /* Vueltas del bucle en ese tiempo.                  */
static u16 vb_antes;                 /* snes_vblank_count de la vuelta anterior.          */
static u8 reloj_tarou;
static u16 ex[NINJAS_MAX], pies[NINJAS_MAX], base_hueco[NINJAS_MAX];
static u8 quiere[NINJAS_MAX], muestra[NINJAS_MAX], reloj[NINJAS_MAX], espera[NINJAS_MAX];
static u8 dueno[NINJAS_MAX];         /* El ninja cuyo hueco usa (él mismo si tiene uno).  */
static u8 orden[NINJAS_MAX];
static u8 turno;                     /* Por qué ninja empieza la ronda de subidas.        */
static u8 huecos;                    /* Cuántos huecos caben con cfg.pares.               */
static u8 oam_usadas;                /* Entradas de la OAM escritas el frame anterior.    */
static u8 fila_hud;
static u8 piezas_frame;              /* Piezas de ninja dibujadas este frame.             */
static u16 bytes_frame;              /* Bytes de ninja apuntados en la lista este frame.  */
static u16 lin, v_ini, v_fin;
static u16 cpu_a, cpu_b;              /* Líneas desde el VBlank en que empezó el frame: tras
                                        Tarou + subidas y tras la OAM (+262 si el frame ya
                                        pisó el VBlank siguiente: frame perdido).          */
static u16 vb_frame;                 /* snes_vblank_count al empezar el frame.            */
static u16 lin_max, v_max, espera_max, perdidos, bytes_max, cpu_max;

/* Tablas de la escala en curso (ninja3_* o ninja2_*, de ninja_datos.h). */
static const u8 *t_n_piezas, *t_tira, *t_piezas;
static const u16 *t_desp, *t_bytes;
static u8 t_paso_bits, t_cuadros;
static u16 t_alto;
static u16 t_tiles[2];               /* Dirección de la tira de andar y la de ataque.     */
static u8 t_banco[2];

/*--- leer_linea / desde_vblank: como en pantalla_nivel1.c --------------------------*/
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

/*--- medir_cpu: dónde va el frame, contado desde el VBlank en que empezó ------------
    desde_vblank() de la línea actual, más 262 si desde entonces llegó otro VBlank:
    un valor >= 262 es un frame que no cupo (la lógica pisó el VBlank siguiente). */
static u16 medir_cpu(void)
{
    u16 l = desde_vblank(leer_linea());
    if (snes_vblank_count != vb_frame)
    {
        l = (u16)(l + 262);                          /* Ya pasó un VBlank: frame perdido.   */
    }
    return l;
}

/*--- esconder: una entrada de la OAM fuera de la pantalla (como tarou_sprite.c) -----*/
static void esconder(u8 entrada)
{
    oamSetEx((u16)entrada * 4, OBJ_SMALL, OBJ_HIDE);
    oamSetXY((u16)entrada * 4, 256, 224);
}

/*--- empezar_config: prepara la configuración n -------------------------------------*/
static void empezar_config(u8 n)
{
    u8 k;
    u16 tile;

    n_cfg = n;
    cfg = CONFIGS[n];
    if (cfg.escala == 3)                             /* Las tablas de la escala elegida.    */
    {
        t_n_piezas = ninja3_n_piezas; t_tira = ninja3_tira; t_piezas = ninja3_piezas;
        t_desp = ninja3_desp; t_bytes = ninja3_bytes; t_paso_bits = NINJA3_PIEZAS_PASO_BITS;
        t_cuadros = NINJA3_CUADROS; t_alto = NINJA3_ALTO;
        t_tiles[0] = (u16)&ninja3_andar_til; t_tiles[1] = (u16)&ninja3_ataque_til;
        t_banco[0] = ninja_bancos[0]; t_banco[1] = ninja_bancos[1];
    }
    else
    {
        t_n_piezas = ninja2_n_piezas; t_tira = ninja2_tira; t_piezas = ninja2_piezas;
        t_desp = ninja2_desp; t_bytes = ninja2_bytes; t_paso_bits = NINJA2_PIEZAS_PASO_BITS;
        t_cuadros = NINJA2_CUADROS; t_alto = NINJA2_ALTO;
        t_tiles[0] = (u16)&ninja2_andar_til; t_tiles[1] = (u16)&ninja2_ataque_til;
        t_banco[0] = ninja_bancos[2]; t_banco[1] = ninja_bancos[3];
    }
    if (!cfg.ataque)
    {
        t_cuadros = 8;                               /* Solo los 8 cuadros de andar.        */
    }

    /* Huecos: cuántos caben en 8 pares de filas, sin dividir; y su primer tile. */
    huecos = 0;
    tile = TILE_ENEMIGOS;
    for (k = cfg.pares; k <= 8; k = (u8)(k + cfg.pares))
    {
        if (huecos < NINJAS_MAX)
        {
            base_hueco[huecos] = tile;
        }
        huecos++;
        tile = (u16)(tile + ((u16)cfg.pares << 5)); /* pares * 32 tiles.                   */
    }

    for (k = 0; k < NINJAS_MAX; k++)
    {
        ex[k] = (u16)(104 + (k * 24));               /* Apiñados delante de Tarou.          */
        pies[k] = (u16)(SUELO_Y + (cfg.reparto ? (k * 12) : 0));
        quiere[k] = (u8)(k * 3);                     /* Cada uno en un punto del ciclo.     */
        if (quiere[k] >= t_cuadros)
        {
            quiere[k] = 0;
        }
        muestra[k] = SIN_CUADRO;                     /* Nada en la VRAM todavía.            */
        reloj[k] = (u8)k;                            /* Escalonados.                        */
        espera[k] = 0;
        orden[k] = k;
        dueno[k] = (k < huecos) ? k : (u8)(k - huecos); /* Sin hueco: comparte uno.        */
    }
    turno = 0;
    frames_cfg = 0;
    vueltas_cfg = 0;
    lin_max = v_max = espera_max = perdidos = bytes_max = cpu_max = 0;
    for (k = OAM_PRIMERA; k < oam_usadas; k++)
    {
        esconder(k);                                 /* Lo de la configuración anterior.    */
    }
    oam_usadas = OAM_PRIMERA;
}

/*--- subir_cuadros: los ninjas que cambiaron de cuadro, como mucho cfg.subidas --------
    Cada uno apunta en la lista de DMA el cuadro entero (sus pares de filas, de un
    tirón) hacia su hueco, y desde este mismo frame la OAM lo enseña: tiles y OAM
    llegan en el mismo VBlank. */
static void subir_cuadros(void)
{
    u8 i, k, f, cupo;

    cupo = cfg.subidas;
    bytes_frame = 0;
    k = turno;
    for (i = 0; i < cfg.n; i++)
    {
        if (dueno[k] == k && quiere[k] != muestra[k])
        {
            if (cupo != 0)
            {
                f = quiere[k];
                dma_lista_anadir((u16)(t_tiles[t_tira[f]] + t_desp[f]),
                                 (u16)(VRAM_SPRITES + (base_hueco[k] << 4)),  /* tile * 16 palabras. */
                                 t_bytes[f], t_banco[t_tira[f]], DMA_VMAIN_TILES);
                bytes_frame = (u16)(bytes_frame + t_bytes[f]);
                muestra[k] = f;
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
    turno++;                                         /* La ronda siguiente empieza por otro. */
    if (turno >= cfg.n)
    {
        turno = 0;
    }
    if (bytes_frame > bytes_max)
    {
        bytes_max = bytes_frame;
    }
}

/*--- dibujar_ninja: las piezas del ninja k desde la entrada `e`; devuelve la siguiente --
    Con juego/oam_piezas.asm: un bucle en ensamblador escribe todas sus piezas en el
    búfer de la OAM (con oamSet + oamSetEx desde C costaban ~5 líneas por pieza). */
static u8 dibujar_ninja(u8 k, u8 e)
{
    u8 f, n;
    u16 tile;

    f = muestra[dueno[k]];
    if (f == SIN_CUADRO)
    {
        return e;                                    /* Aún sin tiles: no se dibuja.        */
    }
    n = t_n_piezas[f];
    tile = base_hueco[dueno[k]];
    /* Atributos vhoopppN: espejo (0x40, mira a la izquierda, hacia Tarou), prioridad 3
       (0x30), su paleta y el bit 8 del tile (los huecos están en 256..511). */
    oam_piezas((u16)e << 2, (u16)(ex[k] + ESPEJO_X), (u16)(pies[k] - t_alto), tile & 0xFF,
               (u16)(0x40 | (PRIORIDAD << 4) | (PALETAS[k] << 1) | (tile >> 8)),
               t_piezas + ((u16)f << (t_paso_bits + 1)), n, 1);
    piezas_frame = (u8)(piezas_frame + n);
    return (u8)(e + n);
}

/*--- dibujar_sombra: 2 piezas de 16x16 con el centro en x, a la altura de los pies ---*/
static const u8 SOMBRA_PIEZAS[4] = {0, 0, 16, 0};  /* (dx, dy) de sus dos mitades.     */
static u8 dibujar_sombra(u16 x, u16 y_pies, u8 e)
{
    oam_piezas((u16)e << 2, (u16)(x - 16), (u16)(y_pies - SOMBRA_ALTO), TILE_SOMBRA,
               (u16)((PRIORIDAD << 4) | (PALETA_SOMBRA << 1)), SOMBRA_PIEZAS, 2, 0);
    return (u8)(e + 2);
}

/*--- hud: una fila por frame (8 filas) ---------------------------------------------*/
static void hud(void)
{
    u16 *p;

    /* Siempre, aunque el frame vaya cargado: con un tope (como el HUD del nivel), en
       las configuraciones pesadas las filas se quedaban con datos de la anterior. Su
       coste va DESPUÉS de las medidas CPU (pero cuenta en PERD). */
    p = hud_debug_empezar(fila_hud);
    switch (fila_hud)
    {
        case 0:                                      /* CFG 4  X2  N3                       */
            p = hud_debug_texto(p, "CFG ");
            p = hud_debug_numero(p, n_cfg);
            p = hud_debug_texto(p, "  X");
            p = hud_debug_numero(p, cfg.escala);
            p = hud_debug_texto(p, "  N");
            hud_debug_numero(p, cfg.n);
            break;
        case 1:                                      /* SOMB 1 REP 0 SUB 1 ROT 0 ATQ 0      */
            p = hud_debug_texto(p, "SOMB ");
            p = hud_debug_numero(p, cfg.sombras);
            p = hud_debug_texto(p, " REP ");
            p = hud_debug_numero(p, cfg.reparto);
            p = hud_debug_texto(p, " SUB ");
            p = hud_debug_numero(p, cfg.subidas);
            p = hud_debug_texto(p, " ROT ");
            p = hud_debug_numero(p, cfg.rotar);
            p = hud_debug_texto(p, " ATQ ");
            hud_debug_numero(p, cfg.ataque);
            break;
        case 2:                                      /* OAM 45  PIEZAS 39  HUECOS 4         */
            p = hud_debug_texto(p, "OAM ");
            p = hud_debug_numero(p, oam_usadas);
            p = hud_debug_texto(p, "  PIEZAS ");
            p = hud_debug_numero(p, piezas_frame);
            p = hud_debug_texto(p, "  HUECOS ");
            hud_debug_numero(p, huecos);
            break;
        case 3:                                      /* CPU 60 140  MAX 300 (>= 262: no cupo) */
            p = hud_debug_texto(p, "CPU ");
            p = hud_debug_numero(p, (s16)cpu_a);
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
        default:                                     /* T 120 (segundos en la config ×6)    */
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
    u8 i, k, e;

    vb_frame = snes_vblank_count;
    dma_lista_vaciar();

    /* Tarou: cambia de cuadro cada FRAMES_TAROU frames (subida en todos los VBlanks). */
    reloj_tarou++;
    if (reloj_tarou >= FRAMES_TAROU)
    {
        reloj_tarou = 0;
        tarou.cuadro++;
        if (tarou.cuadro >= TAROU_CAMINAR_CUADROS)
        {
            tarou.cuadro = 0;
        }
    }
    tarou_sprite_dibujar(&tarou, 0);                 /* Primero: sus copias van delante.    */

    /* Ninjas: avanzan su cuadro (lo que «quieren» enseñar) y suben por turnos. */
    for (k = 0; k < cfg.n; k++)
    {
        reloj[k]++;
        if (reloj[k] >= FRAMES_NINJA)
        {
            reloj[k] = 0;
            quiere[k]++;
            if (quiere[k] >= t_cuadros)
            {
                quiere[k] = 0;
            }
        }
    }
    subir_cuadros();
    cpu_a = medir_cpu();                             /* Medida: Tarou + subidas.            */

    /* OAM: ninjas por profundidad (los pies más abajo, primero = delante), y detrás
       las sombras. En la configuración con rotación, los frames impares al revés. */
    profundidad_ordenar(orden, pies, cfg.n);
    e = OAM_PRIMERA;
    piezas_frame = 0;
    if (cfg.rotar && (frames_cfg & 1))
    {
        for (i = cfg.n; i != 0; i--)
        {
            e = dibujar_ninja(orden[i - 1], e);
        }
    }
    else
    {
        for (i = 0; i < cfg.n; i++)
        {
            e = dibujar_ninja(orden[i], e);
        }
    }
    if (cfg.sombras)
    {
        e = dibujar_sombra(TAROU_X + 48, SUELO_Y, e);   /* La de Tarou, bajo su centro.     */
        for (k = 0; k < cfg.n; k++)
        {
            e = dibujar_sombra(ex[k], pies[k], e);
        }
    }
    for (k = e; k < oam_usadas; k++)
    {
        esconder(k);                                 /* Lo que sobró del frame anterior.    */
    }
    oam_usadas = e;
    cpu_b = medir_cpu();                             /* Medida: + orden y OAM.              */
    if (cpu_b > cpu_max)
    {
        cpu_max = cpu_b;
    }

    lin = leer_linea();                              /* Medida: lógica + OAM (sin el HUD).  */
    diag_linea[1] = lin;
    if (lin < 225 && lin > lin_max)
    {
        lin_max = lin;
    }
    hud();
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

    tarou_sprite_cargar();                           /* OBSEL 16/32 y la paleta 0.          */
    dmaCopyCGram((u8 *)&ninja_pal, 128 + 16 * 1, 32 * 4);   /* Paletas de sprites 1..4.      */
    dmaCopyVram((u8 *)&sombra_til, VRAM_SPRITES + TILE_SOMBRA * 16, 128);         /* Fila de arriba. */
    dmaCopyVram((u8 *)&sombra_til + 128, VRAM_SPRITES + (TILE_SOMBRA + 16) * 16, 128); /* Y la de abajo. */

    /* Sombras: media suma con la pantalla secundaria (BG1 + BG2), solo sprites 4..7. */
    REG_TS = 0x03;                                   /* BG1 y BG2 también en la secundaria. */
    REG_CGWSEL = 0x02;                               /* Operando: la pantalla secundaria.   */
    REG_CGADSUB = 0x50;                              /* Sumar a la mitad (0x40) los sprites (0x10). */

    tarou_iniciar(&tarou, TAROU_X, SUELO_Y);
    tarou.estado = ESTADO_CAMINANDO;                 /* Solo dibujo: no hay física.         */
    reloj_tarou = 0;
    fila_hud = 0;
    oam_usadas = OAM_PRIMERA;
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
        /* Frames perdidos: VBlanks que pasaron menos vueltas del bucle. */
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
