/*---------------------------------------------------------------------------------
    cambio_protagonista.c — El relevo entre Tarou y Bárbara (ver
    cambio_protagonista.h).

    PROPÓSITO
      Cada frame: baja el enfriamiento y la nube, cura al que espera fuera, mira L
      y mira si el activo ha caído; después actualiza al activo con el mando. Hacer
      el relevo es: dejar la nube donde estaba el que sale, ponerlo fuera y hacer
      entrar al otro cayendo en ese mismo sitio y mirando hacia donde miraba.

    SE CONECTA CON
      Incluye:  juego/cambio_protagonista.h, juego/cifras.h.
      Lo usan:  tests/prueba_relevo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna propia.
    → glosario: Relevo, Golpe de entrada
---------------------------------------------------------------------------------*/
#include "juego/cambio_protagonista.h"
#include "juego/cifras.h"

void relevo_iniciar(Relevo *r, u16 x, u16 y)
{
    luchador_iniciar_como(&r->tarou, QUIEN_TAROU, x, y, VIDA_PROTAGONISTA);
    luchador_iniciar_como(&r->barbara, QUIEN_BARBARA, x, y, VIDA_PROTAGONISTA);
    r->activo = QUIEN_TAROU;
    r->enfriamiento = 0;
    r->cura = FRAMES_CURA_FUERA;
    r->nube = 0;
    r->nube_x = 0;
    r->nube_y = 0;
}

Luchador *relevo_activo(Relevo *r)
{
    if (r->activo == QUIEN_BARBARA)
        return &r->barbara;
    return &r->tarou;
}

Luchador *relevo_fuera(Relevo *r)
{
    if (r->activo == QUIEN_BARBARA)
        return &r->tarou;
    return &r->barbara;
}

/* Hace el relevo: el activo sale (nube donde estaba) y entra el otro cayendo sobre ese sitio.
   `por_ko` = 1 si sale porque cayó: entra sin enfriamiento y con FRAMES_INVULNERABLE de protección. */
static void hacer_relevo(Relevo *r, u8 por_ko)
{
    Luchador *sale;
    Luchador *entra;
    u8 invulnerable;

    sale = relevo_activo(r);
    entra = relevo_fuera(r);

    r->nube = FRAMES_NUBE;                           /* La nube, donde estaba el que sale.  */
    r->nube_x = sale->c.x_fp;
    r->nube_y = sale->c.y_fp;

    invulnerable = FRAMES_ENTRADA_INVULNERABLE;
    if (por_ko)
        invulnerable = FRAMES_INVULNERABLE;          /* Tras un KO, 60 frames (spec §5.5).  */
    entra->mira_izquierda = sale->mira_izquierda;    /* Mira hacia donde miraba el otro.    */
    luchador_entrar(entra, sale->c.x_fp, sale->c.y_fp, invulnerable);

    if (r->activo == QUIEN_TAROU)
        r->activo = QUIEN_BARBARA;
    else
        r->activo = QUIEN_TAROU;
    if (!por_ko)
        r->enfriamiento = ENFRIAMIENTO_RELEVO;      /* El enfriamiento solo cuenta en un relevo voluntario. */
}

u8 relevo_actualizar(Relevo *r, const Mando *m, const Carril *carril)
{
    Luchador *activo;
    Luchador *fuera;
    u8 resultado;

    resultado = RELEVO_NADA;
    if (r->enfriamiento)
        r->enfriamiento--;
    if (r->nube)
        r->nube--;

    /* El que espera fuera se cura poco a poco, hasta el tope (no se cura del todo). */
    fuera = relevo_fuera(r);
    if (r->cura)
        r->cura--;
    if (r->cura == 0)
    {
        r->cura = FRAMES_CURA_FUERA;
        if (fuera->g.vida < TOPE_CURA_FUERA)
            fuera->g.vida++;
    }

    activo = relevo_activo(r);

    /* El activo ha caído (su KO terminó): entra el otro; si también está a 0, se pierde una vida. */
    if (activo->vencido)
    {
        if (fuera->g.vida == 0)
            return RELEVO_PIERDE_VIDA;               /* Caen los dos: quien llama reinicia. */
        hacer_relevo(r, 1);
        activo = relevo_activo(r);
        resultado = RELEVO_CAMBIO;
    }
    else if (m->relevo_pulsado && r->enfriamiento == 0 && fuera->g.vida > 0
             && luchador_puede_salir(activo))
    {
        hacer_relevo(r, 0);                          /* L: el relevo de siempre.            */
        activo = relevo_activo(r);
        resultado = RELEVO_CAMBIO;
    }

    luchador_actualizar(activo, m, carril);          /* Solo el activo se mueve con el mando. */
    return resultado;
}
