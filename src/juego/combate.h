/*---------------------------------------------------------------------------------
    combate.h — Quién golpea a quién: la caja de golpe de Tarou contra los ninjas.

    PROPÓSITO
      Es el módulo que, como dice docs/diseno-beat-em-up.md §5.1, hace que «un luchador cambie
      el estado de otro»: el golpe de Tarou que conecta pone al ninja en RECIBIR o
      DERRIBADO (enemigos/ninja.c: ninja_recibir). La caja de golpe es un rectángulo en el
      suelo delante de Tarou: de COMBATE_ATRAS px detrás de sus pies a COMBATE_ALCANCE px
      delante (hacia donde mira), con una tolerancia de COMBATE_PROFUNDIDAD píxeles en la
      profundidad (y). Solo cuenta en los cuadros activos del golpe (mundo_golpe_activo) y
      cada tajo (Cuerpo.id) pega UNA vez a cada ninja. El combo es 1 + 1 + 2 de daño; el
      tercer golpe derriba. Es lógica pura, sin snes.h: probada en el PC
      (tests/prueba_combate.c). Los ninjas aún no pegan: ese es el siguiente paso.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h, enemigos/ninja.h.
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_combate.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Caja de golpe, Combo, Derribo, Parada de impacto
---------------------------------------------------------------------------------*/
#ifndef COMBATE_H
#define COMBATE_H

#include "sistema/tipos.h"
#include "juego/mundo.h"
#include "enemigos/ninja.h"

#define COMBATE_ALCANCE 52               /* Tarou: alcance corto (Bárbara, 72: personaje.h). */
#define COMBATE_ATRAS 8                  /* La caja empieza un poco detrás de los pies.   */
#define COMBATE_PROFUNDIDAD 14           /* Diferencia máxima de y entre golpeador y golpeado. */
#define PARADA_GOLPE 3                   /* Cuadros de «parada de impacto» al conectar.   */
#define PARADA_DERRIBO 5                 /* Y si el golpe derriba.                        */

/* Daño del golpe `golpe` (1..GOLPES) del combo: 1, 1 y 2. */
u8 combate_danio(u8 golpe);

/* 1 si el golpe `golpe` derriba (el último del combo). */
u8 combate_derriba(u8 golpe);

/* 1 si un objetivo con los pies en (tx, ty) está dentro de la caja de golpe del cuerpo `c`
   (sin mirar si el golpe está activo). */
u8 combate_en_caja(const Cuerpo *c, u16 tx, u16 ty);

/* 1 si un objetivo con los pies en (tx, ty) está dentro de una caja de golpe que parte de los
   pies (ax, ay) hacia donde mira `mira_izq`: de COMBATE_ATRAS px detrás a `alcance` delante, con
   una tolerancia de `profundidad` en y. Es la caja de los dos lados (Tarou y los ninjas). */
u8 combate_caja(u16 ax, u16 ay, u8 mira_izq, u16 alcance, u16 profundidad, u16 tx, u16 ty);

/* Un frame de combate de los ninjas contra Tarou: cada ninja con el tajo activo cuyo golpe
   alcanza a Tarou le hace ninja_danio() (2, 3 o 4 según el tipo) (mundo_recibir) y no vuelve a pegar en ese tajo (aunque
   Tarou fuera invulnerable: el golpe se pierde). Devuelve PARADA_GOLPE si alguno le dio,
   PARADA_DERRIBO si lo dejó KO, 0 si ninguno. */
u8 combate_ninjas_resolver(Ninja *n, Cuerpo *c);

/* Un frame de combate de la protagonista contra las NINJAS_MAX ranuras: si su golpe está
   activo, cada ninja vivo dentro de la caja (de `alcance` px por delante: COMBATE_ALCANCE
   para Tarou, personaje_alcance() en general) que no haya recibido ya este tajo es
   golpeado. Devuelve los cuadros de parada de impacto que hay que aplicar (0 si no
   conectó nada). */
u8 combate_resolver(const Cuerpo *c, Ninja *n, u16 alcance);

#endif /* COMBATE_H */
