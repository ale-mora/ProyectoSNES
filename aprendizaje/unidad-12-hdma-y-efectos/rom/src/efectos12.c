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
      La demo se ejecuta sola, en ciclo de 10 s (sin teclas, para poder
      capturarla con capturar_mesen.ps1), y también responde al mando:
        2 s   (o A)  fundido a negro y vuelta (setFadeEffectEx, 2 frames por paso)
        4 s   (o B)  mosaico que crece y se deshace en las montañas (setMosaicEffect)
        6 s   (o X)  cambia la tabla: día <-> atardecer
        8 s   (o Y)  apaga el HDMA (setModeHdmaReset(0)): el cielo se queda del
                     ÚLTIMO color que escribió el canal
        10 s         lo vuelve a encender con el cielo de día y el ciclo empieza

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
      vueltas    u16     0..599   vueltas del bucle en este ciclo de 10 s
      total      u16     0..      vueltas desde el arranque (lo que muestra LOOP)
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

/*--- El ciclo automático, en vueltas del bucle (60 = 1 s) ---------------------------*/
#define T_FUNDIDO 120                /* 2 s.                                              */
#define T_MOSAICO 240                /* 4 s.                                              */
#define T_CIELO 360                  /* 6 s.                                              */
#define T_APAGAR 480                 /* 8 s.                                              */
#define T_CICLO 600                  /* 10 s: vuelta a empezar.                           */
#define FRAMES_PASO 2                /* setFadeEffectEx: 2 VBlanks por paso x 16 = 32.    */

static u16 vueltas;                  /* Vueltas en este ciclo.                            */
static u16 total;                    /* Vueltas desde el arranque.                        */
static u8 atardecer;                 /* 0 = cielo de día, 1 = atardecer.                  */
static u8 hdma;                      /* 1 = canal 6 encendido.                            */
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

/* Escribe en pantalla el estado de la demo (en inglés: es texto de la SNES). */
static void rotulos(void)
{
    if (atardecer)
    {
        consoleDrawText(1, 3, "SKY TABLE: SUNSET");
    }
    else
    {
        consoleDrawText(1, 3, "SKY TABLE: DAY   ");
    }
    if (hdma)
    {
        consoleDrawText(1, 4, "HDMA CH6: ON ");
    }
    else
    {
        consoleDrawText(1, 4, "HDMA CH6: OFF");
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

    consoleDrawText(1, 1, "UNIT 12 - HDMA AND EFFECTS");
    consoleDrawText(1, 25, "A FADE  B MOSAIC");
    consoleDrawText(1, 26, "X SKY TABLE  Y HDMA OFF");

    vueltas = 0;                                     /* static: NO empiezan en 0 solas.    */
    total = 0;
    atardecer = 0;
    hdma = 0;
    setScreenOn();                                   /* Brillo 15 (espera un VBlank).      */
    poner_cielo();                                   /* El degradado, desde el frame que viene. */
    rotulos();

    while (1)
    {
        pulsado = padsDown(0);                       /* Lo que bajó desde el frame anterior. */

        /*--- Fundido: 16 pasos de brillo, FRAMES_PASO VBlanks cada uno, BLOQUEANTE --- */
        if (vueltas == T_FUNDIDO || (pulsado & KEY_A))
        {
            setFadeEffectEx(FADE_OUT, FRAMES_PASO);  /* 15 → 0: acaba con INIDISP = $00:   */
                                                     /* negro, pero la pantalla ENCENDIDA. */
            setFadeEffectEx(FADE_IN, FRAMES_PASO);   /* 0 → 15.                            */
        }

        /*--- Mosaico: 16 tamaños, 3 VBlanks cada uno (48 frames), BLOQUEANTE --------- */
        if (vueltas == T_MOSAICO || (pulsado & KEY_B))
        {
            setMosaicEffect(MOSAIC_OUT, MOSAIC_BG2); /* Tamaño 0 → 15: los bloques CRECEN. */
            setMosaicEffect(MOSAIC_IN, MOSAIC_BG2);  /* 15 → 0: vuelve a nítido.           */
        }

        /*--- Otra tabla para el mismo canal --------------------------------------------- */
        if (vueltas == T_CIELO || (pulsado & KEY_X))
        {
            atardecer = (u8)!atardecer;              /* Día <-> atardecer.                 */
            poner_cielo();
            rotulos();
        }

        /*--- Apagar el canal: el color 0 se queda como lo dejó la última franja -------- */
        if (vueltas == T_APAGAR || (pulsado & KEY_Y))
        {
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

        /*--- Fin del ciclo: cielo de día y a empezar ------------------------------------- */
        if (vueltas == T_CICLO)
        {
            vueltas = 0;
            atardecer = 0;
            poner_cielo();
            rotulos();
        }

        consoleDrawText(1, 6, "LOOP %d  VBL %d   ", total, snes_vblank_count); /* Ver PROPÓSITO. */
        vueltas++;
        total++;
        WaitForVBlank();                             /* Una vuelta = un frame (si nada bloquea). */
    }
    return 0;                                        /* Nunca se llega: C lo exige.        */
}
