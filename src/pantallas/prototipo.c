/*---------------------------------------------------------------------------------
    prototipo.c — El prototipo de combate: Tarou y dos ninjas en un carril
    (docs/diseno-beat-em-up.md, Etapa 2, entregable 3). Ver prototipo.h.

    PROPÓSITO
      Une la lógica pura (luchador, ninja, combate, marcador) con la capa de
      sprites de la prueba de carga y el fondo del nivel 1. Cada frame:
        1. START alterna la pausa (en pausa no hay lógica, sí dibujo).
        2. El mando dice qué quiere Tarou; el luchador decide su estado.
        3. Cada ninja se acerca o ataca. Tienen permiso los dos: son 2, el máximo
           de ataques a la vez (§6).
        4. Los golpes se resuelven con combate_golpear en los dos sentidos. Un
           ataque solo le da una vez a cada uno. Cada golpe que conecta suma sus
           puntos; cada ninja vencido, los suyos (una sola vez).
        5. Si Tarou cae, pierde una vida; sin vidas, la escena vuelve a empezar.
        6. Las entradas de la OAM se rellenan y sprites_dibujar las escribe.
      Tarou y los ninjas usan, por ahora, la pose de andar (no hay arte de
      combate). HUD de texto (BG3): fila 0 con vida, puntos y vidas; fila 1 con la vida
      de cada ninja y PAUSA. Una fila por frame, por la lista de DMA.

    SE CONECTA CON
      Incluye:  <snes.h>, pantallas/prototipo.h, sistema/comun.h, juego/mando.h,
                juego/luchador.h, enemigos/ninja.h, juego/combate.h,
                juego/cifras.h, juego/mundo.h, juego/marcador.h, juego/sprites.h,
                juego/subida.h, juego/p16.h, juego/dma_lista.h,
                juego/fondo_nivel.h, juego/animaciones.h, tarou_anim.h.
      Lee de la ROM: tarou_caminar_p16, tarou_pal, tarou_p16_bancos (data.asm);
                ninja64_andar, ninja_pal, ninja_bancos (data.asm).
      Lo usan:  sistema/main.c (con -DPROTOTIPO).

    HARDWARE QUE TOCA
      PPU: Mode 1 con el fondo del nivel 1 (fondo_nivel, fondo16_cargar) y BG3 de
      texto. OAM: sprites_dibujar (16x16, OBSEL 16/32). VRAM: tiles de los sprites
      por la lista de DMA, en VBlank. CGRAM: paleta 0 (Tarou) y 1 (los ninjas).

    VARIABLES (estáticas de este archivo)
      tarou, enemigos[2]    Luchador, Ninja[2]  los protagonistas de la escena
      carril                Carril           por dónde se puede andar (x 48..200)
      mando                 Mando            lo que pide el jugador este frame
      pausado               u8               1 mientras dura la pausa (START)
      marcador              Marcador         puntos, vidas y continues (texto del HUD)
      caja_dano_tarou, caja_dano_ninja  CajaCuadro  dónde pueden golpear a cada uno
      golpeados_tarou[2], golpeados_ninja[2]  u8  a quién ya le dio cada ataque
      contado[2]            u8               1 cuando el ninja ya dio sus puntos
      sub_tarou, sub_ninja  Subida           la subida de sus tiles (una vez, al empezar)
      listo_tarou, listo_ninja  u8           1 cuando sus tiles están en la VRAM
    → glosario: Prototipo de combate, Caja de golpe, Caja de daño, OAM, Marcador
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
#include "juego/marcador.h"
#include "juego/hud_debug.h"
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
#define TILE_NINJA_PROTO 192         /* Los tiles de los ninjas: el hueco 6-8 (los dos).   */
#define TOPE_SUBIDA_PROTO 1536       /* Bytes por VBlank al subir (3 copias de 512).       */
#define PRIORIDAD_PROTO 3            /* Prioridad de los sprites (en BG1 alto, por encima). */
#define ALTO_CELDA_TAROU 64
#define ANCHO_CELDA_TAROU 96
#define CENTRO_TAROU 48              /* x de los pies dentro de la celda de Tarou.         */
#define NINJAS_PROTO 2               /* Dos ninjas: el máximo de ataques a la vez (§6).    */
#define VRAM_TILE_PROTO(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))  /* 16 palabras por tile. */

/*--- Estado de la escena ---------------------------------------------------------------*/
static Luchador tarou;
static Ninja enemigos[NINJAS_PROTO];
static Carril carril;
static Mando mando;
static u8 pausado;
static Marcador marcador;
static CajaCuadro caja_dano_tarou;
static CajaCuadro caja_dano_ninja;
static Celda celda_tarou;
static Celda celda_ninja;
static u8 golpeados_tarou[NINJAS_PROTO];
static u8 golpeados_ninja[NINJAS_PROTO];
static u8 contado[NINJAS_PROTO];
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

/* Dibuja el ninja `i`: su celda 64x96 y su pose, mirando hacia Tarou. */
static void poner_ninja(u8 i)
{
    SpriteEnt *s;
    Ninja *n;
    P16Cuadro c;

    n = enemigos + i;
    s = sprites_ent + 1 + i;
    p16_cuadro((const u8 *)&ninja64_andar, 0, &c);
    s->x0 = (u16)(PF_ENTERO(n->c.x_fp) - NINJA_MITAD);
    s->y0 = (u16)(mundo_y_dibujo(&n->c) - NINJA_ALTO);
    s->tile = TILE_NINJA_PROTO;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD_PROTO) | SPRITE_PALETA(1) |
                    (n->mira_izquierda ? SPRITE_HFLIP : 0));
    s->piezas = (u16)((u16)&ninja64_andar + (n->mira_izquierda ? c.espejo : c.piezas));
    s->banco = ninja_bancos[0];
    s->n = listo_ninja ? c.n : 0;
    s->borde = sprites_borde(s->x0, s->y0, NINJA_ANCHO, NINJA_ALTO);
    sprites_y[1 + i] = PF_ENTERO(n->c.y_fp);
    if (n->vencido)
        s->n = 0;
}

/* Tarou pega a un ninja (el `i`): resuelve el golpe y, si conecta, suma y reacciona. */
static void golpe_de_tarou(const CajaCuadro *caja, u8 i)
{
    CajaMundo golpe;
    CajaMundo cuerpo;
    u8 r;

    combate_caja_mundo(caja, &celda_tarou, &tarou.c, tarou.mira_izquierda, &golpe);
    combate_caja_mundo(&caja_dano_ninja, &celda_ninja, &enemigos[i].c,
                       enemigos[i].mira_izquierda, &cuerpo);
    r = combate_golpear(&golpe, caja, &tarou.c, &tarou.g, &cuerpo, &enemigos[i].c,
                        &enemigos[i].g, &golpeados_tarou[i], 1);
    if (r != GOLPE_FALLA)
    {
        luchador_marcar_conecto(&tarou);
        marcador_sumar(&marcador, PUNTOS_GOLPE);
        ninja_reaccionar(&enemigos[i], r);
    }
}

/* El ninja `i` pega a Tarou con su shinai, si le toca. */
static void golpe_del_ninja(u8 i)
{
    CajaCuadro ataque;
    CajaMundo golpe;
    CajaMundo cuerpo;
    u8 r;

    if (!ninja_caja_golpe(&enemigos[i], &ataque))
    {
        golpeados_ninja[i] = 0;              /* Fuera de la caja: el ataque ya acabó.     */
        return;
    }
    combate_caja_mundo(&ataque, &celda_ninja, &enemigos[i].c, enemigos[i].mira_izquierda, &golpe);
    combate_caja_mundo(&caja_dano_tarou, &celda_tarou, &tarou.c, tarou.mira_izquierda, &cuerpo);
    r = combate_golpear(&golpe, &ataque, &enemigos[i].c, &enemigos[i].g, &cuerpo, &tarou.c,
                        &tarou.g, &golpeados_ninja[i], 1);
    if (r != GOLPE_FALLA)
        luchador_reaccionar(&tarou, r);
}

/* Un frame de la lógica: luchador, ninjas y los golpes entre ellos. */
static void logica(void)
{
    const CajaCuadro *caja;
    u8 i;

    luchador_actualizar(&tarou, &mando, &carril);
    for (i = 0; i < NINJAS_PROTO; i++)
        ninja_actualizar(&enemigos[i], &tarou.c, 1, &carril);

    caja = luchador_caja_golpe(&tarou);
    for (i = 0; i < NINJAS_PROTO; i++)
    {
        if (caja == 0)
            golpeados_tarou[i] = 0;          /* Sin caja activa: el próximo ataque cuenta. */
        else
            golpe_de_tarou(caja, i);
        golpe_del_ninja(i);
        if (enemigos[i].vencido == 0 && enemigos[i].g.vida == 0 && contado[i] == 0)
        {
            contado[i] = 1;                  /* Sus puntos, una sola vez.                  */
            marcador_sumar(&marcador, cifras_puntos_ninja(enemigos[i].rango));
        }
    }
}

/*--- hud_prototipo: el HUD de juego en la capa de texto, una fila por frame ----------
    Fila 0: vida de Tarou, puntos y vidas. Fila 1: vida de cada ninja y PAUSA.
    Se usan las funciones de hud_debug para escribir filas (la capa de texto y su
    DMA ya son suyas); el HUD de depuración NO se enciende en el prototipo, así que
    no pisa estas filas. Una fila por frame, como el HUD de depuración, para no
    cargar el VBlank de golpe. */
static u8 turno_hud;

static void hud_fila0(void)
{
    u16 *p;
    char puntos[12];

    p = hud_debug_empezar(0);
    p = hud_debug_texto(p, "VIDA ");
    p = hud_debug_numero(p, (s16)tarou.g.vida);
    p = hud_debug_texto(p, " PUNTOS ");
    marcador_texto(&marcador, puntos);               /* Con miles: 6 cifras, sin pasarse.  */
    p = hud_debug_texto(p, puntos);
    p = hud_debug_texto(p, " VIDAS ");
    p = hud_debug_numero(p, (s16)marcador.vidas);
    hud_debug_enviar(0);
}

static void hud_fila1(void)
{
    u16 *p;

    p = hud_debug_empezar(1);
    p = hud_debug_texto(p, "NINJA ");
    p = hud_debug_numero(p, (s16)enemigos[0].g.vida);
    p = hud_debug_texto(p, "  NINJA ");
    p = hud_debug_numero(p, (s16)enemigos[1].g.vida);
    if (pausado)
        p = hud_debug_texto(p, "  PAUSA");
    hud_debug_enviar(1);
}

static void hud_prototipo(void)
{
    if (turno_hud == 0)
        hud_fila0();
    else
        hud_fila1();
    turno_hud = (u8)(1 - turno_hud);
}

/* Empieza la escena de cero: Tarou con vida, dos ninjas en su sitio, marcador a cero. */
static void empezar(void)
{
    u8 i;

    luchador_iniciar(&tarou, 120, SUELO_Y_PROTO, VIDA_PROTAGONISTA);
    ninja_iniciar(&enemigos[0], 180, SUELO_Y_PROTO, RANGO_BLANCO);
    ninja_iniciar(&enemigos[1], 150, SUELO_Y_PROTO - 12, RANGO_AMARILLO);
    for (i = 0; i < NINJAS_PROTO; i++)
    {
        golpeados_tarou[i] = 0;
        golpeados_ninja[i] = 0;
        contado[i] = 0;
    }
}

/* Tarou ha caído: pierde una vida. Sin vidas, vuelve la partida entera; si no, reaparece. */
static void tarou_cae(void)
{
    if (marcador_perder_vida(&marcador))
    {
        marcador_iniciar(&marcador);         /* Se acabó la partida: de nuevo desde cero.  */
        empezar();
    }
    else
    {
        luchador_iniciar(&tarou, 120, SUELO_Y_PROTO, VIDA_PROTAGONISTA);  /* Reaparece.    */
    }
}

void prototipo_ejecutar(void)
{
    u8 i;

    /*--- Carga, con la pantalla apagada (main la deja así) --------------------------*/
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);
    fondo_nivel_cargar(0);                           /* La capa cercana del nivel 1.       */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    bgSetScroll(0, 0, 0);
    bgSetScroll(1, 0, 0);

    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE16_L32);    /* Piezas pequeñas de 16x16.          */
    sprites_iniciar();
    sprites_usar(1 + NINJAS_PROTO);                  /* Tarou y los ninjas.                */
    dmaCopyCGram((u8 *)&tarou_pal, 128, 32);         /* Paleta de sprites 0: Tarou.        */
    dmaCopyCGram((u8 *)&ninja_pal, 128 + 16, 32);    /* Paleta 1: los ninjas.              */

    /* El carril y las celdas. Los límites dejan a Tarou entero en pantalla. */
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

    marcador_iniciar(&marcador);
    pausado = 0;
    empezar();

    hud_debug_iniciar();                             /* Filas de texto en blanco; BG3 apagado. */
    bgSetEnable(2);                                  /* BG3 (la capa de texto) encendida.       */

    listo_tarou = 0;
    listo_ninja = 0;
    subir(&sub_tarou, &listo_tarou, (const u8 *)&tarou_caminar_p16, tarou_p16_bancos[ESTADO_CAMINANDO],
          VRAM_TILE_PROTO(TILE_TAROU_PROTO), 1);
    subir(&sub_ninja, &listo_ninja, (const u8 *)&ninja64_andar, ninja_bancos[0],
          VRAM_TILE_PROTO(TILE_NINJA_PROTO), 1);

    setBrightness(15);

    /*--- El bucle: un frame, dibujo, esperar al VBlank, copias de la VRAM ----------*/
    while (1)
    {
        dma_lista_vaciar();
        mando_leer(&mando);
        if (mando.salir_pulsado)
            pausado = (u8)(1 - pausado);     /* START: pausa o sigue.                      */

        subir(&sub_tarou, &listo_tarou, (const u8 *)&tarou_caminar_p16,
              tarou_p16_bancos[ESTADO_CAMINANDO], 0, 0);
        subir(&sub_ninja, &listo_ninja, (const u8 *)&ninja64_andar, ninja_bancos[0], 0, 0);

        if (!pausado)
        {
            logica();
            if (tarou.vencido)
                tarou_cae();
        }
        poner_tarou();
        for (i = 0; i < NINJAS_PROTO; i++)
            poner_ninja(i);
        hud_prototipo();                             /* Una fila de texto, a la lista de DMA. */
        sprites_ordenar();                           /* Por la y de los pies.              */
        sprites_dibujar();                           /* UNA llamada: toda la OAM.          */
        WaitForVBlank();
        dma_lista_ejecutar();                        /* Las copias de la VRAM, en VBlank.  */
    }
}

#endif /* PROTOTIPO */
