/*---------------------------------------------------------------------------------
    tarou_sprite.h — Cómo se dibuja Tarou: el metasprite dinámico de 64x64.

    PROPÓSITO
      Es la capa de hardware del protagonista: lee la struct Tarou (estado,
      cuadro, dirección, posición) y la convierte en entradas de la OAM y copias
      de tiles a la VRAM. La lógica (tarou.c) no sabe que existe este archivo.
      Técnica (docs/diseno-demo-jugable.md §7): cada cuadro de 64x64 son hasta
      cuatro sprites de 32x32 descritos por una tabla metasprite que genera
      gfx4snes -T (omite las piezas transparentes); el motor de sprites
      dinámicos del SDK copia a la VRAM solo el cuadro que se muestra, así los
      ~55 cuadros viven en la ROM.

    SE CONECTA CON
      Incluye:  <snes.h> (t_metasprite, oambuffer), juego/tarou.h.
      Lo usan:  pantallas/pantalla_nivel1.c (cargar, dibujar, ocultar). El
                dibujo estático de intro y título (pantallas/personajes.c) tiene
                su propia tira, tarou_reposo_estatico, y no pasa por aquí.

    HARDWARE QUE TOCA
      OAM (entradas 0..3 y el buffer del motor dinámico), VRAM (0x0000..0x1FFF,
      copias por DMA en cada VBlank que haga falta), CGRAM (paleta de sprites
      0, colores 128..143).

    VARIABLES: ninguna (ver tarou_sprite.c).
    → glosario: Metasprite, Sprite dinámico, OAM, VRAM, Dirección
---------------------------------------------------------------------------------*/
#ifndef TAROU_SPRITE_H
#define TAROU_SPRITE_H

#include <snes.h>
#include "juego/tarou.h"

/* Arranca el motor de sprites dinámicos y carga la paleta de Tarou. Una vez al
   entrar en el nivel (pisa lo que hubiera en la OAM). */
void tarou_sprite_cargar(void);

/* Escribe en la OAM el cuadro actual de Tarou, en coordenadas de pantalla
   (x del mundo menos la cámara). Llamar una vez por frame, antes de
   oamInitDynamicSpriteEndFrame(). */
void tarou_sprite_dibujar(const Tarou *t, u16 camara);

/* Esconde todos los sprites (al salir del nivel). */
void tarou_sprite_ocultar(void);

/* Tabla metasprite (mirando a la derecha) del cuadro `cuadro` del estado `estado`. */
const t_metasprite *tarou_sprite_tabla(u8 estado, u8 cuadro);

/* Tiles en ROM de la animación del estado `estado` (la etiqueta tarou_<anim>_til). */
u8 *tarou_sprite_tiles(u8 estado);

#endif /* TAROU_SPRITE_H */
