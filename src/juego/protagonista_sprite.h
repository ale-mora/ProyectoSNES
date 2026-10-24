/*---------------------------------------------------------------------------------
    protagonista_sprite.h — Dibuja al protagonista del beat 'em up: su cuadro de
    animación en piezas de 16x16, su sombra y la subida de tiles a la VRAM.

    PROPÓSITO
      Une un Cuerpo (juego/mundo.h) con la capa de sprites (juego/sprites.h): elige la
      animación según su estado (parado, andando, subiendo, bajando), avanza los cuadros,
      sube a la VRAM el cuadro nuevo en el búfer oculto (doble búfer, ≤ 1,5 KB por VBlank,
      con juego/subida.c y la lista de DMA) y rellena las dos entidades fijas: la 0 es el
      cuerpo (y de los pies = c->y, para el orden por profundidad; dibujado c->y − z) y la
      1 es la sombra (en el suelo, y = 0: detrás de todo; media suma de color con los
      fondos, paleta 4). Es el sucesor de tarou_sprite.c del plataformas
      (docs/diseno-beat-em-up.md §10: «tarou_sprite.c → protagonista_sprite.c»); hoy solo
      dibuja a Tarou; Bárbara llegará con su propia tira y paleta.
      El arte de la sombra es el provisional de la prueba de carga (res/prueba_carga/).

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h.
      Lo usan:  pantallas/pantalla_fase1.c.
      Lee de la ROM: tarou_*_p16 (las tiras de Tarou), tarou_p16_bancos, tarou_pal,
                ninja_pal, sombra_til, sombra_piezas, sombra_banco (data.asm).

    HARDWARE QUE TOCA
      OAM: oamInitGfxAttr (piezas 16x16 y 32x32). VRAM: los tiles de sprites por la lista de
      DMA (dma_lista.h) y la sombra al cargar. CGRAM: paletas de sprites 0 (Tarou) y 4
      (sombra). PPU: matemática de color ($212D TS, $2130 CGWSEL, $2131 CGADSUB).

    VARIABLES: ver protagonista_sprite.c.
    → glosario: Pieza de 16×16, Orden por profundidad, OAM, Doble búfer, Matemática de color
---------------------------------------------------------------------------------*/
#ifndef PROTAGONISTA_SPRITE_H
#define PROTAGONISTA_SPRITE_H

#include "sistema/tipos.h"
#include "juego/mundo.h"

/* Con la pantalla apagada: OAM, paletas, sombra y matemática de color. */
void protagonista_sprite_cargar(void);

/* Un frame, DESPUÉS de dma_lista_vaciar(): avanza la animación, añade a la lista de DMA
   lo que toque subir, rellena las entidades y las ordena por profundidad. `camara` es el
   desplazamiento horizontal del escenario: la x de pantalla es c->x − camara. */
void protagonista_sprite_actualizar(const Cuerpo *c, u16 camara);

/* 1 cuando el cuadro actual ya está entero en la VRAM (antes del fade in, para no
   enseñar basura). */
u8 protagonista_sprite_al_dia(void);

/* Al salir: esconde las 128 entradas de la OAM y apaga la matemática de color. */
void protagonista_sprite_ocultar(void);

#endif /* PROTAGONISTA_SPRITE_H */
