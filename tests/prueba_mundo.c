/*---------------------------------------------------------------------------------
    prueba_mundo.c — Pruebas de juego/mundo.c.

    PROPÓSITO
      Fijar el modelo x/y/z (docs/diseno-beat-em-up.md §3): x e y se quedan
      dentro del carril, el salto sube y vuelve a z = 0 en el mismo sitio, el
      aterrizaje se avisa una sola vez y el dibujo va en y − z.

    SE CONECTA CON
      Incluye:  minitest.h, juego/mundo.h, juego/cifras.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_mundo).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Carril, Profundidad (y) y altura (z)
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/mundo.h"
#include "juego/cifras.h"

void pruebas_mundo(void)
{
    Cuerpo c;
    Carril carril;
    u16 frames;                                      /* Frames en el aire.                  */
    u16 altura_max;                                  /* Cota más alta del salto, en píxel.  */
    u8 aterrizajes;                                  /* Veces que mundo_mover avisó.        */

    carril.x_min_fp = PF(16);                        /* Carril del prototipo.               */
    carril.x_max_fp = PF(240);
    carril.y_arriba_fp = PF(150);
    carril.y_abajo_fp = PF(208);

    COMPROBAR(sizeof(Cuerpo) == 16);                 /* Potencia de 2: sin tcc__mul.        */

    /* Colocar: quieto y apoyado. */
    mundo_colocar(&c, 100, 180);
    COMPROBAR(c.x_fp == PF(100) && c.y_fp == PF(180) && c.z_fp == 0);
    COMPROBAR(c.en_suelo == 1);
    COMPROBAR(mundo_y_dibujo(&c) == 180);

    /* Andar: x avanza 1,5 px por frame. */
    c.vx_fp = VEL_ANDAR_TAROU;
    mundo_mover(&c, &carril);
    mundo_mover(&c, &carril);
    COMPROBAR(c.x_fp == PF(103));

    /* Los bordes del carril recortan x e y. */
    c.vx_fp = -PF(200);
    c.vy_fp = -PF(100);
    mundo_mover(&c, &carril);
    COMPROBAR(c.x_fp == PF(16) && c.y_fp == PF(150));
    c.vx_fp = PF(500);
    c.vy_fp = PF(100);
    mundo_mover(&c, &carril);
    COMPROBAR(c.x_fp == PF(240) && c.y_fp == PF(208));

    /* El salto: sube, baja, aterriza en z = 0 y avisa una sola vez. */
    mundo_colocar(&c, 100, 180);
    COMPROBAR(mundo_saltar(&c) == 1);
    COMPROBAR(mundo_saltar(&c) == 0);                /* No hay doble salto.                 */
    frames = 0;
    altura_max = 0;
    aterrizajes = 0;
    while (frames < 200 && !c.en_suelo)
    {
        aterrizajes += mundo_mover(&c, &carril);
        if (PF_ENTERO(c.z_fp) > altura_max)
            altura_max = PF_ENTERO(c.z_fp);
        frames++;
    }
    COMPROBAR(c.en_suelo == 1 && c.z_fp == 0 && c.vz_fp == 0);
    COMPROBAR(aterrizajes == 1);
    COMPROBAR(altura_max >= 45 && altura_max <= 55); /* ~50 px (§5.5).                      */
    COMPROBAR(frames >= 36 && frames <= 44);         /* ~40 frames en el aire.              */
    COMPROBAR(mundo_mover(&c, &carril) == 0);        /* Apoyado: no vuelve a avisar.        */
    COMPROBAR(c.y_fp == PF(180));                    /* La profundidad no cambia al saltar. */

    /* El dibujo va en y − z; la sombra seguiría en y. */
    mundo_colocar(&c, 100, 180);
    c.z_fp = PF(30);
    COMPROBAR(mundo_y_dibujo(&c) == 150);
    c.z_fp = PF(200);                                /* Más alto que la propia y.           */
    COMPROBAR(mundo_y_dibujo(&c) == 0);
}
