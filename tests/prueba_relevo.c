/*---------------------------------------------------------------------------------
    prueba_relevo.c — Pruebas de juego/cambio_protagonista.c (el relevo).

    PROPÓSITO
      Fijar el relevo de la spec §4 con las decisiones del 2026-10-24: L cambia de
      protagonista solo con el enfriamiento terminado, sin estar bloqueado y si el
      otro tiene vida; el que entra cae desde arriba sobre el sitio del que sale y
      da el golpe de entrada una sola vez (4 de daño, empuje a menos de 48 px a la
      misma profundidad); el que espera fuera se cura 1 cada 120 frames hasta 48; si
      el activo cae KO entra el otro sin enfriamiento y con 60 frames de protección;
      si caen los dos, se pierde una vida.

    SE CONECTA CON
      Incluye:  minitest.h, juego/cambio_protagonista.h, juego/combate.h, juego/cifras.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_relevo).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Relevo, Golpe de entrada
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/cambio_protagonista.h"
#include "juego/combate.h"
#include "juego/cifras.h"

static void limpiar(Mando *m)
{
    m->izquierda = 0;
    m->derecha = 0;
    m->arriba = 0;
    m->abajo = 0;
    m->correr = 0;
    m->saltar = 0;
    m->saltar_pulsado = 0;
    m->atacar_pulsado = 0;
    m->espada_pulsado = 0;
    m->hud_pulsado = 0;
    m->salir_pulsado = 0;
    m->actividad = 0;
    m->relevo_pulsado = 0;
}

/* `n` frames sin tocar nada. */
static void pasar(Relevo *r, const Carril *c, u8 n)
{
    Mando m;

    limpiar(&m);
    while (n--)
        relevo_actualizar(r, &m, c);
}

void pruebas_relevo(void)
{
    Relevo r;
    Mando m;
    Carril c;
    Cuerpo enemigo;
    u8 n;
    u8 veces;

    c.x_min_fp = PF(16);
    c.x_max_fp = PF(240);
    c.y_arriba_fp = PF(150);
    c.y_abajo_fp = PF(208);

    /* Al empezar: Tarou activo, Bárbara fuera, las dos con 64. */
    relevo_iniciar(&r, 100, 180);
    COMPROBAR(r.activo == QUIEN_TAROU && relevo_activo(&r) == &r.tarou && relevo_fuera(&r) == &r.barbara);
    COMPROBAR(r.tarou.g.vida == 64 && r.barbara.g.vida == 64);
    COMPROBAR(r.tarou.quien == QUIEN_TAROU && r.barbara.quien == QUIEN_BARBARA);

    /* L: cambia. El que sale deja una nube donde estaba; el que entra cae sobre ese sitio, mirando
       hacia donde miraba, invulnerable, y empieza el enfriamiento. */
    limpiar(&m);
    m.izquierda = 1;
    relevo_actualizar(&r, &m, &c);                   /* Tarou anda a la izquierda.          */
    limpiar(&m);
    m.relevo_pulsado = 1;
    n = relevo_actualizar(&r, &m, &c);
    COMPROBAR(n == RELEVO_CAMBIO && r.activo == QUIEN_BARBARA);
    COMPROBAR(r.barbara.estado == LQ_ENTRADA && r.barbara.c.en_suelo == 0);
    COMPROBAR(r.barbara.c.x_fp == r.tarou.c.x_fp && r.barbara.c.y_fp == r.tarou.c.y_fp);
    COMPROBAR(r.barbara.mira_izquierda == r.tarou.mira_izquierda);
    COMPROBAR(r.nube == FRAMES_NUBE - 0 && r.nube_x == r.tarou.c.x_fp);
    COMPROBAR(r.barbara.g.invulnerable > 0);
    COMPROBAR(r.enfriamiento == ENFRIAMIENTO_RELEVO);

    /* Con el enfriamiento en marcha, L no hace nada. */
    pasar(&r, &c, 40);                               /* Ya aterrizó y acabó su golpe de entrada. */
    COMPROBAR(r.barbara.estado == LQ_QUIETO && r.enfriamiento == ENFRIAMIENTO_RELEVO - 40);
    limpiar(&m);
    m.relevo_pulsado = 1;
    COMPROBAR(relevo_actualizar(&r, &m, &c) == RELEVO_NADA && r.activo == QUIEN_BARBARA);

    /* Acabado el enfriamiento, L vuelve a cambiar: a Tarou. */
    pasar(&r, &c, ENFRIAMIENTO_RELEVO);
    COMPROBAR(r.enfriamiento == 0);
    limpiar(&m);
    m.relevo_pulsado = 1;
    COMPROBAR(relevo_actualizar(&r, &m, &c) == RELEVO_CAMBIO && r.activo == QUIEN_TAROU);

    /* El golpe de entrada: se da una sola vez, al aterrizar. */
    limpiar(&m);
    veces = 0;
    for (n = 0; n < 60; n++)
    {
        relevo_actualizar(&r, &m, &c);
        veces = (u8)(veces + luchador_entrada_activa(relevo_activo(&r)));
    }
    COMPROBAR(veces == 1);

    /* Empuje: a menos de 48 px y a la misma profundidad sí; a 60 px o en otra y, no. */
    mundo_colocar(&enemigo, 100, 180);
    r.tarou.c.x_fp = PF(100);
    r.tarou.c.y_fp = PF(180);
    enemigo.x_fp = PF(100 + 47);
    COMPROBAR(combate_empuje_cercano(&r.tarou.c, &enemigo, ALCANCE_ENTRADA_RELEVO) == 1);
    enemigo.x_fp = PF(100 - 48);
    COMPROBAR(combate_empuje_cercano(&r.tarou.c, &enemigo, ALCANCE_ENTRADA_RELEVO) == 1);
    enemigo.x_fp = PF(100 + 60);
    COMPROBAR(combate_empuje_cercano(&r.tarou.c, &enemigo, ALCANCE_ENTRADA_RELEVO) == 0);
    enemigo.x_fp = PF(100 + 20);
    enemigo.y_fp = PF(180 + 12);
    COMPROBAR(combate_empuje_cercano(&r.tarou.c, &enemigo, ALCANCE_ENTRADA_RELEVO) == 0);

    /* Bloqueado: recibiendo un golpe, no puede salir. */
    relevo_iniciar(&r, 100, 180);
    r.tarou.estado = LQ_RECIBIR;
    limpiar(&m);
    m.relevo_pulsado = 1;
    COMPROBAR(relevo_actualizar(&r, &m, &c) == RELEVO_NADA && r.activo == QUIEN_TAROU);

    /* Si el de fuera no tiene vida, tampoco. */
    relevo_iniciar(&r, 100, 180);
    r.barbara.g.vida = 0;
    COMPROBAR(relevo_actualizar(&r, &m, &c) == RELEVO_NADA && r.activo == QUIEN_TAROU);

    /* La cura de fuera: +1 cada 120 frames, hasta 48 de 64 (no se cura del todo). */
    relevo_iniciar(&r, 100, 180);
    r.barbara.g.vida = 40;
    pasar(&r, &c, FRAMES_CURA_FUERA - 1);
    COMPROBAR(r.barbara.g.vida == 40);
    pasar(&r, &c, 1);
    COMPROBAR(r.barbara.g.vida == 41);
    pasar(&r, &c, FRAMES_CURA_FUERA);
    COMPROBAR(r.barbara.g.vida == 42);
    r.barbara.g.vida = TOPE_CURA_FUERA;
    pasar(&r, &c, FRAMES_CURA_FUERA * 2);
    COMPROBAR(r.barbara.g.vida == TOPE_CURA_FUERA);  /* En el tope no sube más.             */
    r.barbara.g.vida = 60;                           /* Por encima del tope: tampoco baja.  */
    pasar(&r, &c, FRAMES_CURA_FUERA);
    COMPROBAR(r.barbara.g.vida == 60);

    /* El activo cae KO (su KO terminó): entra el otro al instante, SIN enfriamiento y con 60 frames
       de protección (los de un levantarse). */
    relevo_iniciar(&r, 100, 180);
    r.tarou.g.vida = 0;
    r.tarou.vencido = 1;
    limpiar(&m);
    n = relevo_actualizar(&r, &m, &c);
    COMPROBAR(n == RELEVO_CAMBIO && r.activo == QUIEN_BARBARA);
    COMPROBAR(r.enfriamiento == 0);
    COMPROBAR(r.barbara.estado == LQ_ENTRADA || r.barbara.estado == LQ_ENTRADA_GOLPE);
    COMPROBAR(r.barbara.g.invulnerable >= FRAMES_INVULNERABLE - 2);

    /* Y puede hacer un relevo voluntario enseguida (no hay enfriamiento tras el KO), en cuanto el
       que está fuera (a 0) haya recuperado un punto: el KO'd se cura y ya tiene vida. */
    pasar(&r, &c, FRAMES_CURA_FUERA);
    COMPROBAR(r.tarou.g.vida == 1);
    limpiar(&m);
    m.relevo_pulsado = 1;
    COMPROBAR(relevo_actualizar(&r, &m, &c) == RELEVO_CAMBIO && r.activo == QUIEN_TAROU);
    COMPROBAR(r.tarou.vencido == 0);                 /* Vuelve a estar en pie.              */
    pasar(&r, &c, 3);
    COMPROBAR(relevo_actualizar(&r, &m, &c) != RELEVO_PIERDE_VIDA);

    /* Si caen los dos, se pierde una vida. */
    relevo_iniciar(&r, 100, 180);
    r.tarou.g.vida = 0;
    r.tarou.vencido = 1;
    r.barbara.g.vida = 0;
    limpiar(&m);
    COMPROBAR(relevo_actualizar(&r, &m, &c) == RELEVO_PIERDE_VIDA);
    COMPROBAR(r.activo == QUIEN_TAROU);              /* No cambia: quien llama reinicia.    */
}
