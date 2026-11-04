/*---------------------------------------------------------------------------------
    hud_juego.c — La barra de vida de Tarou en la capa de texto (ver hud_juego.h).

    PROPÓSITO
      La barra es una cadena de texto, «TAROU [=======-----]», que se escribe en una fila propia
      del HUD de depuración (hud_debug_empezar / texto / enviar: el mismo método barato, probado, de
      las filas de texto, sin consoleDrawText) y se manda a la VRAM por la lista de DMA. La vida
      se pasa a celdas con un desplazamiento: (vida + 1) >> 1, sin dividir. Solo se reescribe
      cuando cambia (`ultimo`), está `sucio` o pasan REPETIR_CADA frames (la copia va por la lista de
      DMA y, si el VBlank se llena, la última copia se pierde en silencio: se vuelve a mandar enseguida).

    SE CONECTA CON
      Incluye:  <snes.h>, hud/hud_juego.h, juego/hud_debug.h.
      Lo usan:  pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      Ver hud_juego.h.

    VARIABLES (estáticas de este archivo)
      ultimo  u8   la vida dibujada la última vez (0xFF: ninguna)
      sucio   u8   1 = hay que volver a mandar la fila
      repetir u8   frames desde la última vez que se mandó (se repite cada REPETIR_CADA)
    → glosario: Consola de texto, Tilemap
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "hud/hud_juego.h"
#include "juego/hud_debug.h"
#include "juego/personaje.h"      /* personaje_elegido: el nombre de la barra. */

#define FILA_BARRA 4                 /* Fila propia del HUD de depuración (0..7) de la barra. */
#define CELDAS 12                    /* Celdas de la barra: 24 de vida / 2 puntos por celda. */
#define REPETIR_CADA 16              /* Cada cuántos frames se repite la fila aunque no cambie. */

static u8 ultimo;
static u8 sucio;
static u8 repetir;

void hud_juego_iniciar(void)
{
    ultimo = 0xFF;                                   /* Nada dibujado todavía.              */
    sucio = 1;
    repetir = 0;
    bgSetEnable(2);                                  /* La capa de texto, siempre encendida. */
}

void hud_juego_reparar(void)
{
    sucio = 1;                                       /* La fila hay que mandarla otra vez...  */
    bgSetEnable(2);                                  /* ...y la capa la apagó el HUD de depuración. */
}

void hud_juego_actualizar(u8 vida)
{
    char barra[CELDAS + 1];                          /* Las celdas y el 0 final.            */
    u16 *p;
    u8 llenas, k;

    if (vida != ultimo)
    {
        ultimo = vida;
        sucio = 1;
    }
    repetir++;
    if (repetir >= REPETIR_CADA)                     /* Por si una copia se perdió (el VBlank   */
    {                                                /* se llena en los frames de más carga).   */
        repetir = 0;
        sucio = 1;
    }
    if (!sucio)
    {
        return;                                      /* Nada nuevo: no gastar ni un ciclo.  */
    }
    sucio = 0;
    llenas = (u8)((vida + 1) >> 1);                  /* 2 puntos por celda, redondeando arriba. */
    for (k = 0; k < CELDAS; k++)
    {
        barra[k] = (char)(k < llenas ? '=' : '-');
    }
    barra[CELDAS] = 0;
    p = hud_debug_empezar(FILA_BARRA);               /* La fila en blanco y su primer sitio. */
    /* El nombre de la protagonista (if y no ?: con cadenas: tcc816 se atraganta). */
    if (personaje_elegido == PERSONAJE_BARBARA)
    {
        p = hud_debug_texto(p, "BARBARA [");
    }
    else
    {
        p = hud_debug_texto(p, "TAROU [");
    }
    p = hud_debug_texto(p, barra);
    hud_debug_texto(p, "]");
    hud_debug_enviar(FILA_BARRA);                    /* 64 bytes a la lista de DMA.         */
}
