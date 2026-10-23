/*---------------------------------------------------------------------------------
    efectos12.c — Mini-ROM de la unidad 12: un cielo degradado por HDMA, un
    fundido y un mosaico, todo con funciones de PVSnesLib.

    PROPÓSITO
      Las montañas del título en el fondo 1 (su cielo es transparente) y, detrás,
      el color 0 de la CGRAM cambiado en cada franja de líneas por el canal 6 de
      HDMA (setModeHdmaColor con una tabla de src/cielos.c). Una línea de texto
      cuenta dos cosas a la vez:
        LOOP  vueltas del bucle principal (una por WaitForVBlank nuestro);
        VBL   VBlanks de verdad desde el arranque (snes_vblank_count).
      Mientras dura un fundido o un mosaico de la librería, VBL sigue subiendo y
      LOOP no: esas funciones BLOQUEAN el programa (esperan ellas los VBlanks).
      La demo se ejecuta sola (sin teclas, para poder capturarla con
      capturar_mesen.ps1) en un ciclo de 22 s medido con snes_vblank_count, es
      decir, en tiempo real: un efecto que bloquea no retrasa los siguientes.
      También responde al mando:
        6 s   (o A)  fundido a negro y vuelta (setFadeEffectEx, 2 frames por paso)
        9 s   (o B)  mosaico que crece y se deshace en las montañas (setMosaicEffect)
        13 s  (o X)  cambia la tabla: día <-> atardecer
        16 s  (o L)  scroll vertical -1 en los fondos 0 y 1: la última línea de la
                     pantalla deja de enseñar el color del cielo (ver README)
        19 s  (o Y)  apaga el HDMA (setModeHdmaReset(0)): el cielo se queda del
                     ÚLTIMO color que escribió el canal
        22 s         cielo de día, HDMA encendido, scroll 0, y el ciclo empieza
      Los segundos cuentan desde el arranque (VBL / 60), no desde la primera
      captura.

    SE CONECTA CON
      Incluye:  <snes.h> (PVSnesLib), src/cielos.h (las tablas).
      Lee de la ROM: fuente_til/_til_end/_pal, montanas_til/_til_end/_pal/_map/
                _map_end (data.asm).
      Modelo:   src/pantallas/pantalla_titulo.c (setModeHdmaColor con el canal 6,
                setModeHdmaReset al salir) y src/sistema/comun.c (fade_in/fade_out).
      Ejemplos del SDK: snes-examples/graphics/Effects/GradientColors (la base),
                Fading y Mosaic.

    HARDWARE QUE TOCA
      PPU: modo y fondos (setMode, bgSetEnable...), INIDISP (setFadeEffectEx
      escribe el brillo 15..0 y 0..15), MOSAIC $2106 (setMosaicEffect).
      HDMA: canal 6 ($4360-$4364) y HDMAEN $420C (setModeHdmaColor,
      setModeHdmaReset). CGRAM: el color 0, escrito por el HDMA en cada franja.
      VRAM: tiles, mapas y texto (bgInitTileSet, bgInitMapSet, consoleDrawText).

    VARIABLES (estáticas de este archivo)
      inicio     u16     VBL      snes_vblank_count al empezar el ciclo
      t          u16     0..1320  frames de verdad desde el inicio del ciclo
      paso       u8      0..5     el siguiente evento del ciclo que toca
      total      u16     0..      vueltas desde el arranque (lo que muestra LOOP)
      refresco   u8      0..15    vueltas hasta volver a escribir LOOP/VBL
      scroll_menos1 u8   0 / 1    si los fondos 0 y 1 tienen scroll vertical -1
      atardecer  u8      0 / 1    qué tabla está puesta
      hdma       u8      0 / 1    si el canal 6 está encendido
      pulsado    u16     bits     botones que acaban de bajar (padsDown)
    → glosario: DMA / HDMA, Fade / brillo, Mosaico, CGRAM, Frame, VBlank
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "cielos.h"

/*--- Datos en la ROM (data.asm) -------------------------------------------------*/
extern char fuente_til, fuente_til_end, fuente_pal;          /* Fuente 4bpp del juego.  */
extern char montanas_til, montanas_til_end, montanas_pal;    /* Tiles y paleta.         */
extern char montanas_map, montanas_map_end;                  /* Mapa de 32x28 entradas. */

/*--- Reparto de la VRAM: el mismo que src/sistema/comun.h (en palabras) -------------*/
#define VRAM_FONDO_B 0x4000          /* Tiles de las montañas.                            */
#define VRAM_FUENTE 0x6000           /* Los 96 tiles de la fuente.                        */
#define VRAM_MAPA_TEXTO 0x6800       /* Mapa 32x32 de la capa de texto (fondo 0).         */
#define VRAM_MAPA_B 0x7400           /* Mapa 32x32 de las montañas (fondo 1).             */
#define PALETA_MONTANAS 2            /* gfx4snes -e 2: CGRAM 32..47, como en el título.   */

/*--- El ciclo automático, en frames de verdad (60 = 1 s) -------------------------------*/
#define T_FUNDIDO 360                /* 6 s.                                              */
#define T_MOSAICO 540                /* 9 s.                                              */
#define T_CIELO 780                  /* 13 s.                                             */
#define T_SCROLL 960                 /* 16 s.                                             */
#define T_APAGAR 1140                /* 19 s.                                             */
#define T_CICLO 1320                 /* 22 s: vuelta a empezar.                           */
#define FRAMES_PASO 2                /* setFadeEffectEx: 2 VBlanks por paso x 16 = 32.    */
#define VUELTAS_TEXTO 16             /* Cada cuántas vueltas se reescribe LOOP/VBL: una   */
                                     /* consoleDrawText con dos %d tarda más de un frame  */
                                     /* (medido: con una por vuelta, el bucle iba a 30 Hz).*/
#define SCROLL_MENOS1 0xFFFF         /* -1 en u16: el registro guarda 10 bits = $3FF.     */

static u16 inicio;                   /* VBL al empezar el ciclo.                          */
static u16 t;                        /* Frames desde el inicio del ciclo.                 */
static u8 paso;                      /* Siguiente evento del ciclo (0..5).                */
static u16 total;                    /* Vueltas desde el arranque.                        */
static u8 refresco;                  /* Vueltas que faltan para reescribir LOOP/VBL.      */
static u8 atardecer;                 /* 0 = cielo de día, 1 = atardecer.                  */
static u8 hdma;                      /* 1 = canal 6 encendido.                            */
static u8 scroll_menos1;             /* 1 = fondos 0 y 1 con scroll vertical -1.          */
static u16 pulsado;                  /* Botones recién pulsados en esta vuelta.           */

/* Enciende el canal 6 con la tabla que toque. setModeHdmaColor apaga TODOS los
   canales, espera dos interrupciones (wai) y enciende solo el que se le pasa. */
static void poner_cielo(void)
{
    if (atardecer)
    {
        setModeHdmaColor(HDMA_CHANNEL6, (u8 *)cielo_atardecer);  /* Tabla del ejercicio. */
    }
    else
    {
        setModeHdmaColor(HDMA_CHANNEL6, (u8 *)cielo_dia);        /* Tabla de día.       */
    }
    hdma = 1;                                        /* El canal queda encendido.          */
}

/* Pone el scroll vertical de los fondos 0 (texto) y 1 (montañas): 0 o -1. */
static void poner_scroll(void)
{
    if (scroll_menos1)
    {
        bgSetScroll(0, 0, SCROLL_MENOS1);            /* La línea 0 de la imagen enseña la  */
        bgSetScroll(1, 0, SCROLL_MENOS1);            /* fila 0 del fondo, no la 1.         */
    }
    else
    {
        bgSetScroll(0, 0, 0);                        /* Lo normal: la fila 224 (vacía) se  */
        bgSetScroll(1, 0, 0);                        /* ve en la última línea.             */
    }
}

/* Escribe en pantalla el estado de la demo (en inglés: es texto de la SNES). */
static void rotulos(void)
{
    if (atardecer)
    {
        consoleDrawText(1, 10, "SKY TABLE: SUNSET");
    }
    else
    {
        consoleDrawText(1, 10, "SKY TABLE: DAY   ");
    }
    if (hdma)
    {
        consoleDrawText(1, 11, "HDMA CH6: ON ");
    }
    else
    {
        consoleDrawText(1, 11, "HDMA CH6: OFF");
    }
    if (scroll_menos1)
    {
        consoleDrawText(1, 12, "BG VSCROLL: -1");
    }
    else
    {
        consoleDrawText(1, 12, "BG VSCROLL: 0 ");
    }
}

int main(void)
{
    /*--- 1. Texto en el fondo 0 (como la mini-ROM de la unidad 06) --------------------*/
    bgSetGfxPtr(0, VRAM_FUENTE);                     /* Tiles del fondo 0 = la fuente.     */
    bgSetMapPtr(0, VRAM_MAPA_TEXTO, SC_32x32);       /* Mapa del fondo 0 = mapa de texto.  */
    consoleSetTextGfxPtr(VRAM_FUENTE);               /* Dónde copia la consola la fuente.  */
    consoleSetTextMapPtr(VRAM_MAPA_TEXTO);           /* Dónde escribe el texto.            */
    consoleSetTextOffset(0);                         /* El tile 0 de la fuente es el 0.    */
    consoleInitText(0, 2 * 2, (u8 *)&fuente_til, (u8 *)&fuente_pal); /* 2 colores x 2 B.  */

    /*--- 2. Las montañas en el fondo 1 ------------------------------------------------*/
    bgInitTileSet(1, (u8 *)&montanas_til, (u8 *)&montanas_pal, PALETA_MONTANAS,
                  (u16)(&montanas_til_end - &montanas_til), 16 * 2, BG_16COLORS, VRAM_FONDO_B);
    bgInitMapSet(1, (u8 *)&montanas_map, (u16)(&montanas_map_end - &montanas_map),
                 SC_32x32, VRAM_MAPA_B);

    /*--- 3. Mode 1, fondos 0 y 1 encendidos ---------------------------------------------*/
    setMode(BG_MODE1, 0);                            /* 16 + 16 + 4 colores.               */
    bgSetEnable(0);                                  /* Texto.                             */
    bgSetEnable(1);                                  /* Montañas.                          */
    bgSetDisable(2);                                 /* El tercer fondo no se usa.         */

    consoleDrawText(1, 1, "UNIT 12 - HDMA AND EFFECTS"); /* Filas 1 y 10..13: libres  */
    consoleDrawText(1, 24, "A FADE  B MOSAIC  L VSCROLL"); /* del logo de las montañas. */
    consoleDrawText(1, 25, "X SKY TABLE  Y HDMA OFF");

    total = 0;                                       /* static: NO empiezan en 0 solas.    */
    refresco = 0;
    atardecer = 0;
    hdma = 0;
    scroll_menos1 = 0;
    paso = 0;
    setScreenOn();                                   /* Brillo 15 (espera un VBlank).      */
    poner_cielo();                                   /* El degradado, desde el frame que viene. */
    poner_scroll();
    rotulos();
    inicio = 0;                                      /* El ciclo cuenta desde el arranque: */
                                                     /* t = VBL, y VBL / 60 = segundos.    */

    while (1)
    {
        pulsado = padsDown(0);                       /* Lo que bajó desde el frame anterior. */
        t = snes_vblank_count - inicio;              /* Tiempo de verdad del ciclo (resta  */
                                                     /* u16: vale aunque el contador dé la vuelta). */

        /*--- Fundido: 16 pasos de brillo, FRAMES_PASO VBlanks cada uno, BLOQUEANTE --- */
        if ((paso == 0 && t >= T_FUNDIDO) || (pulsado & KEY_A))
        {
            if (paso == 0) paso = 1;                 /* Evento del ciclo hecho.            */
            setFadeEffectEx(FADE_OUT, FRAMES_PASO);  /* 15 → 0: acaba con INIDISP = $00:   */
                                                     /* negro, pero la pantalla ENCENDIDA. */
            setFadeEffectEx(FADE_IN, FRAMES_PASO);   /* 0 → 15.                            */
        }

        /*--- Mosaico: 16 tamaños, 3 VBlanks cada uno (48 frames), BLOQUEANTE --------- */
        if ((paso == 1 && t >= T_MOSAICO) || (pulsado & KEY_B))
        {
            if (paso == 1) paso = 2;
            setMosaicEffect(MOSAIC_OUT, MOSAIC_BG2); /* Tamaño 0 → 15: los bloques CRECEN. */
            setMosaicEffect(MOSAIC_IN, MOSAIC_BG2);  /* 15 → 0: vuelve a nítido.           */
        }

        /*--- Otra tabla para el mismo canal --------------------------------------------- */
        if ((paso == 2 && t >= T_CIELO) || (pulsado & KEY_X))
        {
            if (paso == 2) paso = 3;
            atardecer = (u8)!atardecer;              /* Día <-> atardecer.                 */
            poner_cielo();
            rotulos();
        }

        /*--- Scroll vertical -1: la última línea deja de ser «cielo» ------------------- */
        if ((paso == 3 && t >= T_SCROLL) || (pulsado & KEY_L))
        {
            if (paso == 3) paso = 4;
            scroll_menos1 = (u8)!scroll_menos1;      /* 0 <-> -1.                          */
            poner_scroll();
            rotulos();
        }

        /*--- Apagar el canal: el color 0 se queda como lo dejó la última franja -------- */
        if ((paso == 4 && t >= T_APAGAR) || (pulsado & KEY_Y))
        {
            if (paso == 4) paso = 5;
            if (hdma)
            {
                setModeHdmaReset(0);                 /* HDMAEN = 0: TODOS los canales.     */
                hdma = 0;
            }
            else
            {
                poner_cielo();                       /* Y con Y otra vez, se enciende.     */
            }
            rotulos();
        }

        /*--- Fin del ciclo: cielo de día, HDMA encendido, scroll 0 y a empezar --------- */
        if (paso == 5 && t >= T_CICLO)
        {
            paso = 0;
            inicio = snes_vblank_count;              /* El ciclo siguiente cuenta desde ya. */
            atardecer = 0;
            scroll_menos1 = 0;
            poner_cielo();
            poner_scroll();
            rotulos();
        }

        if (refresco == 0)                           /* Cada VUELTAS_TEXTO vueltas: ver la */
        {                                            /* constante.                         */
            consoleDrawText(1, 13, "LOOP %d  VBL %d   ", total, snes_vblank_count); /* Ver PROPÓSITO. */
            refresco = VUELTAS_TEXTO;
        }
        refresco--;
        total++;
        WaitForVBlank();                             /* Una vuelta = un frame (si nada bloquea). */
    }
    return 0;                                        /* Nunca se llega: C lo exige.        */
}
