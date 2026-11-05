/*---------------------------------------------------------------------------------
    dialogo.c — Las escenas de texto de la fase 1 (ver dialogo.h).

    PROPÓSITO
      Contar los frames de cada página y pasarla con A (después de DIALOGO_MIN) o sola
      (a los DIALOGO_MAX); al pasar la última, la escena acaba.

    SE CONECTA CON
      Incluye:  juego/dialogo.h.
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_dialogo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo va en el Dialogo).
    → glosario: Máquina de estados
---------------------------------------------------------------------------------*/
#include "juego/dialogo.h"

void dialogo_iniciar(Dialogo *d)
{
    d->lineas = 0;
    d->paginas = 0;
    d->pagina = 0;
    d->reloj = 0;
    d->activo = 0;
}

void dialogo_empezar(Dialogo *d, const char *const *lineas, u8 paginas)
{
    d->lineas = lineas;
    d->paginas = paginas;
    d->pagina = 0;
    d->reloj = 0;
    d->activo = (u8)(paginas != 0);
}

u8 dialogo_actualizar(Dialogo *d, u8 avanzar_pulsado)
{
    if (!d->activo)
    {
        return 0;
    }
    d->reloj++;
    if ((avanzar_pulsado && d->reloj >= DIALOGO_MIN) || d->reloj >= DIALOGO_MAX)
    {
        d->reloj = 0;                                /* La página siguiente...              */
        d->pagina++;
        if (d->pagina >= d->paginas)
        {
            d->activo = 0;                           /* ...o el fin de la escena.           */
        }
        return 1;
    }
    return (u8)(d->reloj == 1 && d->pagina == 0);    /* La primera, al empezar.             */
}

const char *dialogo_linea(const Dialogo *d, u8 k)
{
    return d->lineas[((u16)d->pagina << 1) + k];     /* 2 líneas por página.                */
}
