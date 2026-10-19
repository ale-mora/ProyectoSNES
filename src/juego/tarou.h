/*---------------------------------------------------------------------------------
    tarou.h — El protagonista (Tarou adulto): su estado, su física y sus cajas.
    Lógica pura: no sabe nada de la PPU ni del mando real.

    PROPÓSITO
      Define la struct Tarou (todo lo que hay que saber del protagonista en un
      frame), las constantes de movimiento de docs/diseno-demo-jugable.md §5 y
      las cuatro operaciones: iniciar, actualizar un frame a partir de un Mando,
      y consultar la caja del cuerpo y la caja de golpe de la espada. Quien
      dibuja (tarou_sprite.c) y quien muestra números (hud_debug.c) solo LEEN
      la struct.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, sistema/puntofijo.h, juego/mando.h,
                juego/animaciones.h (enum Estado).
      Lo usan:  juego/tarou.c, juego/tarou_sprite.c, juego/hud_debug.c,
                pantallas/pantalla_nivel1.c, tests/prueba_tarou.c.

    HARDWARE QUE TOCA
      ninguno (se compila también en el PC).

    VARIABLES (campos de Tarou)
      x_fp, y_fp       s16  0..6656 / 0..3584, punto fijo 12.4  esquina superior izquierda de la celda (96x64), en el mundo
      vx_fp, vy_fp     s16  -96..96, punto fijo 12.4            velocidad por frame (negativa = izquierda / arriba)
      estado           u8   un valor de Estado                  qué hace ahora (una sola cosa)
      espada           u8   ESPADA_ENVAINADA / ESPADA_FUERA     persiste entre estados
      direccion        u8   DIR_DERECHA / DIR_IZQUIERDA         hacia dónde mira (decide el espejo)
      en_suelo         u8   0 / 1                               1 si los pies tocan el suelo
      cuadro           u8   0..cuadros-1                        cuadro actual de la animación del estado
      frames_cuadro    u8   0..frames_por_cuadro                frames que lleva el cuadro actual
      anim_terminada   u8   0 / 1                               1 cuando una animación sin bucle llegó al final
      frames_quieto    u16  0..FRAMES_PARA_DORMIR               frames seguidos sin actividad (solo cuenta en PARADO)

    → glosario: Protagonista, Estado, Espada, Dirección, Punto fijo, struct
---------------------------------------------------------------------------------*/
#ifndef TAROU_H
#define TAROU_H

#include "sistema/tipos.h"
#include "sistema/puntofijo.h"
#include "juego/mando.h"
#include "juego/animaciones.h"

/*--- Constantes de movimiento (§5; "a afinar" en Mesen con el HUD) ----------------*/
#define VEL_CAMINAR_MAX PF(1)        /* 16: 1,0 px por frame = 60 px/s.                   */
#define VEL_CORRER_MAX PF(2)         /* 32: 2,0 px por frame.                             */
#define VEL_AGACHADO_MAX 8           /* 0,5 px por frame.                                 */
#define ACEL_SUELO 2                 /* 8 frames hasta caminar, 16 hasta correr.          */
#define FRENADO_SUELO 2              /* 8 frames hasta parar desde caminar.               */
#define ACEL_AIRE 1                  /* Control aéreo suave.                              */
#define GRAVEDAD 4                   /* 0,25 px por frame cada frame.                     */
#define VEL_TERMINAL PF(6)           /* 96: velocidad máxima de caída.                    */
#define VEL_SALTO (-96)              /* -6,0 px por frame al despegar (~72 px de altura). */
#define VEL_SALTO_CORTE (-32)        /* Al soltar B subiendo, la subida se recorta a esto.*/
#define FRAMES_PARA_DORMIR 600       /* 10 s sin actividad.                               */

/*--- Valores de los campos espada y direccion -------------------------------------*/
enum { ESPADA_ENVAINADA = 0, ESPADA_FUERA = 1 };
enum { DIR_DERECHA = 0, DIR_IZQUIERDA = 1 };

/*--- Caja: rectángulo en píxeles del mundo (esquina superior izquierda y tamaño) --*/
typedef struct
{
    s16 x, y;                        /* Puede ser negativa (golpe hacia la izquierda en x=0). */
    u8 ancho, alto;
} Caja;

/*--- El protagonista ------------------------------------------------------------*/
typedef struct
{
    s16 x_fp, y_fp;                  /* Posición en el mundo, punto fijo 12.4, nunca negativa. */
    s16 vx_fp, vy_fp;                /* Velocidad, punto fijo 12.4, puede ser negativa.        */
    u8 estado;                       /* Estado (animaciones.h): una sola cosa a la vez.        */
    u8 espada;                       /* ESPADA_ENVAINADA o ESPADA_FUERA.                       */
    u8 direccion;                    /* DIR_DERECHA o DIR_IZQUIERDA.                           */
    u8 en_suelo;                     /* 1 si está apoyado en el suelo.                         */
    u8 cuadro;                       /* Cuadro actual dentro de la animación del estado.       */
    u8 frames_cuadro;                /* Frames que lleva el cuadro actual.                     */
    u8 anim_terminada;               /* 1 cuando una animación sin bucle llegó al final.       */
    u16 frames_quieto;               /* Frames seguidos sin actividad (para dormirse).         */
} Tarou;

/* Deja a Tarou PARADO, con la espada envainada, mirando a la derecha, con la celda
   empezando en x_pix y los pies sobre suelo_y (y = suelo_y - ALTO_CELDA). */
void tarou_iniciar(Tarou *t, u16 x_pix, u16 suelo_y);

/* Un frame completo (§5): transiciones de estado → aceleración/frenado → gravedad →
   integrar → recortar x → aterrizar → avanzar la animación. suelo_y es la altura
   del suelo bajo el centro de Tarou (nivel_suelo_en). */
void tarou_actualizar(Tarou *t, const Mando *m, u16 suelo_y);

/* Caja del cuerpo en píxeles del mundo (§5): 24x56 de pie, 24x32 agachado. */
void tarou_caja_cuerpo(const Tarou *t, Caja *c);

/* Caja de golpe de la espada. Devuelve 1 (y rellena c) solo mientras ataca. */
u8 tarou_caja_golpe(const Tarou *t, Caja *c);

#endif /* TAROU_H */
