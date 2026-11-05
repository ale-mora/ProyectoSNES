/*---------------------------------------------------------------------------------
    pantalla_fase1.c — La fase 1 del beat 'em up: ESQUELETO jugable (Revisión 3).

    PROPÓSITO
      Docs/diseno-beat-em-up.md paso a paso: Tarou anda por el carril (cruceta: x y
      profundidad), salta con B y lleva su sombra, y ahora hay CÁMARA y ZONAS: la cámara
      sigue a Tarou solo hacia delante, se bloquea en el punto de cada zona (juego/oleadas.c)
      y no se libera hasta vencer la oleada. Al bloquearse salen los NINJAS de la zona
      (enemigos/ninja.c: caminan hacia Tarou y se quedan a distancia de ataque); la oleada se
      vence cuando no queda ninguno. Tarou GOLPEA con A: combo de tres golpes (juego/mundo.c),
      caja de golpe y daño (juego/combate.c); el ninja se aturde, retrocede y parpadea, y el tercer
      golpe lo derriba. Al conectar hay «parada de impacto» (el mundo se congela 3-5 cuadros).
      Los ninjas también ATACAN (tajo de shinai, como mucho dos a la vez): cada golpe quita 2 de
      los 24 puntos de vida de Tarou, lo aturde y lo deja parpadeando e invulnerable ~1 s; la barra
      de vida está en la fila 0 de la capa de texto (hud/hud_juego.c). Sin vida cae KO y 2,5 s
      después vuelve al título (provisional). La protagonista es la que se eligió en el
      selector (juego/personaje.h: Tarou o Bárbara, con su arte y su alcance); cada uno se cuelga de
      este bucle en su módulo (§10). El fondo es la ALDEA al atardecer (2026-11-05,
      scripts/preparar_aldea.py): la calle de 2048 px con fachadas, puestos y aldeanos
      (capa cercana, por columnas), el farol y los barriles por delante de los
      personajes (bit de prioridad del mapa: los sprites van con prioridad 2), el noren
      de los puestos ondeando (animación de tiles, cada 8 frames), las montañas con el
      castillo a la mitad de velocidad y el cielo del violeta al naranja (HDMA). Encima,
      el humo de los tejados y una bandada de pájaros (juego/ambiente.c, sprites de la
      paleta 7, translúcidos); en pausa se quedan quietos.
      Por frame: mando_leer → mundo_actualizar → límites de pantalla → cámara y zonas →
      protagonista_sprite_actualizar → fondo (columnas que entran) → sprites_dibujar. El scroll
      de los fondos se escribe en el VBlank (pantalla_fase1_vblank), con la lista de DMA y no
      antes: bgSetScroll escribe en el acto, y a mitad del dibujo partiría la imagen.
      START pausa (SELECT en pausa vuelve al título); SELECT jugando enciende el HUD de
      depuración. Sin vida: KO, GAME OVER y CONTINUE; tras la última oleada, STAGE CLEAR y
      «STAGE 2 / COMING SOON» (juego/estado_fase.c).

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h, juego/mando.h,
                juego/mundo.h, juego/camara.h, juego/oleadas.h, juego/combate.h, juego/protagonista_sprite.h,
                enemigos/ninja.h, enemigos/ninja_sprite.h,
                juego/sprites.h, juego/fondo_nivel.h, juego/dma_lista.h, juego/hud_debug.h,
                nivel1_datos.h.
      Lo usan:  sistema/secuencia.c (iniciar, actualizar, vblank); el título la abre con Start.
      Lee de la ROM: la aldea (data.asm: aldea_*, hdmaGradatardecerList), aldea_datos.h.

    HARDWARE QUE TOCA
      PPU: Mode 1, scroll de los fondos 1 y 2 (en el VBlank), BG3 del HUD; HDMA del cielo
      (canal 6). El resto lo hace protagonista_sprite.c (OAM, paletas, matemática de color)
      y fondo_nivel.c (las columnas del mapa, por la lista de DMA).

    VARIABLES (estáticas de este archivo)
      cuerpo      Cuerpo    el protagonista en el carril
      mando       Mando     lo que pide el jugador este frame
      oleadas     Oleadas   la zona en curso y si la cámara está bloqueada
      ninjas      Ninja[3]  las ranuras de enemigos (enemigos/ninja.h)
      camara      u16       scroll horizontal; se calcula en el frame y se escribe en el VBlank
      hud_visible u8        1 si SELECT encendió el HUD
      parada      u8        cuadros de parada de impacto que quedan; a_guardada: A pulsada en la parada
      partida     EstadoFase  jugando, pausa, KO, GAME OVER con CONTINUE o STAGE CLEAR (estado_fase.h)
      atenuada, cambiar_luz, avisos  u8  media luz puesta, pendiente de cambiar, filas de aviso con texto
      oscuras     u16[64]   las paletas de sprites 0..3 a media luz (los sprites 0..3 no entran
                            en la matemática de color: se oscurecen con su paleta)
      fila_hud    u8        qué fila del HUD toca preparar este frame (0..3)
    → glosario: Carril, Cámara, Zona, Oleada, Parallax, Máquina de estados
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "juego/nivel.h"             /* ANCHO_PANTALLA.                                   */
#include "juego/mando.h"
#include "juego/mundo.h"
#include "juego/camara.h"
#include "juego/oleadas.h"
#include "juego/combate.h"
#include "juego/personaje.h"
#include "juego/estado_fase.h"
#include "juego/protagonista_sprite.h"
#include "enemigos/ninja.h"
#include "enemigos/ninja_sprite.h"
#include "juego/sprites.h"
#include "juego/fondo_nivel.h"
#include "juego/ambiente.h"
#include "juego/ambiente_sprite.h"
#include "objetos/objetos.h"
#include "objetos/objetos_sprite.h"
#include "juego/dma_lista.h"
#include "juego/hud_debug.h"
#include "hud/hud_juego.h"
#include "hud/hud_apoyo.h"
#include "juego/dialogo.h"
#include "juego/apoyo.h"
extern char aldea_lejos_til, aldea_lejos_til_end, aldea_lejos_pal;   /* La lejana: montañas, */
extern char aldea_lejos_map, aldea_lejos_map_end;                     /* castillo y sol.      */
extern const unsigned char hdmaGradatardecerList[];   /* El cielo, un color por línea (gfx4snes -n). */
extern char bosque_lejos_til, bosque_lejos_til_end, bosque_lejos_pal, bosque_lejos_map, bosque_lejos_map_end;
extern char monasterio_lejos_til, monasterio_lejos_til_end, monasterio_lejos_pal, monasterio_lejos_map,
    monasterio_lejos_map_end;
extern char dojo_lejos_til, dojo_lejos_til_end, dojo_lejos_pal, dojo_lejos_map, dojo_lejos_map_end;
extern const unsigned char hdmaGradcielobosqueList[], hdmaGradcielomonasterioList[], hdmaGradcielodojoList[];
extern char ninja1_pal, ninja2_pal, ninja3_pal;   /* Las paletas de los ninjas (data.asm). */

#define X_INICIAL 64                 /* Donde nace Tarou: a la izquierda.                 */
#define Y_INICIAL 200                /* En medio del carril (184..216).                   */
#define FILAS_HUD 5                  /* Filas del HUD de depuración: 0..3 y la 5 (la V).   */
#define REG_SLHV (*(volatile u8 *)0x2137)   /* Leer congela los contadores del haz.         */
#define REG_OPVCT (*(volatile u8 *)0x213D)  /* La línea del haz (dos lecturas: 8 + 1 bits). */
#define FILA_AVISO 6                 /* Filas del HUD (6 y 7) de PAUSE, GAME OVER y STAGE CLEAR. */
#define NOREN_CADA 7                 /* El noren cambia de cuadro cada 8 frames (máscara). */
#define NOREN_HUECO 2560             /* Si la lista ya lleva más bytes, el noren espera.   */
#define MARGEN_NINJA 32              /* Un ninja ya dentro no pasa de este margen del borde: su cuerpo (~50 px) se ve entero. */
#define REG_MOSAICO (*(volatile u8 *)0x2106) /* Mosaico: tamaño (bits 4..7) y fondos (0..3).  */
#define EFECTO_MOSAICO 1             /* La onda de Rex.                                   */
#define EFECTO_TEMBLOR 2             /* La embestida de Fenrir.                           */
#define EFECTO_FRAMES 24
#define SALIDA_MARGEN 32             /* A cuántos px del final del escenario se pasa al siguiente. */
#ifndef ESCENARIO_INICIAL
#define ESCENARIO_INICIAL ESCENARIO_ALDEA   /* make con -DESCENARIO_INICIAL=3: directo al dojo. */
#endif

static Cuerpo cuerpo;
static ProtaSprite prota;             /* Cómo se dibuja la protagonista elegida.           */
static Mando mando;
static Oleadas oleadas;
static Ninja ninjas[NINJAS_MAX];
static u16 camara;
static u8 hud_visible;
static u8 parada;                    /* Cuadros de parada de impacto que quedan.          */
static u8 a_guardada;                /* 1 si se pulsó A durante la parada.                */
static EstadoFase partida;           /* Jugando, pausa, KO, GAME OVER o STAGE CLEAR.      */
static u8 atenuada;                  /* 1 si la pantalla está a media luz.                */
static u8 cambiar_luz;               /* 1 = en el próximo VBlank, poner o quitar la media luz. */
static u8 avisos;                    /* 1 si las filas de aviso tienen texto (hay que borrarlas). */
static u16 oscuras[64];              /* Las paletas de sprites 0..3 a la mitad de brillo. */
static u8 fila_hud;
static u16 v_fin;                    /* Línea del VBlank en que acabó este (0 = su comienzo). */
static u16 v_max;                    /* La peor de los últimos 64 frames (37 = el límite).  */
static u8 v_reloj;                   /* Cuenta 64 frames para volver a medir el máximo.    */
static u8 reloj_noren;               /* Frames para el noren (cada 8, el otro cuadro).    */
static Ambiente ambiente;            /* El humo y los pájaros.                            */
static Objetos objetos;              /* Rompibles, ítems, proyectiles y chispa.           */
static u8 escenario;                 /* ESCENARIO_* que se juega (copia de fase1_escenario). */
static Dialogo dialogo;              /* La escena de texto en curso (Rex, el jefe).       */
static Apoyo apoyo;                  /* La barra y el apoyo invocado.                     */
static Cuerpo cuerpo_apoyo;          /* Dónde y cómo se dibuja el apoyo.                  */
static u8 cambiar_paleta;            /* 1 = en el VBlank, la paleta 0 del que se dibuja.  */
static u8 efecto, efecto_reloj;      /* Mosaico o temblor de un apoyo, y sus frames.       */
static u8 escena_fin;                /* 0 = falta el jefe, 1 = su escena, 2 = acabada.    */
static u8 turno_prota;               /* Reparto del presupuesto de subida: 1 = ella primero. */

u8 fase1_escenario;                  /* Ver sistema/pantallas.h.                          */
u8 fase1_vida;
u8 fase1_arma;

/* Los rompibles de cada escenario (x, y de los pies y lo que llevan) y cuál es. */
static const u16 ROMPIBLE_X[ESCENARIOS][4] = {
    {430, 900, 1290, 1720}, {300, 560, 820, 900}, {200, 520, 700, 880}, {180, 420, 600, 900}};
static const u16 ROMPIBLE_Y[ESCENARIOS][4] = {
    {194, 210, 190, 202}, {200, 190, 212, 196}, {196, 210, 192, 204}, {204, 192, 210, 198}};
static const u8 ROMPIBLE_ITEM[ESCENARIOS][4] = {
    {ITEM_ONIGIRI, ITEM_SHURIKEN, ITEM_PESCADO, ITEM_KUNAI},
    {ITEM_PESCADO, ITEM_SHURIKEN, ITEM_ONIGIRI, ITEM_KUNAI},
    {ITEM_ONIGIRI, ITEM_VIDA, ITEM_KUNAI, ITEM_PESCADO},
    {ITEM_SHURIKEN, ITEM_ONIGIRI, ITEM_KUNAI, ITEM_VIDA}};
static const u8 ROMPIBLE_TIPO[ESCENARIOS] = {ROMPIBLE_BARRIL, ROMPIBLE_JARRA, ROMPIBLE_JARRA, ROMPIBLE_CAJA};

/* Las escenas de texto (2 líneas por página; el texto en pantalla, en inglés). */
static const char *const LINEAS_REX[8] = {
    "  REX: YOU CAME THROUGH THE", "  BAMBOO FOREST. GOOD.",
    "  REX: NIKORAEMON'S MEN WAIT", "  IN THE DOJO BEYOND.",
    "  REX: LAND YOUR BLOWS, THEN", "  CALL US WITH X.",
    "  R: CHANGE ALLY", "  LUCINDA - REX - FENRIR"};
static const char *const LINEAS_JEFE[6] = {
    "  GOMASURI-DONO: ENOUGH...", "  YOU FIGHT WELL.",
    "  GOMASURI-DONO: NIKORAEMON", "  IS ALREADY GONE. HA!",
    "  NIKORAEMON ESCAPES...", "  THE HUNT GOES ON."};

void pantalla_fase1_desde_inicio(void)
{
    fase1_escenario = ESCENARIO_INICIAL;             /* La aldea (o la de -DESCENARIO_INICIAL). */
    fase1_vida = 0;                                  /* 0 = la vida entera.                 */
    fase1_arma = ARMA_NINGUNA;
}

/*--- siguiente_escenario: guarda vida y arma y carga el escenario de después --------*/
static void siguiente_escenario(void)
{
    fase1_escenario++;
    fase1_vida = cuerpo.vida;
    fase1_arma = cuerpo.arma;
}

/*--- cargar_lejana: la capa lejana y el cielo del escenario --------------------------*/
static void cargar_lejana(void)
{
    if (escenario == ESCENARIO_BOSQUE)
    {
        fondo16_cargar(1, &bosque_lejos_til, &bosque_lejos_til_end, &bosque_lejos_pal, 2,
                       &bosque_lejos_map, &bosque_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    }
    else if (escenario == ESCENARIO_MONASTERIO)
    {
        fondo16_cargar(1, &monasterio_lejos_til, &monasterio_lejos_til_end, &monasterio_lejos_pal, 2,
                       &monasterio_lejos_map, &monasterio_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    }
    else if (escenario == ESCENARIO_DOJO)
    {
        fondo16_cargar(1, &dojo_lejos_til, &dojo_lejos_til_end, &dojo_lejos_pal, 2,
                       &dojo_lejos_map, &dojo_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    }
    else
    {
        fondo16_cargar(1, &aldea_lejos_til, &aldea_lejos_til_end, &aldea_lejos_pal, 2,
                       &aldea_lejos_map, &aldea_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    }
}

/*--- poner_cielo: el HDMA del cielo del escenario (canal 6) --------------------------*/
static void poner_cielo(void)
{
    if (escenario == ESCENARIO_BOSQUE)
    {
        setModeHdmaColor(HDMA_CHANNEL6, (u8 *)hdmaGradcielobosqueList);
    }
    else if (escenario == ESCENARIO_MONASTERIO)
    {
        setModeHdmaColor(HDMA_CHANNEL6, (u8 *)hdmaGradcielomonasterioList);
    }
    else if (escenario == ESCENARIO_DOJO)
    {
        setModeHdmaColor(HDMA_CHANNEL6, (u8 *)hdmaGradcielodojoList);
    }
    else
    {
        setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradatardecerList);   /* El atardecer. */
    }
}

/*--- impacto_apoyo: el golpe del apoyo a todos los enemigos que se ven ---------------*/
static void impacto_apoyo(void)
{
    u8 k;
    Ninja *n = ninjas;

    for (k = 0; k < NINJAS_MAX; k++)
    {
        if (n->vivo && (u16)(n->x - camara) < ANCHO_PANTALLA)
        {
            ninja_recibir(n, apoyo_danio(&apoyo), apoyo_derriba(&apoyo), (u8)(n->x < apoyo.x),
                          (u8)(APOYO_GOLPE_ID + apoyo.golpe));
        }
        n++;
    }
    if (apoyo.elegido == APOYO_REX)
    {
        efecto = EFECTO_MOSAICO;                     /* La onda: la pantalla se pixela.     */
        efecto_reloj = EFECTO_FRAMES;
    }
    else if (apoyo.elegido == APOYO_FENRIR)
    {
        efecto = EFECTO_TEMBLOR;                     /* La embestida: la pantalla tiembla.  */
        efecto_reloj = EFECTO_FRAMES;
    }
    objetos_chispa(&objetos, apoyo.mira_izq ? (u16)(apoyo.x - 32) : (u16)(apoyo.x + 32), apoyo.y);
}

/*--- mover_apoyo: un frame del apoyo en pantalla (el resto del mundo, quieto) --------*/
static void mover_apoyo(void)
{
    apoyo_actualizar(&apoyo);
    cuerpo_apoyo.x = apoyo.x;
    cuerpo_apoyo.estado = apoyo_atacando(&apoyo) ? CUERPO_ESPECIAL : CUERPO_CORRIENDO;
    cuerpo_apoyo.inv = (u8)(apoyo.estado == APOYO_SALE ? apoyo.reloj | 1 : 0);   /* Parpadea al irse. */
    if (apoyo.evento == APOYO_EVENTO_IMPACTO)
    {
        impacto_apoyo();
    }
    if (!apoyo_activo(&apoyo))                       /* Se fue: vuelve la protagonista.     */
    {
        protagonista_sprite_cambiar(&prota, personaje_elegido);
        cambiar_paleta = 1;
    }
}

/*--- invocar: X con la barra llena; el apoyo ocupa el sitio de la protagonista -------*/
static void invocar(void)
{
    if (!apoyo_invocar(&apoyo, cuerpo.x, cuerpo.y, cuerpo.mira_izq))
    {
        return;
    }
    mundo_iniciar(&cuerpo_apoyo, apoyo.x, cuerpo.y);
    cuerpo_apoyo.mira_izq = cuerpo.mira_izq;
    cuerpo_apoyo.estado = CUERPO_CORRIENDO;
    protagonista_sprite_cambiar(&prota, (u8)(PERSONAJE_LUCINDA + apoyo.elegido));
    cambiar_paleta = 1;
}

/*--- salir: apaga lo que solo usa esta pantalla antes de cambiar -----------------*/
static void salir(Pantalla siguiente)
{
    fade_out(FRAMES_FADE);
    setModeHdmaReset(0);                             /* Corta el HDMA del cielo y el de las ventanas. */
    REG_WOBJSEL = 0;                                 /* Sin ventanas de color.              */
    protagonista_sprite_ocultar();                   /* OAM escondida y sin matemática de color. */
    secuencia_cambiar(siguiente);
}

/*--- leer_linea: línea del haz ahora mismo, contada desde el inicio del VBlank -------
    (225 = 0). Lo que se escriba en la VRAM pasada la 37 se pierde sin avisar. */
static u16 leer_linea(void)
{
    u16 linea;
    u8 bajo, alto;

    bajo = REG_SLHV;                                 /* Congela los contadores.             */
    bajo = REG_OPVCT;                                /* Bits 0..7.                          */
    alto = (u8)(REG_OPVCT & 1);                      /* Bit 8.                              */
    linea = (u16)(bajo | ((u16)alto << 8));
    return (u16)(linea >= 225 ? linea - 225 : linea + 37);
}

/*--- hud: una fila por frame (X Y / Z y estado / cámara y zona / V) ---------------*/
static void hud(void)
{
    u16 *p;

    if (!hud_visible)
    {
        return;
    }
    if (fila_hud == 4)                               /* V 12  MAX 20: líneas del VBlank.    */
    {
        p = hud_debug_empezar(5);
        p = hud_debug_texto(p, "V ");
        p = hud_debug_numero(p, (s16)v_fin);
        p = hud_debug_texto(p, "  MAX ");
        hud_debug_numero(p, (s16)v_max);
        hud_debug_enviar(5);
        fila_hud = 0;
        return;
    }
    p = hud_debug_empezar(fila_hud);
    if (fila_hud == 0)                               /* X 120  Y 200                        */
    {
        p = hud_debug_texto(p, "X ");
        p = hud_debug_numero(p, (s16)cuerpo.x);
        p = hud_debug_texto(p, "  Y ");
        hud_debug_numero(p, (s16)cuerpo.y);
    }
    else if (fila_hud == 1)                          /* Z 12  VZ -40  E 2                   */
    {
        p = hud_debug_texto(p, "Z ");
        p = hud_debug_numero(p, (s16)mundo_altura(&cuerpo));
        p = hud_debug_texto(p, "  VZ ");
        p = hud_debug_numero(p, cuerpo.vz);
        p = hud_debug_texto(p, "  E ");
        hud_debug_numero(p, (s16)cuerpo.estado);
    }
    else if (fila_hud == 2)                          /* CAM 256  ZONA 0  BLOQ               */
    {
        p = hud_debug_texto(p, "CAM ");
        p = hud_debug_numero(p, (s16)camara);
        p = hud_debug_texto(p, "  ZONA ");
        p = hud_debug_numero(p, (s16)oleadas.actual);
        if (oleadas.bloqueada)
        {
            hud_debug_texto(p, "  BLOQ");            /* Oleada viva: hay que vencerla.      */
        }
        else
        {
            hud_debug_texto(p, "  GO");              /* Libre: a avanzar.                   */
        }
    }
    else                                             /* NINJAS 2                            */
    {
        p = hud_debug_texto(p, "NINJAS ");
        p = hud_debug_numero(p, (s16)ninja_vivos(ninjas));
        p = hud_debug_texto(p, "  GOLPE ");
        p = hud_debug_numero(p, (s16)cuerpo.golpe);
        p = hud_debug_texto(p, "  VIDA ");
        hud_debug_numero(p, (s16)cuerpo.vida);
    }
    hud_debug_enviar(fila_hud);
    fila_hud++;
    if (fila_hud >= FILAS_HUD)
    {
        fila_hud = 0;
    }
}

/*--- oscurecer: copia 16 colores BGR555 a la mitad de brillo -------------------------
    Bajar un bit cada canal es v >> 1 con la máscara 0x3DEF (no se cuela un bit de un
    canal al de al lado). */
static void oscurecer(u16 *destino, const u16 *origen)
{
    u8 i;

    for (i = 0; i < 16; i++)
    {
        destino[i] = (u16)((origen[i] >> 1) & 0x3DEF);
    }
}

/*--- aviso: las filas 6 y 7 del HUD con el texto del estado (cada frame) ---------------*/
static void aviso(void)
{
    u16 *p;
    u8 estado = partida.estado;

    if (dialogo.activo)                              /* Una escena de texto: sus dos líneas. */
    {
        avisos = 1;
        p = hud_debug_empezar(FILA_AVISO);
        hud_debug_texto(p, dialogo_linea(&dialogo, 0));
        hud_debug_enviar(FILA_AVISO);
        p = hud_debug_empezar(FILA_AVISO + 1);
        hud_debug_texto(p, dialogo_linea(&dialogo, 1));
        hud_debug_enviar(FILA_AVISO + 1);
        return;
    }
    if (estado == FASE_JUGANDO || estado == FASE_KO)
    {
        if (avisos)                                  /* Se acaba de salir de la pausa.      */
        {
            hud_debug_empezar(FILA_AVISO);           /* Las dos filas, en blanco.           */
            hud_debug_enviar(FILA_AVISO);
            hud_debug_empezar(FILA_AVISO + 1);
            hud_debug_enviar(FILA_AVISO + 1);
            avisos = 0;
        }
        return;
    }
    avisos = 1;
    p = hud_debug_empezar(FILA_AVISO);               /* Se mandan cada frame: si una copia  */
    if (estado == FASE_PAUSA)                        /* se pierde, la siguiente la repone.  */
    {
        hud_debug_texto(p, "             PAUSE");
    }
    else if (estado == FASE_GAME_OVER)
    {
        hud_debug_texto(p, "           GAME OVER");
    }
    else
    {
        hud_debug_texto(p, "          STAGE CLEAR");
    }
    hud_debug_enviar(FILA_AVISO);
    p = hud_debug_empezar(FILA_AVISO + 1);
    if (estado == FASE_GAME_OVER)
    {
        p = hud_debug_texto(p, "          CONTINUE? ");
        hud_debug_numero(p, (s16)partida.cuenta);
    }
    hud_debug_enviar(FILA_AVISO + 1);
}

void pantalla_fase1_iniciar(void)
{
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);      /* Como el título: BG3 por delante.    */
    camara = 0;                                      /* Empieza en el borde izquierdo.      */
    escenario = fase1_escenario;                     /* Aldea, bosque, monasterio o dojo.   */
    fondo_nivel_cargar_escenario(escenario, camara); /* Capa cercana: las columnas a la vista. */
    cargar_lejana();
    reloj_noren = 0;
    bgSetScroll(0, camara, 0);
    bgSetScroll(1, camara >> 1, 0);                  /* La lejana, a la mitad de velocidad. */
    fuente_2bpp(2);                                  /* Texto del HUD en el fondo 2...      */
    hud_debug_iniciar();                             /* ...apagado hasta que lo pida SELECT.*/
    hud_juego_iniciar();                             /* Fila 0: la barra de vida (capa encendida). */
    hud_visible = 0;
    fila_hud = 0;
    v_fin = 0;
    v_max = 0;
    v_reloj = 0;
#ifdef GUION_FASE1
    hud_debug_alternar();                            /* ROM de prueba: el HUD, ya encendido. */
    hud_visible = 1;
#endif

    mando_iniciar();                                 /* (Solo hace algo en la ROM de prueba.) */
    protagonista_escena_cargar();                    /* OAM, sombra y matemática de color.  */
    protagonista_sprite_cargar(&prota, personaje_elegido, 0, 0, 96, 0, 1024);   /* La elegida en
                                                        el selector: entidades 0-1, tiles 0-191,
                                                        paleta 0, 1 KB por VBlank (con un ninja subiendo,
                                                        1,5 KB pasaba de la línea 37).         */
    sprites_usar(2);                                 /* El cuerpo y su sombra.              */
    mundo_iniciar(&cuerpo, X_INICIAL, Y_INICIAL);
    cuerpo.golpes_max = personaje_golpes(personaje_elegido);   /* Tarou 4, Bárbara 3.      */
    if (fase1_vida != 0)                             /* Viene del escenario anterior: con   */
    {                                                /* la vida y el arma que tenía.        */
        cuerpo.vida = fase1_vida;
    }
    cuerpo.arma = fase1_arma;
    oleadas_iniciar_escenario(&oleadas, escenario);
    dialogo_iniciar(&dialogo);
    apoyo_iniciar(&apoyo, (u8)(escenario >= ESCENARIO_MONASTERIO));   /* Rex se une allí.  */
    hud_apoyo_iniciar();
    cambiar_paleta = 0;
    efecto = 0;
    efecto_reloj = 0;
    escena_fin = 0;
    turno_prota = 0;
    if (escenario == ESCENARIO_MONASTERIO)
    {
        dialogo_empezar(&dialogo, LINEAS_REX, 4);    /* Rex, al llegar.                     */
    }
    ninja_iniciar_todos(ninjas);                     /* Sin ninjas hasta la primera oleada. */
    parada = 0;
    a_guardada = 0;
    estado_fase_iniciar(&partida);
    atenuada = 0;
    cambiar_luz = 0;
    avisos = 0;
    oscurecer(oscuras, (const u16 *)protagonista_paleta(personaje_elegido));   /* Para la media */
    oscurecer(oscuras + 16, (const u16 *)&ninja1_pal);                         /* luz (pausa y  */
    oscurecer(oscuras + 32, (const u16 *)&ninja2_pal);                         /* GAME OVER: las */
    oscurecer(oscuras + 48, (const u16 *)&ninja3_pal);                         /* de los ninjas, */
                                                     /* otra vez al pausar, por su tipo).   */
    ninja_sprite_cargar();                           /* Sus paletas y entidades (tras Tarou). */
    ambiente_sprite_cargar();                        /* Humo y pájaros: entradas 8..14.     */
    ambiente_iniciar(&ambiente);
    objetos_sprite_cargar(ROMPIBLE_TIPO[escenario]); /* Objetos: entradas 15..22.           */
    objetos_iniciar(&objetos, ROMPIBLE_X[escenario], ROMPIBLE_Y[escenario], ROMPIBLE_ITEM[escenario], 4);
    do                                               /* Primer cuadro antes del fade in, y  */
    {                                                /* sus tiles a la VRAM YA (la pantalla */
        dma_lista_vaciar();                          /* sigue apagada): media celda por     */
        protagonista_sprite_actualizar(&prota, &cuerpo, camara);   /* vuelta.               */
        dma_lista_ejecutar();
    } while (!protagonista_sprite_al_dia(&prota));
    ninja_sprite_actualizar(ninjas, camara);         /* Todas las ranuras apagadas.         */
    sprites_dibujar();                               /* La OAM, lista para el primer frame. */

    poner_cielo();                                   /* El cielo del escenario (HDMA).      */
    REG_HDMAEN = HDMA_CHANNEL6 | HDMA_CHANNEL5;      /* Y el de las ventanas de los objetos. */
    fade_in(FRAMES_FADE);
}

void pantalla_fase1_actualizar(void)
{
    u8 k, turnos, golpe_ninja;

    dma_lista_vaciar();                              /* La lista de copias de este frame.   */
    hud_juego_actualizar(cuerpo.vida);               /* La barra: la primera de la lista (64 B). */

    mando_leer(&mando);

    /*--- La partida: pausa (Start), KO -> GAME OVER con CONTINUE, STAGE CLEAR al vencer la
           última oleada (juego/estado_fase.c, lógica pura) -------------------------------*/
    k = (u8)(dialogo.activo || apoyo_activo(&apoyo));   /* Sin pausa en una escena o un apoyo. */
    k = estado_fase_actualizar(&partida, (u8)(k ? 0 : mando.salir_pulsado), (u8)(k ? 0 : mando.hud_pulsado),
                               (u8)(cuerpo.vida == 0 && cuerpo.stun == 0),
                               (u8)(escenario == ESCENARIO_DOJO && escena_fin == 2));
    if (k == ACCION_TITULO)
    {
        salir(PANTALLA_TITULO);
        return;
    }
    if (k == ACCION_REINICIAR)                       /* CONTINUE: el escenario desde su     */
    {                                                /* principio, con la vida entera.      */
        fase1_vida = 0;
        fase1_arma = ARMA_NINGUNA;
        salir(PANTALLA_FASE1);
        return;
    }
    if (k == ACCION_PROXIMAMENTE)
    {
        salir(PANTALLA_PROXIMAMENTE);
        return;
    }
    if (estado_fase_atenuada(&partida) != atenuada)  /* Media luz: se pone o se quita en el */
    {                                                /* VBlank (pantalla_fase1_vblank).    */
        atenuada ^= 1;
        cambiar_luz = 1;
        for (k = 0; k < NINJAS_MAX; k++)             /* Las de los ninjas, según su tipo.   */
        {
            oscurecer(oscuras + 16 + ((u16)k << 4), (const u16 *)ninja_sprite_paleta(ninjas, k));
        }
    }
    if (mando.hud_pulsado && partida.estado == FASE_JUGANDO)   /* SELECT: HUD de depuración. */
    {
        hud_debug_alternar();
        hud_juego_reparar();                         /* La capa y la barra, de nuevo.       */
        hud_visible ^= 1;
    }

    /*--- Parada de impacto: al conectar un golpe, el mundo se congela unos cuadros (se sigue
           dibujando). Un A pulsado durante la parada se guarda y vale en cuanto acaba. ---*/
    if (!estado_fase_mundo_avanza(&partida))
    {
        /* Pausa, GAME OVER o STAGE CLEAR: el mundo, congelado (se sigue dibujando). */
    }
    else if (dialogo.activo)                         /* Escena de texto: A pasa la página.  */
    {
        dialogo_actualizar(&dialogo, mando.atacar_pulsado);
        if (!dialogo.activo && escena_fin == 1)
        {
            escena_fin = 2;                          /* Tras el jefe: STAGE CLEAR.          */
        }
    }
    else if (apoyo_activo(&apoyo))                   /* El apoyo en pantalla: solo él se mueve. */
    {
        mover_apoyo();
    }
    else if (parada)
    {
        parada--;
        if (mando.atacar_pulsado)
        {
            a_guardada = 1;
        }
    }
    else
    {
        if (a_guardada)
        {
            mando.atacar_pulsado = 1;
            a_guardada = 0;
        }

        /*--- Lógica pura: el cuerpo, la cámara y las zonas, y los golpes ----------------*/
        mundo_actualizar(&cuerpo, &mando);
        mundo_limitar_x(&cuerpo, (u16)(camara + MARGEN_PANTALLA),      /* Sin salir de la pantalla  */
                        (u16)(camara + ANCHO_PANTALLA - MARGEN_PANTALLA)); /* ni volver atrás.      */
        camara = camara_avanzar(camara, cuerpo.x, oleadas_limite(&oleadas));
        if (oleadas_actualizar(&oleadas, camara))    /* ¿Llegó al punto de una zona? Bloquea */
        {                                            /* y salen los enemigos de esa oleada. */
            ninja_oleada_tipos(ninjas, camara, oleadas_tipos(&oleadas), oleadas_cuantos(&oleadas));
        }
        parada = combate_resolver(&cuerpo, ninjas, personaje_alcance(prota.personaje));  /* ¿Conectó el golpe de Tarou?          */
        if (parada)                                  /* Conectó: una chispa delante y la    */
        {                                            /* barra del apoyo sube.               */
            objetos_chispa(&objetos, (u16)(cuerpo.mira_izq ? cuerpo.x - 28 : cuerpo.x + 28), cuerpo.y);
            apoyo_cargar(&apoyo);
        }
#ifdef GUION_APOYO
        if (apoyo.disponible)                        /* ROM de prueba: la barra, llena.     */
        {
            apoyo.barra = APOYO_BARRA_MAX;
        }
#endif
        if (mando.espada_pulsado)                    /* R: el apoyo siguiente.              */
        {
            apoyo_cambiar(&apoyo);
        }
        if (mando.invocar_pulsado && cuerpo.vida != 0)   /* X: el apoyo, si la barra está llena. */
        {
            invocar();
        }
        combate_contexto(&cuerpo);                   /* Lo que la IA ve del protagonista.   */
        turnos = ninja_atacando(ninjas);             /* Cuántos están dando un tajo ahora.  */
        for (k = 0; k < NINJAS_MAX; k++)             /* IA: cada ninja vivo va hacia Tarou.  */
        {
            if (ninjas[k].vivo)
            {
                ninja_actualizar(ninjas + k, cuerpo.x, cuerpo.y);
                ninja_limitar_x(ninjas + k, (u16)(camara + MARGEN_NINJA),   /* Sin salirse de la    */
                                (u16)(camara + ANCHO_PANTALLA - MARGEN_NINJA)); /* pantalla.       */
                if (ninja_intentar_atacar(ninjas + k, cuerpo.x, cuerpo.y,   /* Si hay turno y está  */
                                          (u8)(turnos < NINJA_TURNOS)))     /* cerca, ataca.        */
                {
                    turnos++;
                }
            }
        }
        golpe_ninja = combate_ninjas_resolver(ninjas, &cuerpo);   /* ¿Le dio algún tajo a Tarou? */
        combate_agarre(&cuerpo, ninjas);             /* Agarre, golpes de agarre, lanzamiento. */
        combate_presa(&cuerpo, ninjas);              /* Si un enemigo lo tiene agarrado.    */
        for (k = 0; k < NINJAS_MAX; k++)             /* Lo que los enemigos hacen salir.    */
        {
            if (ninjas[k].evento == NINJA_EVENTO_SHURIKEN)
            {
                objetos_lanzar(&objetos, (u16)(ninjas[k].mira_izq ? ninjas[k].x - 16 : ninjas[k].x + 16),
                               ninjas[k].y, ninjas[k].mira_izq, 0, ITEM_SHURIKEN);
            }
            else if (ninjas[k].evento == NINJA_EVENTO_CONVOCAR)
            {
                turnos = NINJA_NORMAL;               /* El jefe llama a un ninja normal.    */
                ninja_oleada_tipos(ninjas, camara, &turnos, 1);
            }
        }
        if (cuerpo.evento == EVENTO_LANZA_ARMA)      /* Sale el arma que llevaba.           */
        {
            objetos_lanzar(&objetos, (u16)(cuerpo.mira_izq ? cuerpo.x - 20 : cuerpo.x + 20), cuerpo.y,
                           cuerpo.mira_izq, 1, (u8)(cuerpo.arma == ARMA_KUNAI ? ITEM_KUNAI : ITEM_SHURIKEN));
            cuerpo.arma = ARMA_NINGUNA;
        }
        objetos_actualizar(&objetos, &cuerpo, ninjas, personaje_alcance(prota.personaje), camara);
        if (golpe_ninja > parada)
        {
            parada = golpe_ninja;                    /* Parada de impacto (la mayor).       */
        }
        if (oleadas.bloqueada && ninja_vivos(ninjas) == 0)   /* Oleada sin ninjas: vencida.    */
        {
            oleadas_vencer(&oleadas);
            if (oleadas_terminadas(&oleadas) && escenario == ESCENARIO_DOJO)
            {
                escena_fin = 1;                      /* El jefe cayó: su escena, y Tarou    */
                cuerpo.estado = CUERPO_VICTORIA;     /* celebra (el mundo ya no se mueve).  */
                dialogo_empezar(&dialogo, LINEAS_JEFE, 3);
            }
        }
        if (oleadas_terminadas(&oleadas) && escenario != ESCENARIO_DOJO &&
            cuerpo.x >= (u16)(oleadas_ancho(escenario) - SALIDA_MARGEN))   /* Al final: el    */
        {                                                                  /* siguiente.     */
            siguiente_escenario();
            salir(PANTALLA_FASE1);
            return;
        }
    }

    /*--- Lo que va a la pantalla: Tarou primero en la lista de DMA, luego el fondo ---*/
    /* El presupuesto de subida de tiles (PRESUPUESTO_SUBIDA por VBlank) se reparte a turnos:
       un frame la protagonista primero (lo que le falte) y los ninjas el resto; el siguiente,
       al revés. Un ninja sube su cuadro entero (o a un hueco que no se enseña). */
    turno_prota ^= 1;
    if (turno_prota)
    {
        {
            u16 pend = protagonista_sprite_pendiente(&prota);
            prota.tope = (u16)(pend > PRESUPUESTO_SUBIDA ? PRESUPUESTO_SUBIDA : pend);
        }
        ninja_sprite_reservar(ninjas, (u16)(PRESUPUESTO_SUBIDA - prota.tope));
    }
    else
    {
        prota.tope = (u16)(PRESUPUESTO_SUBIDA - ninja_sprite_reservar(ninjas, PRESUPUESTO_SUBIDA));
    }
    protagonista_sprite_actualizar(&prota, apoyo_activo(&apoyo) ? &cuerpo_apoyo : &cuerpo, camara);
    if (estado_fase_mundo_avanza(&partida) && escenario == ESCENARIO_ALDEA)   /* Humo y pájaros. */
    {
        ambiente_actualizar(&ambiente, camara);
    }
    ambiente_sprite_actualizar(&ambiente);           /* Antes de los ninjas: ellos ordenan. */
    objetos_sprite_actualizar(&objetos, camara);     /* (Y el dibujo de un ítem nuevo.)     */
    ninja_sprite_actualizar(ninjas, camara);         /* Sus cuadros (tras Tarou en la lista). */
    fondo_nivel_actualizar(camara);                  /* Las columnas que entran.            */
    if (estado_fase_mundo_avanza(&partida) && escenario == ESCENARIO_ALDEA)   /* El noren ondea */
    {                                                /* (8 tiles, 256 bytes, cada 8 frames). */
        reloj_noren++;
        if ((reloj_noren & NOREN_CADA) == 0)
        {
            if (dma_lista_bytes() > NOREN_HUECO)     /* Frame de subida de sprites: el VBlank */
            {                                        /* va justo; se repite en el siguiente. */
                reloj_noren--;
            }
            else
            {
                fondo_nivel_noren((u8)((reloj_noren >> 3) & 1));
            }
        }
    }
    sprites_dibujar();                               /* UNA llamada: toda la OAM.           */
    aviso();                                         /* PAUSE / GAME OVER / STAGE CLEAR.    */
    if (!hud_visible)
    {
        hud_apoyo_actualizar(&apoyo);                /* La barra del apoyo (fila 5).        */
    }
    hud();
}

void pantalla_fase1_vblank(void)
{
    u16 temblor = 0;

    if (efecto_reloj)                                /* El efecto de un apoyo.              */
    {
        efecto_reloj--;
        if (efecto == EFECTO_MOSAICO)                /* De 15 a 0: la onda se deshace.      */
        {
            REG_MOSAICO = (u8)(((efecto_reloj >> 1) << 4) | 0x03);   /* BG1 y BG2.            */
        }
        else
        {
            temblor = (u16)((efecto_reloj & 2) ? 3 : 0);   /* Arriba y abajo cada 2 frames. */
        }
        if (efecto_reloj == 0)
        {
            REG_MOSAICO = 0;
        }
    }
    bgSetScroll(0, camara, temblor);                 /* El scroll de las dos capas, a la    */
    bgSetScroll(1, camara >> 1, temblor);            /* vez que sus columnas y la OAM.      */
    if (cambiar_paleta && !atenuada)                 /* Entra o sale un apoyo: su paleta.   */
    {
        cambiar_paleta = 0;
        dmaCopyCGram((u8 *)protagonista_paleta(prota.personaje), 128, 32);
    }
    if (cambiar_luz)                                 /* Media luz: fondos con la matemática */
    {                                                /* de color, sprites con su paleta.    */
        cambiar_luz = 0;
        if (atenuada)
        {
            REG_COLDATA = 0xE0;                      /* Color fijo negro.                   */
            REG_CGWSEL = 0x00;                       /* Operando: el color fijo.            */
            REG_CGADSUB = 0xF3;                      /* Restar y a la mitad: BG1, BG2, sprites 4..7, fondo. */
            dmaCopyCGram((u8 *)oscuras, 128, 128);   /* Paletas de sprites 0..3, oscuras.   */
        }
        else
        {
            REG_CGWSEL = OBJETOS_CGWSEL;             /* Como en objetos_sprite_cargar:      */
            REG_CGADSUB = 0x50;                      /* la sombra, a media suma.            */
            dmaCopyCGram((u8 *)protagonista_paleta(personaje_elegido), 128, 32);
            ninja_sprite_paletas_repuestas();        /* Las de los ninjas, por su tipo: las */
                                                     /* pone ninja_sprite_vblank, abajo.    */
        }
    }
    if (!atenuada)
    {
        ninja_sprite_vblank(ninjas);                 /* El ninja golpeado, en blanco.       */
    }
    objetos_sprite_vblank();                         /* Las ventanas de los objetos.        */
    dma_lista_ejecutar();                            /* Tiles, columnas y HUD: lo último (las
                                                        paletas, antes: una copia a la CGRAM
                                                        fuera del VBlank choca con el HDMA). */
    v_fin = leer_linea();                            /* Medida: dónde acabó el VBlank.      */
    if (v_fin > v_max)
    {
        v_max = v_fin;
    }
    v_reloj++;
    if ((v_reloj & 63) == 0)
    {
        v_max = v_fin;                               /* Cada 64 frames, se vuelve a medir.  */
    }
}
