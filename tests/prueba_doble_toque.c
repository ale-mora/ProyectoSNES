/*---------------------------------------------------------------------------------
    prueba_doble_toque.c — Pruebas de juego/doble_toque.c.

    PROPÓSITO
      Fijar la regla del doble toque (docs/diseno-beat-em-up.md §2): pulsar,
      soltar y volver a pulsar la MISMA dirección en VENTANA_DOBLE_TOQUE frames
      o menos hace correr hasta soltarla; tarde, o con la otra dirección, se
      anda.

    SE CONECTA CON
      Incluye:  minitest.h, juego/doble_toque.h, juego/cifras.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_doble_toque).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Doble toque
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/doble_toque.h"
#include "juego/cifras.h"

/* Mantiene la cruceta `n` frames y devuelve si el último frame corría. */
static u8 mantener(DobleToque *d, u8 izq, u8 der, u8 n)
{
    u8 corre = 0;                                    /* Lo que diga el último frame.        */
    while (n--)
        corre = doble_toque_actualizar(d, izq, der);
    return corre;
}

void pruebas_doble_toque(void)
{
    DobleToque d;

    /* Un solo toque: se anda. */
    doble_toque_iniciar(&d);
    COMPROBAR(mantener(&d, 0, 1, 5) == 0);

    /* Soltar 4 frames y volver a pulsar la misma: se corre mientras se mantenga. */
    COMPROBAR(mantener(&d, 0, 0, 4) == 0);
    COMPROBAR(mantener(&d, 0, 1, 1) == 1);
    COMPROBAR(mantener(&d, 0, 1, 30) == 1);

    /* Al soltar, la carrera se acaba. */
    COMPROBAR(mantener(&d, 0, 0, 1) == 0);

    /* El borde de la ventana: con un hueco de VENTANA_DOBLE_TOQUE + 1 frames aún corre
       (frames_suelta cuenta desde 0 el frame en que se suelta)... */
    doble_toque_iniciar(&d);
    mantener(&d, 1, 0, 3);
    mantener(&d, 0, 0, VENTANA_DOBLE_TOQUE + 1);
    COMPROBAR(mantener(&d, 1, 0, 1) == 1);

    /* ...y con uno más, ya no. */
    doble_toque_iniciar(&d);
    mantener(&d, 1, 0, 3);
    mantener(&d, 0, 0, VENTANA_DOBLE_TOQUE + 2);
    COMPROBAR(mantener(&d, 1, 0, 1) == 0);

    /* Derecha y luego izquierda a tiempo: no es doble toque. */
    doble_toque_iniciar(&d);
    mantener(&d, 0, 1, 3);
    mantener(&d, 0, 0, 2);
    COMPROBAR(mantener(&d, 1, 0, 1) == 0);

    /* Cambiar de dirección corriendo corta la carrera. */
    doble_toque_iniciar(&d);
    mantener(&d, 0, 1, 2);
    mantener(&d, 0, 0, 2);
    COMPROBAR(mantener(&d, 0, 1, 2) == 1);
    COMPROBAR(mantener(&d, 1, 0, 1) == 0);

    /* Mucho tiempo sin tocar nada no desborda el contador (se satura en 255). */
    doble_toque_iniciar(&d);
    mantener(&d, 0, 1, 1);
    mantener(&d, 0, 0, 255);
    mantener(&d, 0, 0, 255);
    COMPROBAR(d.frames_suelta == 255);
    COMPROBAR(mantener(&d, 0, 1, 1) == 0);
}
