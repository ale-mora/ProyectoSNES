/*---------------------------------------------------------------------------------
    pantalla_fase1.c — La fase 1 del beat 'em up: ESQUELETO jugable (Revisión 3).

    PROPÓSITO
      Primer paso de docs/diseno-beat-em-up.md: Tarou anda por el carril de la pantalla
      (cruceta: izquierda/derecha en x, arriba/abajo en la profundidad), salta con B y
      lleva su sombra. Se dibuja en el orden por profundidad de la capa de sprites. Aún
      no hay cámara que avance, ni enemigos, ni oleadas, ni golpes, ni Bárbara: cada uno
      se cuelga de este bucle (mundo → sprite) en su módulo (§10). El fondo es provisional:
      la capa cercana y la lejana del nivel 1 del plataformas, quietas.
      Por frame: mando_leer → mundo_actualizar → protagonista_sprite_actualizar →
      sprites_dibujar; la lista de DMA se ejecuta en el VBlank (pantalla_fase1_vblank).
      START vuelve al título; SELECT enciende el HUD de depuración (X, Y, Z y estado).

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h, juego/mando.h,
                juego/mundo.h, juego/protagonista_sprite.h, juego/sprites.h, juego/fondo_nivel.h,
                juego/dma_lista.h, juego/hud_debug.h, juego/animaciones.h, nivel1_datos.h.
      Lo usan:  sistema/secuencia.c (iniciar, actualizar, vblank); el título la abre con Start.
      Lee de la ROM: los fondos del nivel 1 (data.asm).

    HARDWARE QUE TOCA
      PPU: Mode 1, fondos 1 y 2 quietos (scroll 0), BG3 del HUD; HDMA del cielo (canal 6).
      El resto lo hace protagonista_sprite.c (OAM, paletas, matemática de color).

    VARIABLES (estáticas de este archivo)
      cuerpo      Cuerpo   el protagonista en el carril
      mando       Mando    lo que pide el jugador este frame
      hud_visible u8       1 si SELECT encendió el HUD
      fila_hud    u8       qué fila del HUD toca preparar este frame (0..1)
    → glosario: Carril, Profundidad, Máquina de estados
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "juego/mando.h"
#include "juego/mundo.h"
#include "juego/protagonista_sprite.h"
#include "juego/sprites.h"
#include "juego/fondo_nivel.h"
#include "juego/dma_lista.h"
#include "juego/hud_debug.h"
#include "juego/animaciones.h"
#include "nivel1_datos.h"            /* Solo el parche del tronco del fondo provisional.  */

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];   /* Tabla HDMA del cielo (gfx4snes -n). */

#define X_INICIAL 64                 /* Donde nace Tarou: a la izquierda.                 */
#define Y_INICIAL 200                /* En medio del carril (184..216).                   */
#define FILAS_HUD 2

static Cuerpo cuerpo;
static Mando mando;
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

/*--- hud: una fila por frame (X Y / Z y estado) ----------------------------------*/
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
    else                                             /* Z 12  VZ -40  E 2                   */
    {
        p = hud_debug_texto(p, "Z ");
        p = hud_debug_numero(p, (s16)mundo_altura(&cuerpo));
        p = hud_debug_texto(p, "  VZ ");
        p = hud_debug_numero(p, cuerpo.vz);
        p = hud_debug_texto(p, "  E ");
        hud_debug_numero(p, (s16)cuerpo.estado);
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
    fondo_nivel_parche(NIVEL1_TRONCO_COL, NIVEL1_PARCHE_COL);
    fondo_nivel_cargar(0);                           /* La capa cercana del nivel 1, quieta. */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    bgSetScroll(0, 0, 0);
    bgSetScroll(1, 0, 0);
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
    do                                               /* Primer cuadro antes del fade in, y  */
    {                                                /* sus tiles a la VRAM YA (la pantalla */
        dma_lista_vaciar();                          /* sigue apagada): media celda por     */
        protagonista_sprite_actualizar(&cuerpo);     /* vuelta.                             */
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

    mundo_actualizar(&cuerpo, &mando);               /* Lógica pura: x, y, z.               */
    protagonista_sprite_actualizar(&cuerpo);         /* Animación, subida y entidades.      */
    sprites_dibujar();                               /* UNA llamada: toda la OAM.           */
    hud();
}

void pantalla_fase1_vblank(void)
{
    dma_lista_ejecutar();                            /* Tiles de Tarou y fila del HUD.      */
}
