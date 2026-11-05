/*---------------------------------------------------------------------------------
    hud_apoyo.c — La barra del apoyo (ver hud_apoyo.h).

    PROPÓSITO
      «LUCINDA [####----] X!»: el apoyo elegido, la barra en 8 celdas (4 puntos cada una) y
      «X!» cuando está llena. Se escribe como la de vida (hud/hud_juego.c): una fila del HUD
      de depuración, solo si algo cambió o cada 16 frames (por si una copia se perdió).

    SE CONECTA CON
      Incluye:  hud/hud_apoyo.h, juego/hud_debug.h.
      Lo usa:   pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      VRAM (una fila del mapa de texto), por la lista de DMA.

    VARIABLES (estáticas)
      ultimo   u8  la barra + el elegido dibujados (para no repetir)
      repetir  u8  frames desde el último envío
    → glosario: HUD
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "hud/hud_apoyo.h"
#include "juego/hud_debug.h"

#define FILA_APOYO 5                 /* La fila de la V del HUD de depuración (cuando está). */
#define CELDAS 8
#define REPETIR_CADA 16

static u8 ultimo;
static u8 repetir;

void hud_apoyo_iniciar(void)
{
    ultimo = 0xFF;
    repetir = 0;
}

void hud_apoyo_actualizar(const Apoyo *a)
{
    char barra[CELDAS + 1];
    u16 *p;
    u8 llenas, k, clave;

    if (!a->disponible)
    {
        return;
    }
    clave = (u8)(a->barra | (a->elegido << 6));      /* Barra (0..32) y elegido (0..2).     */
    repetir++;
    if (clave == ultimo && repetir < REPETIR_CADA)
    {
        return;
    }
    ultimo = clave;
    repetir = 0;
    llenas = (u8)(a->barra >> 2);                    /* 4 puntos por celda.                 */
    for (k = 0; k < CELDAS; k++)
    {
        barra[k] = (char)(k < llenas ? '#' : '-');
    }
    barra[CELDAS] = 0;
    p = hud_debug_empezar(FILA_APOYO);
    if (a->elegido == APOYO_LUCINDA)                 /* (if y no ?: con cadenas: tcc816.)   */
    {
        p = hud_debug_texto(p, "LUCINDA [");
    }
    else if (a->elegido == APOYO_REX)
    {
        p = hud_debug_texto(p, "REX [");
    }
    else
    {
        p = hud_debug_texto(p, "FENRIR [");
    }
    p = hud_debug_texto(p, barra);
    p = hud_debug_texto(p, "]");
    if (a->barra >= APOYO_BARRA_MAX)
    {
        hud_debug_texto(p, " X!");                   /* Lista: X la invoca.                 */
    }
    hud_debug_enviar(FILA_APOYO);
}
