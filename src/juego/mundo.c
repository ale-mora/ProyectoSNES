/*---------------------------------------------------------------------------------
    mundo.c — El mundo del beat 'em up en x/y/z (ver mundo.h).

    PROPÓSITO
      Mover un cuerpo un frame. x e y se recortan a los bordes del carril (el
      recorte se hace en punto fijo, antes de convertir a píxel, porque
      PF_ENTERO solo vale para valores >= 0). z sube y baja con la gravedad,
      como el salto de plataformas de la Revisión 1, y se para en 0.

    SE CONECTA CON
      Incluye:  juego/mundo.h, juego/cifras.h (GRAVEDAD_Z, VEL_SALTO_Z).
      Lo usan:  tests/prueba_mundo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna propia.
    → glosario: Carril, Profundidad (y) y altura (z)
---------------------------------------------------------------------------------*/
#include "juego/mundo.h"
#include "juego/cifras.h"

void mundo_colocar(Cuerpo *c, u16 x, u16 y)
{
    c->x_fp = PF(x);                                 /* Píxel → punto fijo.                 */
    c->y_fp = PF(y);
    c->z_fp = 0;                                     /* En el suelo.                        */
    c->vx_fp = 0;                                    /* Quieto.                             */
    c->vy_fp = 0;
    c->vz_fp = 0;
    c->en_suelo = 1;                                 /* Apoyado.                            */
}

u8 mundo_saltar(Cuerpo *c)
{
    if (!c->en_suelo)
        return 0;                                    /* En el aire no se salta otra vez.    */
    c->vz_fp = VEL_SALTO_Z;                          /* Impulso hacia arriba.               */
    c->en_suelo = 0;                                 /* Ya no está apoyado.                 */
    return 1;
}

u8 mundo_mover(Cuerpo *c, const Carril *carril)
{
    c->x_fp += c->vx_fp;                             /* Avance.                             */
    if (c->x_fp < carril->x_min_fp)
        c->x_fp = carril->x_min_fp;                  /* No se sale por la izquierda.        */
    if (c->x_fp > carril->x_max_fp)
        c->x_fp = carril->x_max_fp;                  /* Ni por la derecha.                  */

    c->y_fp += c->vy_fp;                             /* Profundidad.                        */
    if (c->y_fp < carril->y_arriba_fp)
        c->y_fp = carril->y_arriba_fp;               /* No sube más allá del fondo.         */
    if (c->y_fp > carril->y_abajo_fp)
        c->y_fp = carril->y_abajo_fp;                /* Ni baja más allá del borde.         */

    if (c->en_suelo)
        return 0;                                    /* Apoyado: z no cambia.               */

    c->vz_fp -= GRAVEDAD_Z;                          /* La gravedad frena la subida.        */
    c->z_fp += c->vz_fp;                             /* Altura.                             */
    if (c->z_fp <= 0)                                /* Llegó al suelo (o lo pasó):         */
    {
        c->z_fp = 0;                                 /* se queda en él,                     */
        c->vz_fp = 0;                                /* sin velocidad vertical,             */
        c->en_suelo = 1;                             /* apoyado.                            */
        return 1;                                    /* Este es el frame del aterrizaje.    */
    }
    return 0;
}

u16 mundo_y_dibujo(const Cuerpo *c)
{
    if (c->z_fp > c->y_fp)
        return 0;                                    /* Más alto que la pantalla: arriba.   */
    return PF_ENTERO(c->y_fp - c->z_fp);             /* y − z, ya >= 0.                     */
}
