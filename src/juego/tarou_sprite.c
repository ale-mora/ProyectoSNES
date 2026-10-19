/*---------------------------------------------------------------------------------
    tarou_sprite.c — Dibujo de Tarou con el motor de sprites dinámicos del SDK.

    PROPÓSITO
      Traducir (estado, cuadro, dirección, posición) a una llamada
      oamMetaDrawDyn32(): el motor reserva una entrada de la OAM por pieza a
      partir de la 0, coloca las piezas de 32x32 según la tabla metasprite y, si
      el cuadro cambió (oamrefresh = 1), encola la copia de sus tiles a la VRAM,
      que oamVramQueueUpdate() ejecuta después del VBlank. Mirando a la izquierda
      se construye en RAM una copia de la tabla con las piezas intercambiadas y
      el bit de espejo horizontal (no hay tablas espejadas en la ROM). Un cuadro
      de 96x64 tiene HASTA seis piezas: gfx4snes -T omite las transparentes, y
      el recorrido para en el terminador (juego/metasprite.c).

    SE CONECTA CON
      Incluye:  juego/tarou_sprite.h, juego/animaciones.h, juego/metasprite.h,
                sistema/puntofijo.h y los 13 tarou_<anim>_meta.inc que genera
                gfx4snes -T (por la ruta -Ires/personajes/tarou del Makefile).
                Esos .inc se incluyen SOLO aquí: definen tablas const con nombre
                global (personajes.c incluye el suyo, tarou_reposo_estatico_meta.inc).
      Lee de la ROM: tarou_<anim>_til (13) y tarou_pal (data.asm).
      Lo usan:  pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      OAM: oamInitDynamicSprite, oambuffer[0].oamrefresh, oamMetaDrawDyn32,
      oamClear. VRAM: 0x0000 (piezas grandes de 32x32) y 0x1000 (pequeñas, sin
      uso), por el motor. CGRAM: dmaCopyCGram a la paleta de sprites 0.

    VARIABLES (globales y estáticas de este archivo)
      espejo[7]         t_metasprite  tabla del cuadro actual mirando a la izquierda (RAM):
                                      hasta 6 piezas + el terminador
      ultimo_estado     u8            estado dibujado en el frame anterior (0xFF = ninguno)
      ultimo_cuadro     u8            cuadro dibujado en el frame anterior
      ultima_direccion  u8            dirección dibujada en el frame anterior

    → glosario: Metasprite, Sprite dinámico, OAM, VRAM, DMA / HDMA
---------------------------------------------------------------------------------*/
#include "juego/tarou_sprite.h"
#include "juego/animaciones.h"
#include "juego/metasprite.h"
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

/*--- Tiles y paleta en ROM (data.asm) ------------------------------------------------*/
extern char tarou_reposo_til, tarou_caminar_til, tarou_correr_til, tarou_saltar_til;
extern char tarou_caer_til, tarou_agachado_til, tarou_agachado_caminar_til;
extern char tarou_desenvainar_til, tarou_envainar_til, tarou_atacar_til;
extern char tarou_echarse_til, tarou_dormido_til, tarou_despertar_til;
extern char tarou_pal;

/*--- Constantes ------------------------------------------------------------------------*/
#define OAM_TAROU 0                  /* Primera entrada de la OAM; el metasprite usa 0..5.*/
#define PALETA_TAROU 0               /* Paleta de sprites 0 = colores 128..143 de la CGRAM.*/
#define VRAM_DIN_GRANDES 0x0000      /* Piezas de 32x32 del motor dinámico.              */
#define VRAM_DIN_PEQUENAS 0x1000     /* Piezas de 16x16 (no se usan).                    */
#define SIN_ESTADO 0xFF              /* "Todavía no se dibujó nada".                     */

/*--- Estado ----------------------------------------------------------------------------*/
static t_metasprite espejo[METASPRITE_PIEZAS_MAX + 1];   /* Hasta 6 piezas + terminador. */
static u8 ultimo_estado, ultimo_cuadro, ultima_direccion;

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
        default:                        return (u8 *)&tarou_reposo_til;
    }
}

void tarou_sprite_cargar(void)
{
    /* Motor dinámico: piezas "grandes" de 32x32 desde 0x0000, "pequeñas" de 16x16
       desde 0x1000 (OBJ_SIZE16_L32). Las entradas de la OAM empiezan en la 0. */
    oamInitDynamicSprite(VRAM_DIN_GRANDES, VRAM_DIN_PEQUENAS, 0, 0, OBJ_SIZE16_L32);
    dmaCopyCGram((u8 *)&tarou_pal, 128 + 16 * PALETA_TAROU, 16 * 2);   /* Paleta 0.    */
    ultimo_estado = SIN_ESTADO;                      /* El primer dibujo copia los tiles.   */
    ultimo_cuadro = 0;
    ultima_direccion = DIR_DERECHA;
}

void tarou_sprite_dibujar(const Tarou *t, u16 camara)
{
    const t_metasprite *tabla = tarou_sprite_tabla(t->estado, t->cuadro);
    s16 x = (s16)(PF_ENTERO(t->x_fp) - camara);     /* Pantalla = mundo - cámara.         */
    s16 y = (s16)PF_ENTERO(t->y_fp);                /* No hay scroll vertical.            */

    /* Si cambió el cuadro (o el estado, o el espejo), hay que volver a copiar los
       tiles a la VRAM: el motor lo hace cuando ve oamrefresh = 1 en la entrada 0. */
    if (t->estado != ultimo_estado || t->cuadro != ultimo_cuadro || t->direccion != ultima_direccion)
    {
        oambuffer[OAM_TAROU].oamrefresh = 1;
        ultimo_estado = t->estado;
        ultimo_cuadro = t->cuadro;
        ultima_direccion = t->direccion;
    }

    /* Mirando a la izquierda: copia de la tabla con las piezas intercambiadas
       (dx 0 <-> 64; la columna del medio se queda) y el bit de espejo, con el
       terminador detrás de la última pieza que haya (pueden ser menos de seis):
       juego/metasprite.c. */
    if (t->direccion == DIR_IZQUIERDA)
    {
        metasprite_espejar(tabla, espejo);
        tabla = espejo;
    }

    oamMetaDrawDyn32(OAM_TAROU, x, y, (u8 *)tabla, tarou_sprite_tiles(t->estado));
}

void tarou_sprite_ocultar(void)
{
    oamClear(0, 0);                                  /* 0 entradas = esconder las 128.      */
}
