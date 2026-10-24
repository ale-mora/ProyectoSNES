/*---------------------------------------------------------------------------------
    ninja.h — El ninja básico del beat 'em up: aparece, camina hacia el protagonista y se
    queda a distancia de ataque.

    PROPÓSITO
      Primer enemigo (docs/diseno-beat-em-up.md decisión 21: «ninja básico»). Es lógica
      pura, sin snes.h, probada en el PC (tests/prueba_ninja.c). Cada ninja tiene una
      posición en el carril (x, y de los pies, como el Cuerpo de juego/mundo.h), mira hacia
      el protagonista y camina hacia él: en x un píxel por frame hasta NINJA_DISTANCIA y en
      y un píxel cada dos frames hasta su carril. Aún no ataca, ni recibe golpes: eso es
      combate.c (siguiente paso); hoy A los «derrota» a todos (provisional, pantalla_fase1.c).
      Hay NINJAS_MAX ranuras fijas: la oleada rellena las libres. El dibujo es de
      enemigos/ninja_sprite.c: cada ranura tiene su hueco de tiles en la VRAM.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  enemigos/ninja_sprite.c, pantallas/pantalla_fase1.c y tests/prueba_ninja.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Ninja; 16 bytes: potencia de 2)
      x, y        u16  pies: x en el mundo, y línea de pantalla del suelo
      vivo        u8   1 si la ranura está en uso
      mira_izq    u8   1 si mira a la izquierda (hacia el protagonista)
      cuadro      u8   cuadro de la tira de andar (0..NINJA_CUADROS-1)
      reloj       u8   frames que lleva en ese cuadro; también reparte el paso en y
      estado      u8   NINJA_PARADO o NINJA_ANDANDO
      dentro      u8   1 desde que entró en la pantalla (a partir de ahí ya no puede salir)
    → glosario: Oleada, IA, Carril
---------------------------------------------------------------------------------*/
#ifndef NINJA_H
#define NINJA_H

#include "sistema/tipos.h"

#define NINJAS_MAX 3                     /* Ranuras (las que caben en la VRAM de sprites).  */
#define NINJA_CUADROS 8                  /* Cuadros de la tira de andar.                    */
#define NINJA_FRAMES_CUADRO 4            /* Cada cuántos frames cambia de cuadro.           */
#define NINJA_PASO_X 1                   /* Anda más lento que Tarou (2): se le puede huir. */
#define NINJA_DISTANCIA 40               /* Se detiene a esta distancia en x del objetivo.  */
#define NINJA_ENTRADA 40                 /* Aparece a esta distancia fuera del borde.       */
#define NINJA_ANCHO 64                   /* Celda de 64x96 con el cuerpo en el centro.      */
#define NINJA_ALTO 96

typedef enum
{
    NINJA_PARADO = 0,
    NINJA_ANDANDO
} EstadoNinja;

typedef struct
{
    u16 x;
    u16 y;
    u8 vivo;
    u8 mira_izq;
    u8 cuadro;
    u8 reloj;
    u8 estado;
    u8 dentro;
    u16 libre2[3];                       /* Relleno hasta 16 bytes.                         */
} Ninja;

/* Deja las NINJAS_MAX ranuras libres. */
void ninja_iniciar_todos(Ninja *n);

/* Una oleada: ocupa hasta (zona + 1) ranuras libres, con un máximo de NINJAS_MAX, alternando
   por la izquierda y la derecha, fuera de la pantalla (cámara − NINJA_ENTRADA o cámara + 256 +
   NINJA_ENTRADA) y en carriles distintos. Devuelve cuántos nacieron. */
u8 ninja_oleada(Ninja *n, u16 camara, u8 zona);

/* Un frame de UN ninja vivo: mira y camina hacia (obj_x, obj_y), los pies del protagonista. */
void ninja_actualizar(Ninja *n, u16 obj_x, u16 obj_y);

/* Tras ninja_actualizar: una vez que el ninja ha entrado en [x_min, x_max] (la pantalla con
   un margen) ya no puede salir de ahí; mientras viene de fuera, no se toca. Así no se queda
   esperando fuera de la pantalla cuando el protagonista está pegado al borde. */
void ninja_limitar_x(Ninja *n, u16 x_min, u16 x_max);

/* Cuántos están vivos. */
u8 ninja_vivos(const Ninja *n);

/* Los quita a todos (provisional: «oleada vencida»; luego, el combate los irá derrotando). */
void ninja_derrotar_todos(Ninja *n);

#endif /* NINJA_H */
