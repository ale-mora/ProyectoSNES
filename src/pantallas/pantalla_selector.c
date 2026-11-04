/*---------------------------------------------------------------------------------
    pantalla_selector.c — Pantalla (6b): «SELECT YOUR FIGHTER», Tarou o Bárbara.

    PROPÓSITO
      Entre el título y la fase 1 (decisión del 2026-11-04). Sobre la capa lejana del
      nivel 1 (las montañas del título sin el logotipo) y el cielo por HDMA, Tarou a la
      izquierda y Bárbara a la derecha, mirándose y en su guardia animada, cada uno con
      su nombre debajo. Izquierda / derecha marca uno (el otro se oscurece: su paleta a
      la mitad de brillo); Start o A confirma y entra en la fase 1 con él; B vuelve al
      título. Se juega solo con el elegido. Empieza marcado el último elegido
      (personaje_elegido, que main.c pone a Tarou al arrancar).
      Los dos se dibujan con la misma capa de sprites que la fase (protagonista_sprite.c):
      cada uno con sus dos búferes en la VRAM y la mitad del tope de subida por VBlank.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h, sistema/secuencia.h, sistema/comun.h, juego/mundo.h,
                juego/personaje.h, juego/protagonista_sprite.h, juego/sprites.h,
                juego/dma_lista.h.
      Lo usan:  sistema/secuencia.c (iniciar, actualizar, vblank); el título la abre con Start.
      Lee de la ROM: nivel1_lejos_*, hdmaGradcieloList y las tiras de los dos (data.asm).

    HARDWARE QUE TOCA
      PPU: Mode 1, el fondo 1 (capa lejana), BG3 (texto); el fondo 0 apagado. HDMA:
      canal 6 (cielo). CGRAM: paletas de sprites 0 (Tarou), 1 (Tarou oscuro), 2 (Bárbara)
      y 3 (Bárbara oscura). VRAM: Tarou en los tiles 0-191 y Bárbara en 192-383, por la
      lista de DMA en el VBlank. Mando: padsCurrent() (con su flanco propio).

    VARIABLES (estáticas de este archivo)
      tarou, barbara       ProtaSprite  cómo se dibuja cada uno
      cuerpo_t, cuerpo_b   Cuerpo       dónde están (quietos: solo la guardia)
      marcado              u8           PERSONAJE_TAROU / PERSONAJE_BARBARA
      oscura               u16[16]      una paleta a la mitad de brillo (para la CGRAM)
      antes                u16          botones pulsados el frame anterior: el flanco se
                                        calcula aquí con padsCurrent, porque padsDown solo
                                        dura un VBlank y un frame largo se lo puede saltar
    → glosario: Sprite, Paleta, Doble búfer
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "sistema/pantallas.h"
#include "sistema/secuencia.h"
#include "sistema/comun.h"
#include "juego/mundo.h"
#include "juego/personaje.h"
#include "juego/protagonista_sprite.h"
#include "juego/sprites.h"
#include "juego/dma_lista.h"

extern char nivel1_lejos_til, nivel1_lejos_til_end, nivel1_lejos_pal;
extern char nivel1_lejos_map, nivel1_lejos_map_end;
extern const unsigned char hdmaGradcieloList[];   /* Tabla HDMA del cielo (gfx4snes -n). */

#define X_TAROU 72                   /* Centro de cada uno en la pantalla.                */
#define X_BARBARA 184
#define Y_PIES 176                   /* Los pies, sobre la fila de los nombres.           */
#define FILA_TITULO 3
#define FILA_NOMBRES 23
#define COL_TAROU 5                  /* "> TAROU <" (9 letras) centrado en x = 72.        */
#define COL_BARBARA 17               /* "> BARBARA <" (11 letras) centrado en x = 184.    */
#define TOPE_CADA_UNO 768            /* Bytes por VBlank para cada uno (1,5 KB entre los dos). */

static ProtaSprite tarou, barbara;
static Cuerpo cuerpo_t, cuerpo_b;
static u8 marcado;
static u16 oscura[16];
static u16 antes;                    /* Botones del frame anterior (flancos propios).     */

/*--- paleta_oscura: copia una paleta a la mitad de brillo en la CGRAM -----------------
    Cada color es BGR555 (0bbbbbgggggrrrrr): bajar un bit cada canal es v >> 1 con la
    máscara 0x3DEF, que quita el bit que se cuela de un canal al de al lado. */
static void paleta_oscura(u8 personaje, u8 paleta)
{
    const u16 *origen = (const u16 *)protagonista_paleta(personaje);
    u8 i;

    for (i = 0; i < 16; i++)
    {
        oscura[i] = (u16)((origen[i] >> 1) & 0x3DEF);
    }
    dmaCopyCGram((u8 *)oscura, (u16)(128 + ((u16)paleta << 4)), 32);
}

/*--- nombres: los dos nombres, con flechas en el marcado ---------------------------*/
static void nombres(void)
{
    if (marcado == PERSONAJE_TAROU)
    {
        consoleDrawText(COL_TAROU, FILA_NOMBRES, "> TAROU <");
        consoleDrawText(COL_BARBARA, FILA_NOMBRES, "  BARBARA  ");
    }
    else
    {
        consoleDrawText(COL_TAROU, FILA_NOMBRES, "  TAROU  ");
        consoleDrawText(COL_BARBARA, FILA_NOMBRES, "> BARBARA <");
    }
    tarou.paleta = (u8)(marcado == PERSONAJE_TAROU ? 0 : 1);     /* El otro, oscuro.        */
    barbara.paleta = (u8)(marcado == PERSONAJE_BARBARA ? 2 : 3);
}

/*--- dibujar: un frame de los dos (animación, subida, entidades) ---------------------*/
static void dibujar(void)
{
    protagonista_sprite_actualizar(&tarou, &cuerpo_t, 0);
    protagonista_sprite_actualizar(&barbara, &cuerpo_b, 0);
    sprites_ordenar();
}

/*--- salir: apaga lo que solo usa esta pantalla antes de cambiar -----------------*/
static void salir(Pantalla siguiente)
{
    fade_out(FRAMES_FADE);
    setModeHdmaReset(0);                             /* Corta el HDMA del cielo.            */
    protagonista_sprite_ocultar();                   /* OAM escondida y sin matemática de color. */
    secuencia_cambiar(siguiente);
}

void pantalla_selector_iniciar(void)
{
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);      /* Como el título: BG3 por delante.    */
    bgSetDisable(0);                                 /* Sin capa cercana.                   */
    fondo16_cargar(1, &nivel1_lejos_til, &nivel1_lejos_til_end, &nivel1_lejos_pal, 2,
                   &nivel1_lejos_map, &nivel1_lejos_map_end, VRAM_FONDO_B, VRAM_MAPA_B, SC_64x32);
    bgSetScroll(1, 0, 0);
    fuente_2bpp(2);                                  /* Texto en el fondo 2 (BG3).          */
    bgSetEnable(2);
    limpiar_texto();
    texto_centrado(FILA_TITULO, "SELECT YOUR FIGHTER");

    marcado = personaje_elegido;                     /* El último elegido.                  */
    protagonista_escena_cargar();                    /* OAM, sombra y matemática de color.  */
    protagonista_sprite_cargar(&tarou, PERSONAJE_TAROU, 0, 0, 96, 0, TOPE_CADA_UNO);
    protagonista_sprite_cargar(&barbara, PERSONAJE_BARBARA, 2, 192, 288, 2, TOPE_CADA_UNO);
    paleta_oscura(PERSONAJE_TAROU, 1);
    paleta_oscura(PERSONAJE_BARBARA, 3);
    sprites_usar(4);                                 /* Los dos cuerpos y sus sombras.      */
    mundo_iniciar(&cuerpo_t, X_TAROU, Y_PIES);
    mundo_iniciar(&cuerpo_b, X_BARBARA, Y_PIES);
    cuerpo_b.mira_izq = 1;                           /* Se miran el uno al otro.            */
    nombres();
    antes = padsCurrent(0);                          /* El Start del título no cuenta aquí. */

    do                                               /* El primer cuadro de los dos, entero */
    {                                                /* en la VRAM antes del fade in (la    */
        dma_lista_vaciar();                          /* pantalla sigue apagada).            */
        dibujar();
        dma_lista_ejecutar();
    } while (!protagonista_sprite_al_dia(&tarou) || !protagonista_sprite_al_dia(&barbara));
    sprites_dibujar();

    setModeHdmaColor(HDMA_CHANNEL6, (u8 *)&hdmaGradcieloList);   /* Cielo por línea.   */
    fade_in(FRAMES_FADE);
}

void pantalla_selector_actualizar(void)
{
    u16 actual = padsCurrent(0);
    u16 pulsado = (u16)(actual & ~antes);            /* Lo que se acaba de pulsar.          */

    antes = actual;
    dma_lista_vaciar();
    if (pulsado & (KEY_START | KEY_A))               /* Confirma: a la fase 1 con él.       */
    {
        personaje_elegido = marcado;
        salir(PANTALLA_FASE1);
        return;
    }
    if (pulsado & KEY_B)                             /* Atrás: al título.                   */
    {
        salir(PANTALLA_TITULO);
        return;
    }
    if ((pulsado & KEY_LEFT) && marcado != PERSONAJE_TAROU)
    {
        marcado = PERSONAJE_TAROU;
        nombres();
    }
    else if ((pulsado & KEY_RIGHT) && marcado != PERSONAJE_BARBARA)
    {
        marcado = PERSONAJE_BARBARA;
        nombres();
    }
    dibujar();
    sprites_dibujar();
}

void pantalla_selector_vblank(void)
{
    dma_lista_ejecutar();                            /* Los tiles nuevos de los dos.        */
}
