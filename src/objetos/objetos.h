/*---------------------------------------------------------------------------------
    objetos.h — Objetos de la fase: lo que se rompe, lo que se recoge y lo que vuela.

    PROPÓSITO
      Lógica pura (sin snes.h, probada en el PC: tests/prueba_objetos.c), 2026-11-05
      (docs/diseno-beat-em-up.md decisión 25 y §7):
        - ROMPIBLES: barriles, cajas o jarras puestos por la zona; un golpe del
          protagonista los rompe (saltan dos astillas) y sueltan lo que llevan.
        - ÍTEMS: comida (onigiri cura poco, pescado mucho), una vida extra (cura del
          todo) o un arma arrojadiza (shuriken o kunai: el siguiente A la lanza). Se
          recogen pasando por encima. Como mucho uno en el suelo a la vez.
        - PROYECTILES: los shuriken y kunai que lanza el protagonista (dañan a los
          enemigos) y los shuriken del ninja rápido (dañan al protagonista).
        - CHISPA: un destello pequeño donde conecta un golpe (game feel).
      La pantalla le pasa el cuerpo, los ninjas y la cámara; objetos_sprite.c lo dibuja.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h, enemigos/ninja.h.
      Implementa: objetos/objetos.c.
      Lo usan:  objetos/objetos_sprite.c, pantallas/pantalla_fase1.c, tests/prueba_objetos.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos; cada struct mide 8 bytes: potencia de 2)
      Rompible  x, y (pies), vivo, contiene (ITEM_*)
      Item      x, y, tipo, vivo, reloj (parpadea los últimos frames antes de irse)
      Proyectil x, y, vivo, izq, dueno (0 enemigo, 1 protagonista), tipo (ITEM_SHURIKEN/KUNAI), reloj
      Astilla   x, y, vx, vy, vivo, reloj
    → glosario: Sprite, Caja de colisión / caja de golpe
---------------------------------------------------------------------------------*/
#ifndef OBJETOS_H
#define OBJETOS_H

#include "sistema/tipos.h"
#include "juego/mundo.h"
#include "enemigos/ninja.h"

#define ROMPIBLES_MAX 8                  /* Por zona.                                     */
#define PROYECTILES_MAX 2
#define ASTILLAS_MAX 2
#define ITEM_VIDA_FRAMES 255             /* Frames en el suelo antes de desaparecer.      */
#define PROYECTIL_PASO 4                 /* px por frame.                                 */
#define PROYECTIL_FRAMES 70
#define CHISPA_FRAMES 8
#define CURA_ONIGIRI 6
#define CURA_PESCADO 12
#define DANIO_SHURIKEN 2
#define DANIO_KUNAI 3

typedef enum
{
    ITEM_NADA = 0,
    ITEM_ONIGIRI,
    ITEM_PESCADO,
    ITEM_VIDA,
    ITEM_SHURIKEN,
    ITEM_KUNAI
} TipoItem;

typedef struct
{
    u16 x;
    u16 y;
    u8 vivo;
    u8 contiene;
    u8 libre[2];
} Rompible;

typedef struct
{
    u16 x;
    u16 y;
    u8 tipo;
    u8 vivo;
    u8 reloj;
    u8 libre;
} Item;

typedef struct
{
    u16 x;
    u16 y;
    u8 vivo;
    u8 izq;
    u8 dueno;
    u8 tipo;
} Proyectil;

typedef struct
{
    u16 x;
    s16 y;                               /* Altura de pantalla (sube y baja).             */
    s8 vx;
    s8 vy;
    u8 vivo;
    u8 reloj;
} Astilla;

typedef struct
{
    Rompible r[ROMPIBLES_MAX];
    Proyectil p[PROYECTILES_MAX];
    Astilla a[ASTILLAS_MAX];
    Item item;
    u16 chispa_x;
    u16 chispa_y;
    u8 chispa;                           /* Frames de chispa que quedan.                  */
    u8 n_rompibles;
    u8 item_nuevo;                       /* 1 = acaba de aparecer un ítem (subir su dibujo). */
    u8 libre;
} Objetos;

/* La zona: `n` rompibles (x, y y lo que contiene), en tablas de tres u16/u8. */
void objetos_iniciar(Objetos *o, const u16 *xs, const u16 *ys, const u8 *contiene, u8 n);

/* Un frame: el golpe del protagonista rompe lo que toca (alcance `alcance`); recoge el
   ítem que pisa; mueve proyectiles y astillas; los proyectiles hieren a quien toquen. */
void objetos_actualizar(Objetos *o, Cuerpo *c, Ninja *n, u16 alcance, u16 camara);

/* Lanza un proyectil desde (x, y): `dueno` 1 = el protagonista, 0 = un enemigo. */
void objetos_lanzar(Objetos *o, u16 x, u16 y, u8 izq, u8 dueno, u8 tipo);

/* Una chispa en (x, y) (donde conectó un golpe). */
void objetos_chispa(Objetos *o, u16 x, u16 y);

#endif /* OBJETOS_H */
