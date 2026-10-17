/*---------------------------------------------------------------------------------
    hud_debug.c — Escribe los valores del protagonista directamente en el tilemap
    de la capa de texto, sin pasar por la consola del SDK.

    PROPÓSITO
      Siete filas (docs/diseno-demo-jugable.md §9):
        X 123  VX -16        píxel entero de x y velocidad en punto fijo
        Y 144  VY 0
        EST CAMINANDO        nombre del estado, tal cual está en el código
        ESP FUERA  DIR DER
        CAM 0  F 1234        cámara y frames desde que empezó el nivel (velocidad real)
        LIN 80 95 110  VB 1234  línea del haz tras la lógica, tras el sprite y al final
                             del frame (>= 225 = frame pasado de tiempo), y VBlanks totales
        GOLPE 167 168 28 16  caja de golpe en píxeles del mundo, o "-"
      CÓMO: un tilemap es una cuadrícula de palabras de 16 bits, una por celda
      de 8x8: el número de tile (los 10 bits bajos), la paleta (bits 10-12) y
      las volteretas/prioridad. La fuente de consola tiene 96 tiles para los
      caracteres ASCII 32..127 en orden, así que el tile de una letra es
      `caracter - 32` y, con la paleta 0, la palabra es ese número tal cual.
      Este archivo rellena en RAM las 6 filas (6 x 32 palabras = 384 bytes) y
      pantalla_nivel1_vblank() las copia a la VRAM por DMA justo después del
      VBlank. Es muchísimo más barato que consoleDrawText (cuyo formateo de
      %d hacía correr el juego a un 60 %: perdía VBlanks) y enseña cómo
      funciona el texto de verdad en la SNES. Es la única pantalla con texto
      que no está en inglés: muestra los nombres del código.

    SE CONECTA CON
      Incluye:  <snes.h> (dmaCopyVram, bgSetEnable), juego/hud_debug.h,
                juego/animaciones.h (NOMBRES_ESTADO), sistema/comun.h
                (VRAM_MAPA_TEXTO, limpiar_texto), sistema/puntofijo.h.
      Lo usan:  pantallas/pantalla_nivel1.c (iniciar, alternar, dibujar, vblank).

    HARDWARE QUE TOCA
      PPU: bgSetEnable(2) / bgSetDisable(2). VRAM: filas 1..6 del mapa de texto
      (VRAM_MAPA_TEXTO) por DMA en cada VBlank mientras el HUD está visible.

    VARIABLES (globales y estáticas de este archivo)
      visible   u8     0 / 1                 1 = el HUD se está mostrando
      turno     u8     0..5                  fila que se prepara en este frame
      mapa      u16[192]  palabras de tilemap  las 6 filas seguidas, listas para copiar a VRAM
      celda     u16*                           puntero a la fila que se está escribiendo
      golpe     Caja   píxeles del mundo     caja de golpe del frame (si la hay)
      POTENCIAS u16[4] const                 10000, 1000, 100, 10 para convertir a decimal
      i, n      u8                           índices de bucle

    → glosario: HUD de depuración, Tilemap, Tile, DMA / HDMA, Caja de colisión / caja de golpe
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/hud_debug.h"
#include "juego/animaciones.h"
#include "sistema/comun.h"
#include "sistema/puntofijo.h"
#include "sistema/secuencia.h"      /* frames_en_pantalla: para medir la velocidad real. */

#define ANCHO_TEXTO 32             /* Celdas por fila de la capa de texto.               */
#define FILAS_HUD 7                /* Filas que ocupa el HUD.                            */
#define FILA_INICIAL 1             /* Primera fila del HUD dentro del mapa de texto.     */
#define TILE_ESPACIO 0             /* ' ' es el ASCII 32: el tile 0 de la fuente.        */
#define TILE(c) ((u16)((c) - 32))  /* Tile de un carácter ASCII (paleta 0: nada más).    */

static u8 visible;                 /* 1 mientras el HUD está encendido.                  */
static u8 turno;                   /* Fila que toca preparar en este frame (0..5).       */
static u16 mapa[FILAS_HUD * ANCHO_TEXTO];  /* Las 6 filas del HUD, seguidas, como palabras de tilemap. */
static u16 *celda;                 /* Puntero a la fila en curso: evita multiplicar en cada acceso. */
static Caja golpe;                 /* Caja de golpe del frame actual.                    */
static u8 i, n;
extern u16 diag_linea[3];          /* Líneas del haz medidas en pantalla_nivel1.c.        */

/*--- limpiar_fila: toda la fila a espacios -------------------------------------------*/
static void limpiar_fila(u8 fila)
{
    /* tcc816 no tiene multiplicación por hardware: mapa[fila][col] multiplicaría en
       cada acceso. Se calcula el puntero de la fila UNA vez y luego solo se suma. */
    celda = mapa + (u16)fila * ANCHO_TEXTO;
    for (i = 0; i < ANCHO_TEXTO; i++)
    {
        celda[i] = TILE_ESPACIO;
    }
}

/*--- poner_texto: escribe una cadena desde la columna `col`; devuelve la siguiente -----*/
static u8 poner_texto(u8 fila, u8 col, const char *texto)
{
    (void)fila;                                  /* La fila ya está en `celda` (limpiar_fila). */
    while (*texto != 0 && col < ANCHO_TEXTO)
    {
        celda[col] = TILE(*texto);
        col++;
        texto++;
    }
    return col;
}

/*--- poner_numero: escribe un entero con signo en decimal; devuelve la columna siguiente
    SIN dividir: el 65816 no tiene instrucción de división y la rutina que tcc816
    llama para `/` y `%` cuesta miles de ciclos; con ~40 divisiones por frame el
    juego bajaba a un cuarto de su velocidad. En su lugar, para cada potencia de
    diez se resta mientras quepa (como se hace a mano): como mucho 9 restas por
    cifra. ----------------------------------------------------------------------------*/
static const u16 POTENCIAS[4] = {10000, 1000, 100, 10};
static u8 poner_numero(u8 fila, u8 col, s16 valor)
{
    u16 v;
    u8 cifra, empezado;
    (void)fila;                                  /* La fila ya está en `celda` (limpiar_fila). */
    if (valor < 0)
    {
        celda[col] = TILE('-');
        col++;
        v = (u16)(-valor);
    }
    else
    {
        v = (u16)valor;
    }
    empezado = 0;                                /* Para no escribir ceros a la izquierda.*/
    for (n = 0; n < 4; n++)
    {
        cifra = 0;
        while (v >= POTENCIAS[n])
        {
            v -= POTENCIAS[n];
            cifra++;
        }
        if (cifra != 0 || empezado)
        {
            celda[col] = TILE((char)('0' + cifra));
            col++;
            empezado = 1;
        }
    }
    celda[col] = TILE((char)('0' + v));          /* Las unidades, siempre.               */
    col++;
    return col;
}

void hud_debug_iniciar(void)
{
    visible = 0;
    turno = 0;
    for (n = 0; n < FILAS_HUD; n++)
    {
        limpiar_fila(n);                             /* Nuestras filas, en blanco.          */
    }
    limpiar_texto();                                 /* La consola deja el mapa en blanco.  */
    bgSetDisable(2);                                 /* Apagado hasta que lo pida SELECT.   */
}

void hud_debug_alternar(void)
{
    if (visible)
    {
        visible = 0;
        limpiar_texto();                             /* Borra nuestras filas en el próximo VBlank. */
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
    u8 c;
    if (!visible)
    {
        return;                                      /* Apagado: no gastar un ciclo.        */
    }

    /* Una fila por frame, por turnos (cada dato se refresca cada 6 frames = 0,1 s):
       así el HUD nunca hace que un frame se pase de los 16,7 ms. */
    switch (turno)
    {
        case 0:
            limpiar_fila(0);                         /* X 123  VX -16                       */
            c = poner_texto(0, 0, "X ");
            c = poner_numero(0, c, (s16)PF_ENTERO(t->x_fp));
            c = poner_texto(0, c, "  VX ");
            poner_numero(0, c, t->vx_fp);
            break;
        case 1:
            limpiar_fila(1);                         /* Y 144  VY 0                         */
            c = poner_texto(1, 0, "Y ");
            c = poner_numero(1, c, (s16)PF_ENTERO(t->y_fp));
            c = poner_texto(1, c, "  VY ");
            poner_numero(1, c, t->vy_fp);
            break;
        case 2:
            limpiar_fila(2);                         /* EST CAMINANDO                       */
            c = poner_texto(2, 0, "EST ");
            poner_texto(2, c, NOMBRES_ESTADO[t->estado]);
            break;
        case 3:
            limpiar_fila(3);                         /* ESP FUERA  DIR DER                  */
            c = poner_texto(3, 0, "ESP ");
            if (t->espada == ESPADA_FUERA)
            {
                c = poner_texto(3, c, "FUERA");
            }
            else
            {
                c = poner_texto(3, c, "ENV");
            }
            c = poner_texto(3, c, "  DIR ");
            if (t->direccion == DIR_DERECHA)
            {
                poner_texto(3, c, "DER");
            }
            else
            {
                poner_texto(3, c, "IZQ");
            }
            break;
        case 4:
            limpiar_fila(4);                         /* CAM 0  F 1234 (frames en el nivel)  */
            c = poner_texto(4, 0, "CAM ");
            c = poner_numero(4, c, (s16)camara);
            c = poner_texto(4, c, "  F ");
            poner_numero(4, c, (s16)(frames_en_pantalla & 0x7FFF));
            break;
        case 6:
            limpiar_fila(6);                         /* LIN a b c  VB n: coste del frame    */
            c = poner_texto(6, 0, "LIN ");
            c = poner_numero(6, c, (s16)diag_linea[0]);
            c = poner_texto(6, c, " ");
            c = poner_numero(6, c, (s16)diag_linea[1]);
            c = poner_texto(6, c, " ");
            c = poner_numero(6, c, (s16)diag_linea[2]);
            c = poner_texto(6, c, "  VB ");
            poner_numero(6, c, (s16)(snes_vblank_count & 0x7FFF));
            break;
        default:
            limpiar_fila(5);                         /* GOLPE 167 168 28 16  o  GOLPE -     */
            c = poner_texto(5, 0, "GOLPE ");
            if (tarou_caja_golpe(t, &golpe))
            {
                c = poner_numero(5, c, golpe.x);
                c = poner_texto(5, c, " ");
                c = poner_numero(5, c, golpe.y);
                c = poner_texto(5, c, " ");
                c = poner_numero(5, c, (s16)golpe.ancho);
                c = poner_texto(5, c, " ");
                poner_numero(5, c, (s16)golpe.alto);
            }
            else
            {
                poner_texto(5, c, "-");
            }
            break;
    }
    turno++;
    if (turno >= FILAS_HUD)
    {
        turno = 0;
    }
}

void hud_debug_vblank(void)
{
    if (visible)
    {
        /* 6 filas x 32 palabras = 384 bytes, a partir de la fila FILA_INICIAL del mapa.
           La dirección de VRAM va en palabras; el tamaño, en bytes. */
        dmaCopyVram((u8 *)mapa, VRAM_MAPA_TEXTO + FILA_INICIAL * ANCHO_TEXTO, sizeof(mapa));
    }
}
