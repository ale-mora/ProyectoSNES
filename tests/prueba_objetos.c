/*---------------------------------------------------------------------------------
    prueba_objetos.c — Pruebas de objetos/objetos.c (2026-11-05): romper un barril,
    recoger comida y armas, lanzar y recibir shuriken.

    PROPÓSITO
      Fijar las reglas de objetos.h.

    SE CONECTA CON
      Incluye:  minitest.h, objetos/objetos.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_objetos).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Sprite
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "objetos/objetos.h"

void pruebas_objetos(void)
{
    static const u16 XS[2] = {350, 600};
    static const u16 YS[2] = {200, 190};
    static const u8 CONTIENE[2] = {ITEM_ONIGIRI, ITEM_SHURIKEN};
    Objetos o;
    Cuerpo c;
    Mando m;
    Ninja n[NINJAS_MAX];
    u16 f;

    objetos_iniciar(&o, XS, YS, CONTIENE, 2);
    ninja_iniciar_todos(n);
    COMPROBAR(o.r[0].vivo && o.r[1].vivo && !o.r[2].vivo && !o.item.vivo);

    /*--- Un golpe rompe el barril: astillas, chispa y el onigiri en el suelo ---*/
    mundo_iniciar(&c, 310, 200);
    c.golpe = 1; c.reloj = GOLPE_ACTIVO_DESDE;
    objetos_actualizar(&o, &c, n, 52, 200);
    COMPROBAR(!o.r[0].vivo && o.r[1].vivo && o.item.vivo && o.item.tipo == ITEM_ONIGIRI);
    COMPROBAR(o.item_nuevo == 1 && o.a[0].vivo && o.a[1].vivo && o.chispa != 0);
    /* Se recoge pasando por encima: cura CURA_ONIGIRI. */
    mundo_iniciar(&c, 350, 200);
    c.vida = 10;
    objetos_actualizar(&o, &c, n, 52, 200);
    COMPROBAR(!o.item.vivo && c.vida == 10 + CURA_ONIGIRI);
    /* Con la vida casi llena no pasa del tope. */
    o.item.vivo = 1; o.item.tipo = ITEM_PESCADO; o.item.reloj = 10;
    c.vida = VIDA_TAROU - 2;
    objetos_actualizar(&o, &c, n, 52, 200);
    COMPROBAR(c.vida == VIDA_TAROU);
    /* El ítem que nadie coge se va. */
    o.item.vivo = 1; o.item.x = 900; o.item.reloj = 5;
    for (f = 0; f < 6; f++) objetos_actualizar(&o, &c, n, 52, 200);
    COMPROBAR(!o.item.vivo);

    /*--- El shuriken: recogerlo da el arma; lanzarlo hiere a un ninja ---*/
    mundo_iniciar(&c, 560, 190);
    c.golpe = 1; c.reloj = GOLPE_ACTIVO_DESDE;
    objetos_actualizar(&o, &c, n, 52, 400);                  /* Rompe la caja del shuriken. */
    COMPROBAR(o.item.vivo && o.item.tipo == ITEM_SHURIKEN);
    mundo_iniciar(&c, 600, 190);
    objetos_actualizar(&o, &c, n, 52, 400);
    COMPROBAR(c.arma == ARMA_SHURIKEN && !o.item.vivo);
    ninja_oleada(n, 0, 0);
    n[0].x = 680; n[0].y = 190; n[0].dentro = 1; n[0].vida = 4;
    objetos_lanzar(&o, 610, 190, 0, 1, ITEM_SHURIKEN);
    for (f = 0; f < 30 && o.p[0].vivo; f++) objetos_actualizar(&o, &c, n, 52, 500);
    COMPROBAR(!o.p[0].vivo && n[0].vida == 4 - DANIO_SHURIKEN && n[0].estado == NINJA_RECIBIR);

    /*--- El shuriken enemigo hiere al protagonista ---*/
    mundo_iniciar(&c, 600, 190);
    memset(&m, 0, sizeof(m));
    objetos_lanzar(&o, 700, 190, 1, 0, ITEM_SHURIKEN);
    for (f = 0; f < 40 && o.p[0].vivo; f++) objetos_actualizar(&o, &c, n, 52, 500);
    COMPROBAR(c.vida == VIDA_TAROU - DANIO_SHURIKEN && c.estado == CUERPO_RECIBIR);
    /* Y se pierde si sale de la pantalla. */
    objetos_lanzar(&o, 700, 100, 1, 0, ITEM_SHURIKEN);
    for (f = 0; f < PROYECTIL_FRAMES + 2; f++) objetos_actualizar(&o, &c, n, 52, 500);
    COMPROBAR(!o.p[0].vivo && !o.p[1].vivo);
}
