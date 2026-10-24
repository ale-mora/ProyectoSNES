/*---------------------------------------------------------------------------------
    pantalla_fase1.c — La fase 1 del beat 'em up: ESQUELETO jugable (Revisión 3).

    PROPÓSITO
      Docs/diseno-beat-em-up.md paso a paso: Tarou anda por el carril (cruceta: x y
      profundidad), salta con B y lleva su sombra, y ahora hay CÁMARA y ZONAS: la cámara
      sigue a Tarou solo hacia delante, se bloquea en el punto de cada zona (juego/oleadas.c)
      y no se libera hasta vencer la oleada. Aún no hay enemigos: A hace de «oleada vencida»
      mientras la cámara está bloqueada. Tampoco golpes ni Bárbara; cada uno se cuelga de
      este bucle en su módulo (§10). El fondo es provisional: la capa cercana y la lejana
      del nivel 1 del plataformas (2048 px, la lejana a la mitad de velocidad).
      Por frame: mando_leer → mundo_actualizar → límites de pantalla → cámara y zonas →
      protagonista_sprite_actualizar → fondo (columnas que entran) → sprites_dibujar. El scroll
      de los fondos se escribe en el VBlank (pantalla_fase1_vblank), con la lista de DMA y no
      antes: bgSetScroll escribe en el acto, y a mitad del dibujo partiría la imagen.
      START vuelve al título; SELECT enciende el HUD de depuración.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h, juego/mando.h,
                juego/mundo.h, juego/camara.h, juego/oleadas.h, juego/protagonista_sprite.h,
                juego/sprites.h, juego/fondo_nivel.h, juego/dma_lista.h, juego/hud_debug.h,
                nivel1_datos.h.
      Lo usan:  sistema/secuencia.c (iniciar, actualizar, vblank); el título la abre con Start.
      Lee de la ROM: los fondos del nivel 1 (data.asm).

    HARDWARE QUE TOCA
      PPU: Mode 1, scroll de los fondos 1 y 2 (en el VBlank), BG3 del HUD; HDMA del cielo
      (canal 6). El resto lo hace protagonista_sprite.c (OAM, paletas, matemática de color)
      y fondo_nivel.c (las columnas del mapa, por la lista de DMA).

    VARIABLES (estáticas de este archivo)
      cuerpo      Cuerpo    el protagonista en el carril
      mando       Mando     lo que pide el jugador este frame
      oleadas     Oleadas   la zona en curso y si la cámara está bloqueada
      camara      u16       scroll horizontal; se calcula en el frame y se escribe en el VBlank
      hud_visible u8        1 si SELECT encendió el HUD
      fila_hud    u8        qué fila del HUD toca preparar este frame (0..2)
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
#include "juego/protagonista_sprite.h"
#include "juego/sprites.h"
#include "juego/fondo_nivel.h"
#include "juego/dma_lista.h"
#include "juego/hud_debug.h"
#include "nivel1_datos.h"            /* Solo el parche del tronco del fondo provisional.  */

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];   /* Tabla HDMA del cielo (gfx4snes -n). */

#define X_INICIAL 64                 /* Donde nace Tarou: a la izquierda.                 */
#define Y_INICIAL 200                /* En medio del carril (184..216).                   */
#define FILAS_HUD 3

static Cuerpo cuerpo;
static Mando mando;
static Oleadas oleadas;
static u16 camara;
static u8 hud_visible;
static u8 fila_hud;

/*--- salir: apaga lo que solo usa esta pantalla antes de cambiar -----------------*/
static void salir(Pantalla siguiente)
{
    fade_out(FRAMES_FADE);
    setModeHdmaReset(0);                             /* Corta el HDMA del cielo.            */
    protagonista_sprite_ocultar();                   /* OAM escondida y sin matemática de color. */
    secuencia_cambiar(siguiente);
}

/*--- hud: una fila por frame (X Y / Z y estado / cámara y zona) ------------------*/
static void hud(void)
{
    u16 *p;

    if (!hud_visible)
    {
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
    else                                             /* CAM 256  ZONA 0  BLOQ               */
    {
        p = hud_debug_texto(p, "CAM ");
        p = hud_debug_numero(p, (s16)camara);
        p = hud_debug_texto(p, "  ZONA ");
        p = hud_debug_numero(p, (s16)oleadas.actual);
        if (oleadas.bloqueada)
        {
            hud_debug_texto(p, "  BLOQ");            /* Oleada viva: A la vence (provisional). */
        }
        else
        {
            hud_debug_texto(p, "  GO");              /* Libre: a avanzar.                   */
        }
    }
    hud_debug_enviar(fila_hud);
    fila_hud++;
    if (fila_hud >= FILAS_HUD)
    {
        fila_hud = 0;
    }
}

void pantalla_fase1_iniciar(void)
{
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);      /* Como el título: BG3 por delante.    */
    camara = 0;                                      /* Empieza en el borde izquierdo.      */
    fondo_nivel_parche(NIVEL1_TRONCO_COL, NIVEL1_PARCHE_COL);
    fondo_nivel_cargar(camara);                      /* Capa cercana: las columnas a la vista. */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    bgSetScroll(0, camara, 0);
    bgSetScroll(1, camara >> 1, 0);                  /* La lejana, a la mitad de velocidad. */
    fuente_2bpp(2);                                  /* Texto del HUD en el fondo 2...      */
    hud_debug_iniciar();                             /* ...apagado hasta que lo pida SELECT.*/
    hud_visible = 0;
    fila_hud = 0;
#ifdef GUION_FASE1
    hud_debug_alternar();                            /* ROM de prueba: el HUD, ya encendido. */
    hud_visible = 1;
#endif

    mando_iniciar();                                 /* (Solo hace algo en la ROM de prueba.) */
    protagonista_sprite_cargar();                    /* OAM, paletas, sombra.               */
    mundo_iniciar(&cuerpo, X_INICIAL, Y_INICIAL);
    oleadas_iniciar(&oleadas);
    do                                               /* Primer cuadro antes del fade in, y  */
    {                                                /* sus tiles a la VRAM YA (la pantalla */
        dma_lista_vaciar();                          /* sigue apagada): media celda por     */
        protagonista_sprite_actualizar(&cuerpo, camara);   /* vuelta.                       */
        dma_lista_ejecutar();
    } while (!protagonista_sprite_al_dia());
    sprites_dibujar();                               /* La OAM, lista para el primer frame. */

    setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradcieloList);   /* Cielo por línea.   */
    fade_in(FRAMES_FADE);
}

void pantalla_fase1_actualizar(void)
{
    dma_lista_vaciar();                              /* La lista de copias de este frame.   */

    mando_leer(&mando);
    if (mando.salir_pulsado)                         /* START: volver al título.            */
    {
        salir(PANTALLA_TITULO);
        return;
    }
    if (mando.hud_pulsado)                           /* SELECT: HUD de depuración sí/no.    */
    {
        hud_debug_alternar();
        hud_visible ^= 1;
    }

    /*--- Lógica pura: el cuerpo, y la cámara y las zonas que lo acompañan ------------*/
    mundo_actualizar(&cuerpo, &mando);
    mundo_limitar_x(&cuerpo, (u16)(camara + MARGEN_PANTALLA),      /* Sin salir de la pantalla  */
                    (u16)(camara + ANCHO_PANTALLA - MARGEN_PANTALLA)); /* ni volver atrás.      */
    camara = camara_avanzar(camara, cuerpo.x, oleadas_limite(&oleadas));
    oleadas_actualizar(&oleadas, camara);            /* ¿Llegó al punto de una zona? Bloquea. */
    if (mando.atacar_pulsado)                        /* Provisional: A = «oleada vencida».  */
    {
        oleadas_vencer(&oleadas);
    }

    /*--- Lo que va a la pantalla: Tarou primero en la lista de DMA, luego el fondo ---*/
    protagonista_sprite_actualizar(&cuerpo, camara); /* Animación, subida y entidades.      */
    fondo_nivel_actualizar(camara);                  /* Las columnas que entran.            */
    sprites_dibujar();                               /* UNA llamada: toda la OAM.           */
    hud();
}

void pantalla_fase1_vblank(void)
{
    bgSetScroll(0, camara, 0);                       /* El scroll de las dos capas, a la    */
    bgSetScroll(1, camara >> 1, 0);                  /* vez que sus columnas y la OAM.      */
    dma_lista_ejecutar();                            /* Tiles de Tarou, columnas y HUD.     */
}
