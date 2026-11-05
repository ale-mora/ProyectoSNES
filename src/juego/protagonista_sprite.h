/*---------------------------------------------------------------------------------
    protagonista_sprite.h — Dibuja a una protagonista del beat 'em up (Tarou o Bárbara):
    su cuadro de animación en piezas de 16x16, su sombra y la subida de tiles a la VRAM.

    PROPÓSITO
      Une un Cuerpo (juego/mundo.h) con la capa de sprites (juego/sprites.h): elige la
      animación según su estado (juego/personaje.c), avanza los cuadros, sube a la VRAM
      el cuadro nuevo en el búfer oculto (doble búfer, como mucho `tope` bytes por
      VBlank, con juego/subida.c y la lista de DMA) y rellena sus dos entidades: `ent`
      es el cuerpo (y de los pies = c->y, para el orden por profundidad; dibujado c->y − z)
      y `ent + 1` la sombra (en el suelo, y = 0: detrás de todo; media suma de color con
      los fondos, paleta 4). Cada protagonista en pantalla es un ProtaSprite: la fase 1
      tiene uno (el elegido) y el selector dos (Tarou y Bárbara a la vez, cada uno con
      sus búferes en la VRAM y su paleta). Es el sucesor de tarou_sprite.c del
      plataformas (docs/diseno-beat-em-up.md §10).
      El arte de la sombra es el provisional de la prueba de carga (res/prueba_carga/).

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h, juego/personaje.h, juego/subida.h.
      Lo usan:  pantallas/pantalla_fase1.c, pantallas/pantalla_selector.c.
      Lee de la ROM: tarou_*_p16, tarou_p16_bancos, tarou_pal, barbara_*_p16,
                barbara_p16_bancos, barbara_pal, ninja_pal (la sombra), sombra_til,
                sombra_piezas, sombra_banco (data.asm).

    HARDWARE QUE TOCA
      OAM: oamInitGfxAttr (piezas 16x16 y 32x32). VRAM: los tiles de sprites por la lista de
      DMA (dma_lista.h) y la sombra al cargar. CGRAM: la paleta de cada protagonista y la
      4 (sombra). PPU: matemática de color ($212D TS, $2130 CGWSEL, $2131 CGADSUB).

    VARIABLES: los campos de ProtaSprite (abajo).
    → glosario: Pieza de 16×16, Orden por profundidad, OAM, Doble búfer, Matemática de color
---------------------------------------------------------------------------------*/
#ifndef PROTAGONISTA_SPRITE_H
#define PROTAGONISTA_SPRITE_H

#include "sistema/tipos.h"
#include "juego/mundo.h"
#include "juego/personaje.h"
#include "juego/subida.h"

#define SIN_CUADRO 0xFF

typedef struct
{
    AnimEstado a;                    /* La animación que toca (anim, cuadro, reloj).       */
    u8 personaje;                    /* PERSONAJE_TAROU / PERSONAJE_BARBARA.               */
    u8 ent;                          /* Su entidad del cuerpo; la sombra es ent + 1.       */
    u8 paleta;                       /* Paleta de sprites (0..3) con la que se dibuja.     */
    u8 bufer;                        /* 0/1: qué búfer enseña la OAM.                      */
    u8 m_anim, m_cuadro;             /* Lo que hay ENTERO en la VRAM (SIN_CUADRO = nada).  */
    u8 s_anim, s_cuadro;             /* Lo que se está subiendo (SIN_CUADRO = nada).       */
    u8 ancho, alto;                  /* Su celda (personaje_ancho/alto).                   */
    u16 tile_a, tile_b;              /* Sus dos búferes en la VRAM (96 tiles cada uno).    */
    u16 tope;                        /* Bytes que puede subir por VBlank.                  */
    const AnimProta *anims;          /* Su tabla de animaciones (personaje_anims).         */
    Subida subida;                   /* El plan de copias de la subida en marcha.          */
} ProtaSprite;

/* Con la pantalla apagada, una vez por pantalla: OAM, sombra (tiles y paleta 4) y
   matemática de color. Después, protagonista_sprite_cargar para cada protagonista y
   sprites_usar con cuántas entidades hay. */
void protagonista_escena_cargar(void);

/* Con la pantalla apagada: prepara a una protagonista con sus búferes de VRAM (tile_a,
   tile_b), su paleta (la copia a la CGRAM) y el tope de subida. Usa las entidades ent y
   ent + 1. */
void protagonista_sprite_cargar(ProtaSprite *p, u8 personaje, u8 ent, u16 tile_a,
                                u16 tile_b, u8 paleta, u16 tope);

/* Un frame, DESPUÉS de dma_lista_vaciar(): avanza la animación, añade a la lista de DMA
   lo que toque subir y rellena sus entidades. `camara` es el desplazamiento horizontal
   del escenario: la x de pantalla es c->x − camara. Luego hay que llamar a
   sprites_ordenar() (una vez por frame, después de todas las entidades). */
void protagonista_sprite_actualizar(ProtaSprite *p, const Cuerpo *c, u16 camara);

/* Con la pantalla encendida: cambia el personaje que se dibuja (un apoyo entra en el sitio
   de la protagonista, o ella vuelve) sin tocar el hardware. Se oculta hasta que su primer
   cuadro esté en la VRAM. La paleta la copia quien llama, en el VBlank
   (protagonista_paleta). */
void protagonista_sprite_cambiar(ProtaSprite *p, u8 personaje);

/* Bytes que le faltan por subir (la subida en marcha, o el cuadro nuevo que toca empezar);
   0 si la VRAM ya tiene el cuadro que se enseña. Para repartir PRESUPUESTO_SUBIDA. */
u16 protagonista_sprite_pendiente(const ProtaSprite *p);

/* 1 cuando el cuadro actual ya está entero en la VRAM (antes del fade in). */
u8 protagonista_sprite_al_dia(const ProtaSprite *p);

/* Los 32 bytes de la paleta de un personaje (BGR555, en la ROM). */
const u8 *protagonista_paleta(u8 personaje);

/* Al salir: esconde las 128 entradas de la OAM y apaga la matemática de color. */
void protagonista_sprite_ocultar(void);

#endif /* PROTAGONISTA_SPRITE_H */
