/*---------------------------------------------------------------------------------
    prueba_enemigos.c — Pruebas de los enemigos nuevos (2026-11-05): ranuras por tipo,
    el rápido que guarda distancia y lanza shuriken, el kendoka que bloquea de frente,
    golpea pesado y agarra, el ninja normal que alterna puño y patada y se pone en
    guardia, y el jefe que embiste y llama ninjas.

    PROPÓSITO
      Fijar las reglas de enemigos/ninja.c y de combate_presa / combate_resolver.

    SE CONECTA CON
      Incluye:  minitest.h, enemigos/ninja.h, juego/combate.h, juego/mundo.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_enemigos).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: IA, Máquina de estados
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "enemigos/ninja.h"
#include "juego/combate.h"
#include "juego/mundo.h"

/* Un enemigo del tipo `tipo` ya dentro de la pantalla en (x, 200), mirando a la izquierda. */
static Ninja *uno(Ninja *n, u8 tipo, u16 x)
{
    u8 t = tipo;

    ninja_iniciar_todos(n);
    ninja_oleada_tipos(n, 0, &t, 1);
    if (n[0].vivo) { n[0].x = x; n[0].y = 200; n[0].dentro = 1; n[0].espera = 0; n[0].mira_izq = 1; return n; }
    n[1].x = x; n[1].y = 200; n[1].dentro = 1; n[1].espera = 0; n[1].mira_izq = 1;
    return n + 1;
}

void pruebas_enemigos(void)
{
    Ninja n[NINJAS_MAX], *p;
    Cuerpo c;
    Mando m;
    u16 f;
    u8 tipos[3], eventos, k;

    /*--- Ranuras por tipo ---*/
    ninja_iniciar_todos(n);
    tipos[0] = NINJA_MEDIANO; tipos[1] = NINJA_KENDOKA; tipos[2] = NINJA_RAPIDO;
    COMPROBAR(ninja_oleada_tipos(n, 0, tipos, 3) == 3);
    COMPROBAR(n[0].tipo == NINJA_RAPIDO && n[1].tipo == NINJA_MEDIANO && n[2].tipo == NINJA_KENDOKA);
    ninja_iniciar_todos(n);
    tipos[0] = NINJA_JEFE; tipos[1] = NINJA_GRANDE; tipos[2] = NINJA_NORMAL;
    COMPROBAR(ninja_oleada_tipos(n, 0, tipos, 3) == 2);    /* El grande no cabe: el jefe usa el 2. */
    COMPROBAR(n[1].tipo == NINJA_JEFE && n[1].vida == 40 && !n[2].vivo && n[0].tipo == NINJA_NORMAL);
    ninja_iniciar_todos(n);
    tipos[0] = NINJA_GRANDE;
    COMPROBAR(ninja_oleada_tipos(n, 0, tipos, 1) == 1 && !n[0].vivo && n[1].vivo);   /* Nunca en la 0. */

    /*--- El rápido: se aleja si está cerca y, a distancia, lanza su shuriken ---*/
    p = uno(n, NINJA_RAPIDO, 330);
    for (f = 0; f < 60; f++)
    {
        ninja_actualizar(p, 300, 200);
    }
    COMPROBAR(p->x - 300 >= NINJA_RAPIDO_CERCA && p->x - 300 <= NINJA_RAPIDO_LEJOS);
    COMPROBAR(ninja_intentar_atacar(p, 300, 200, 1) == 1 && p->estado == NINJA_DISPARANDO);
    eventos = 0;
    for (f = 0; f < NINJA_DISPARO_FRAMES; f++)
    {
        ninja_actualizar(p, 300, 200);
        eventos = (u8)(eventos + (p->evento == NINJA_EVENTO_SHURIKEN));
    }
    COMPROBAR(eventos == 1 && p->estado == NINJA_PARADO);

    /*--- El kendoka: bloquea de frente (no por la espalda ni el aéreo) ---*/
    p = uno(n, NINJA_KENDOKA, 340);                         /* Mira a la izquierda.        */
    COMPROBAR(ninja_bloquea(p, 0, 0, 10) == 1 && p->estado == NINJA_BLOQUEO);   /* Golpe desde la izq. */
    COMPROBAR(ninja_bloquea(p, 0, 0, 10) == 0);              /* El mismo golpe: ya visto.   */
    COMPROBAR(ninja_bloquea(p, 1, 0, 11) == 0);              /* Por la espalda.             */
    COMPROBAR(ninja_bloquea(p, 0, 1, 12) == 0);              /* Aéreo o especial: rompe.    */
    /* Con el combate: el combo de frente rebota, la vida no baja. */
    p = uno(n, NINJA_KENDOKA, 340);
    mundo_iniciar(&c, 300, 200);
    c.golpe = 1; c.reloj = GOLPE_ACTIVO_DESDE; c.id = 20;
    COMPROBAR(combate_resolver(&c, n, COMBATE_ALCANCE) == PARADA_GOLPE && p->vida == 16);
    /* Su golpe pesado derriba al protagonista. */
    p = uno(n, NINJA_KENDOKA, 340);
    ninja_contexto(0, 0, 300, 200, 0);
    mundo_iniciar(&c, 300, 200);
    COMPROBAR(ninja_intentar_atacar(p, 300, 200, 1) == 1 && p->ataque == NINJA_ATQ_PESADO);
    for (f = 0; f < NINJA_PESADO_FRAMES; f++)
    {
        ninja_actualizar(p, 300, 200);
        combate_ninjas_resolver(n, &c);
    }
    COMPROBAR(c.estado == CUERPO_DERRIBADO && c.vida == VIDA_TAROU - 4);

    /*--- Un enemigo agarra al protagonista y lo lanza ---*/
    for (k = 0; k < 40; k++)                                 /* El azar: 1 de cada 4.       */
    {
        p = uno(n, NINJA_KENDOKA, 330);
        mundo_iniciar(&c, 300, 200);
        memset(&m, 0, sizeof(m));
        combate_contexto(&c);
        ninja_intentar_atacar(p, 300, 200, 1);
        if (p->estado == NINJA_AGARRANDO)
        {
            break;
        }
    }
    COMPROBAR(p->estado == NINJA_AGARRANDO);
    combate_presa(&c, n);
    COMPROBAR(c.estado == CUERPO_AGARRADO && c.preso != 0);
    for (f = 0; f < NINJA_AGARRE_FRAMES + NINJA_LANZAR_FRAMES; f++)
    {
        mundo_actualizar(&c, &m);
        ninja_actualizar(p, c.x, c.y);
        combate_presa(&c, n);
    }
    COMPROBAR(c.preso == 0 && c.estado == CUERPO_DERRIBADO && c.vida == VIDA_TAROU - 5);

    /*--- El normal: puño, patada, puño...; y se pone en guardia si ve venir un golpe ---*/
    p = uno(n, NINJA_NORMAL, 340);
    ninja_contexto(0, 0, 300, 200, 0);
    ninja_intentar_atacar(p, 300, 200, 1);
    COMPROBAR(p->ataque == NINJA_ATQ_PUNO);
    for (f = 0; f < NINJA_ATAQUE_FRAMES; f++) ninja_actualizar(p, 300, 200);
    p->espera = 0;
    ninja_intentar_atacar(p, 300, 200, 1);
    COMPROBAR(p->ataque == NINJA_ATQ_PATADA && ninja_alcance(p) == NINJA_PATADA_ALCANCE);
    eventos = 0;
    p = uno(n, NINJA_NORMAL, 340);                           /* (Una sola vez: la semilla sigue.) */
    for (k = 1; k < 60; k++)                                 /* Golpes distintos: alguno lo para. */
    {
        p->estado = NINJA_PARADO; p->x = 340; p->espera = 10;
        ninja_contexto(0, k, 300, 200, 0);                   /* Le ataca desde la izquierda. */
        ninja_actualizar(p, 300, 200);
        eventos = (u8)(eventos + (p->estado == NINJA_BLOQUEO));
    }
    COMPROBAR(eventos > 5 && eventos < 30);                  /* Más o menos 1 de cada 4.    */
    ninja_contexto(0, 0, 0, 0, 0);

    /*--- El jefe: embiste si está lejos; enfadado, llama a un ninja ---*/
    p = uno(n, NINJA_JEFE, 450);
    COMPROBAR(ninja_intentar_atacar(p, 300, 200, 1) == 1 && p->estado == NINJA_CARGANDO);
    COMPROBAR(ninja_golpe_activo(p) && ninja_derriba(p));
    for (f = 0; f < NINJA_CARGA_FRAMES; f++) ninja_actualizar(p, 300, 200);
    COMPROBAR(p->estado == NINJA_PARADO && p->x == 450 - NINJA_CARGA_PASO * NINJA_CARGA_FRAMES);
    p->vida = NINJA_JEFE_FURIA;
    p->convoca = 2;
    eventos = 0;
    for (f = 0; f < 40; f++)
    {
        ninja_actualizar(p, p->x - 40, 200);
        eventos = (u8)(eventos + (p->evento == NINJA_EVENTO_CONVOCAR));
    }
    COMPROBAR(eventos == 1);
}
