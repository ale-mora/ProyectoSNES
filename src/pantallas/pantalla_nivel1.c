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
      ESTADO (tarea 4.1): fondos, cielo y cámara; la cruceta mueve la cámara 2 px
      por frame para probar el scroll. Tarou, el mando y el HUD llegan en 4.2-4.4.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h,
                juego/nivel.h (ANCHO_NIVEL, ANCHO_PANTALLA), juego/camara.h.
      Lee de la ROM: cerca_* (título), nivel1_lejos_* y hdmaGradcieloList (data.asm).
      Lo usan:  sistema/secuencia.c (_iniciar, _actualizar y _vblank).

    HARDWARE QUE TOCA
      PPU: modo (setMode), fondos por comun.c (VRAM, CGRAM), scroll de los
      fondos 0 y 1 (bgSetScroll), BG2 apagado (bgSetDisable). HDMA: canal 6 con
      un color por línea (setModeHdmaColor / setModeHdmaReset). Mando:
      padsDown() (START) y padsCurrent() (cruceta, provisional).

    VARIABLES (globales y estáticas de este archivo)
      camara  u16  0..ANCHO_NIVEL-ANCHO_PANTALLA (0..256) px  desplazamiento del nivel

    → glosario: Nivel, Cámara, Parallax, BG / capas, HDMA, VBlank
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "juego/nivel.h"
#include "juego/camara.h"

extern char cerca_til, cerca_til_end, cerca_pal, cerca_map, cerca_map_end;
extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];   /* Tabla HDMA del cielo (gfx4snes -n). */

#define CAMARA_MAX (ANCHO_NIVEL - ANCHO_PANTALLA)   /* 256.                               */
#define PASO_CAMARA_PRUEBA 2                        /* px por frame con la cruceta (4.1). */

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u16 camara;                 /* Desplazamiento horizontal del nivel en pantalla.    */

/*--- salir: apaga lo que solo usa esta pantalla antes de cambiar -----------------*/
static void salir(Pantalla siguiente)
{
    fade_out(FRAMES_FADE);
    setModeHdmaReset(0);                             /* Corta el HDMA del cielo.            */
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

    camara = 0;
    bgSetScroll(0, 0, 0);
    bgSetScroll(1, 0, 0);

    setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradcieloList);   /* Cielo por línea.  */
    fade_in(FRAMES_FADE);
}

void pantalla_nivel1_actualizar(void)
{
    /*--- 1. START: volver al título ----------------------------------------------*/
    if (padsDown(0) & KEY_START)
    {
        salir(PANTALLA_TITULO);
        return;
    }

    /*--- 2. Cámara de prueba (solo hasta la tarea 4.3): la cruceta la desplaza -----*/
    if (padsCurrent(0) & KEY_RIGHT)
    {
        camara = (camara + PASO_CAMARA_PRUEBA > CAMARA_MAX) ? CAMARA_MAX : camara + PASO_CAMARA_PRUEBA;
    }
    if (padsCurrent(0) & KEY_LEFT)
    {
        camara = (camara < PASO_CAMARA_PRUEBA) ? 0 : camara - PASO_CAMARA_PRUEBA;
    }

    /*--- 3. Parallax: la cercana sigue a la cámara; la lejana, a la mitad ---------*/
    bgSetScroll(0, camara, 0);
    bgSetScroll(1, camara >> 1, 0);
}

void pantalla_nivel1_vblank(void)
{
    /* Vacía hasta la tarea 4.2 (oamVramQueueUpdate del sprite dinámico). */
}
