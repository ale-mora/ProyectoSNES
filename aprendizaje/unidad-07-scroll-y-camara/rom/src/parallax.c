/*---------------------------------------------------------------------------------
    parallax.c — Mini-ROM de la unidad 07 (ESQUELETO del ejercicio: TODO 1..3;
    solución en ../../solucion/parallax.c): dos capas de
    512 px que se desplazan a distinta velocidad con la cruceta, una cámara
    recortada a un "nivel" de 2048 px y una columna del mapa-anillo pintada a mano.

    PROPÓSITO
      - Derecha/Izquierda mueven la cámara 2 px por frame (8 con Y mantenido),
        recortada a 0..CAMARA_MAX en s16, como camara_para (src/juego/camara.c).
      - Fondo 0 = capa CERCANA (los arbustos del título): scroll = cámara.
      - Fondo 1 = capa LEJANA (las montañas del nivel 1): scroll = cámara / 2.
        Las dos miden 512 px y el scroll de la PPU da la vuelta a los 512: se
        repiten solas a lo largo de los 2048 del "nivel".
      - Fondo 2 = texto (CAM, NEAR, FAR, COL, RING). Las dos líneas fijas, con
        consoleDrawText antes de encender; las tres que cambian, escritas a mano
        en un mapa de texto en la WRAM (como src/juego/hud_debug.c): una fila
        por frame, sin %d ni divisiones, subida por DMA (64 bytes) en el VBlank.
        Con consoleDrawText y %d, cada vuelta del bucle costaba unos 3 frames
        (la cámara iba a ~40 px/s en vez de 120) y la NMI copiaba el texto a
        medias: filas de frames distintos.
      - A pinta de rojo, en el mapa de 64x32 de la capa cercana, la columna que
        está en el centro de la pantalla. Como el mapa es un anillo, la raya
        vuelve a aparecer cada 512 px de cámara. Se escribe con VMAIN = 0x81
        (la dirección de la VRAM avanza 32 palabras = baja una fila del mapa),
        que ninguna función de PVSnesLib usa: por eso, registros a mano.
      Tal como está compila y mueve la capa cercana; el recorte, la capa lejana
      y la raya son los TODO (enunciado en ejercicio_07.md).

    SE CONECTA CON
      Incluye:  <snes.h> (PVSnesLib).
      Lee de la ROM: fuente_til/_pal, cerca_*, lejos_* (data.asm).
      Modelo:   src/juego/camara.c (recorte), src/juego/nivel.c (nivel_columna_vram),
                src/pantallas/pantalla_nivel1.c (paso 3: bgSetScroll de las dos capas),
                src/juego/fondo_nivel.c (las columnas, por DMA con VMAIN 0x81).
      Ejemplos del SDK: graphics/Backgrounds/Mode1Scroll, graphics/Effects/ParallaxScrolling.

    HARDWARE QUE TOCA
      PPU: modo y prioridad de BG3 (setMode), fondos (bgInitTileSet, bgInitMapSet,
      bgSetEnable), scroll (bgSetScroll → BG1HOFS/BG2HOFS $210D/$210F, dos
      escrituras de 8 bits). VRAM: un tile propio (dmaCopyVram) y la columna roja
      (REG_VMAIN $2115, REG_VMADDLH $2116, REG_VMDATALH $2118). CGRAM: cielo y
      rojo (setPaletteColor). Mando: padsCurrent/padsDown (los lee la NMI del SDK).

    VARIABLES (estáticas de este archivo)
      camara       u16  0..CAMARA_MAX px   borde izquierdo de la pantalla en el "nivel"
      texto_mapa   u16[96]  entradas       las 3 filas de texto que cambian (tilemap en WRAM)
      turno        u8   0..2               fila de texto que se prepara y sube en este frame
      columna      u16  0..255             columna del nivel que se pinta con A
      pintar       u8   0 / 1              1 = pintar `columna` en el próximo VBlank
      teclas       u16  bits               botones mantenidos este frame
      i            u8   0..31              fila de la columna
    → glosario: Cámara, Parallax, BG / capas, Streaming de columnas, Registro de doble escritura
---------------------------------------------------------------------------------*/
#include <snes.h>
#include <string.h>                  /* memset: la de la librería está en ensamblador.    */

/*--- Datos en la ROM (data.asm) -------------------------------------------------*/
extern char fuente_til, fuente_pal;
extern char cerca_til, cerca_til_end, cerca_pal, cerca_map, cerca_map_end;
extern char lejos_til, lejos_til_end, lejos_pal, lejos_map, lejos_map_end;

/*--- Reparto de la VRAM: el de src/sistema/comun.h (en palabras) --------------------*/
#define VRAM_FONDO_A 0x2000          /* Tiles de la capa cercana.                         */
#define VRAM_FONDO_B 0x4000          /* Tiles de la capa lejana.                          */
#define VRAM_FUENTE 0x6000           /* Fuente de 2bpp.                                   */
#define VRAM_MAPA_TEXTO 0x6800       /* Mapa 32x32 del texto.                             */
#define VRAM_MAPA_A 0x6C00           /* Mapa 64x32 de la cercana: el anillo.              */
#define VRAM_MAPA_B 0x7400           /* Mapa 64x32 de la lejana.                          */

/*--- El "nivel" y la cámara (los números de src/juego/nivel.h) ----------------------*/
#define ANCHO_NIVEL 2048             /* Como el tutorial: 8 pantallas.                    */
#define ANCHO_PANTALLA 256
#define CAMARA_MAX (ANCHO_NIVEL - ANCHO_PANTALLA)   /* 1792.                              */
#define PASO 2                       /* px por frame (Tarou corre a unos 2).              */
#define PASO_RAPIDO 8                /* Con Y: para ver la vuelta de los 512 px antes.    */

/*--- La raya roja: un tile macizo propio y su entrada de mapa ------------------------*/
#define TILE_RAYA 511                /* El último hueco de VRAM_FONDO_A (512 tiles).      */
#define PALETA_RAYA 3                /* Paleta de fondos 3: CGRAM 48..63, libre.          */
#define ENTRADA_RAYA ((TILE_RAYA) | ((PALETA_RAYA) << 10))   /* Tile 511, paleta 3.       */
#define VMAIN_COLUMNA 0x81           /* Avanza tras el byte alto, de 32 en 32 palabras.   */
#define VMAIN_NORMAL 0x80            /* Avanza de 1 en 1: lo que esperan las del SDK.     */

/* Un tile de 4bpp con todos sus píxeles en el índice 1: plano 0 a unos, resto a 0. */
static const u8 tile_macizo[32] = {
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/*--- El texto que cambia: 3 filas escritas a mano (como src/juego/hud_debug.c) -----*/
#define FILA_TEXTO 4                 /* Primera fila de pantalla del texto que cambia.    */
#define FILAS_TEXTO 3                /* CAM / NEAR FAR / COL RING.                        */
/* Tile de un carácter: la fuente empieza en ' ' (ASCII 32) = tile 0; paleta 0 y el bit
   de prioridad (0x2000) para ir delante de las otras capas, como el HUD del juego. */
#define TILE(c) ((u16)(((c) - 32) | 0x2000))

/*--- Estado -------------------------------------------------------------------------*/
static u16 camara, columna, teclas;
static u8 pintar, i, turno;
static u16 texto_mapa[FILAS_TEXTO * 32];   /* Las 3 filas, seguidas, como en la VRAM.    */
static const u16 POTENCIAS[4] = {10000, 1000, 100, 10};
static u16 columna_vram(u16 col);                    /* Definida más abajo.               */

/*--- texto: escribe una cadena en p y devuelve dónde sigue -------------------------*/
static u16 *texto(u16 *p, const char *s)
{
    while (*s != 0)
    {
        *p++ = TILE(*s);                             /* Un carácter = una celda.          */
        s++;
    }
    return p;
}

/*--- numero: un u16 en decimal, SIN dividir (restas por potencia de diez, como
    numero() de hud_debug.c): como mucho 9 restas por cifra. --------------------------*/
static u16 *numero(u16 *p, u16 v)
{
    const u16 *pot = POTENCIAS;                      /* Se recorre con puntero, no [k].    */
    u8 cifra;
    u8 empezado = 0;                                 /* Sin ceros a la izquierda.          */
    u8 k;

    for (k = 4; k != 0; k--, pot++)
    {
        cifra = '0';
        while (v >= *pot)
        {
            v -= *pot;                               /* Una resta = una unidad de la cifra.*/
            cifra++;
        }
        if (cifra != '0' || empezado)
        {
            *p++ = TILE(cifra);
            empezado = 1;
        }
    }
    *p++ = TILE('0' + v);                            /* Las unidades, siempre.             */
    return p;
}

/*--- preparar_fila: la fila `turno` del texto, con los valores de este frame -------
    Se borra entera (memset: 0 = espacio) y se reescribe: así un número que se acorta
    no deja cifras viejas detrás. Una fila por frame: las tres juntas, como hace el
    HUD del juego, no merecen el riesgo de pasarse del frame. */
static void preparar_fila(void)
{
    u16 *p = texto_mapa + ((u16)turno << 5);         /* Fila turno = palabra turno * 32.   */
    u16 col = camara >> 3;                           /* Columna del nivel (8 px).          */

    memset(p, 0, 32 * 2);                            /* 32 espacios.                       */
    p++;                                             /* Columna 1 de la pantalla.          */
    if (turno == 0)
    {
        p = texto(p, "CAM ");
        p = numero(p, camara);
    }
    else if (turno == 1)
    {
        p = texto(p, "NEAR ");
        p = numero(p, camara & 511);                 /* Lo que usa la PPU de la cercana.   */
        p = texto(p, "  FAR ");
        p = numero(p, (camara >> 1) & 511);          /* Y de la lejana.                    */
    }
    else
    {
        p = texto(p, "COL ");
        p = numero(p, col);                          /* 0..224 a la izquierda.             */
        p = texto(p, "  RING ");
        p = numero(p, col & 63);                     /* Columna del anillo: 0..63.         */
        p = texto(p, "  +");
        p = numero(p, columna_vram(col));            /* Palabra en el mapa: 0..31 o 1024..1055. */
    }
}

/*--- columna_vram: columna del nivel → palabra de su fila 0 en el mapa de 64x32 -----
    La misma cuenta que nivel_columna_vram (src/juego/nivel.c): (col & 32) dice la
    página (0 o 0x400 palabras más), (col & 31) la columna dentro de ella. */
static u16 columna_vram(u16 col)
{
    return (u16)(((col & 32) << 5) | (col & 31));
}

/*--- pintar_columna: las 32 filas de una columna del anillo, de rojo --------------
    Solo en el VBlank. PVSnesLib no sirve aquí: dmaCopyVram pone
    siempre VMAIN = 0x80 (avanzar 1 palabra: por filas). Con 0x81 cada palabra
    escrita baja una fila. El juego hace lo mismo por DMA (src/juego/dma_ejecutar.asm,
    entradas con DMA_VMAIN_COLUMNA); aquí, 32 escrituras de la CPU desde C. */
static void pintar_columna(u16 col)
{
    /* TODO 3: con REG_VMAIN = VMAIN_COLUMNA, poner en REG_VMADDLH la dirección de
       la fila 0 de la columna (VRAM_MAPA_A + columna_vram(col)) y escribir
       ENTRADA_RAYA 32 veces en REG_VMDATALH. Al terminar, dejar REG_VMAIN como
       lo esperan las funciones del SDK (VMAIN_NORMAL). */
    (void)col;                                       /* Quitar al resolver el TODO.      */
}

/*--- mover_camara: cruceta → cámara recortada al nivel -----------------------------*/
static void mover_camara(void)
{
    s16 c = (s16)camara;                             /* En s16: puede bajar de 0.         */
    u16 paso = (teclas & KEY_Y) ? PASO_RAPIDO : PASO;
    if (teclas & KEY_RIGHT)
    {
        c = (s16)(c + paso);
    }
    if (teclas & KEY_LEFT)
    {
        c = (s16)(c - paso);
    }
    /* TODO 1: recortar c a 0..CAMARA_MAX, como camara_para (src/juego/camara.c).
       Sin el recorte, Izquierda al principio da c = -2 y camara = 65534. */
    camara = (u16)c;
}

int main(void)
{
    /*--- 1. Texto en el fondo 2 (BG3, 4 colores), como el título y el HUD -----------*/
    bgSetGfxPtr(2, VRAM_FUENTE);
    bgSetMapPtr(2, VRAM_MAPA_TEXTO, SC_32x32);
    consoleSetTextGfxPtr(VRAM_FUENTE);
    consoleSetTextMapPtr(VRAM_MAPA_TEXTO);
    consoleSetTextOffset(0);
    consoleInitText(0, 2 * 2, (u8 *)&fuente_til, (u8 *)&fuente_pal);

    /*--- 2. Las dos capas, con mapas de 64x32 ---------------------------------------*/
    bgInitTileSet(0, (u8 *)&cerca_til, (u8 *)&cerca_pal, 1,
                  (u16)(&cerca_til_end - &cerca_til), 16 * 2, BG_16COLORS, VRAM_FONDO_A);
    bgInitMapSet(0, (u8 *)&cerca_map, (u16)(&cerca_map_end - &cerca_map), SC_64x32, VRAM_MAPA_A);
    bgInitTileSet(1, (u8 *)&lejos_til, (u8 *)&lejos_pal, 2,
                  (u16)(&lejos_til_end - &lejos_til), 16 * 2, BG_16COLORS, VRAM_FONDO_B);
    bgInitMapSet(1, (u8 *)&lejos_map, (u16)(&lejos_map_end - &lejos_map), SC_64x32, VRAM_MAPA_B);

    /*--- 3. El tile de la raya y sus colores; el cielo -------------------------------*/
    dmaCopyVram((u8 *)tile_macizo, VRAM_FONDO_A + TILE_RAYA * 16, 32);   /* 16 palabras. */
    setPaletteColor(PALETA_RAYA * 16 + 1, RGB5(31, 4, 4));               /* Rojo.        */
    setPaletteColor(0, RGB5(12, 20, 31));            /* Color de fondo: cielo (sin HDMA). */

    /*--- 4. Mode 1 con BG3 delante, tres capas --------------------------------------*/
    setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);
    bgSetEnable(0);
    bgSetEnable(1);
    bgSetEnable(2);

    consoleDrawText(1, 1, "UNIT 07 - SCROLL AND PARALLAX");
    consoleDrawText(1, 2, "LEFT/RIGHT MOVE Y FAST A PAINT");   /* 30 letras: cabe en 31. */
    camara = 0;
    columna = 0;
    pintar = 0;
    turno = 0;                                       /* static NO empieza a 0: se pone.   */
    setScreenOn();

    while (1)
    {
        /*--- Lógica del frame ---------------------------------------------------*/
        teclas = padsCurrent(0);
        mover_camara();
        if (padsDown(0) & KEY_A)
        {
            columna = (u16)((camara + ANCHO_PANTALLA / 2) >> 3); /* Centro de pantalla. */
            pintar = 1;
        }
        preparar_fila();                             /* Una fila de texto, en la WRAM.    */

        WaitForVBlank();                             /* Lo que sigue, ya en el VBlank.     */

        /*--- Registros: scroll y columna, al principio del VBlank ---------------
            bgSetScroll escribe los registros en el acto: si se llamara a mitad del
            dibujo, la parte de arriba de la pantalla saldría con el scroll viejo. */
        bgSetScroll(0, camara, 0);                   /* Cercana: 1:1 con la cámara.        */
        /* TODO 2: la capa lejana (fondo 1), a la mitad de la cámara. */
        /* La fila de texto preparada: 64 bytes por DMA a su fila del mapa de texto. */
        dmaCopyVram((u8 *)(texto_mapa + ((u16)turno << 5)),
                    VRAM_MAPA_TEXTO + ((u16)(FILA_TEXTO + turno) << 5), 32 * 2);
        if (pintar)
        {
            pintar_columna(columna);
            pintar = 0;
        }
        turno++;                                     /* La siguiente fila, en el próximo frame. */
        if (turno == FILAS_TEXTO)
        {
            turno = 0;
        }
    }
    return 0;
}
