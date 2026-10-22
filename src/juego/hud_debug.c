/*---------------------------------------------------------------------------------
    hud_debug.c — Escribe los valores del protagonista directamente en el tilemap
    de la capa de texto, sin pasar por la consola del SDK.

    PROPÓSITO
      Ocho filas (docs/diseno-demo-jugable.md §9):
        X 123  VX -16        píxel entero de x y velocidad en punto fijo
        Y 144  VY 0
        EST CAMINANDO        nombre del estado, tal cual está en el código
        ESP FUERA  DIR DER
        CAM 0  F 1234        cámara y frames desde que empezó el nivel (velocidad real)
        GOLPE 167 168 28 16  caja de golpe en píxeles del mundo, o "-"
        LIN 80 95 110        línea del haz tras la lógica, tras el sprite y al final
                             del frame (>= 225 = frame pasado de tiempo)
        V5 20  VB 1234       líneas del VBlank al entrar en _vblank y al salir (pasar
                             de 37 es escribir la VRAM con la PPU dibujando) y VBlanks
                             totales (con F, la velocidad real: F / VB)
      CÓMO: un tilemap es una cuadrícula de palabras de 16 bits, una por celda
      de 8x8: el número de tile (los 10 bits bajos), la paleta (bits 10-12) y
      las volteretas/prioridad. La fuente de consola tiene 96 tiles para los
      caracteres ASCII 32..127 en orden, así que el tile de una letra es
      `caracter - 32`; con la paleta 0 y el bit de prioridad (0x2000, para ir
      delante de la capa cercana), la palabra es `(caracter - 32) | 0x2000`.
      Este archivo rellena en RAM las 8 filas (8 x 32 palabras = 512 bytes), una
      por frame si queda tiempo (ver CUÁNTO CUESTA), y apunta la que acaba de
      preparar (64 bytes) en la lista de DMA que pantalla_nivel1_vblank()
      dispara (juego/dma_lista.h); al encenderse, las apunta todas. Copiar las
      384 bytes enteras con dmaCopyVram en cada VBlank costaba 5 líneas de las
      37 (la sobrecarga del SDK, sobre todo).
      Es muchísimo más barato que consoleDrawText (cuyo formateo de
      %d hacía correr el juego a un 60 %: perdía VBlanks) y enseña cómo
      funciona el texto de verdad en la SNES. Es la única pantalla con texto
      que no está en inglés: muestra los nombres del código.

    SE CONECTA CON
      Incluye:  <snes.h> (bgSetEnable), <string.h> (memset), juego/hud_debug.h,
                juego/animaciones.h (NOMBRES_ESTADO), sistema/comun.h
                (VRAM_MAPA_TEXTO, limpiar_texto), sistema/puntofijo.h.
      Lo usan:  pantallas/pantalla_nivel1.c (iniciar, alternar, dibujar, vblank).

    HARDWARE QUE TOCA
      PPU: bgSetEnable(2) / bgSetDisable(2). VRAM: filas 1..8 del mapa de texto
      (VRAM_MAPA_TEXTO) por DMA en cada VBlank mientras el HUD está visible.

    VARIABLES (globales y estáticas de este archivo)
      visible      u8        0 / 1               1 = el HUD se está mostrando
      enviar_todo  u8        0 / 1               1 = recién encendido: apuntar las 8 filas
      turno        u8        0..7                fila que se prepara en este frame
      mapa         u16[256]  palabras de tilemap las 8 filas seguidas, listas para la VRAM
      golpe        Caja      píxeles del mundo   caja de golpe del frame (si la hay)
      POTENCIAS    u16[4]    const               10000, 1000, 100, 10 para pasar a decimal

    CUÁNTO CUESTA (medido en Mesen el 2026-10-21, ROM FastROM)
      Con el HUD encendido el juego iba a ~47 FPS: preparar una fila costaba de
      48 a 150 líneas del haz, y en los frames en que Tarou planifica un cuadro
      nuevo la lógica y el sprite ya acaban en la línea 170-220. Tres cambios:
        - La fila se deja en blanco con memset (ensamblador de la librería):
          el bucle en C costaba ~30 líneas; memset, ~3.
        - Se escribe con un PUNTERO que avanza (`*p++ = ...`) y variables
          locales, no con globales indexadas (`celda[col]`, `POTENCIAS[n]`).
        - TOPE: si cuando toca el HUD el haz ya pasó de HUD_LINEA_MAX, la fila
          espera al frame siguiente.
      Coste por fila después (líneas): X 47, Y 44, EST 33, ESP 36, GOLPE 24,
      LIN 58, V/VB 57. Con el HUD encendido todo el recorrido del tutorial se
      pierden ~6 frames más que con él apagado (antes, ~1 de cada 5).

    → glosario: HUD de depuración, Tilemap, Tile, DMA / HDMA, Caja de colisión / caja de golpe
---------------------------------------------------------------------------------*/
#include <snes.h>
#include <string.h>                 /* memset: la de la librería está en ensamblador.      */
#include "juego/hud_debug.h"
#include "juego/animaciones.h"
#include "sistema/comun.h"
#include "sistema/puntofijo.h"
#include "sistema/secuencia.h"      /* frames_en_pantalla: para medir la velocidad real. */
#include "juego/dma_lista.h"

#define ANCHO_TEXTO 32             /* Celdas por fila de la capa de texto.               */
#define FILAS_HUD 8                /* Filas que ocupa el HUD.                            */
#define FILA_INICIAL 1             /* Primera fila del HUD dentro del mapa de texto.     */
#define TILE_ESPACIO 0             /* ' ' es el ASCII 32: el tile 0 de la fuente.        */
/* Tile de un carácter ASCII, paleta 0 y el bit de PRIORIDAD (bit 13, 0x2000): en Mode 1
   con BG3_MODE1_PRIORITY_HIGH, solo los tiles de BG3 con ese bit van delante de todo.
   Sin él, el techo del túnel del tutorial (capa cercana, arriba del todo) tapaba el texto. */
#define TILE(c) ((u16)(((c) - 32) | 0x2000))
/* Si al llegar al HUD el haz va por debajo de esta línea, la fila espera: una fila
   cuesta de 24 a 63 líneas (medido) y el VBlank empieza en la 225: 225 - 63 - 12. */
#define HUD_LINEA_MAX 150

#define BANCO_WRAM 0x7E            /* Las variables de los .c viven en el banco $7E.     */
static u8 visible;                 /* 1 mientras el HUD está encendido.                  */
static u8 enviar_todo;             /* 1 = acaba de encenderse: apuntar las 8 filas.      */
static u8 turno;                   /* Fila que toca preparar en este frame (0..7).       */
static u16 mapa[FILAS_HUD * ANCHO_TEXTO];  /* Las 8 filas del HUD, seguidas, como palabras de tilemap. */
static Caja golpe;                 /* Caja de golpe del frame actual.                    */
extern u16 diag_linea[3];          /* Líneas del haz medidas en pantalla_nivel1.c.        */
extern u16 diag_vblank[2];         /* Líneas del VBlank al entrar y al salir de _vblank.  */

/*--- apuntar_fila: la fila `fila` del HUD, a la lista de DMA (64 bytes) ------------*/
static void apuntar_fila(u8 fila)
{
    /* fila * 32 como desplazamiento (<< 5): tcc816 no multiplica por hardware. */
    dma_lista_anadir((u16)(mapa + ((u16)fila << 5)),
                     (u16)(VRAM_MAPA_TEXTO + ((u16)(FILA_INICIAL + fila) << 5)),
                     ANCHO_TEXTO * 2, BANCO_WRAM, DMA_VMAIN_TILES);
}

/*--- empezar_fila: deja la fila en blanco y devuelve un puntero a su primera celda ---
    Las filas caben siempre en 32 caracteres (la más larga ocupa unos 22): no se
    comprueba el borde en cada carácter. En blanco con memset y no con un bucle en C:
    tcc816 traduce cada vuelta de `*p++ = 0` a unas 60 instrucciones (guarda y
    recarga el puntero de 32 bits en la pila), ~30 líneas del haz para 32 celdas; el
    memset de la librería es un bucle de ensamblador de 3 instrucciones, ~3 líneas. */
static u16 *empezar_fila(u8 fila)
{
    u16 *inicio = mapa + ((u16)fila << 5);           /* fila * 32, con un desplazamiento.   */

    memset(inicio, TILE_ESPACIO, ANCHO_TEXTO * 2);   /* TILE_ESPACIO es 0: bytes a 0.       */
    return inicio;
}

/*--- texto: escribe una cadena en p y devuelve dónde sigue ---------------------------*/
static u16 *texto(u16 *p, const char *s)
{
    while (*s != 0)
    {
        *p++ = TILE(*s);
        s++;
    }
    return p;
}

/*--- numero: escribe un entero con signo en decimal y devuelve dónde sigue
    SIN dividir: el 65816 no tiene instrucción de división y la rutina que tcc816
    llama para `/` y `%` cuesta miles de ciclos; con ~40 divisiones por frame el
    juego bajaba a un cuarto de su velocidad. En su lugar, para cada potencia de
    diez se resta mientras quepa (como se hace a mano): como mucho 9 restas por
    cifra. La potencia se recorre con un puntero, no con POTENCIAS[n]. ------------*/
static const u16 POTENCIAS[4] = {10000, 1000, 100, 10};
static u16 *numero(u16 *p, s16 valor)
{
    const u16 *pot = POTENCIAS;
    u16 v;
    u8 cifra;
    u8 empezado = 0;                                 /* Para no escribir ceros a la izquierda.*/
    u8 k;

    if (valor < 0)
    {
        *p++ = TILE('-');
        v = (u16)(-valor);
    }
    else
    {
        v = (u16)valor;
    }
    for (k = 4; k != 0; k--, pot++)
    {
        cifra = '0';
        while (v >= *pot)
        {
            v -= *pot;
            cifra++;
        }
        if (cifra != '0' || empezado)
        {
            *p++ = TILE(cifra);
            empezado = 1;
        }
    }
    *p++ = TILE('0' + v);                            /* Las unidades, siempre.               */
    return p;
}

void hud_debug_iniciar(void)
{
    u8 k;

    visible = 0;
    turno = 0;
    for (k = 0; k < FILAS_HUD; k++)
    {
        empezar_fila(k);                             /* Nuestras filas, en blanco.          */
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
        enviar_todo = 1;                             /* Las 7 filas de golpe en este frame. */
        bgSetEnable(2);
    }
}

void hud_debug_dibujar(const Tarou *t, u16 camara)
{
    u16 *p;
    u8 k;

    if (!visible)
    {
        return;                                      /* Apagado: no gastar un ciclo.        */
    }
    if (enviar_todo)                                 /* Recién encendido: las 7 filas tal   */
    {                                                /* como están (cuestan 3 líneas del    */
        enviar_todo = 0;                             /* VBlank y nada de CPU).              */
        for (k = 0; k < FILAS_HUD; k++)
        {
            apuntar_fila(k);
        }
        return;
    }
    if (diag_linea[1] > HUD_LINEA_MAX)               /* Frame cargado: esta fila, al        */
    {                                                /* siguiente (ver CUÁNTO CUESTA).      */
        return;
    }

    /* Una fila por frame, por turnos (cada dato se refresca cada 8 frames, ~0,13 s,
       o algo más si hay frames cargados). */
    p = empezar_fila(turno);
    switch (turno)
    {
        case 0:                                      /* X 123  VX -16                       */
            p = texto(p, "X ");
            p = numero(p, (s16)PF_ENTERO(t->x_fp));
            p = texto(p, "  VX ");
            numero(p, t->vx_fp);
            break;
        case 1:                                      /* Y 144  VY 0                         */
            p = texto(p, "Y ");
            p = numero(p, (s16)PF_ENTERO(t->y_fp));
            p = texto(p, "  VY ");
            numero(p, t->vy_fp);
            break;
        case 2:                                      /* EST CAMINANDO                       */
            p = texto(p, "EST ");
            texto(p, NOMBRES_ESTADO[t->estado]);
            break;
        case 3:                                      /* ESP FUERA  DIR DER                  */
            p = texto(p, "ESP ");
            if (t->espada == ESPADA_FUERA)
            {
                p = texto(p, "FUERA");
            }
            else
            {
                p = texto(p, "ENV");
            }
            p = texto(p, "  DIR ");
            if (t->direccion == DIR_DERECHA)
            {
                texto(p, "DER");
            }
            else
            {
                texto(p, "IZQ");
            }
            break;
        case 4:                                      /* CAM 0  F 1234 (frames en el nivel)  */
            p = texto(p, "CAM ");
            p = numero(p, (s16)camara);
            p = texto(p, "  F ");
            numero(p, (s16)(frames_en_pantalla & 0x7FFF));
            break;
        case 6:                                      /* LIN a b c                           */
            p = texto(p, "LIN ");
            p = numero(p, (s16)diag_linea[0]);
            p = texto(p, " ");
            p = numero(p, (s16)diag_linea[1]);
            p = texto(p, " ");
            numero(p, (s16)diag_linea[2]);
            break;
        case 7:                                      /* V5 20  VB n                         */
            p = texto(p, "V");                       /* Coste del VBlank: entrada y salida. */
            p = numero(p, (s16)diag_vblank[0]);
            p = texto(p, " ");
            p = numero(p, (s16)diag_vblank[1]);
            p = texto(p, "  VB ");
            numero(p, (s16)(snes_vblank_count & 0x7FFF));
            break;
        default:                                     /* GOLPE 167 168 28 16  o  GOLPE -     */
            p = texto(p, "GOLPE ");
            if (tarou_caja_golpe(t, &golpe))
            {
                p = numero(p, golpe.x);
                p = texto(p, " ");
                p = numero(p, golpe.y);
                p = texto(p, " ");
                p = numero(p, (s16)golpe.ancho);
                p = texto(p, " ");
                numero(p, (s16)golpe.alto);
            }
            else
            {
                texto(p, "-");
            }
            break;
    }
    apuntar_fila(turno);                             /* Solo la fila de este frame: 64 B.   */
    turno++;
    if (turno >= FILAS_HUD)
    {
        turno = 0;
    }
}
