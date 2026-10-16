/*---------------------------------------------------------------------------------
    pantalla_titulo.c — Pantalla (6): título del juego.

    Capas (Mode 1 con BG3 en prioridad alta):
      fondo 0 = capa CERCANA: arbustos, 512 px de ancho en bucle, scroll por
                software cada frame (parallax).
      fondo 1 = capa LEJANA: montañas y pinos, quieta, con el logo horneado.
      fondo 2 = TEXTO ("PRESS START", ©) con la fuente de 4 colores (BG3 solo
                puede mostrar 2 bits por píxel).
      cielo   = el color de fondo, que un canal HDMA cambia en cada línea con la
                tabla generada de res/titulo/cielo.bmp (degradado).
      sprites = Tarouh y Bárbara en reposo, delante de todo.
    Espera Start; a los 15 s sin input vuelve a la intro (attract).
    → glosario: BG / capas, HDMA, Mando, Sprite
---------------------------------------------------------------------------------*/
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "pantallas/personajes.h"

extern char cerca_til, cerca_til_end, cerca_pal, cerca_map, cerca_map_end;
extern char lejos_til, lejos_til_end, lejos_pal, lejos_map, lejos_map_end;
extern const unsigned char hdmaGradcieloList[];   /* Tabla HDMA generada por gfx4snes -n. */

/*--- Estado propio de esta pantalla -------------------------------------------*/
static u16 frames_sin_input;   /* Frames seguidos sin tocar ningún botón (para el attract). */
static u8  frames_parpadeo;    /* Cuenta frames hasta el próximo cambio de PRESS START.    */
static u8  texto_visible;      /* 1 = PRESS START se ve, 0 = está oculto.                  */
static u16 scroll_cerca;       /* Desplazamiento horizontal de la capa cercana.            */

/*--- salir: apaga lo que solo usa esta pantalla antes de cambiar -----------------*/
static void salir(Pantalla siguiente)
{
    fade_out(FRAMES_FADE);
    setModeHdmaReset(0);                             /* Corta el HDMA del cielo.            */
    personajes_ocultar();
    secuencia_cambiar(siguiente);
}

void pantalla_titulo_iniciar(void)
{
    /* BG3_MODE1_PRIORITY_HIGH: el fondo 2 (texto) se dibuja delante de los otros. */
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);
    personajes_cargar();                             /* Mode 3 / Mode 7 pisaron la VRAM.    */
    fondo16_cargar(0, &cerca_til, &cerca_til_end, &cerca_pal, 1,
                   &cerca_map, &cerca_map_end, VRAM_FONDO_A, VRAM_MAPA_A, SC_64x32);
    fondo16_cargar(1, &lejos_til, &lejos_til_end, &lejos_pal, 2,
                   &lejos_map, &lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_32x32);
    fuente_2bpp(2);                                  /* Texto en el fondo 2 (BG3).          */
    bgSetEnable(2);

    limpiar_texto();
    texto_centrado(26, "(C) 2026 DON POLLO STUDIOS");
    texto_visible = 1;
    texto_centrado(22, "PRESS START");

    personajes_mostrar(40, 96, 160, 96);             /* Pies en el píxel 160, sobre arbustos.*/

    scroll_cerca = 0;
    bgSetScroll(0, 0, 0);
    bgSetScroll(1, 0, 0);
    frames_sin_input = 0;
    frames_parpadeo = 0;

    /* Cielo: el canal 6 de HDMA escribe un color de fondo distinto en cada línea. */
    setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradcieloList);

    fade_in(FRAMES_FADE);
}

void pantalla_titulo_actualizar(void)
{
    /*--- 1. Start: pasamos a "coming soon" --------------------------------------*/
    if (padsDown(0) & KEY_START)
    {
        salir(PANTALLA_PROXIMAMENTE);
        return;
    }

    /*--- 2. Attract: 15 s sin ningún botón → volvemos a la intro ---------------*/
    if (padsCurrent(0) != 0)
    {
        frames_sin_input = 0;
    }
    else
    {
        frames_sin_input++;
    }
    if (frames_sin_input >= ATTRACT_SIN_INPUT)
    {
        salir(PANTALLA_INTRO);
        return;
    }

    /*--- 3. Parallax: la capa cercana avanza 1 píxel por frame; el mapa de 512 px
           da la vuelta solo (la imagen es original + espejo, sin costura). ------*/
    scroll_cerca++;
    bgSetScroll(0, scroll_cerca, 0);

    /*--- 4. Sprites en reposo ------------------------------------------------*/
    personajes_animar();

    /*--- 5. Parpadeo de PRESS START ---------------------------------------------*/
    frames_parpadeo++;
    if (frames_parpadeo >= FRAMES_PARPADEO)
    {
        frames_parpadeo = 0;
        texto_visible = !texto_visible;
        if (texto_visible)
        {
            texto_centrado(22, "PRESS START");
        }
        else
        {
            texto_centrado(22, "           ");
        }
    }
}
