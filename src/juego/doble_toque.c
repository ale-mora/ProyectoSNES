/*---------------------------------------------------------------------------------
    doble_toque.c — Correr con un doble toque de la cruceta (ver doble_toque.h).

    PROPÓSITO
      Cada frame se mira qué dirección horizontal se mantiene. Si acaba de
      pulsarse (no se mantenía el frame anterior) y es la misma que la última,
      soltada hace como mucho VENTANA_DOBLE_TOQUE frames, empieza la carrera.
      La carrera dura mientras se mantenga esa dirección. Si se pulsan las dos
      a la vez, cuenta la derecha (una sola dirección cada frame).

    SE CONECTA CON
      Incluye:  juego/doble_toque.h, juego/cifras.h (VENTANA_DOBLE_TOQUE).
      Lo usan:  tests/prueba_doble_toque.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna propia (todo está en la struct del llamador).
    → glosario: Doble toque
---------------------------------------------------------------------------------*/
#include "juego/doble_toque.h"
#include "juego/cifras.h"

void doble_toque_iniciar(DobleToque *d)
{
    d->dir_previa = TOQUE_NINGUNA;                   /* Nada mantenido.                     */
    d->ultima_dir = TOQUE_NINGUNA;                   /* Nada pulsado todavía.               */
    d->frames_suelta = 255;                          /* "Hace mucho": no hay doble toque.   */
    d->corriendo = 0;                                /* Se empieza andando.                 */
}

u8 doble_toque_actualizar(DobleToque *d, u8 izquierda, u8 derecha)
{
    u8 dir;                                          /* Dirección mantenida este frame.     */

    dir = TOQUE_NINGUNA;                             /* Por defecto, ninguna.               */
    if (izquierda)
        dir = TOQUE_IZQUIERDA;
    if (derecha)
        dir = TOQUE_DERECHA;                         /* Con las dos, gana la derecha.       */

    if (dir != d->dir_previa)                        /* Algo cambió respecto al anterior.   */
    {
        if (d->dir_previa != TOQUE_NINGUNA)          /* Se soltó (o se cambió) la que había: */
        {
            d->corriendo = 0;                        /* la carrera se acaba,                */
            d->frames_suelta = 0;                    /* y empieza a contar el hueco.        */
        }
        if (dir != TOQUE_NINGUNA)                    /* Se acaba de pulsar una dirección.   */
        {
            if (dir == d->ultima_dir && d->frames_suelta <= VENTANA_DOBLE_TOQUE)
                d->corriendo = 1;                    /* La misma y a tiempo: ¡a correr!     */
            d->ultima_dir = dir;                     /* Se recuerda para el próximo toque.  */
        }
    }
    else if (dir == TOQUE_NINGUNA && d->frames_suelta != 255)
    {
        d->frames_suelta++;                          /* Sin tocar nada: el hueco crece.     */
    }

    d->dir_previa = dir;                             /* Para comparar el frame siguiente.   */
    return d->corriendo;
}
