/*---------------------------------------------------------------------------------
    pantalla_fase.c — La pantalla de la fase 1 (ver pantalla_fase.h).

    PROPÓSITO
      Cada frame: dma_lista_vaciar → mando → (según el modo) un frame del director o de una
      escena → dibujar las entidades → columnas del fondo → una fila de texto → WaitForVBlank →
      efectos de la escena, scroll y lista de DMA. El director (fase.c) decide TODO lo que pasa
      en el juego y devuelve eventos; aquí solo se aplican:
        ESCENA   las cinco escenas de diálogo: la imagen se apaga y se pixela con el motor de
                 escenas (escena_dialogo.c), salen los cuadros de texto (dialogos.c) y, al acabar,
                 se afila otra vez.
        SALIDA   la zona siguiente: fundido, se reescribe el anillo del fondo con la pantalla
                 apagada y la fase la empieza (fase_siguiente_zona).
        CLEAR    STAGE CLEAR con la puntuación y STAGE 2 / COMING SOON; luego vuelve a empezar.
        FIN_JUEGO  GAME OVER, con la cuenta atrás del continue (START o A para seguir).
      PLACEHOLDERS (a sustituir cuando haya arte): TODOS usan el fondo del nivel 1 en las cuatro
      zonas; Tarou y Bárbara dibujan la pose de andar de Tarou (el HUD dice quién está activo);
      los ninjas dibujan su pose de andar con la paleta de su rango (el rojo repite la del verde);
      no hay sombras, ni nube de relevo, ni los efectos de pantalla de los apoyos, ni rompibles.
      Sin cielo en HDMA: el canal 6 lo usan los efectos de las escenas.

    SE CONECTA CON
      Incluye:  <snes.h>, pantallas/pantalla_fase.h, sistema/comun.h, sistema/pantallas.h,
                juego/mando.h, juego/fase.h, juego/dialogos.h, juego/luchador.h, juego/cifras.h,
                juego/mundo.h, juego/sprites.h, juego/subida.h, juego/p16.h, juego/dma_lista.h,
                juego/fondo_nivel.h, juego/hud_debug.h, juego/animaciones.h, escena/escena.h,
                escena/escena_dialogo.h, escena/efectos.h, tarou_anim.h.
      Lee de la ROM: tarou_caminar_p16, tarou_pal, tarou_p16_bancos, ninja64_andar, ninja_pal,
                ninja_bancos (data.asm) y los fondos del nivel 1.
      Lo usan:  sistema/main.c (con -DFASE1).

    HARDWARE QUE TOCA
      PPU: Mode 1 (BG1 cercano, BG2 lejano, BG3 texto), scroll de BG1 y BG2 en el VBlank, mosaico
      y brillo por escena/efectos.c. OAM por sprites_dibujar. VRAM por la lista de DMA. CGRAM:
      paletas de sprites 0 (Tarou) y 1..3 (ninjas). Fundidos de pantalla (fade_in/fade_out).

    VARIABLES (estáticas de este archivo)
      fase             Fase     la partida
      escena           Escena   el motor de escenas para los fundidos de diálogo
      mando            Mando    lo que pide el jugador este frame
      modo             u8       MODO_*: qué está pasando en la pantalla
      pausado          u8       1 mientras dura la pausa (START)
      cuadro           u8       el cuadro de texto en curso
      contador         u16      frames en el estado actual
      segundos         u8       la cuenta atrás del continue
      turno_hud        u8       qué fila del texto toca este frame
      sub_tarou, sub_ninja, listo_tarou, listo_ninja   las subidas de tiles de las poses
    → glosario: Fase, Director de fase, Motor de escenas, Bloqueo de cámara
---------------------------------------------------------------------------------*/
#ifdef FASE1

#include <snes.h>
#include "pantallas/pantalla_fase.h"
#include "sistema/comun.h"
#include "sistema/pantallas.h"
#include "juego/mando.h"
#include "juego/fase.h"
#include "juego/dialogos.h"
#include "juego/luchador.h"
#include "juego/cifras.h"
#include "juego/mundo.h"
#include "juego/sprites.h"
#include "juego/subida.h"
#include "juego/p16.h"
#include "juego/dma_lista.h"
#include "juego/fondo_nivel.h"
#include "juego/hud_debug.h"
#include "juego/animaciones.h"
#include "escena/escena.h"
#include "escena/escena_dialogo.h"
#include "escena/efectos.h"
#include "tarou_anim.h"

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern char tarou_caminar_p16, tarou_pal;
extern u8 tarou_p16_bancos[];        /* Banco de cada tira .p16 de Tarou (enum Estado).   */
extern char ninja64_andar, ninja_pal;
extern u8 ninja_bancos[];            /* Andar, ataque.                                    */

/*--- Constantes ------------------------------------------------------------------------*/
#define TILE_TAROU_F 0               /* Los tiles de Tarou: pares 0-2 (96x64 = 3 KB).       */
#define TILE_NINJA_F 192             /* Los de los ninjas: el hueco 6-8 (los tres lo comparten). */
#define TOPE_SUBIDA_F 1536           /* Bytes por VBlank al subir (3 copias de 512).        */
#define PRIORIDAD_F 3                /* Prioridad de los sprites.                           */
#define ANCHO_CELDA_TAROU 96
#define ALTO_CELDA_TAROU 64
#define CENTRO_TAROU 48              /* x de los pies dentro de la celda de Tarou.          */
#define VRAM_TILE_F(t) ((u16)(VRAM_SPRITES + ((u16)(t) << 4)))  /* 16 palabras por tile.   */
#define MASCARA_FONDOS 0x03          /* El mosaico solo afecta a BG1 y BG2: el texto (BG3) queda nítido. */
#define FRAMES_CUADRO 150            /* Un cuadro de texto dura 2,5 s (A lo adelanta).      */
#define FRAMES_SIN_SALTAR 20         /* Los primeros 20 frames de un cuadro, A no lo salta. */
#define FRAMES_CLEAR 300             /* STAGE CLEAR y STAGE 2 duran 5 s cada uno.           */
#define FRAMES_SIN_CONTINUE 240      /* Sin continues, GAME OVER dura 4 s.                  */

/* Qué hace la pantalla. */
enum
{
    MODO_JUEGO = 0,                    /* Se juega (o está en pausa).                         */
    MODO_ENTRA,                        /* La escena entra: se apaga y se pixela.              */
    MODO_DIALOGO,                      /* Los cuadros de texto.                               */
    MODO_SALE,                         /* La escena sale: vuelve nítida.                      */
    MODO_SALE_FIN,                     /* Un frame más con los valores finales ya aplicados.  */
    MODO_CLEAR,                        /* STAGE CLEAR.                                        */
    MODO_GAMEOVER                      /* GAME OVER y el continue.                            */
};

/* Los nombres de los apoyos, en el orden de APOYO_*. */
static const char *const NOMBRE_APOYO[APOYOS] = {"LUCINDA", "REX", "FENRIR"};

/* Qué fila del texto toca en cada uno de 8 frames: las dos del marcador, el doble de veces. */
static const u8 ORDEN_HUD[8] = {0, 1, 2, 3, 4, 5, 6, 0};

/*--- Estado ----------------------------------------------------------------------------*/
static Fase fase;
static Escena escena;
static Mando mando;
static Subida sub_tarou;
static Subida sub_ninja;
static u8 listo_tarou;
static u8 listo_ninja;
static u8 modo;
static u8 pausado;
static u8 cuadro;
static u16 contador;
static u8 segundos;
static u8 turno_hud;

/*--- Tiles de las poses ----------------------------------------------------------------*/

/* Sube, un VBlank tras otro, los tiles de una pose al hueco de la VRAM. La primera vez se llama con
   `iniciar` = 1 para preparar la subida; después, cada frame hasta que `listo` valga 1. */
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
    subida_tomar(s, TOPE_SUBIDA_F, &copias, &n);
    dma_lista_copias(copias, n, s->banco);
    if (subida_terminada(s))
        *listo = 1;
}

/*--- Dibujar las entidades -------------------------------------------------------------*/

/* La x de pantalla (con signo) del borde izquierdo de una celda cuyos pies están en x_px. */
static s16 x_pantalla(u16 x_px, u8 mitad)
{
    return (s16)x_px - (s16)fase.camara - (s16)mitad;
}

/* Tarou y Bárbara: provisionalmente, la pose de andar de Tarou. */
static void poner_protagonista(void)
{
    Luchador *p;
    SpriteEnt *s;
    P16Cuadro c;

    p = relevo_activo(&fase.relevo);
    s = sprites_ent;
    p16_cuadro((const u8 *)&tarou_caminar_p16, 0, &c);
    s->x0 = (u16)x_pantalla(PF_ENTERO(p->c.x_fp), CENTRO_TAROU);
    s->y0 = (u16)(mundo_y_dibujo(&p->c) - ALTO_CELDA_TAROU + 2);       /* Hundido 2 px.    */
    s->tile = TILE_TAROU_F;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD_F) | SPRITE_PALETA(0) |
                    (p->mira_izquierda ? SPRITE_HFLIP : 0));
    s->piezas = (u16)((u16)&tarou_caminar_p16 + (p->mira_izquierda ? c.espejo : c.piezas));
    s->banco = tarou_p16_bancos[ESTADO_CAMINANDO];
    s->n = listo_tarou ? c.n : 0;                    /* Sin tiles en la VRAM, no se dibuja.  */
    s->borde = sprites_borde(s->x0, s->y0, ANCHO_CELDA_TAROU, ALTO_CELDA_TAROU);
    sprites_y[0] = PF_ENTERO(p->c.y_fp);
}

/* El enemigo `i`: su pose de andar, con la paleta de su rango. Fuera de la pantalla, no se dibuja. */
static void poner_enemigo(u8 i)
{
    Ninja *n;
    SpriteEnt *s;
    P16Cuadro c;
    s16 x;
    u8 paleta;

    n = &fase.ninjas[i];
    s = sprites_ent + 1 + i;
    x = x_pantalla(PF_ENTERO(n->c.x_fp), NINJA_MITAD);
    if (!fase.vivo[i] || n->vencido || x <= -NINJA_ANCHO || x >= ANCHO_PANTALLA)
    {
        s->n = 0;                                    /* Hueco libre, KO acabado o fuera.     */
        sprites_y[1 + i] = 0;
        return;
    }
    p16_cuadro((const u8 *)&ninja64_andar, 0, &c);
    paleta = n->rango;
    if (paleta > 2)
        paleta = 2;                                  /* Solo hay 3 paletas de ninja: el rojo repite. */
    s->x0 = (u16)x;
    s->y0 = (u16)(mundo_y_dibujo(&n->c) - NINJA_ALTO);
    s->tile = TILE_NINJA_F;
    s->atrib = (u8)(SPRITE_PRIORIDAD(PRIORIDAD_F) | SPRITE_PALETA(1 + paleta) |
                    (n->mira_izquierda ? SPRITE_HFLIP : 0));
    s->piezas = (u16)((u16)&ninja64_andar + (n->mira_izquierda ? c.espejo : c.piezas));
    s->banco = ninja_bancos[0];
    s->n = listo_ninja ? c.n : 0;
    s->borde = sprites_borde(s->x0, s->y0, NINJA_ANCHO, NINJA_ALTO);
    sprites_y[1 + i] = PF_ENTERO(n->c.y_fp);
}

static void dibujar(void)
{
    u8 i;

    poner_protagonista();
    for (i = 0; i < FASE_ENEMIGOS; i++)
        poner_enemigo(i);
    sprites_ordenar();                               /* Por la y de los pies.               */
    sprites_dibujar();                               /* UNA llamada: toda la OAM.           */
}

/*--- El texto ---------------------------------------------------------------------------*/

/* Escribe la fila `fila` del texto y la apunta en la lista de DMA. */
static void hud_fila(u8 fila)
{
    u16 *p;
    char buf[12];
    const Luchador *a;

    p = hud_debug_empezar(fila);
    switch (fila)
    {
    case 0:                          /* Quién juega, su vida, los puntos y las vidas.        */
        a = relevo_activo(&fase.relevo);
        if (fase.relevo.activo == QUIEN_BARBARA)
            p = hud_debug_texto(p, "BARBARA ");
        else
            p = hud_debug_texto(p, "TAROU ");
        p = hud_debug_numero(p, (s16)a->g.vida);
        p = hud_debug_texto(p, " PTS ");
        marcador_texto(&fase.marcador, buf);
        p = hud_debug_texto(p, buf);
        p = hud_debug_texto(p, " VIDAS ");
        p = hud_debug_numero(p, (s16)fase.marcador.vidas);
        break;
    case 1:                          /* La barra de ataque y el apoyo elegido.               */
        if (!fase.apoyos.activa)
        {
            p = hud_debug_texto(p, "BARRA BLOQUEADA");
        }
        else
        {
            p = hud_debug_texto(p, "BARRA ");
            p = hud_debug_numero(p, (s16)fase.apoyos.golpes);
            p = hud_debug_texto(p, "/");
            p = hud_debug_numero(p, BARRA_LLENA);
            p = hud_debug_texto(p, " R:");
            p = hud_debug_texto(p, NOMBRE_APOYO[fase.apoyos.seleccionado]);
            if (apoyos_puede_invocar(&fase.apoyos))
                p = hud_debug_texto(p, " X!");
        }
        break;
    case 2:                          /* El cartel de arriba.                                 */
        if (modo == MODO_CLEAR)
        {
            if (contador < FRAMES_CLEAR)
                p = hud_debug_texto(p, "STAGE CLEAR");
            else
                p = hud_debug_texto(p, "STAGE 2");
        }
        else if (modo == MODO_GAMEOVER)
        {
            p = hud_debug_texto(p, "GAME OVER");
        }
        else if (pausado)
        {
            p = hud_debug_texto(p, "PAUSA");
        }
        else if (oleadas_go_activo(&fase.oleadas))
        {
            p = hud_debug_texto(p, "GO >>");
        }
        break;
    case 3:                          /* El cartel de abajo.                                  */
        if (modo == MODO_CLEAR)
        {
            if (contador < FRAMES_CLEAR)
            {
                p = hud_debug_texto(p, "SCORE ");
                marcador_texto(&fase.marcador, buf);
                p = hud_debug_texto(p, buf);
            }
            else
            {
                p = hud_debug_texto(p, "COMING SOON");
            }
        }
        else if (modo == MODO_GAMEOVER && fase.marcador.continues > 0)
        {
            p = hud_debug_texto(p, "CONTINUE? ");
            p = hud_debug_numero(p, (s16)segundos);
        }
        break;
    case 4:                          /* Quién habla.                                         */
        if (modo == MODO_DIALOGO)
            p = hud_debug_texto(p, dialogo_orador(fase.escena, cuadro));
        else if (modo == MODO_GAMEOVER)
            p = hud_debug_texto(p, "PIMENTON-SAMA");
        break;
    case 5:                          /* Su primera línea.                                    */
        if (modo == MODO_DIALOGO)
            p = hud_debug_texto(p, dialogo_linea1(fase.escena, cuadro));
        else if (modo == MODO_GAMEOVER)
            p = hud_debug_texto(p, "Don't give up, Tarou.");
        break;
    case 6:                          /* Y la segunda.                                        */
        if (modo == MODO_DIALOGO)
            p = hud_debug_texto(p, dialogo_linea2(fase.escena, cuadro));
        else if (modo == MODO_GAMEOVER)
            p = hud_debug_texto(p, "I'll be waiting.");
        break;
    default:
        break;
    }
    hud_debug_enviar(fila);
}

/* Una fila por frame: el VBlank no aguanta más texto de golpe (ver prototipo.c). */
static void hud_siguiente(void)
{
    hud_fila(ORDEN_HUD[turno_hud & 7]);
    turno_hud++;
}

/*--- Cargar el mundo --------------------------------------------------------------------*/

/* Con la pantalla apagada: reescribe el anillo del fondo para la cámara actual y deja el scroll y
   la OAM listos. Se usa al cambiar de zona y al reiniciar. */
static void recargar_mundo(void)
{
    fondo_nivel_rellenar(fase.camara);
    bgSetScroll(0, fase.camara, 0);
    bgSetScroll(1, fase.camara >> 1, 0);
    dibujar();                                       /* Las posiciones nuevas, a la OAM.     */
    dma_lista_vaciar();
}

/* La zona siguiente: fundido, carga con la pantalla apagada y la fase la empieza. */
static void cambiar_zona(void)
{
    fade_out(FRAMES_FADE);
    setScreenOff();
    fase_siguiente_zona(&fase);
    recargar_mundo();
    fade_in(FRAMES_FADE);
}

/* Empezar la partida otra vez desde la aldea. */
static void reiniciar_partida(void)
{
    fade_out(FRAMES_FADE);
    setScreenOff();
    fase_iniciar(&fase);
    modo = MODO_JUEGO;
    pausado = 0;
    recargar_mundo();
    fade_in(FRAMES_FADE);
}

/*--- Aplicar los eventos del director -----------------------------------------------------*/

static void manejar(u8 ev)
{
    if (ev & FASE_EV_ESCENA)
    {
        if (dialogo_cuadros(fase.escena) == 0)
        {
            manejar(fase_escena_terminada(&fase));   /* Sin texto: no hay escena que jugar.  */
        }
        else
        {
            efectos_preparar(MASCARA_FONDOS);        /* Mosaico solo en los fondos, no en el texto. */
            escena_empezar(&escena, ESCENA_DIALOGO_ENTRA);
            modo = MODO_ENTRA;
            cuadro = 0;
            contador = 0;
        }
    }
    if (ev & FASE_EV_SALIDA)
        cambiar_zona();
    if (ev & FASE_EV_CLEAR)
    {
        modo = MODO_CLEAR;
        contador = 0;
    }
    if (ev & FASE_EV_FIN_JUEGO)
    {
        modo = MODO_GAMEOVER;
        contador = 0;
        segundos = 9;                                /* La cuenta atrás del continue.         */
    }
}

/*--- Un frame según el modo ---------------------------------------------------------------*/

static void frame_juego(void)
{
    if (mando.salir_pulsado)
        pausado = (u8)(1 - pausado);                 /* START: pausa o sigue.                 */
    if (!pausado)
        manejar(fase_actualizar(&fase, &mando));
}

static void frame_dialogo(void)
{
    contador++;
    if ((mando.atacar_pulsado && contador > FRAMES_SIN_SALTAR) || contador >= FRAMES_CUADRO)
    {
        cuadro++;                                    /* Siguiente cuadro (A lo adelanta).     */
        contador = 0;
    }
    if (cuadro >= dialogo_cuadros(fase.escena))
    {
        escena_empezar(&escena, ESCENA_DIALOGO_SALE);
        modo = MODO_SALE;
    }
}

static void frame_gameover(void)
{
    contador++;
    if (fase.marcador.continues == 0)
    {
        if (contador >= FRAMES_SIN_CONTINUE)
            reiniciar_partida();                     /* Sin continues: de nuevo desde cero.   */
        return;
    }
    if (contador >= 60)
    {
        contador = 0;
        if (segundos > 0)
            segundos--;                              /* Un segundo menos.                     */
    }
    if (mando.salir_pulsado || mando.atacar_pulsado)
    {
        if (fase_continuar(&fase))
        {
            modo = MODO_JUEGO;                         /* Sigue donde estaba.                   */
            pausado = 0;
            return;
        }
    }
    if (segundos == 0)
        reiniciar_partida();                         /* Se acabó la cuenta atrás.             */
}

/*--- Punto de entrada ---------------------------------------------------------------------*/

void fase1_ejecutar(void)
{
    /*--- Carga, con la pantalla apagada (main la deja así) --------------------------*/
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);      /* BG3 (el texto) por delante.          */
    fondo_nivel_cargar(0);                           /* La capa cercana del nivel 1.         */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    bgSetScroll(0, 0, 0);
    bgSetScroll(1, 0, 0);
    fuente_2bpp(2);                                  /* La fuente y el mapa de texto en BG3. */
    hud_debug_iniciar();                             /* Filas en blanco; BG3 apagado...      */
    bgSetEnable(2);                                  /* ...y lo encendemos.                  */

    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE16_L32);    /* Piezas pequeñas de 16x16.            */
    sprites_iniciar();
    sprites_usar(1 + FASE_ENEMIGOS);                 /* El protagonista y los 3 enemigos.    */
    dmaCopyCGram((u8 *)&tarou_pal, 128, 32);         /* Paleta de sprites 0: Tarou.          */
    dmaCopyCGram((u8 *)&ninja_pal, 128 + 16, 32 * 3);   /* Paletas 1..3: los ninjas.         */

    fase_iniciar(&fase);
    modo = MODO_JUEGO;
    pausado = 0;
    cuadro = 0;
    contador = 0;
    segundos = 0;
    turno_hud = 0;
    listo_tarou = 0;
    listo_ninja = 0;
    subir(&sub_tarou, &listo_tarou, (const u8 *)&tarou_caminar_p16, tarou_p16_bancos[ESTADO_CAMINANDO],
          VRAM_TILE_F(TILE_TAROU_F), 1);
    subir(&sub_ninja, &listo_ninja, (const u8 *)&ninja64_andar, ninja_bancos[0],
          VRAM_TILE_F(TILE_NINJA_F), 1);

    setBrightness(15);

    /*--- El bucle: un frame, esperar al VBlank, y entonces la PPU ------------------*/
    while (1)
    {
        dma_lista_vaciar();
        mando_leer(&mando);
        subir(&sub_tarou, &listo_tarou, (const u8 *)&tarou_caminar_p16,
              tarou_p16_bancos[ESTADO_CAMINANDO], 0, 0);
        subir(&sub_ninja, &listo_ninja, (const u8 *)&ninja64_andar, ninja_bancos[0], 0, 0);

        switch (modo)
        {
        case MODO_JUEGO:
            frame_juego();
            break;
        case MODO_ENTRA:
            escena_avanzar(&escena);
            if (escena.terminada)
                modo = MODO_DIALOGO;                   /* Se queda con los valores finales.    */
            break;
        case MODO_DIALOGO:
            frame_dialogo();
            break;
        case MODO_SALE:
            escena_avanzar(&escena);
            if (escena.terminada)
                modo = MODO_SALE_FIN;                  /* Un frame más para aplicar 15 y 0.    */
            break;
        case MODO_SALE_FIN:
            efectos_terminar();                      /* Sin mosaico ni luz; el brillo ya es 15. */
            modo = MODO_JUEGO;
            manejar(fase_escena_terminada(&fase));   /* Lucinda y Rex se unen; la zona sigue. */
            break;
        case MODO_CLEAR:
            contador++;
            if (contador >= 2 * FRAMES_CLEAR)
                reiniciar_partida();
            break;
        default:
            frame_gameover();
            break;
        }

        dibujar();
        fondo_nivel_actualizar(fase.camara);         /* Las columnas que entran, a la lista.  */
        hud_siguiente();
        WaitForVBlank();
        if (modo == MODO_ENTRA || modo == MODO_DIALOGO || modo == MODO_SALE || modo == MODO_SALE_FIN)
            efectos_aplicar(&escena);                /* Brillo y mosaico de la escena.        */
        bgSetScroll(0, fase.camara, 0);              /* El scroll, en el VBlank.              */
        bgSetScroll(1, fase.camara >> 1, 0);
        dma_lista_ejecutar();                        /* Columnas, texto y tiles.              */
    }
}

#endif /* FASE1 */
