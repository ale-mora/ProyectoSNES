/*---------------------------------------------------------------------------------
    prototipo.c — El prototipo de combate: Tarou y un ninja de prueba en un carril
    (docs/diseno-beat-em-up.md, Etapa 2, entregable 3). Ver prototipo.h.

    PROPÓSITO
      Une la lógica pura (luchador, ninja, combate, mundo, doble toque) con la
      capa de sprites de la prueba de carga y el fondo del nivel 1. Cada frame:
        1. El mando dice qué quiere Tarou; el luchador decide su estado.
        2. El ninja se acerca o ataca; tiene permiso siempre (es el único).
        3. Si hay caja de golpe activa, se pasa al mundo y se resuelve con
           combate_golpear: el golpe conecta o no; si conecta, cambia el estado
           del que recibe (reaccionar) y el atacante marca el conecto.
        4. Las entradas de la OAM se rellenan con su posición, y sprites_ordenar y
           sprites_dibujar las escriben.
      Tarou y el ninja usan, por ahora, la pose de andar (no hay arte de combate):
      la animación de verdad vendrá con Pixellab. Sin HUD de texto: la vida se
      mira en Mesen (las variables del luchador y el ninja).

    SE CONECTA CON
      Incluye:  <snes.h>, pantallas/prototipo.h, sistema/comun.h, juego/mando.h,
                juego/luchador.h, enemigos/ninja.h, juego/combate.h,
                juego/cifras.h, juego/mundo.h, juego/sprites.h, juego/subida.h,
                juego/p16.h, juego/dma_lista.h, juego/fondo_nivel.h,
                juego/animaciones.h, tarou_anim.h.
      Lee de la ROM: tarou_caminar_p16, tarou_pal, tarou_p16_bancos (data.asm);
                ninja64_andar, ninja_pal, ninja_bancos (data.asm).
      Lo usan:  sistema/main.c (con -DPROTOTIPO).

    HARDWARE QUE TOCA
      PPU: Mode 1 con el fondo del nivel 1 (fondo_nivel, fondo16_cargar) y BG3 de
      texto. OAM: sprites_dibujar (16x16, OBSEL 16/32). VRAM: tiles de los sprites
      por la lista de DMA, en VBlank. CGRAM: paleta 0 (Tarou) y 1 (el ninja).

    VARIABLES (estáticas de este archivo)
      tarou, enemigo        Luchador, Ninja  los dos protagonistas de la escena
      carril                Carril           por dónde se puede andar (x 48..200)
      mando                 Mando            lo que pide el jugador este frame
      caja_dano_tarou, caja_dano_ninja  CajaCuadro  dónde pueden golpear a cada uno
      golpeados_tarou, golpeados_ninja  u8   a quién ya le dio cada ataque
      sub_tarou, sub_ninja  Subida           la subida de sus tiles (una vez, al empezar)
      listo_tarou, listo_ninja  u8           1 cuando sus tiles están en la VRAM
    → glosario: Prototipo de combate, Caja de golpe, Caja de daño, OAM
---------------------------------------------------------------------------------*/
#ifdef PROTOTIPO

#include <snes.h>
#include "pantallas/prototipo.h"
#include "sistema/comun.h"
#include "juego/mando.h"
#include "juego/luchador.h"
#include "enemigos/ninja.h"
#include "juego/combate.h"
#include "juego/cifras.h"
#include "juego/mundo.h"
#include "juego/sprites.h"
#include "juego/subida.h"
#include "juego/p16.h"
#include "juego/dma_lista.h"
#include "juego/fondo_nivel.h"
#include "juego/animaciones.h"
#include "tarou_anim.h"

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern char tarou_caminar_p16, tarou_pal;
extern u8 tarou_p16_bancos[];        /* Banco de cada tira .p16 de Tarou (enum Estado).   */
extern char ninja64_andar, ninja_pal;
extern u8 ninja_bancos[];            /* Andar, ataque.                                    */

/*--- Constantes de la escena -----------------------------------------------------------*/
#define SUELO_Y_PROTO 180            /* Profundidad de los pies en el carril.              */
#define TILE_TAROU_PROTO 0           /* Los tiles de Tarou: pares 0-2 (96x64 = 3 KB).       */
#define TILE_NINJA_PROTO 192         /* Los tiles del ninja: el hueco 6-8.                 */
#define TOPE_SUBIDA_PROTO 1536       /* Bytes por VBlank al subir (3 copias de 512).       */
#define PRIORIDAD_PROTO 3            /* Prioridad de los sprites (en BG1 alto, por encima). */
#define ALTO_CELDA_TAROU 64
#define ANCHO_CELDA_TAROU 96
#define CENTRO_TAROU 48              /* x de los pies dentro de la celda de Tarou.         */
#define VRAM_TILE_PROTO(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))  /* 16 palabras por tile. */

/*--- Estado de la escena ---------------------------------------------------------------*/
static Luchador tarou;
static Ninja enemigo;
static Carril carril;
static Mando mando;
static CajaCuadro caja_dano_tarou;     /* Cuerpo de Tarou: dónde le pueden pegar.           */
static CajaCuadro caja_dano_ninja;      /* Cuerpo del ninja: dónde le pueden pegar.          */
static Celda celda_tarou;
static Celda celda_ninja;
static u8 golpeados_tarou;
static u8 golpeados_ninja;
static Subida sub_tarou;
static Subida sub_ninja;
static u8 listo_tarou;
static u8 listo_ninja;

/* Sube, un VBlank tras otro, los tiles de una pose al hueco de la VRAM. La primera vez
   se llama con `iniciar` = 1 para preparar la subida; después, cada frame. */
static void subir(Subida *s, u8 *listo, const u8 *tira, u8 banco, u16 vram, u8 iniciar)
{
    P16Cuadro c;
    const Copia *copias;
    u8 n;

    if (*listo)
        return;
    if (iniciar)
    {
        p16_cuadro(tira, 0, &c);
        subida_empezar(s, c.n, (u16)((u16)tira + c.tiles), vram, banco);
        return;
    }
    subida_tomar(s, TOPE_SUBIDA_PROTO, &copias, &n);
    dma_lista_copias(copias, n, s->banco);
    if (subida_terminada(s))
        *listo = 1;                          /* Ya está entera en la VRAM.                 */
}

/* Dibuja Tarou: su celda y su pose, mirando hacia donde va. */
static void poner_tarou(void)
{
    SpriteEnt *s;
    P16Cuadro c;

    s = sprites_ent;
    p16_cuadro((const u8 *)&tarou_caminar_p16, 0, &c);
    s->x0 = (u16)(PF_ENTERO(tarou.c.x_fp) - CENTRO_TAROU);
    s->y0 = (u16)(mundo_y_dibujo(&tarou.c) - ALTO_CELDA_TAROU + 2);  /* Hundido 2 px.      */
    s->tile = TILE_TAROU_PROTO;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD_PROTO) | SPRITE_PALETA(0) |
                    (tarou.mira_izquierda ? SPRITE_HFLIP : 0));
    s->piezas = (u16)((u16)&tarou_caminar_p16 + (tarou.mira_izquierda ? c.espejo : c.piezas));
    s->banco = tarou_p16_bancos[ESTADO_CAMINANDO];
    s->n = listo_tarou ? c.n : 0;            /* Sin tiles, no se dibuja.                   */
    s->borde = sprites_borde(s->x0, s->y0, ANCHO_CELDA_TAROU, ALTO_CELDA_TAROU);
    sprites_y[0] = PF_ENTERO(tarou.c.y_fp);
    if (tarou.vencido)
        s->n = 0;
}

/* Dibuja el ninja: su celda 64x96 y su pose, mirando hacia Tarou. */
static void poner_ninja(void)
{
    SpriteEnt *s;
    P16Cuadro c;

    s = sprites_ent + 1;
    p16_cuadro((const u8 *)&ninja64_andar, 0, &c);
    s->x0 = (u16)(PF_ENTERO(enemigo.c.x_fp) - NINJA_MITAD);
    s->y0 = (u16)(mundo_y_dibujo(&enemigo.c) - NINJA_ALTO);
    s->tile = TILE_NINJA_PROTO;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD_PROTO) | SPRITE_PALETA(1) |
                    (enemigo.mira_izquierda ? SPRITE_HFLIP : 0));
    s->piezas = (u16)((u16)&ninja64_andar + (enemigo.mira_izquierda ? c.espejo : c.piezas));
    s->banco = ninja_bancos[0];
    s->n = listo_ninja ? c.n : 0;
    s->borde = sprites_borde(s->x0, s->y0, NINJA_ANCHO, NINJA_ALTO);
    sprites_y[1] = PF_ENTERO(enemigo.c.y_fp);
    if (enemigo.vencido)
        s->n = 0;
}

/* Un frame de la lógica: mando, luchador, ninja y los golpes entre los dos. */
static void logica(void)
{
    const CajaCuadro *caja;
    CajaCuadro ataque;
    CajaMundo golpe;
    CajaMundo cuerpo_ninja;
    CajaMundo cuerpo_tarou;
    CajaMundo golpe_ninja;
    u8 r;

    mando_leer(&mando);
    luchador_actualizar(&tarou, &mando, &carril);
    ninja_actualizar(&enemigo, &tarou.c, 1, &carril);   /* Permiso siempre: es el único.   */

    /* Tarou pega al ninja. Un ataque solo le da una vez: se olvida al acabar la caja. */
    caja = luchador_caja_golpe(&tarou);
    if (caja == 0)
    {
        golpeados_tarou = 0;
    }
    else
    {
        combate_caja_mundo(caja, &celda_tarou, &tarou.c, tarou.mira_izquierda, &golpe);
        ninja_caja_dano(&enemigo, &caja_dano_ninja);     /* Cuerpo del ninja.                */
        combate_caja_mundo(&caja_dano_ninja, &celda_ninja, &enemigo.c, enemigo.mira_izquierda,
                           &cuerpo_ninja);
        r = combate_golpear(&golpe, caja, &tarou.c, &tarou.g, &cuerpo_ninja, &enemigo.c,
                            &enemigo.g, &golpeados_tarou, 1);
        if (r != GOLPE_FALLA)
        {
            luchador_marcar_conecto(&tarou);
            ninja_reaccionar(&enemigo, r);
        }
    }

    /* El ninja pega a Tarou con su shinai. */
    if (ninja_caja_golpe(&enemigo, &ataque))
    {
        combate_caja_mundo(&ataque, &celda_ninja, &enemigo.c, enemigo.mira_izquierda, &golpe_ninja);
        combate_caja_mundo(&caja_dano_tarou, &celda_tarou, &tarou.c, tarou.mira_izquierda,
                           &cuerpo_tarou);
        r = combate_golpear(&golpe_ninja, &ataque, &enemigo.c, &enemigo.g, &cuerpo_tarou,
                            &tarou.c, &tarou.g, &golpeados_ninja, 1);
        if (r != GOLPE_FALLA)
            luchador_reaccionar(&tarou, r);
    }
    else
    {
        golpeados_ninja = 0;
    }
}

void prototipo_ejecutar(void)
{
    /*--- Carga, con la pantalla apagada (main la deja así) --------------------------*/
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);
    fondo_nivel_cargar(0);                           /* La capa cercana del nivel 1.       */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    bgSetScroll(0, 0, 0);
    bgSetScroll(1, 0, 0);

    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE16_L32);    /* Piezas pequeñas de 16x16.          */
    sprites_iniciar();
    sprites_usar(2);                                 /* Tarou y el ninja.                  */
    dmaCopyCGram((u8 *)&tarou_pal, 128, 32);         /* Paleta de sprites 0: Tarou.        */
    dmaCopyCGram((u8 *)&ninja_pal, 128 + 16, 32);    /* Paleta 1: el ninja.                */

    /* El carril y la posición de partida. Los límites dejan a Tarou entero en pantalla. */
    carril.x_min_fp = PF(48);
    carril.x_max_fp = PF(200);
    carril.y_arriba_fp = PF(150);
    carril.y_abajo_fp = PF(208);
    celda_tarou.ancho = ANCHO_CELDA_TAROU;
    celda_tarou.mitad_ancho = CENTRO_TAROU;
    celda_tarou.alto = ALTO_CELDA_TAROU;
    celda_ninja.ancho = NINJA_ANCHO;
    celda_ninja.mitad_ancho = NINJA_MITAD;
    celda_ninja.alto = NINJA_ALTO;
    caja_dano_tarou.x = 10;                          /* Cuerpo de Tarou en su celda.       */
    caja_dano_tarou.y = 8;
    caja_dano_tarou.ancho = 40;
    caja_dano_tarou.alto = 52;
    caja_dano_tarou.dano = 0;
    caja_dano_tarou.efecto = EFECTO_RETROCESO;
    caja_dano_tarou.relleno[0] = 0;
    caja_dano_tarou.relleno[1] = 0;

    luchador_iniciar(&tarou, 120, SUELO_Y_PROTO, VIDA_PROTAGONISTA);
    ninja_iniciar(&enemigo, 180, SUELO_Y_PROTO, RANGO_BLANCO);
    golpeados_tarou = 0;
    golpeados_ninja = 0;

    listo_tarou = 0;
    listo_ninja = 0;
    subir(&sub_tarou, &listo_tarou, (const u8 *)&tarou_caminar_p16, tarou_p16_bancos[ESTADO_CAMINANDO],
          VRAM_TILE_PROTO(TILE_TAROU_PROTO), 1);
    subir(&sub_ninja, &listo_ninja, (const u8 *)&ninja64_andar, ninja_bancos[0],
          VRAM_TILE_PROTO(TILE_NINJA_PROTO), 1);

    setBrightness(15);

    /*--- El bucle: un frame, esperar al VBlank, copias de la VRAM -------------------*/
    while (1)
    {
        dma_lista_vaciar();
        subir(&sub_tarou, &listo_tarou, (const u8 *)&tarou_caminar_p16,
              tarou_p16_bancos[ESTADO_CAMINANDO], 0, 0);
        subir(&sub_ninja, &listo_ninja, (const u8 *)&ninja64_andar, ninja_bancos[0], 0, 0);
        logica();
        poner_tarou();
        poner_ninja();
        sprites_ordenar();                           /* Por la y de los pies.              */
        sprites_dibujar();                           /* UNA llamada: toda la OAM.          */
        WaitForVBlank();
        dma_lista_ejecutar();                        /* Las copias de la VRAM, en VBlank.  */
    }
}

#endif /* PROTOTIPO */
