/*---------------------------------------------------------------------------------
    prueba_partida.c — Pruebas de juego/partida.c sobre el nivel REAL del tutorial.

    PROPÓSITO
      Comprobar las reglas de docs/diseno-tutorial-movimiento.md §1-§2: caer al
      foso y tocar una zarza mandan reaparecer en el último cartel pasado; el
      tronco para a Tarou hasta que la espada lo rompe (y sin espada fuera, A no
      hace nada); llegar a la salida termina el nivel.

    SE CONECTA CON
      Incluye:  minitest.h, juego/partida.h, juego/colision.h, juego/tarou.h,
                juego/mando.h, juego/nivel.h, sistema/puntofijo.h,
                nivel1_datos.h (res/nivel1, generado).
      Lo usa:   tests/pruebas.c (pruebas_partida).

    HARDWARE QUE TOCA
      ninguno (solo PC).

    VARIABLES (estáticas)
      controles  u16[]  las x de los carteles (NIVEL1_CONTROLES_X)
    → glosario: Punto de control
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/partida.h"
#include "juego/colision.h"
#include "juego/tarou.h"
#include "juego/mando.h"
#include "juego/nivel.h"
#include "sistema/puntofijo.h"
#include "nivel1_datos.h"

static const u16 controles[] = NIVEL1_CONTROLES_X;

/* Tarou en x_pix (celda) sobre el suelo base del tutorial, partida nueva. */
static void empezar(Tarou *t, Mando *m, Partida *p, u16 x_pix)
{
    memset(m, 0, sizeof(*m));
    colision_usar(nivel1_colision);
    partida_iniciar(p, NIVEL1_INICIO_X, controles, NIVEL1_CONTROLES, NIVEL1_TRONCO_X, NIVEL1_SALIDA_X);
    tarou_iniciar(t, x_pix, SUELO_Y);
}

/* Avanza hasta `n` frames; para en el primer evento que no sea SIGUE y lo devuelve. */
static u8 jugar(Tarou *t, const Mando *m, Partida *p, u16 n)
{
    u16 i;
    u8 e;
    for (i = 0; i < n; i++)
    {
        tarou_actualizar(t, m);
        e = partida_actualizar(p, t);
        if (e != PARTIDA_SIGUE)
        {
            return e;
        }
    }
    return PARTIDA_SIGUE;
}

static void prueba_foso_reaparece(void)
{
    Tarou t; Mando m; Partida p;
    empezar(&t, &m, &p, 1000);
    m.derecha = 1; m.actividad = 1;                  /* Camina hasta caer en el foso.      */
    COMPROBAR(jugar(&t, &m, &p, 400) == PARTIDA_REAPARECER);
    COMPROBAR(p.reaparicion == 1048);                /* Pasó el cartel de «HOLD Y» (1048). */
}

static void prueba_zarzas(void)
{
    Tarou t; Mando m; Partida p;
    empezar(&t, &m, &p, 1260);                       /* Antes de las zarzas (1360..1487).  */
    m.derecha = 1; m.actividad = 1;
    COMPROBAR(jugar(&t, &m, &p, 300) == PARTIDA_REAPARECER);
    COMPROBAR(p.reaparicion == 1304);                /* El cartel «MIND THORNS».           */
}

static void prueba_tronco(void)
{
    Tarou t; Mando m; Partida p;
    u8 e;
    empezar(&t, &m, &p, 1540);
    m.derecha = 1; m.actividad = 1;
    COMPROBAR(jugar(&t, &m, &p, 120) == PARTIDA_SIGUE);
    COMPROBAR(PF_ENTERO(t.x_fp) == NIVEL1_TRONCO_X - 24 - 52);   /* Parado contra el tronco. */
    COMPROBAR(p.tronco_entero == 1);
    /* A sin la espada fuera: nada. */
    m.derecha = 0;
    m.atacar_pulsado = 1;
    COMPROBAR(jugar(&t, &m, &p, 1) == PARTIDA_SIGUE);
    m.atacar_pulsado = 0;
    COMPROBAR(jugar(&t, &m, &p, 30) == PARTIDA_SIGUE);
    COMPROBAR(p.tronco_entero == 1);
    /* R (desenvainar), esperar a que termine, A: se rompe. */
    m.espada_pulsado = 1;
    jugar(&t, &m, &p, 1);
    m.espada_pulsado = 0;
    jugar(&t, &m, &p, 90);
    COMPROBAR(t.espada == ESPADA_FUERA);
    m.atacar_pulsado = 1;
    e = jugar(&t, &m, &p, 1);                        /* El golpe puede llegar ya en este   */
    m.atacar_pulsado = 0;                            /* mismo cuadro...                    */
    if (e == PARTIDA_SIGUE)
    {
        e = jugar(&t, &m, &p, 30);                   /* ...o en los siguientes.            */
    }
    COMPROBAR(e == PARTIDA_TRONCO_ROTO);
    COMPROBAR(p.tronco_entero == 0);
    /* Ya se pasa. */
    m.derecha = 1;
    jugar(&t, &m, &p, 200);
    COMPROBAR(PF_ENTERO(t.x_fp) > NIVEL1_TRONCO_X);
}

static void prueba_salida(void)
{
    Tarou t; Mando m; Partida p;
    empezar(&t, &m, &p, 1800);
    partida_actualizar(&p, &t);
    colision_bloque(0, 0, 0, 0);                     /* (El tronco queda atrás de todos modos.) */
    m.derecha = 1; m.actividad = 1;
    COMPROBAR(jugar(&t, &m, &p, 400) == PARTIDA_SALIDA);
    COMPROBAR(PF_ENTERO(t.x_fp) + ANCHO_CELDA / 2 >= NIVEL1_SALIDA_X);
}

void pruebas_partida(void)
{
    prueba_foso_reaparece();
    prueba_zarzas();
    prueba_tronco();
    prueba_salida();
}
