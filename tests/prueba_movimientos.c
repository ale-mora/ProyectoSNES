/*---------------------------------------------------------------------------------
    prueba_movimientos.c — Pruebas de los movimientos del protagonista (2026-11-05):
    correr con doble toque, ataque aéreo, especial A + B, lanzar arma, agarre (golpes y
    lanzamiento, y el lanzado que derriba a otro) y derribo → levantarse → invulnerable.

    PROPÓSITO
      Fijar las reglas de mundo.h y combate_agarre (docs/diseno-beat-em-up.md §2 y §5.3).

    SE CONECTA CON
      Incluye:  minitest.h, juego/mundo.h, juego/combate.h, enemigos/ninja.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_movimientos).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Combo, Máquina de estados
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/mundo.h"
#include "juego/combate.h"
#include "enemigos/ninja.h"

static void frames(Cuerpo *c, const Mando *m, u8 n)
{
    while (n--)
    {
        mundo_actualizar(c, m);
    }
}

void pruebas_movimientos(void)
{
    Cuerpo c;
    Mando m;
    Ninja n[NINJAS_MAX];
    u16 x0;
    u8 f, vida, eventos;

    /*--- Correr: derecha, soltar, derecha otra vez antes de 12 frames ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    m.derecha = 1; frames(&c, &m, 3);
    COMPROBAR(c.estado == CUERPO_ANDANDO && !c.corre);
    m.derecha = 0; frames(&c, &m, 5);
    m.derecha = 1; x0 = c.x; frames(&c, &m, 4);
    COMPROBAR(c.corre && c.estado == CUERPO_CORRIENDO && c.x == x0 + 4 * PASO_CORRER);
    m.derecha = 0; frames(&c, &m, 1);
    COMPROBAR(!c.corre);                                   /* Al soltar, deja de correr.  */
    m.derecha = 0; frames(&c, &m, VENTANA_DOBLE_TOQUE + 2);
    m.derecha = 1; frames(&c, &m, 2);
    COMPROBAR(!c.corre && c.estado == CUERPO_ANDANDO);    /* Demasiado tarde: anda.      */
    m.derecha = 0; frames(&c, &m, 3);
    m.izquierda = 1; frames(&c, &m, 2);
    COMPROBAR(!c.corre);                                   /* Otra dirección: no corre.   */

    /*--- Ataque aéreo: una vez por salto; derriba y quita 2 ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    m.saltar_pulsado = 1; m.saltar = 1; frames(&c, &m, 1);
    m.saltar_pulsado = 0; frames(&c, &m, 4);
    m.atacar_pulsado = 1; frames(&c, &m, 1); m.atacar_pulsado = 0;
    COMPROBAR(c.estado == CUERPO_AEREO && c.aereo == 1);
    frames(&c, &m, AEREO_ACTIVO_DESDE);
    COMPROBAR(mundo_golpe_activo(&c) && mundo_golpe_derriba(&c) && mundo_golpe_danio(&c) == 2);
    m.atacar_pulsado = 1; f = c.id; frames(&c, &m, 1); m.atacar_pulsado = 0;
    COMPROBAR(c.id == f);                                  /* Otro A en el aire: nada.    */
    frames(&c, &m, 40);
    COMPROBAR(c.z == 0 && c.aereo == 0 && !mundo_golpe_activo(&c));

    /*--- Especial A + B: invulnerable, golpea a los dos lados, cuesta 2 solo si conecta ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    m.atacar = 1; m.saltar_pulsado = 1; m.saltar = 1; frames(&c, &m, 1);
    memset(&m, 0, sizeof(m));
    COMPROBAR(c.estado == CUERPO_ESPECIAL && c.z == 0);     /* No salta.                   */
    COMPROBAR(mundo_recibir(&c, 4, 0) == 0 && c.vida == VIDA_TAROU);   /* Invulnerable.  */
    frames(&c, &m, ESPECIAL_ACTIVO_DESDE);
    COMPROBAR(mundo_golpe_activo(&c) && mundo_golpe_alrededor(&c));
    ninja_iniciar_todos(n);
    ninja_oleada(n, 0, 1);                                 /* Dos ninjas: delante y detrás. */
    n[0].x = 330; n[0].y = 200; n[0].dentro = 1;
    n[1].x = 270; n[1].y = 200; n[1].dentro = 1;
    COMPROBAR(combate_resolver(&c, n, COMBATE_ALCANCE) != 0);
    COMPROBAR(n[0].estado == NINJA_DERRIBADO && n[1].estado == NINJA_DERRIBADO);
    COMPROBAR(c.vida == VIDA_TAROU - COSTE_ESPECIAL);
    combate_resolver(&c, n, COMBATE_ALCANCE);
    COMPROBAR(c.vida == VIDA_TAROU - COSTE_ESPECIAL);      /* Cobra una sola vez.        */
    frames(&c, &m, ESPECIAL_FRAMES);
    COMPROBAR(c.estado == CUERPO_PARADO);
    c.vida = 1;                                            /* Nunca mata.                 */
    mundo_cobrar_especial(&c);
    COMPROBAR(c.vida == 1);
    c.cobro = 0; mundo_cobrar_especial(&c);
    COMPROBAR(c.vida == 1);

    /*--- Lanzar arma: A con arma, el evento sale en su frame y se queda sin arma ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    c.arma = ARMA_SHURIKEN;
    m.atacar_pulsado = 1; frames(&c, &m, 1); m.atacar_pulsado = 0;
    COMPROBAR(c.estado == CUERPO_LANZAR_ARMA && c.golpe == 0);
    eventos = 0;
    for (f = 1; f <= LANZAR_ARMA_FRAMES; f++)
    {
        mundo_actualizar(&c, &m);
        eventos = (u8)(eventos + (c.evento == EVENTO_LANZA_ARMA));
    }
    COMPROBAR(eventos == 1 && c.estado == CUERPO_PARADO);

    /*--- Agarre: andar contra un ninja AGARRE_EMPUJE frames ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    ninja_iniciar_todos(n);
    ninja_oleada(n, 0, 1);
    n[0].x = 340; n[0].y = 200; n[0].dentro = 1; n[0].estado = NINJA_PARADO; n[0].vida = 8;
    n[1].x = 380; n[1].y = 200; n[1].dentro = 1; n[1].estado = NINJA_PARADO;
    m.derecha = 1;
    for (f = 0; f < 40 && !c.agarra; f++)
    {
        mundo_actualizar(&c, &m);
        combate_agarre(&c, n);
    }
    COMPROBAR(c.agarra == 1 && c.estado == CUERPO_AGARRANDO && n[0].estado == NINJA_AGARRADO);
    COMPROBAR(n[0].x == c.x + AGARRE_DELANTE && n[0].y == c.y);
    /* A: golpe de agarre (1 de daño en su frame); dos veces. */
    m.derecha = 0;
    vida = n[0].vida;
    for (f = 0; f < 2; f++)
    {
        m.atacar_pulsado = 1; mundo_actualizar(&c, &m); combate_agarre(&c, n);
        m.atacar_pulsado = 0;
        while (c.estado == CUERPO_GOLPE_AGARRE)
        {
            mundo_actualizar(&c, &m);
            combate_agarre(&c, n);
        }
    }
    COMPROBAR(n[0].vida == vida - 2 && c.agarre_golpes == 2 && c.estado == CUERPO_AGARRANDO);
    /* Atrás + A: lanzamiento; sale volando y derriba al de detrás de él (n[1]). */
    m.izquierda = 1; m.atacar_pulsado = 1;
    mundo_actualizar(&c, &m); combate_agarre(&c, n);
    m.izquierda = 0; m.atacar_pulsado = 0;
    COMPROBAR(c.estado == CUERPO_LANZANDO);
    for (f = 0; f < LANZAR_FRAMES; f++)
    {
        mundo_actualizar(&c, &m);
        combate_agarre(&c, n);
        ninja_actualizar(&n[0], c.x, c.y);
        ninja_actualizar(&n[1], c.x, c.y);
    }
    COMPROBAR(c.agarra == 0 && n[0].estado == NINJA_DERRIBADO && n[0].vida == vida - 2 - DANIO_LANZAMIENTO);
    COMPROBAR(n[1].estado == NINJA_DERRIBADO);             /* El choque lo derribó.       */

    /*--- Tres golpes de agarre: lo suelta derribado ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    ninja_iniciar_todos(n);
    ninja_oleada(n, 0, 0);
    n[0].x = 340; n[0].y = 200; n[0].dentro = 1; n[0].estado = NINJA_PARADO; n[0].vida = 8;
    m.derecha = 1;
    for (f = 0; f < 40 && !c.agarra; f++)
    {
        mundo_actualizar(&c, &m);
        combate_agarre(&c, n);
    }
    m.derecha = 0;
    for (f = 0; f < AGARRE_GOLPES; f++)
    {
        m.atacar_pulsado = 1; mundo_actualizar(&c, &m); combate_agarre(&c, n);
        m.atacar_pulsado = 0;
        while (c.estado == CUERPO_GOLPE_AGARRE)
        {
            mundo_actualizar(&c, &m);
            combate_agarre(&c, n);
        }
    }
    COMPROBAR(c.agarra == 0 && c.estado == CUERPO_PARADO && n[0].estado == NINJA_DERRIBADO);
    COMPROBAR(n[0].vida == 8 - AGARRE_GOLPES);
    /* Sin hacer nada, se escapa. */
    mundo_iniciar(&c, 300, 200);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 0, 0);
    n[0].x = 340; n[0].y = 200; n[0].dentro = 1; n[0].estado = NINJA_PARADO;
    m.derecha = 1;
    for (f = 0; f < 40 && !c.agarra; f++)
    {
        mundo_actualizar(&c, &m);
        combate_agarre(&c, n);
    }
    m.derecha = 0;
    for (f = 0; f < AGARRE_MAX + 2; f++)
    {
        mundo_actualizar(&c, &m);
        combate_agarre(&c, n);
    }
    COMPROBAR(c.agarra == 0 && n[0].estado == NINJA_PARADO);

    /*--- Derribado con vida: cae, se levanta y queda invulnerable ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    COMPROBAR(mundo_derribar(&c, 3, 0) == 1 && c.estado == CUERPO_DERRIBADO && c.vida == VIDA_TAROU - 3);
    COMPROBAR(mundo_recibir(&c, 2, 0) == 0);               /* En el suelo no le pegan.    */
    frames(&c, &m, DERRIBO_FRAMES);
    COMPROBAR(c.estado == CUERPO_LEVANTARSE);
    COMPROBAR(mundo_recibir(&c, 2, 0) == 0);
    frames(&c, &m, LEVANTARSE_FRAMES);
    COMPROBAR(c.estado == CUERPO_PARADO && c.inv == INVULNERABLE_FRAMES);
    COMPROBAR(mundo_recibir(&c, 2, 0) == 0);
    frames(&c, &m, INVULNERABLE_FRAMES);
    COMPROBAR(mundo_recibir(&c, 2, 0) == 1);

    /*--- El combo de Tarou: 4 golpes, el cuarto derriba ---*/
    mundo_iniciar(&c, 300, 200);
    memset(&m, 0, sizeof(m));
    c.golpes_max = GOLPES_TAROU;
    c.golpe = 3;
    COMPROBAR(!mundo_golpe_derriba(&c) && mundo_golpe_danio(&c) == 1);
    c.golpe = 4;
    COMPROBAR(mundo_golpe_derriba(&c) && mundo_golpe_danio(&c) == 2);
}
