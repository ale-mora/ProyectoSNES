/*---------------------------------------------------------------------------------
    main.c — Mini-ROM de la unidad 09: Tarou (96x64) como metasprite dinámico,
    dibujado con el motor de sprites dinámicos de PVSnesLib.

    PROPÓSITO
      Enseña la manera "de librería" de dibujar un personaje más grande que un
      sprite y con más cuadros de los que caben en la VRAM: los cuadros viven en
      la ROM y, cuando cambia el cuadro, el motor del SDK copia a la VRAM solo
      los tiles del cuadro actual. Tres llamadas por frame:
        1. oamMetaDrawDyn32(...)        pone las piezas en la OAM y, si hace falta,
                                        encola la copia de sus tiles;
        2. oamInitDynamicSpriteEndFrame prepara el motor para el frame siguiente;
        3. oamVramQueueUpdate()         justo después del VBlank: hace las copias.
      Con la cruceta Tarou camina a izquierda o derecha (animación "caminar",
      con espejo al mirar a la izquierda); sin cruceta, "reposo". Es el mismo
      esquema que usó el juego hasta el 2026-10-21 (ver README, "Por qué el
      juego dejó el motor del SDK").

    SE CONECTA CON
      Incluye:  <snes.h>, tarou_reposo_meta.inc y tarou_caminar_meta.inc (las
                tablas t_metasprite que genera gfx4snes -T, en res/).
      Lee de la ROM: tarou_reposo_til, tarou_caminar_til, tarou_pal, fuente_til,
                fuente_pal (data.asm).
      Modelo:   tools/pvsneslib/snes-examples/graphics/Sprites/DynamicEngineMetaSprite
                y la versión de src/juego/tarou_sprite.c anterior al 2026-10-21
                (git show 8a32523:src/juego/tarou_sprite.c).

    HARDWARE QUE TOCA
      OAM y VRAM 0x0000 (piezas de 32x32) a través del motor dinámico del SDK.
      CGRAM: paleta de sprites 0 (colores 128..143) y paleta 0 de la fuente.
      BG1: la consola de texto (tiles en 0x3000, mapa en 0x6800).

    VARIABLES (estáticas de este archivo; todas se inicializan en main: la WRAM
               arranca con basura)
      x            u16             16..144   columna de pantalla de la celda de 96x64
      anim         u8              0 / 1     ANIM_REPOSO o ANIM_CAMINAR
      cuadro       u8              0..8      cuadro actual de la animación
      frames       u8              0..11     frames que lleva el cuadro actual
      izquierda    u8              0 / 1     1 = mira a la izquierda (espejo)
      ult_anim, ult_cuadro, ult_izq  u8      lo dibujado el frame anterior (0xFF = nada)
      espejo       t_metasprite[7] la tabla espejada del cuadro actual (RAM)
      pad          u16             botones mantenidos (padsCurrent(0))
      texto_pendiente u8           1 = la línea de estado está por escribir (se
                                   escribe un frame SIN subida de tiles)
      vb_lineas    u16             0..60     líneas del VBlank usadas en la última
                                   subida (medidas con el haz; el límite es 37)

    → glosario: Metasprite, Sprite dinámico, OAM, VRAM, VBlank, DMA / HDMA
---------------------------------------------------------------------------------*/
#include <snes.h>

#include "tarou_reposo_meta.inc"           /* tarou_reposo_metasprites[9]: un cuadro por tabla. */
#include "tarou_caminar_meta.inc"          /* tarou_caminar_metasprites[8].                     */

/*--- Datos de data.asm -------------------------------------------------------------*/
extern char tarou_reposo_til, tarou_caminar_til, tarou_pal;
extern char fuente_til, fuente_pal;

/*--- Constantes --------------------------------------------------------------------*/
#define ANIM_REPOSO 0                      /* Índices de animación.                           */
#define ANIM_CAMINAR 1
#define REPOSO_CUADROS 9                   /* Cuadros de cada tira (los de tarou_anim.h del    */
#define CAMINAR_CUADROS 8                  /* juego: 864 y 768 px / 96).                      */
#define REPOSO_FRAMES 12                   /* Frames por cuadro: los de src/juego/animaciones.c.*/
#define CAMINAR_FRAMES 6
#define Y_TAROU 120                        /* Fila de pantalla de la celda (sin scroll).      */
#define X_MIN 16                           /* Márgenes para que la celda no salga de la       */
#define X_MAX 144                          /* pantalla (256 - 96 - 16).                       */
#define PIEZAS_MAX 6                       /* 3x2 piezas de 32x32 en una celda de 96x64.      */
#define FIN_TABLA 0xFF80                   /* dx del terminador: metasprite_end (-128) en u16. */
#define ESPEJO_DX 64                       /* 96 - 32: la pieza en dx pasa a 64 - dx.         */
#define BIT_HFLIP 0x40                     /* Bit h del atributo (OBJ_FLIPX): espejo.         */
#define NADA 0xFF                          /* "Todavía no se dibujó nada".                    */
#define LINEA_VBLANK 225                   /* Primera línea del VBlank (NTSC, 224 visibles).  */
#define LINEAS_VBLANK 37                   /* 262 - 225: lo que dura el VBlank.               */

/* Contadores del haz (PVSnesLib no les da nombre; los mismos que usa
   src/pantallas/pantalla_nivel1.c): leer $2137 congela el contador y $213D da
   la línea en dos lecturas (bits 0..7 y bit 8). $213F rearma esa doble lectura. */
#define REG_SLHV (*(vuint8 *)0x2137)
#define REG_OPVCT (*(vuint8 *)0x213D)

/*--- Estado ------------------------------------------------------------------------*/
static u16 x, pad;
static u8 anim, cuadro, frames, izquierda;
static u8 ult_anim, ult_cuadro, ult_izq;
static u8 texto_pendiente;                    /* 1 = escribir la línea de estado.       */
static u16 vb_lineas;                         /* Líneas del VBlank de la última subida. */
static t_metasprite espejo[PIEZAS_MAX + 1];   /* Hasta 6 piezas + el terminador.        */

/*--- espejar: copia `tabla` en `espejo` mirando a la izquierda ------------------------
    Lo mismo que metasprite_espejar() de src/juego/metasprite.c: cada pieza cambia
    de lado (dx 0 <-> 64; la del medio, 32, se queda), conserva su bloque y gana el
    bit de espejo; el terminador va JUSTO detrás de la última pieza, porque una
    tabla de gfx4snes -T puede tener menos de 6 (omite las piezas transparentes). */
static void espejar(const t_metasprite *tabla)
{
    u8 n = 0;                                          /* Piezas copiadas.                  */
    while (n < PIEZAS_MAX && tabla[n].dx != FIN_TABLA)  /* Hasta el terminador.             */
    {
        espejo[n].dx = (u16)(ESPEJO_DX - tabla[n].dx);  /* Cambia de lado.                  */
        espejo[n].dy = tabla[n].dy;                     /* Misma fila.                      */
        espejo[n].dtile = tabla[n].dtile;               /* Mismo bloque de la tira.         */
        espejo[n].props = (u8)(tabla[n].props | BIT_HFLIP); /* Volteada.                    */
        espejo[n].unused = 0;                           /* Relleno.                         */
        n++;                                            /* Siguiente pieza.                 */
    }
    espejo[n].dx = FIN_TABLA;                          /* El terminador.                    */
}

/*--- linea_haz: en qué línea de la pantalla va el haz ahora mismo (0..261) ------------*/
static u16 linea_haz(void)
{
    u8 bajo, alto;
    bajo = REG_STAT78;                                 /* Rearma la doble lectura de $213D. */
    bajo = REG_SLHV;                                   /* Congela los contadores del haz.   */
    bajo = REG_OPVCT;                                  /* Primera lectura: bits 0..7.       */
    alto = (u8)(REG_OPVCT & 1);                        /* Segunda: el bit 8.                */
    return (u16)(((u16)alto << 8) | bajo);
}

/*--- texto: la línea de estado, solo cuando algo cambió --------------------------------*/
static void texto(void)
{
    if (anim == ANIM_CAMINAR)                          /* Sin ?: con cadenas: tcc816 se     */
    {                                                  /* rompe (error interno).            */
        consoleDrawText(2, 20, "ANIM WALK  FRAME %d ", (u16)cuadro);   /* u8 -> u16: ver README. */
    }
    else
    {
        consoleDrawText(2, 20, "ANIM IDLE  FRAME %d ", (u16)cuadro);
    }
    consoleDrawText(2, 22, "VBLANK LINES %d/37 ", vb_lineas); /* La última subida.   */
}

int main(void)
{
    const t_metasprite *tabla;                         /* La tabla del cuadro a dibujar.    */
    u8 *tiles;                                         /* La tira de ese cuadro, en la ROM. */

    /*--- 1. Texto (como el ejemplo del SDK): fuente en 0x3000, mapa en 0x6800 ------*/
    consoleSetTextMapPtr(0x6800);                      /* Mapa de la capa de texto.          */
    consoleSetTextGfxPtr(0x3000);                      /* Tiles de la fuente.                */
    consoleSetTextOffset(0x0100);                      /* Primer tile de la fuente en el mapa.*/
    consoleInitText(0, 16 * 2, (u8 *)&fuente_til, (u8 *)&fuente_pal);  /* Carga fuente y paleta.*/
    {
        setPaletteColor(0, RGB5(0, 0, 0));             /* El color 0 de la fuente es el     */
    }                                                  /* magenta clave del PNG, y el color */
                                                       /* 0 de la CGRAM es el fondo: negro. */
    bgSetGfxPtr(0, 0x2000);                            /* BG1: tiles...                      */
    bgSetMapPtr(0, 0x6800, SC_32x32);                  /* ...y mapa (el de la consola).      */
    setMode(BG_MODE1, 0);                              /* Mode 1.                            */
    bgSetDisable(1);                                   /* Solo BG1 (texto) y sprites.        */
    bgSetDisable(2);

    /*--- 2. El motor dinámico: piezas grandes (32x32) desde VRAM 0x0000, pequeñas
           (16x16, sin uso aquí) desde 0x1000; entradas de la OAM desde la 0. ---------*/
    oamInitDynamicSprite(0x0000, 0x1000, 0, 0, OBJ_SIZE16_L32);
    setPalette((u8 *)&tarou_pal, 128 + 0 * 16, 16 * 2); /* Paleta de sprites 0 (CGRAM 128).*/

    consoleDrawText(2, 2, "UNIT 09 - METASPRITE 96X64");  /* Texto en pantalla: inglés.   */
    consoleDrawText(2, 4, "LEFT/RIGHT: WALK");

    /*--- 3. Estado inicial (las static NO empiezan a 0) ----------------------------*/
    x = 80;                                            /* Centro aproximado.                 */
    anim = ANIM_REPOSO;
    cuadro = 0;
    frames = 0;
    izquierda = 0;
    ult_anim = NADA;                                   /* Fuerza la primera copia de tiles.  */
    ult_cuadro = 0;
    ult_izq = 0;
    texto_pendiente = 0;
    vb_lineas = 0;

    WaitForVBlank();                                   /* El texto de arriba (2 KB) sale en */
                                                       /* este VBlank, antes que Tarou.     */
    setScreenOn();                                     /* Encender la pantalla.              */

    while (1)
    {
        /*--- 4. Mando: la cruceta elige animación y dirección -----------------------*/
        pad = padsCurrent(0);                          /* Macro: pad_keys[0].                */
        if (pad & KEY_RIGHT)
        {
            izquierda = 0;
            if (x < X_MAX) x++;                        /* 1 px por frame, con tope.          */
            if (anim != ANIM_CAMINAR) { anim = ANIM_CAMINAR; cuadro = 0; frames = 0; }
        }
        else if (pad & KEY_LEFT)
        {
            izquierda = 1;
            if (x > X_MIN) x--;
            if (anim != ANIM_CAMINAR) { anim = ANIM_CAMINAR; cuadro = 0; frames = 0; }
        }
        else if (anim != ANIM_REPOSO)
        {
            anim = ANIM_REPOSO; cuadro = 0; frames = 0; /* Cambio de animación: cuadro 0.  */
        }

        /*--- 5. Avanzar el cuadro (bucle) ------------------------------------------*/
        frames++;
        if (anim == ANIM_CAMINAR)
        {
            if (frames >= CAMINAR_FRAMES) { frames = 0; cuadro++; if (cuadro >= CAMINAR_CUADROS) cuadro = 0; }
            tabla = tarou_caminar_metasprites[cuadro]; /* La tabla del cuadro...             */
            tiles = (u8 *)&tarou_caminar_til;          /* ...y la tira de la que salen sus   */
        }                                              /* bloques.                           */
        else
        {
            if (frames >= REPOSO_FRAMES) { frames = 0; cuadro++; if (cuadro >= REPOSO_CUADROS) cuadro = 0; }
            tabla = tarou_reposo_metasprites[cuadro];
            tiles = (u8 *)&tarou_reposo_til;
        }

        /*--- 6. ¿Hay que volver a copiar tiles? Solo si cambió el cuadro (o la
               animación, o el espejo). oamrefresh = 1 en la primera entrada se lo
               pide al motor para todo el metasprite. -------------------------------*/
        if (anim != ult_anim || cuadro != ult_cuadro || izquierda != ult_izq)
        {
            oambuffer[0].oamrefresh = 1;
            ult_anim = anim;
            ult_cuadro = cuadro;
            ult_izq = izquierda;
            texto_pendiente = 1;                       /* El texto, en OTRO frame: ver abajo.*/
        }
        else if (texto_pendiente)                      /* Frame sin subida de tiles: ahora   */
        {                                              /* sí cabe la copia del texto.        */
            texto();
            texto_pendiente = 0;
        }
        if (izquierda)
        {
            espejar(tabla);                            /* Tabla espejada en la RAM.          */
            tabla = espejo;
        }

        /*--- 7. Dibujar: una entrada de la OAM por pieza, desde la 0 --------------------*/
        oamMetaDrawDyn32(0, x, Y_TAROU, (u8 *)tabla, tiles);

        oamInitDynamicSpriteEndFrame();                /* Fin del frame para el motor.       */
        WaitForVBlank();                               /* Esperar al VBlank...               */
        oamVramQueueUpdate();                          /* ...y copiar los tiles encolados.   */
        if (texto_pendiente)                           /* Frame con subida: ¿cuánto VBlank   */
        {                                              /* se llevó? (línea >= 225: dentro.)  */
            vb_lineas = linea_haz();
            if (vb_lineas >= LINEA_VBLANK) vb_lineas = vb_lineas - LINEA_VBLANK;
            else vb_lineas = vb_lineas + LINEAS_VBLANK; /* Ya dibujando: se pasó del VBlank.*/
        }
    }
    return 0;                                          /* Nunca se llega aquí.               */
}
