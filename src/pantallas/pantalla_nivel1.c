/*---------------------------------------------------------------------------------
    pantalla_nivel1.c — Pantalla (7): el nivel 1, donde se juega.

    PROPÓSITO
      Orquesta un frame del nivel: lee el mando, actualiza la lógica pura del
      protagonista, calcula la cámara, escribe el scroll de las dos capas y
      dibuja. Es la única pantalla con _vblank(), porque el sprite dinámico
      copia sus tiles a la VRAM justo después del VBlank.
      Capas (Mode 1 con BG3 en prioridad alta): fondo 0 = capa CERCANA (los
      arbustos del título, 512 px, scroll 1:1 con la cámara); fondo 1 = capa
      LEJANA (montañas sin logotipo, 512 px, scroll a la mitad); fondo 2 = HUD
      de depuración (texto de 4 colores, apagado al inicio); cielo = degradado
      HDMA del título. START vuelve al título con fundido.
      Flujo por frame (docs/diseno-demo-jugable.md §11): mando_leer →
      tarou_actualizar (lógica pura) → camara_para → bgSetScroll de las dos
      capas → tarou_sprite_dibujar → oamInitDynamicSpriteEndFrame; y, tras el
      VBlank, oamVramQueueUpdate. El HUD de depuración llega en la tarea 4.4.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h,
                juego/nivel.h, juego/camara.h, juego/mando.h, juego/tarou.h,
                juego/tarou_sprite.h.
      Lee de la ROM: cerca_* (título), nivel1_lejos_* y hdmaGradcieloList (data.asm).
      Lo usan:  sistema/secuencia.c (_iniciar, _actualizar y _vblank).

    HARDWARE QUE TOCA
      PPU: modo (setMode), fondos por comun.c (VRAM, CGRAM), scroll de los
      fondos 0 y 1 (bgSetScroll), BG2 apagado (bgSetDisable). HDMA: canal 6 con
      un color por línea (setModeHdmaColor / setModeHdmaReset). OAM y VRAM de
      sprites por tarou_sprite.c. Mando: por mando.c.

    VARIABLES (globales y estáticas de este archivo)
      camara  u16    0..ANCHO_NIVEL-ANCHO_PANTALLA (0..256) px  desplazamiento del nivel
      tarou   Tarou  ver juego/tarou.h                          el protagonista
      mando   Mando  ver juego/mando.h                          intenciones del jugador este frame

    → glosario: Nivel, Cámara, Parallax, BG / capas, HDMA, VBlank
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "juego/nivel.h"
#include "juego/camara.h"
#include "juego/tarou.h"
#include "juego/tarou_sprite.h"
#include "juego/mando.h"
#include "sistema/puntofijo.h"

extern char cerca_til, cerca_til_end, cerca_pal, cerca_map, cerca_map_end;
extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];   /* Tabla HDMA del cielo (gfx4snes -n). */

#define X_INICIAL 64               /* Tarou arranca a 64 px del borde izquierdo.          */

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u16 camara;                 /* Desplazamiento horizontal del nivel en pantalla.    */
static Tarou tarou;                /* El protagonista (lógica pura, juego/tarou.h).       */
static Mando mando;                /* Lo que el jugador quiere hacer este frame.          */

/*--- salir: apaga lo que solo usa esta pantalla antes de cambiar -----------------*/
static void salir(Pantalla siguiente)
{
    fade_out(FRAMES_FADE);
    setModeHdmaReset(0);                             /* Corta el HDMA del cielo.            */
    tarou_sprite_ocultar();
    secuencia_cambiar(siguiente);
}

void pantalla_nivel1_iniciar(void)
{
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);      /* Como el título: BG3 por delante.    */
    fondo16_cargar(0, &cerca_til, &cerca_til_end, &cerca_pal, 1,
                   &cerca_map, &cerca_map_end, VRAM_FONDO_A, VRAM_MAPA_A, SC_64x32);
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    fuente_2bpp(2);                                  /* Texto del HUD en el fondo 2...      */
    limpiar_texto();
    bgSetDisable(2);                                 /* ...apagado hasta que lo pida SELECT.*/

    tarou_sprite_cargar();                           /* Motor dinámico + paleta de Tarou.   */
    tarou_iniciar(&tarou, X_INICIAL, nivel_suelo_en(X_INICIAL + ANCHO_CELDA / 2));
    camara = camara_para(X_INICIAL);                 /* 0: cerca del borde izquierdo.       */
    bgSetScroll(0, camara, 0);
    bgSetScroll(1, camara >> 1, 0);
    tarou_sprite_dibujar(&tarou, camara);            /* Primer cuadro antes del fade in.    */
    oamInitDynamicSpriteEndFrame();

    setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradcieloList);   /* Cielo por línea.  */
    fade_in(FRAMES_FADE);
}

void pantalla_nivel1_actualizar(void)
{
    /*--- 1. Mando: de bits del pad a intenciones con nombre -------------------------*/
    mando_leer(&mando);
    if (mando.salir_pulsado)                         /* START: volver al título.            */
    {
        salir(PANTALLA_TITULO);
        return;
    }

    /*--- 2. Lógica pura: un frame de física y máquina de estados --------------------*/
    tarou_actualizar(&tarou, &mando, nivel_suelo_en(PF_ENTERO(tarou.x_fp) + ANCHO_CELDA / 2));

    /*--- 3. Cámara y parallax: la cercana sigue a la cámara; la lejana, a la mitad --*/
    camara = camara_para(PF_ENTERO(tarou.x_fp));
    bgSetScroll(0, camara, 0);
    bgSetScroll(1, camara >> 1, 0);

    /*--- 4. Tarou: entradas de la OAM del cuadro actual y cierre del frame del
           motor dinámico (prepara la cola de copias a VRAM para _vblank) ----------*/
    tarou_sprite_dibujar(&tarou, camara);
    oamInitDynamicSpriteEndFrame();
}

void pantalla_nivel1_vblank(void)
{
    oamVramQueueUpdate();                            /* Copia a VRAM los tiles pendientes.  */
}
