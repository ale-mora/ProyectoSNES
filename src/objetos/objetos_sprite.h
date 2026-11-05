/*---------------------------------------------------------------------------------
    objetos_sprite.h — Los objetos de la fase en la tabla de sprites.

    PROPÓSITO
      La capa de hardware de objetos/objetos.c: dos rompibles a la vista (4 piezas cada
      uno), el ítem del suelo, dos proyectiles, dos astillas y la chispa del golpe, en
      las entradas 15..22 de la tabla de sprites (juego/sprites.h), ordenados por la y de
      sus pies con los personajes, con la paleta de sprites 7 (la del humo y los pájaros).
      VRAM (desde el 2026-11-05 el par de filas 14 es de los objetos):
        tiles 448..455  el rompible de la zona (barril, caja o jarra: TL, TR, BL, BR)
        456             el ítem de comida del suelo (se sube al aparecer)
        458             astillas      460  shuriken     462  kunai
        492, 494        la chispa (par 15, tras la sombra y el humo)

    SE CONECTA CON
      Incluye:  sistema/tipos.h, objetos/objetos.h.
      Implementa: objetos/objetos_sprite.c.
      Lo usa:   pantallas/pantalla_fase1.c.
      Datos:    objetos_til, obj_pieza1, obj_pieza4, obj_banco (data.asm), objetos_datos.h.

    HARDWARE QUE TOCA
      VRAM de sprites (al cargar; el ítem de comida, por la lista de DMA). Ventanas 1 y 2
      (WH0..WH3 por HDMA, canal 5), WOBJSEL, WOBJLOG y CGWSEL: los objetos, fuera de la
      matemática de color (opacos).

    VARIABLES: ninguna (ver objetos_sprite.c).
    → glosario: Sprite, OAM
---------------------------------------------------------------------------------*/
#ifndef OBJETOS_SPRITE_H
#define OBJETOS_SPRITE_H

#include "sistema/tipos.h"
#include "objetos/objetos.h"

#define OBJETOS_ENT 15               /* Entradas 15..22.                                   */
#define OBJETOS_ENTIDADES (OBJETOS_ENT + 8)

/* CGWSEL de la fase: matemática de color solo FUERA de las ventanas de los objetos (bits
   5-4 = 2) con la pantalla secundaria de operando (bit 1). */
#define OBJETOS_CGWSEL 0x22

#define ROMPIBLE_BARRIL 0            /* Qué rompible usa la zona.                          */
#define ROMPIBLE_CAJA 1
#define ROMPIBLE_JARRA 2

/* Con la pantalla apagada: los tiles de la zona (su rompible) y los fijos, y las 8
   entradas apagadas. Activa OBJETOS_ENTIDADES entradas. */
void objetos_sprite_cargar(u8 rompible);

/* Un frame: coloca lo que se ve (y apunta en la lista de DMA el dibujo de un ítem nuevo)
   y prepara las ventanas que los dejan opacos. */
void objetos_sprite_actualizar(const Objetos *o, u16 camara);

/* En el VBlank: si hay tabla de ventanas nueva, el canal 5 pasa a leerla. */
void objetos_sprite_vblank(void);

#endif /* OBJETOS_SPRITE_H */
