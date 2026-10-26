/*---------------------------------------------------------------------------------
    sprites.h — La capa de sprites del beat 'em up: una tabla de entidades que una
    sola rutina en ensamblador pasa a la OAM cada frame.

    PROPÓSITO
      Cada cosa que se ve (protagonista, enemigos, sombras, objetos) tiene su
      entrada FIJA en la tabla (sprites_usar(n) activa las n primeras). Las entradas
      persisten de un frame a otro: cada frame solo se tocan las que cambian (x, y
      y la clave de orden de lo que se mueve; las piezas y el tile, solo cuando
      cambia el cuadro). Rehacer la tabla entera cada frame desde C costaba ~9
      líneas por entidad (prueba de carga v3). Luego sprites_ordenar() ordena por
      la y de los pies (juego/profundidad.c: el de más abajo, delante) y sprites_dibujar()
      (sprites_oam.asm) escribe TODAS sus piezas de 16x16 en el búfer de la OAM del
      SDK en un solo bucle de ensamblador y esconde las entradas que sobran.
      La prueba de carga midió que, desde C, cada pieza costaba ~5,4 líneas con
      oamSet + oamSetEx y ~2,2 con una llamada a ensamblador por entidad (casi todo
      era la llamada): aquí hay UNA llamada por frame (docs/diseno-beat-em-up.md §9.1).
      Mirando a la izquierda, la entidad usa la tabla espejada del .p16 y el bit de
      volteo; así el ensamblador siempre suma x0 + dx. Si la celda entera está en la
      pantalla (borde = 0), cada pieza es UNA suma de 16 bits: (dx | dy << 8) +
      (x0 | y0 << 8), sin comprobaciones.
      Reglas de hardware que respeta (§8): el bit 8 del tile va en cada pieza (un
      hueco de la VRAM puede cruzar el tile 256); una pieza con x negativa pone el
      bit 8 de x en la tabla alta; las piezas fuera de la pantalla y las entradas
      sobrantes se esconden con y = 224, nunca con x = -256 (con x = -256 la PPU
      cuenta igualmente sus 8 px por línea si está en líneas visibles).

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: juego/sprites.c (tabla, nuevo, ordenar) y juego/sprites_oam.asm
                (sprites_dibujar).
      Lo usan:  pantallas/prueba_carga.c.

    HARDWARE QUE TOCA
      ninguno directamente: escribe en oamMemory (WRAM), que la NMI del SDK copia
      a la OAM en el VBlank.

    VARIABLES (globales, en sprites.c; el .asm las lee por su nombre)
      sprites_ent      SpriteEnt[SPRITES_MAX]  las entidades de este frame
      sprites_y        u16[SPRITES_MAX]        y de los pies de cada una (para ordenar)
      sprites_orden    u8[SPRITES_MAX]         orden de dibujo (la primera, delante)
      sprites_n        u8                      cuántas hay
      sprites_oam_antes u8                     entradas de la OAM usadas el frame anterior
    → glosario: OAM, Metasprite, Orden por profundidad, Pieza de 16×16
---------------------------------------------------------------------------------*/
#ifndef SPRITES_H
#define SPRITES_H

#include "sistema/tipos.h"

#define SPRITES_MAX 16               /* Entidades por frame.                              */
#define SPRITES_OAM_MAX 128          /* Entradas de la OAM.                               */

/* Atributos de una entidad (byte vhoopppN de la OAM, sin el bit N: lo pone cada pieza). */
#define SPRITE_PRIORIDAD(p) ((u8)((p) << 4))
#define SPRITE_PALETA(p) ((u8)((p) << 1))
#define SPRITE_HFLIP 0x40

/* Una entidad: 16 bytes (potencia de 2: sprites_ent[i] es un desplazamiento). Lo que
   lee sprites_oam.asm; no cambiar el orden de los campos. */
typedef struct
{
    u16 x0;                          /* x de pantalla del borde izquierdo de la celda.    */
    u16 y0;                          /* y de pantalla del borde de arriba de la celda.    */
    u16 piezas;                      /* Dirección (en su banco) de la tabla (dx, dy); mirando
                                        a la izquierda, la tabla ESPEJADA del .p16.         */
    u8 banco;                        /* Banco de esa tabla.                               */
    u8 n;                            /* Cuántas piezas.                                   */
    u16 tile;                        /* Tile de la pieza 0 (0..511); la k, tile + (k/8)·32 + (k%8)·2. */
    u8 atrib;                        /* vhooppp0: espejo, prioridad, paleta.              */
    u8 borde;                        /* 0 = la celda entera está en la pantalla (camino
                                        rápido: una suma por pieza); 1 = puede salirse
                                        (se comprueba cada pieza). Ver sprites_borde().     */
    u16 libre[2];                    /* Relleno.                                          */
} SpriteEnt;

extern SpriteEnt sprites_ent[SPRITES_MAX];
extern u16 sprites_y[SPRITES_MAX];
extern u8 sprites_orden[SPRITES_MAX];
extern u8 sprites_n;
extern u8 sprites_oam_antes;

/* 1 si una celda de ancho x alto en (x0, y0) puede salirse de la pantalla. */
#define sprites_borde(x0, y0, ancho, alto) \
    ((u8)((x0) > 256 - (ancho) || (y0) > 224 - (alto)))

/* Esconde las 128 entradas de la OAM (y = 224). Al entrar en la pantalla. */
void sprites_iniciar(void);

/* Activa las n primeras entradas de la tabla (las demás no se dibujan) y deja el
   orden en 0..n-1. Al preparar la escena: las entradas no se vacían solas. */
void sprites_usar(u8 n);

/* Ordena por la y de los pies (sprites_y): la de más abajo, primera (delante). Las
   que deban ir detrás de todo (sombras) llevan y = 0. Casi siempre ya viene ordenado:
   cuesta una comparación por entidad. */
void sprites_ordenar(void);

/* Escribe en el búfer de la OAM todas las piezas, en orden, desde la entrada 0, y
   esconde las que sobran del frame anterior (sprites_oam.asm). Devuelve cuántas
   entradas usó. */
u8 sprites_dibujar(void);

#endif /* SPRITES_H */
