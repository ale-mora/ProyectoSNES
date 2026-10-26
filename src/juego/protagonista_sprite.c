/*---------------------------------------------------------------------------------
    protagonista_sprite.c — El protagonista y su sombra en pantalla (ver .h).

    PROPÓSITO
      Cada frame: (1) la animación: si cambia el estado empieza por el cuadro 0; si no,
      cuenta frames y pasa al siguiente cuadro (como tarou.c del plataformas, con la
      tabla ANIMACIONES). (2) La subida: si el cuadro que toca no es el que está en la
      VRAM y no hay otra subida en marcha, planifica sus piezas hacia el búfer OCULTO;
      cada frame toma hasta TOPE_SUBIDA bytes y los apunta en la lista de DMA; al
      terminar, los búferes se intercambian y las entidades enseñan el nuevo. (3) Las
      entidades: la posición cambia cada frame; las piezas, solo con la mirada o el cuadro.
      Es el mismo método que midió la prueba de carga (pantallas/prueba_carga.c §9.2).

    SE CONECTA CON
      Incluye:  <snes.h>, juego/protagonista_sprite.h, sistema/comun.h (VRAM_SPRITES),
                juego/nivel.h (ANCHO_CELDA, ALTO_CELDA), juego/animaciones.h, juego/sprites.h,
                juego/subida.h, juego/p16.h, juego/dma_lista.h.
      Lo usan:  pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      Ver protagonista_sprite.h. La matemática de color queda así: TS = BG1+BG2, CGWSEL =
      segundo operando la pantalla secundaria, CGADSUB = media suma solo en los sprites de
      las paletas 4..7 (la sombra); los cuerpos opacos usan las paletas 0..3.

    VARIABLES (estáticas de este archivo)
      a_estado, a_cuadro, a_reloj   u8   la animación que toca (estado de Tarou, cuadro, frames en él)
      m_estado, m_cuadro            u8   lo que hay ENTERO en la VRAM (SIN_CUADRO = nada)
      s_cuadro, s_estado            u8   lo que se está subiendo (SIN_CUADRO = nada)
      bufer                         u8   0/1: qué búfer enseña la OAM
      subida                        Subida   el plan de copias de la subida en marcha
    → glosario: Doble búfer, Pieza de 16×16, OAM
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/protagonista_sprite.h"
#include "sistema/comun.h"
#include "juego/nivel.h"
#include "juego/animaciones.h"
#include "juego/sprites.h"
#include "juego/subida.h"
#include "juego/p16.h"
#include "juego/dma_lista.h"

extern char tarou_reposo_p16, tarou_caminar_p16, tarou_saltar_p16, tarou_caer_p16, tarou_atacar_p16;
extern char tarou_pal, ninja_pal, sombra_til, sombra_piezas;
extern u8 tarou_p16_bancos[];        /* Banco de cada tira .p16 de Tarou (enum Estado).   */
extern u8 sombra_banco;

#define TILE_TAROU_A 0               /* Pares de filas 0-2: un búfer de Tarou.            */
#define TILE_TAROU_B 96              /* Pares 3-5: el otro.                               */
#define TILE_SOMBRA 480              /* Par 15: la sombra.                                */
#define PALETA_SOMBRA 4              /* 4..7: las que entran en la matemática de color.   */
#define PRIORIDAD 3
#define TOPE_SUBIDA 1536             /* Bytes por VBlank (3 copias de 512).               */
#define SOMBRA_ALTO 10               /* La sombra, 10 px por encima de los pies.          */
#define SIN_CUADRO 0xFF
#define ENT_CUERPO 0                 /* Entradas fijas de la tabla de entidades.          */
#define ENT_SOMBRA 1
#define VRAM_TILE(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))   /* 16 palabras por tile.  */

/* Qué animación de Tarou enseña cada EstadoCuerpo (mundo.h): el orden es el del enum. */
static const u8 ESTADO_ANIM[CUERPO_ESTADOS] = {ESTADO_PARADO, ESTADO_CAMINANDO,
                                               ESTADO_SALTANDO, ESTADO_CAYENDO,
                                               ESTADO_ATACANDO};   /* Un cuadro fijo para los 3 golpes. */

static u8 a_estado, a_cuadro, a_reloj;
static u8 m_estado, m_cuadro;
static u8 s_cuadro, s_estado;
static u8 bufer;
static Subida subida;

/*--- tira_de: dónde empieza en la ROM la tira .p16 de cada animación -----------------*/
static const u8 *tira_de(u8 e)
{
    switch (e)
    {
        case ESTADO_CAMINANDO: return (const u8 *)&tarou_caminar_p16;
        case ESTADO_SALTANDO:  return (const u8 *)&tarou_saltar_p16;
        case ESTADO_CAYENDO:   return (const u8 *)&tarou_caer_p16;
        case ESTADO_ATACANDO:  return (const u8 *)&tarou_atacar_p16;
        default:               return (const u8 *)&tarou_reposo_p16;
    }
}

void protagonista_sprite_cargar(void)
{
    SpriteEnt *s;

    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE16_L32);    /* Piezas pequeñas de 16x16.           */
    sprites_iniciar();                               /* Las 128 entradas, escondidas.       */
    dmaCopyCGram((u8 *)&tarou_pal, 128, 32);         /* Paleta de sprites 0: Tarou.         */
    dmaCopyCGram((u8 *)&ninja_pal + 96, 128 + 64, 32);   /* Paleta 4: la sombra.            */
    dmaCopyVram((u8 *)&sombra_til, VRAM_TILE(TILE_SOMBRA), 128);            /* Fila de arriba. */
    dmaCopyVram((u8 *)&sombra_til + 128, VRAM_TILE(TILE_SOMBRA + 16), 128); /* Y la de abajo.  */

    /* Sombra: media suma con la pantalla secundaria (BG1 + BG2), solo sprites 4..7. */
    REG_TS = 0x03;                                   /* BG1 y BG2 también en la secundaria. */
    REG_CGWSEL = 0x02;                               /* Operando: la pantalla secundaria.   */
    REG_CGADSUB = 0x50;                              /* Sumar a la mitad (0x40) los sprites (0x10). */

    sprites_usar(2);                                 /* El cuerpo y su sombra.              */
    s = sprites_ent + ENT_CUERPO;
    s->n = 0;                                        /* Sin piezas hasta que suba su cuadro. */
    s->tile = TILE_TAROU_A;
    s->banco = tarou_p16_bancos[ESTADO_PARADO];
    s = sprites_ent + ENT_SOMBRA;                    /* La sombra no cambia de forma.       */
    s->piezas = (u16)&sombra_piezas;
    s->banco = sombra_banco;
    s->n = 2;
    s->tile = TILE_SOMBRA;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(PALETA_SOMBRA));
    sprites_y[ENT_SOMBRA] = 0;                       /* y = 0: detrás de todo.              */

    a_estado = ESTADO_PARADO;
    a_cuadro = 0;
    a_reloj = 0;
    m_estado = SIN_CUADRO;                           /* Nada en la VRAM todavía.            */
    m_cuadro = SIN_CUADRO;
    s_cuadro = SIN_CUADRO;
    s_estado = SIN_CUADRO;
    bufer = 1;                                       /* El primer cuadro va al búfer A.     */
}

/*--- animar: el estado cambia → cuadro 0; si no, cuenta frames y avanza ---------------*/
static void animar(u8 estado)
{
    const AnimDef *d;

    if (estado != a_estado)
    {
        a_estado = estado;
        a_cuadro = 0;
        a_reloj = 0;
        return;
    }
    d = ANIMACIONES + a_estado;                      /* 3 bytes por entrada: sin tcc__mul.  */
    a_reloj++;
    if (a_reloj >= d->frames_por_cuadro)
    {
        a_reloj = 0;
        a_cuadro++;
        if (a_cuadro >= d->cuadros)
        {
            a_cuadro = (u8)(d->bucle ? 0 : d->cuadros - 1);
        }
    }
}

/*--- subir: empieza la subida del cuadro que toca y sigue la que está en marcha -------*/
static void subir(void)
{
    P16Cuadro cu;
    const Copia *copias;
    const u8 *tira;
    u8 n;

    if (s_cuadro == SIN_CUADRO && (a_cuadro != m_cuadro || a_estado != m_estado))
    {
        tira = tira_de(a_estado);
        p16_cuadro(tira, a_cuadro, &cu);
        subida_empezar(&subida, cu.n, (u16)((u16)tira + cu.tiles),
                       VRAM_TILE(bufer ? TILE_TAROU_A : TILE_TAROU_B),   /* El búfer oculto. */
                       tarou_p16_bancos[a_estado]);
        s_cuadro = a_cuadro;
        s_estado = a_estado;
    }
    if (s_cuadro != SIN_CUADRO)
    {
        subida_tomar(&subida, TOPE_SUBIDA, &copias, &n);
        dma_lista_copias(copias, n, subida.banco);   /* En ensamblador: dma_copias.asm.     */
        if (subida_terminada(&subida))
        {
            m_cuadro = s_cuadro;                     /* Tras este VBlank está entero.       */
            m_estado = s_estado;
            bufer ^= 1;                              /* El búfer nuevo pasa a ser el visible. */
            s_cuadro = SIN_CUADRO;
        }
    }
}

void protagonista_sprite_actualizar(const Cuerpo *c, u16 camara)
{
    SpriteEnt *s;
    P16Cuadro cu;
    const u8 *tira;

    animar(ESTADO_ANIM[c->estado]);
    subir();

    /*--- El cuerpo: x0, y0 son el borde izquierdo y el de arriba de la celda ---------*/
    s = sprites_ent + ENT_CUERPO;
    s->x0 = (u16)(c->x - camara - ANCHO_CELDA / 2);  /* x de pantalla, centrado (puede ser < 0). */
    s->y0 = (u16)(c->y - ALTO_CELDA + 2 - mundo_altura(c));   /* 2 px hundido, como en el nivel. */
    s->borde = sprites_borde(s->x0, s->y0, ANCHO_CELDA, ALTO_CELDA);
    s->tile = (u16)(bufer ? TILE_TAROU_B : TILE_TAROU_A);
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD) | SPRITE_PALETA(0) |
                    (c->mira_izq ? SPRITE_HFLIP : 0));
    if (m_estado != SIN_CUADRO)                      /* Ya hay un cuadro entero que enseñar. */
    {
        tira = tira_de(m_estado);
        p16_cuadro(tira, m_cuadro, &cu);
        s->piezas = (u16)((u16)tira + (c->mira_izq ? cu.espejo : cu.piezas));
        s->banco = tarou_p16_bancos[m_estado];
        s->n = cu.n;
    }
    sprites_y[ENT_CUERPO] = c->y;                    /* Los pies: la clave del orden.       */

    /*--- La sombra: en el suelo (no sube con el salto), bajo el centro -----------------*/
    s = sprites_ent + ENT_SOMBRA;
    s->x0 = (u16)(c->x - camara - 16);
    s->y0 = (u16)(c->y - SOMBRA_ALTO);
    s->borde = sprites_borde(s->x0, s->y0, 32, 16);

    sprites_ordenar();                               /* Por la y de los pies.               */
}

u8 protagonista_sprite_al_dia(void)
{
    return (u8)(m_estado == a_estado && m_cuadro == a_cuadro);
}

void protagonista_sprite_ocultar(void)
{
    sprites_iniciar();                               /* Las 128 entradas, escondidas.       */
    REG_TS = 0;                                      /* Sin matemática de color.            */
    REG_CGWSEL = 0;
    REG_CGADSUB = 0;
}
