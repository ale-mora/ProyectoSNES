/*---------------------------------------------------------------------------------
    ninja_sprite.h — Dibuja a los ninjas básicos y sus sombras sobre la capa de sprites.

    PROPÓSITO
      Une las ranuras de enemigos/ninja.h con juego/sprites.h: cada ninja tiene su hueco
      fijo de tiles en la VRAM (HUECOS: tiles 192, 288 y 384 = pares de filas 6-8, 9-11 y
      12-14, con UN solo búfer), su paleta de sprites (1, 2 y 3: tres variantes del mismo
      arte, para distinguirlos) y dos entidades fijas: el cuerpo (entradas 2..4, ordenado
      por la y de sus pies con el protagonista) y la sombra (entradas 5..7, y = 0: detrás de
      todo; media suma de color con los fondos, paleta 4). Cuando cambia el cuadro de un
      ninja, su cuadro entero (≤ 3 KB) se sube en UN VBlank y la OAM pasa a él en ese mismo
      frame (ver docs/diseno-beat-em-up.md §8-§9). Como mucho UN ninja sube cuadro por
      frame (el reparto es por turnos): con 3 ninjas más Tarou el VBlank llega a la línea
      36 de 37 (§9.2); una segunda subida lo desbordaría.
      El arte es el provisional de la prueba de carga (el samurái humano del pack, 64×96).

    SE CONECTA CON
      Incluye:  sistema/tipos.h, enemigos/ninja.h.
      Lo usan:  pantallas/pantalla_fase1.c.
      Lee de la ROM: ninja64_andar, ninja_pal, ninja_bancos, sombra_piezas, sombra_banco
                (data.asm; los .bin de res/prueba_carga/).

    HARDWARE QUE TOCA
      CGRAM: paletas de sprites 1..3 (los ninjas; la 0 es de Tarou y la 4, de la sombra, las
      carga juego/protagonista_sprite.c). VRAM: los tiles de cada ninja, por la lista de DMA.

    VARIABLES: ver ninja_sprite.c.
    → glosario: Pieza de 16×16, Orden por profundidad, OAM
---------------------------------------------------------------------------------*/
#ifndef NINJA_SPRITE_H
#define NINJA_SPRITE_H

#include "sistema/tipos.h"
#include "enemigos/ninja.h"

/* Con la pantalla apagada y DESPUÉS de protagonista_sprite_cargar(): las paletas de los
   ninjas, las entidades fijas 2..7 y la tabla de entidades con 8 entradas. */
void ninja_sprite_cargar(void);

/* Un frame, DESPUÉS de protagonista_sprite_actualizar() (Tarou primero en la lista de DMA):
   sube el cuadro nuevo de un ninja, si toca, y rellena sus entidades. `n` son las NINJAS_MAX
   ranuras; `camara`, el scroll. Debe ir antes de sprites_ordenar... (lo llama ella misma). */
void ninja_sprite_actualizar(const Ninja *n, u16 camara);

#endif /* NINJA_SPRITE_H */
