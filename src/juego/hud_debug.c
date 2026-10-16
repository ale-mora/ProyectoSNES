/*---------------------------------------------------------------------------------
    hud_debug.c — Escribe los valores del protagonista con la consola de texto.

    PROPÓSITO
      Seis filas (docs/diseno-demo-jugable.md §9):
        X 123  VX -16        píxel entero de x y velocidad en punto fijo
        Y 144  VY 0
        EST CAMINANDO        nombre del estado, tal cual está en el código
        ESP FUERA  DIR DER
        CAM 0
        GOLPE 167 168 28 16  caja de golpe en píxeles del mundo, o "-"
      Cada fila termina con espacios para borrar los dígitos del frame anterior
      (la consola no limpia sola). Es la única pantalla con texto que no está en
      inglés: muestra los nombres del código para que coincidan con lo que se lee.

    SE CONECTA CON
      Incluye:  juego/hud_debug.h, juego/animaciones.h (NOMBRES_ESTADO),
                sistema/comun.h (limpiar_texto), sistema/puntofijo.h, <snes.h>.
      Lo usan:  pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      PPU: bgSetEnable(2) / bgSetDisable(2). VRAM: mapa de texto, a través de
      consoleDrawText (copiado por el SDK en el VBlank).

    VARIABLES (globales y estáticas de este archivo)
      visible  u8    0 / 1  1 = el HUD se está mostrando
      golpe    Caja  píxeles del mundo  caja de golpe del frame (si la hay)

    → glosario: HUD de depuración, Consola de texto, Caja de colisión / caja de golpe
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/hud_debug.h"
#include "juego/animaciones.h"
#include "sistema/comun.h"
#include "sistema/puntofijo.h"

#define COLUMNA 1                  /* Un carácter de margen por la izquierda.             */
#define FILA_X 1                   /* Filas de la consola (de 28) que ocupa cada dato.    */
#define FILA_Y 2
#define FILA_ESTADO 3
#define FILA_ESPADA 4
#define FILA_CAMARA 5
#define FILA_GOLPE 6

static u8 visible;                 /* 1 mientras el HUD está encendido.                  */
static Caja golpe;                 /* Caja de golpe del frame actual.                    */
static const char *texto_espada;   /* "FUERA" o "ENV": elegido antes de imprimir.        */
static const char *texto_direccion;/* "DER" o "IZQ". tcc816 no admite ?: en los argumentos.*/

void hud_debug_iniciar(void)
{
    visible = 0;
    limpiar_texto();
    bgSetDisable(2);                                 /* Apagado hasta que lo pida SELECT.   */
}

void hud_debug_alternar(void)
{
    if (visible)
    {
        visible = 0;
        limpiar_texto();                             /* Que no quede texto fantasma.        */
        bgSetDisable(2);
    }
    else
    {
        visible = 1;
        bgSetEnable(2);
    }
}

void hud_debug_dibujar(const Tarou *t, u16 camara)
{
    if (!visible)
    {
        return;                                      /* Apagado: no gastar frames en texto. */
    }
    /* %d imprime enteros con signo; los espacios finales borran dígitos viejos. */
    consoleDrawText(COLUMNA, FILA_X, "X %d  VX %d    ", (int)PF_ENTERO(t->x_fp), (int)t->vx_fp);
    consoleDrawText(COLUMNA, FILA_Y, "Y %d  VY %d    ", (int)PF_ENTERO(t->y_fp), (int)t->vy_fp);
    consoleDrawText(COLUMNA, FILA_ESTADO, "EST %s            ", NOMBRES_ESTADO[t->estado]);
    if (t->espada == ESPADA_FUERA)
    {
        texto_espada = "FUERA";
    }
    else
    {
        texto_espada = "ENV";
    }
    if (t->direccion == DIR_DERECHA)
    {
        texto_direccion = "DER";
    }
    else
    {
        texto_direccion = "IZQ";
    }
    consoleDrawText(COLUMNA, FILA_ESPADA, "ESP %s  DIR %s   ", texto_espada, texto_direccion);
    consoleDrawText(COLUMNA, FILA_CAMARA, "CAM %d    ", (int)camara);
    if (tarou_caja_golpe(t, &golpe))
    {
        consoleDrawText(COLUMNA, FILA_GOLPE, "GOLPE %d %d %d %d    ",
                        (int)golpe.x, (int)golpe.y, (int)golpe.ancho, (int)golpe.alto);
    }
    else
    {
        consoleDrawText(COLUMNA, FILA_GOLPE, "GOLPE -               ");
    }
}
