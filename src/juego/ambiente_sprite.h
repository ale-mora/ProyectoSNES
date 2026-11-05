/*---------------------------------------------------------------------------------
    ambiente_sprite.h — El humo y los pájaros de la aldea en la tabla de sprites.

    PROPÓSITO
      La capa de hardware de juego/ambiente.c: carga sus tiles (4 piezas de 16x16:
      bocanada, penacho, alas arriba, alas abajo) y la paleta de sprites 7, y copia
      cada frame las 7 figuras a las entradas 8..14 de la tabla de sprites
      (juego/sprites.h), detrás de todo en el orden por profundidad (y = 0) y con
      prioridad 2 como los personajes. La paleta 7 entra en la media suma de color de
      la escena (protagonista_escena_cargar): el humo y los pájaros salen translúcidos
      sobre las montañas y las casas.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/ambiente.h.
      Implementa: juego/ambiente_sprite.c.
      Lo usa:   pantallas/pantalla_fase1.c (cargar después de ninja_sprite_cargar;
                actualizar antes de ninja_sprite_actualizar, que ordena la tabla).
      Datos:    ambiente_til, ambiente_pal, ambiente_pieza y ambiente_banco (data.asm,
                res/aldea/ambiente_*.bin).

    HARDWARE QUE TOCA
      VRAM de sprites (tiles 484..491 y 500..507: el par 15, detrás de la sombra) y
      CGRAM (paleta de sprites 7) al cargar; nada por frame (solo la tabla en la WRAM).

    VARIABLES: ninguna (ver ambiente_sprite.c).
    → glosario: Sprite, OAM, Matemática de color
---------------------------------------------------------------------------------*/
#ifndef AMBIENTE_SPRITE_H
#define AMBIENTE_SPRITE_H

#include "sistema/tipos.h"
#include "juego/ambiente.h"

#define AMBIENTE_ENT 8               /* Entradas 8..14 de la tabla de sprites.            */
#define AMBIENTE_ENTIDADES (AMBIENTE_ENT + AMBIENTE_FIGURAS)   /* 15: las que se usan.   */

/* Tiles y paleta a la VRAM y la CGRAM (con la pantalla apagada) y las 7 entradas,
   apagadas. Activa AMBIENTE_ENTIDADES entradas de la tabla. */
void ambiente_sprite_cargar(void);

/* Copia las figuras a sus entradas (posición, cuadro, si se ven). */
void ambiente_sprite_actualizar(const Ambiente *a);

#endif /* AMBIENTE_SPRITE_H */
