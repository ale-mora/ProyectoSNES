/*---------------------------------------------------------------------------------
    tarou_sprite.c — Dibujo de Tarou en el nivel (ver tarou_sprite.h).

    PROPÓSITO
      Cada frame: una entrada de la OAM por pieza de la tabla metasprite del
      cuadro que se MUESTRA (hasta 6, de 32x32), colocadas con sus dx/dy;
      mirando a la izquierda, cada pieza cambia de lado (dx pasa a 64 - dx) y
      se voltea, y sigue mostrando su mismo bloque. Las entradas que el cuadro
      no usa se esconden.

      Cuando la lógica cambia de cuadro, sus 3 KB de bloques tienen que subir a
      la VRAM, y 3 KB de DMA son 24-30 líneas de las 37 del VBlank: demasiado
      para compartirlo con el fondo y el HUD. Por eso la VRAM tiene DOS celdas
      ("búferes": tiles 0..127 y 128..255) y la subida se reparte en dos
      VBlanks de 1,5 KB como mucho: mientras la OAM enseña el cuadro viejo
      desde un búfer, el nuevo se escribe en el otro; cuando está entero, la
      OAM pasa a enseñarlo. El sprite va un frame por detrás de la lógica (la
      posición no: solo el dibujo del cuadro), que no se nota. El plan de
      copias lo hace metasprite_copias() (lógica pura); aquí se apuntan por
      partes en la lista de DMA (dma_lista.h) que dma_ejecutar.asm dispara en
      el VBlank.

      Lo que se dibuja no siempre es el estado de la lógica: PARADO con la
      espada fuera se dibuja con la tira de GUARDIA (la pose fija Fight_Pose de
      Pixellab). Guardia no es un estado de tarou.c, solo un dibujo: por eso
      tiene el número DIBUJO_GUARDIA = ESTADO_CANTIDAD (13), el siguiente a los
      estados, en las tablas de este archivo y en tarou_til_bancos.

    SE CONECTA CON
      Incluye:  juego/tarou_sprite.h, juego/animaciones.h, juego/metasprite.h,
                juego/dma_lista.h, sistema/comun.h (VRAM_SPRITES),
                sistema/puntofijo.h y los 14 tarou_<anim>_meta.inc que genera
                gfx4snes -T (por la ruta -Ires/personajes/tarou del Makefile).
                Esos .inc se incluyen SOLO aquí: definen tablas const con nombre
                global (personajes.c incluye el suyo, tarou_reposo_estatico_meta.inc).
      Lee de la ROM: tarou_<anim>_til (14), tarou_pal y tarou_til_bancos (data.asm).
      Lo usan:  pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      OAM: oamInitGfxAttr, oamSet, oamSetEx, oamSetXY, oamClear (todo va al
      buffer que la NMI del SDK copia a la OAM en cada VBlank). VRAM: tiles
      0..255 desde VRAM_SPRITES (dos celdas de 3x2 bloques), por la lista de
      DMA. CGRAM: dmaCopyCGram a la paleta de sprites 0.

    VARIABLES (estáticas de este archivo)
      mostrado_estado, mostrado_cuadro  u8  el cuadro que enseña la OAM (0xFF = ninguno)
      bufer_mostrado   u8               0 o 1: en qué celda de la VRAM está ese cuadro
      copias           CopiaBloque[16]  el plan de copias del cuadro que se está subiendo
      n_copias         u8               cuántas son (0 = no hay subida en marcha)
      copiadas         u8               cuántas ya se apuntaron en la lista
      subiendo_estado, subiendo_cuadro  u8  qué cuadro se está subiendo
      banco_subiendo   u8               el banco de la ROM de su tira
      dibujo_estado, dibujo_cuadro  u8  la tira y el cuadro que tocan este frame (elegir_dibujo)
    VARIABLES (constantes)
      BYTES_POR_VBLANK 1536  tope de bytes de Tarou por VBlank (media celda: unas 9 líneas)
    → glosario: Metasprite, OAM, VRAM, DMA / HDMA, VBlank
---------------------------------------------------------------------------------*/
#include "juego/tarou_sprite.h"
#include "juego/animaciones.h"
#include "juego/metasprite.h"
#include "juego/dma_lista.h"
#include "sistema/comun.h"
#include "sistema/puntofijo.h"

/*--- Tablas metasprite generadas (una por animación): tarou_<anim>_metasprites[n] ---*/
#include "tarou_reposo_meta.inc"
#include "tarou_caminar_meta.inc"
#include "tarou_correr_meta.inc"
#include "tarou_saltar_meta.inc"
#include "tarou_caer_meta.inc"
#include "tarou_agachado_meta.inc"
#include "tarou_agachado_caminar_meta.inc"
#include "tarou_desenvainar_meta.inc"
#include "tarou_envainar_meta.inc"
#include "tarou_atacar_meta.inc"
#include "tarou_echarse_meta.inc"
#include "tarou_dormido_meta.inc"
#include "tarou_despertar_meta.inc"
#include "tarou_guardia_meta.inc"    /* Unidad 18: la pose de guardia (1 cuadro).          */

/*--- Tiles, paleta y bancos en ROM (data.asm) -------------------------------------------*/
extern char tarou_reposo_til, tarou_caminar_til, tarou_correr_til, tarou_saltar_til;
extern char tarou_caer_til, tarou_agachado_til, tarou_agachado_caminar_til;
extern char tarou_desenvainar_til, tarou_envainar_til, tarou_atacar_til;
extern char tarou_echarse_til, tarou_dormido_til, tarou_despertar_til;
extern char tarou_guardia_til;       /* La pose de guardia (data.asm).                    */
extern char tarou_pal;
extern u8 tarou_til_bancos[];        /* El banco de cada tira, en el orden del enum Estado,
                                        y el de la guardia detrás (índice DIBUJO_GUARDIA). */

/*--- Constantes ------------------------------------------------------------------------*/
#define OAM_TAROU 0                  /* Primera entrada de la OAM; las piezas usan 0..5.  */
#define PALETA_TAROU 0               /* Paleta de sprites 0 = colores 128..143 de la CGRAM.*/
#define PRIORIDAD_TAROU 3            /* Por delante de todos los fondos.                  */
#define TILES_POR_BUFER 128          /* Una celda de 3x2 bloques: 8 filas de 16 tiles.    */
#define VRAM_BUFER(b) ((u16)(VRAM_SPRITES + ((u16)(b) << 11)))  /* 128 tiles x 16 palabras. */
#define BYTES_POR_VBLANK 1536        /* Media celda por VBlank: unas 9 líneas de DMA.     */
/* El arte de Pixellab es una vista lateral un poco elevada: las patas del lado lejano
   acaban más arriba que las cercanas. Con los pies justo en el borde del suelo, Tarou
   parecía flotar; dibujado 2 px más abajo que su posición física, los pies quedan
   DENTRO de la banda de hierba (que empieza 2 px por encima del borde: 4 px en total).
   Solo es dibujo: la física y las pruebas usan el borde exacto de las celdas. */
#define HUNDIR_PIES 2
#define SIN_ESTADO 0xFF              /* "Todavía no hay ningún cuadro en la OAM".         */
#define DIBUJO_GUARDIA ESTADO_CANTIDAD   /* 13: una tira que no es un estado (ver arriba).    */

/*--- Estado ----------------------------------------------------------------------------*/
static u8 mostrado_estado, mostrado_cuadro, bufer_mostrado;
static CopiaBloque copias[METASPRITE_COPIAS_MAX];   /* El plan de copias del cuadro en subida. */
static u8 n_copias, copiadas, subiendo_estado, subiendo_cuadro, banco_subiendo;
static u8 dibujo_estado, dibujo_cuadro;          /* Qué tira y qué cuadro tocan ahora.    */
static u8 i, n;
static u16 bytes;

const t_metasprite *tarou_sprite_tabla(u8 estado, u8 cuadro)
{
    switch (estado)
    {
        case ESTADO_PARADO:             return tarou_reposo_metasprites[cuadro];
        case ESTADO_CAMINANDO:          return tarou_caminar_metasprites[cuadro];
        case ESTADO_CORRIENDO:          return tarou_correr_metasprites[cuadro];
        case ESTADO_SALTANDO:           return tarou_saltar_metasprites[cuadro];
        case ESTADO_CAYENDO:            return tarou_caer_metasprites[cuadro];
        case ESTADO_AGACHADO:           return tarou_agachado_metasprites[cuadro];
        case ESTADO_AGACHADO_CAMINANDO: return tarou_agachado_caminar_metasprites[cuadro];
        case ESTADO_DESENVAINANDO:      return tarou_desenvainar_metasprites[cuadro];
        case ESTADO_ENVAINANDO:         return tarou_envainar_metasprites[cuadro];
        case ESTADO_ATACANDO:           return tarou_atacar_metasprites[cuadro];
        case ESTADO_ECHANDOSE:          return tarou_echarse_metasprites[cuadro];
        case ESTADO_DORMIDO:            return tarou_dormido_metasprites[cuadro];
        case ESTADO_DESPERTANDO:        return tarou_despertar_metasprites[cuadro];
        case DIBUJO_GUARDIA:            return tarou_guardia_metasprites[cuadro];
        default:                        return tarou_reposo_metasprites[0];
    }
}

u8 *tarou_sprite_tiles(u8 estado)
{
    switch (estado)
    {
        case ESTADO_PARADO:             return (u8 *)&tarou_reposo_til;
        case ESTADO_CAMINANDO:          return (u8 *)&tarou_caminar_til;
        case ESTADO_CORRIENDO:          return (u8 *)&tarou_correr_til;
        case ESTADO_SALTANDO:           return (u8 *)&tarou_saltar_til;
        case ESTADO_CAYENDO:            return (u8 *)&tarou_caer_til;
        case ESTADO_AGACHADO:           return (u8 *)&tarou_agachado_til;
        case ESTADO_AGACHADO_CAMINANDO: return (u8 *)&tarou_agachado_caminar_til;
        case ESTADO_DESENVAINANDO:      return (u8 *)&tarou_desenvainar_til;
        case ESTADO_ENVAINANDO:         return (u8 *)&tarou_envainar_til;
        case ESTADO_ATACANDO:           return (u8 *)&tarou_atacar_til;
        case ESTADO_ECHANDOSE:          return (u8 *)&tarou_echarse_til;
        case ESTADO_DORMIDO:            return (u8 *)&tarou_dormido_til;
        case ESTADO_DESPERTANDO:        return (u8 *)&tarou_despertar_til;
        case DIBUJO_GUARDIA:            return (u8 *)&tarou_guardia_til;
        default:                        return (u8 *)&tarou_reposo_til;
    }
}

/*--- elegir_dibujo: la tira y el cuadro que corresponden al estado de la lógica ------
    Casi siempre son el estado y el cuadro de Tarou. Excepción: PARADO con la espada
    fuera se dibuja con la pose de guardia, que tiene un solo cuadro (el 0), así que
    el reposo puede seguir contando sus 9 cuadros sin que la guardia cambie. */
static void elegir_dibujo(const Tarou *t)
{
    if (t->estado == ESTADO_PARADO && t->espada == ESPADA_FUERA)
    {
        dibujo_estado = DIBUJO_GUARDIA;              /* Quieto y en guardia.               */
        dibujo_cuadro = 0;                           /* Pose fija: siempre el cuadro 0.    */
    }
    else
    {
        dibujo_estado = t->estado;                   /* Lo normal: una tira por estado.    */
        dibujo_cuadro = t->cuadro;
    }
}

/*--- esconder: una entrada de la OAM fuera de la pantalla ---------------------------
    OBJ_HIDE pone el bit 8 de x (x = 256, fuera por la derecha); además y = 224,
    porque con y = 240 (el truco clásico) un sprite grande aún asoma por arriba. */
static void esconder(u8 entrada)
{
    oamSetEx((u16)entrada * 4, OBJ_LARGE, OBJ_HIDE);
    oamSetXY((u16)entrada * 4, 256, 224);
}

void tarou_sprite_cargar(void)
{
    /* Tiles de sprites desde VRAM_SPRITES; tamaños "pequeño" 16x16 y "grande" 32x32:
       todas las piezas de Tarou son grandes. */
    oamInitGfxAttr(VRAM_SPRITES, OBJ_SIZE16_L32);
    dmaCopyCGram((u8 *)&tarou_pal, 128 + 16 * PALETA_TAROU, 16 * 2);   /* Paleta 0.    */
    mostrado_estado = SIN_ESTADO;                    /* Nada en la OAM todavía...          */
    mostrado_cuadro = 0;
    bufer_mostrado = 1;                              /* ...el primer cuadro irá al búfer 0. */
    n_copias = 0;                                    /* Ninguna subida en marcha.           */
    copiadas = 0;
    for (i = 0; i < METASPRITE_PIEZAS_MAX; i++)
    {
        esconder((u8)(OAM_TAROU + i));               /* Nada en pantalla hasta dibujar.     */
    }
}

void tarou_sprite_refrescar(void)
{
    mostrado_estado = SIN_ESTADO;                    /* El próximo dibujo vuelve a subirlo. */
    n_copias = 0;                                    /* Olvida cualquier subida a medias.   */
}

u8 tarou_sprite_al_dia(const Tarou *t)
{
    elegir_dibujo(t);                                /* Comparar con lo que se DIBUJARÍA.   */
    return (u8)(n_copias == 0 && dibujo_estado == mostrado_estado && dibujo_cuadro == mostrado_cuadro);
}

/*--- apuntar_copias: hasta BYTES_POR_VBLANK de la subida en marcha, a la lista -------
    Devuelve 1 si con esto la subida queda completa. */
static u8 apuntar_copias(void)
{
    bytes = 0;
    while (copiadas < n_copias)
    {
        if (bytes != 0 && (u16)(bytes + copias[copiadas].bytes) > BYTES_POR_VBLANK)
        {
            return 0;                                /* Lo que queda, al VBlank siguiente.  */
        }
        bytes = (u16)(bytes + copias[copiadas].bytes);
        dma_lista_anadir(copias[copiadas].origen, copias[copiadas].vram, copias[copiadas].bytes,
                         banco_subiendo, DMA_VMAIN_TILES);
        copiadas++;
    }
    return 1;                                        /* Todo apuntado: entero tras el VBlank. */
}

void tarou_sprite_dibujar(const Tarou *t, u16 camara)
{
    const t_metasprite *tabla;
    u16 x = (u16)(PF_ENTERO(t->x_fp) - camara);     /* Pantalla = mundo - cámara.         */
    u16 y = (u16)(PF_ENTERO(t->y_fp) + HUNDIR_PIES); /* No hay scroll vertical. Se dibuja
                                                        2 px más abajo que la física: ver
                                                        HUNDIR_PIES arriba.                */
    u16 dx, tile, base;
    u8 izquierda = (u8)(t->direccion == DIR_IZQUIERDA);

    /*--- 1. La subida de tiles: seguir la que hay en marcha o empezar una nueva. Cada
           frame apunta como mucho media celda (BYTES_POR_VBLANK); cuando la subida
           queda completa, la OAM pasa a enseñar ese cuadro desde el otro búfer. ----*/
    elegir_dibujo(t);                                /* Qué tira y cuadro tocan este frame. */
    if (n_copias == 0 && (dibujo_estado != mostrado_estado || dibujo_cuadro != mostrado_cuadro))
    {
        /* Cuadro nuevo: su plan de copias, hacia el búfer que NO se muestra. El origen
           es la dirección de la tira dentro de su banco (los 16 bits bajos del puntero)
           y el banco viene de la tabla de data.asm. */
        subiendo_estado = dibujo_estado;
        subiendo_cuadro = dibujo_cuadro;
        banco_subiendo = tarou_til_bancos[dibujo_estado];
        n_copias = metasprite_copias(tarou_sprite_tabla(dibujo_estado, dibujo_cuadro),
                                     (u16)tarou_sprite_tiles(dibujo_estado),
                                     VRAM_BUFER(bufer_mostrado ^ 1), copias);
        copiadas = 0;
    }
    if (n_copias != 0 && apuntar_copias())
    {
        mostrado_estado = subiendo_estado;           /* Tras este VBlank estará entero:     */
        mostrado_cuadro = subiendo_cuadro;           /* la OAM ya puede apuntar a él.       */
        bufer_mostrado ^= 1;
        n_copias = 0;
    }
    if (mostrado_estado == SIN_ESTADO)
    {
        return;                                      /* Aún no hay cuadro entero: escondido. */
    }

    /*--- 2. La OAM: una pieza por entrada de la tabla del cuadro MOSTRADO, en la
           posición de este frame. En espejo, cada pieza cambia de lado (dx 0 <-> 64,
           la del medio se queda), se voltea y muestra su MISMO bloque; el tile es
           fijo dentro de la celda: fila * 64 + columna * 4, más el búfer. ----------*/
    tabla = tarou_sprite_tabla(mostrado_estado, mostrado_cuadro);
    base = bufer_mostrado ? TILES_POR_BUFER : 0;
    n = metasprite_piezas(tabla);                    /* 0..6: para en el terminador.        */
    for (i = 0; i < n; i++)
    {
        dx = izquierda ? (u16)(METASPRITE_ESPEJO_DX - tabla[i].dx) : tabla[i].dx;
        tile = (u16)(base + (((tabla[i].dy >> 5) << 6) | ((tabla[i].dx >> 5) << 2)));
        oamSet((u16)(OAM_TAROU + i) * 4, (u16)(x + dx), (u16)(y + tabla[i].dy),
               PRIORIDAD_TAROU, izquierda, 0, tile, PALETA_TAROU);
        oamSetEx((u16)(OAM_TAROU + i) * 4, OBJ_LARGE, OBJ_SHOW);   /* 32x32, visible.   */
    }
    for (; i < METASPRITE_PIEZAS_MAX; i++)
    {
        esconder((u8)(OAM_TAROU + i));               /* Pieza transparente: sin sprite.     */
    }
}

void tarou_sprite_ocultar(void)
{
    oamClear(0, 0);                                  /* 0 entradas = esconder las 128.      */
}
