/*---------------------------------------------------------------------------------
    tarou_sprite.h — Dibujo de Tarou en el nivel: la OAM y los tiles de su cuadro.

    PROPÓSITO
      Es la capa de hardware del protagonista: traduce (estado, cuadro,
      dirección, posición) a seis entradas de la OAM y, cuando el cuadro cambia,
      apunta en la lista de DMA (juego/dma_lista.h) las copias que suben sus
      tiles a la VRAM, repartidas en los dos VBlanks siguientes (3 KB son unas
      18 líneas calculadas: media celda por VBlank; medido en el HUD, V5 17..20
      en esos frames). Hasta el 2026-10-20 lo hacía el motor de sprites
      dinámicos del SDK; su copia de 3 KB costaba 32 líneas de las 37 del
      VBlank y, con las columnas del fondo, se salía (V5 46): los píxeles que
      la PPU descartaba dejaban a Tarou a trozos.

    SE CONECTA CON
      Incluye:  <snes.h>, juego/tarou.h.
      Lo usan:  pantallas/pantalla_nivel1.c (cargar al entrar; dibujar cada
                frame, antes de fondo_nivel_actualizar, para que las copias de
                Tarou vayan primero en la lista; ocultar al salir),
                pantallas/personajes.c (tarou_sprite_tabla, tarou_sprite_tiles).

    HARDWARE QUE TOCA
      OAM: oamInitGfxAttr (tiles desde VRAM_SPRITES, tamaños 16/32), oamSet,
      oamSetEx, oamClear. VRAM: dos celdas de 3x2 bloques de 32x32 (tiles
      0..127 y 128..255), escritas por dma_ejecutar.asm; mientras una se
      muestra, la otra recibe el cuadro siguiente en dos VBlanks. CGRAM: la
      paleta de sprites 0.

    VARIABLES: ninguna (ver tarou_sprite.c).
    → glosario: Metasprite, OAM, VRAM, DMA / HDMA, VBlank
---------------------------------------------------------------------------------*/
#ifndef TAROU_SPRITE_H
#define TAROU_SPRITE_H

#include <snes.h>
#include "juego/tarou.h"

/* Prepara la OAM y carga la paleta de Tarou. Una vez al entrar en el nivel. */
void tarou_sprite_cargar(void);

/* Escribe en la OAM el cuadro de Tarou en coordenadas de pantalla (x del mundo
   menos la cámara) y, si el cuadro cambió, apunta en la lista de DMA la mitad de
   sus tiles (la otra mitad, al frame siguiente): el dibujo del cuadro va un frame
   por detrás de la lógica. Una vez por frame, después de dma_lista_vaciar(). */
void tarou_sprite_dibujar(const Tarou *t, u16 camara);

/* Obliga a que el próximo dibujar vuelva a subir los tiles (tras reaparecer). */
void tarou_sprite_refrescar(void);

/* 1 si la OAM ya enseña el cuadro actual de `t` con sus tiles enteros. Con la
   pantalla apagada se repite dibujar + dma_lista_ejecutar hasta que lo devuelva. */
u8 tarou_sprite_al_dia(const Tarou *t);

/* Esconde todos los sprites (al salir del nivel). */
void tarou_sprite_ocultar(void);

/* Tabla metasprite (mirando a la derecha) del cuadro `cuadro` del estado `estado`. */
const t_metasprite *tarou_sprite_tabla(u8 estado, u8 cuadro);

/* Tiles en ROM de la animación del estado `estado` (la etiqueta tarou_<anim>_til). */
u8 *tarou_sprite_tiles(u8 estado);

#endif /* TAROU_SPRITE_H */
