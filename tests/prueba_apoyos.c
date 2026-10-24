/*---------------------------------------------------------------------------------
    prueba_apoyos.c — Pruebas de juego/apoyos.c.

    PROPÓSITO
      Fijar la barra de ataque y los tres apoyos (spec §5.4 y las decisiones del
      2026-10-24): la barra está bloqueada hasta el monasterio; se llena con 48
      golpes que conectan, sin pasarse, y no suma mientras actúa un apoyo; invocar
      la vacía (un solo uso); R recorre los apoyos disponibles; cada apoyo tiene su
      cronología (3 tajos de Lucinda, el levantar-curar-soltar de Rex, la embestida
      de Fenrir); contra jefes el daño es la mitad y no derriban; Rex cura 16 sin
      pasar de la vida máxima.

    SE CONECTA CON
      Incluye:  minitest.h, juego/apoyos.h, juego/cifras.h, juego/luchador.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_apoyos).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Apoyo, Barra de ataque
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/apoyos.h"
#include "juego/cifras.h"
#include "juego/luchador.h"

/* Llena la barra a golpes y devuelve en qué golpe se llenó (0 si no se llenó). */
static u8 llenar_a_golpes(Apoyos *a, u8 golpes)
{
    u8 i;
    u8 lleno_en;

    lleno_en = 0;
    for (i = 1; i <= golpes; i++)
    {
        if (apoyos_golpe_conecto(a))
            lleno_en = i;
    }
    return lleno_en;
}

/* Corre el apoyo en curso entero: cuenta los eventos y en qué frame ocurrió cada uno. */
typedef struct
{
    u8 golpes, levantar, curar, fin;
    u8 frame_golpe[3];
    u8 frame_levantar, frame_curar, frame_fin;
} Resumen;

static void correr_apoyo(Apoyos *a, Resumen *r)
{
    u8 ev;
    u8 t;

    r->golpes = r->levantar = r->curar = r->fin = 0;
    r->frame_golpe[0] = r->frame_golpe[1] = r->frame_golpe[2] = 0;
    r->frame_levantar = r->frame_curar = r->frame_fin = 0;
    for (t = 1; t <= 200 && a->en_curso; t++)
    {
        ev = apoyos_actualizar(a);
        if (ev & APOYO_EV_GOLPE)
        {
            if (r->golpes < 3)
                r->frame_golpe[r->golpes] = t;
            r->golpes++;
        }
        if (ev & APOYO_EV_LEVANTAR)
        {
            r->levantar++;
            r->frame_levantar = t;
        }
        if (ev & APOYO_EV_CURAR)
        {
            r->curar++;
            r->frame_curar = t;
        }
        if (ev & APOYO_EV_FIN)
        {
            r->fin++;
            r->frame_fin = t;
        }
    }
}

void pruebas_apoyos(void)
{
    Apoyos a;
    Resumen r;
    Luchador L;
    Carril c;
    Mando m;
    u8 i;

    c.x_min_fp = PF(16);
    c.x_max_fp = PF(240);
    c.y_arriba_fp = PF(150);
    c.y_abajo_fp = PF(208);

    /* Antes del monasterio la barra no se llena, aunque haya golpes. */
    apoyos_iniciar(&a);
    apoyos_desbloquear(&a, APOYO_LUCINDA);
    COMPROBAR(llenar_a_golpes(&a, 60) == 0 && a.golpes == 0);
    COMPROBAR(apoyos_puede_invocar(&a) == 0);

    /* Activada (el monasterio): la barra EMPIEZA LLENA, para probar el primer apoyo; activarla otra
       vez no la vuelve a llenar. */
    apoyos_activar_barra(&a);
    COMPROBAR(a.golpes == BARRA_LLENA && apoyos_puede_invocar(&a) == 1);
    a.golpes = 0;
    apoyos_activar_barra(&a);
    COMPROBAR(a.golpes == 0);

    /* Se llena en el golpe 48 exacto, no antes, y no se pasa. */
    COMPROBAR(llenar_a_golpes(&a, 47) == 0 && a.golpes == 47);
    COMPROBAR(apoyos_puede_invocar(&a) == 0);
    COMPROBAR(apoyos_golpe_conecto(&a) == 1 && a.golpes == BARRA_LLENA);
    COMPROBAR(apoyos_golpe_conecto(&a) == 0 && a.golpes == BARRA_LLENA);   /* No se pasa.       */
    COMPROBAR(apoyos_puede_invocar(&a) == 1);

    /* Invocar: empieza el apoyo elegido, la barra vuelve a 0 (un solo uso). */
    COMPROBAR(apoyos_invocar(&a) == 1);
    COMPROBAR(a.en_curso == APOYO_LUCINDA + 1 && a.golpes == 0);
    COMPROBAR(apoyos_puede_invocar(&a) == 0);
    COMPROBAR(apoyos_invocar(&a) == 0);              /* No se invoca dos veces a la vez.    */

    /* Mientras actúa un apoyo, los golpes no suman. */
    COMPROBAR(apoyos_golpe_conecto(&a) == 0 && a.golpes == 0);

    /* Lucinda: 3 tajos a los 12, 24 y 36 frames; acaba a los 48; una sola vez cada evento. */
    correr_apoyo(&a, &r);
    COMPROBAR(r.golpes == 3 && r.frame_golpe[0] == 12 && r.frame_golpe[1] == 24 && r.frame_golpe[2] == 36);
    COMPROBAR(r.levantar == 0 && r.curar == 0);
    COMPROBAR(r.fin == 1 && r.frame_fin == 48 && a.en_curso == 0);
    COMPROBAR(apoyos_actualizar(&a) == APOYO_EV_NADA);   /* Sin apoyo en curso, nada.       */

    /* Acabado el apoyo, la barra vuelve a llenarse. */
    COMPROBAR(apoyos_golpe_conecto(&a) == 0 && a.golpes == 1);

    /* Los 3 tajos de Lucinda suman 24 contra un ninja normal; contra un jefe, la mitad. */
    COMPROBAR(3 * apoyos_dano(APOYO_LUCINDA, 0) == 24);
    COMPROBAR(apoyos_dano(APOYO_LUCINDA, 1) == 4);

    /* Rex: levanta y cura a los 20; suelta (el golpe) a los 60; acaba a los 80. */
    apoyos_iniciar(&a);
    apoyos_activar_barra(&a);
    apoyos_desbloquear(&a, APOYO_REX);
    COMPROBAR(a.seleccionado == APOYO_REX);          /* El primero que llega, elegido.      */
    apoyos_llenar(&a);
    COMPROBAR(apoyos_invocar(&a) == 1 && a.en_curso == APOYO_REX + 1);
    correr_apoyo(&a, &r);
    COMPROBAR(r.levantar == 1 && r.frame_levantar == 20 && r.curar == 1 && r.frame_curar == 20);
    COMPROBAR(r.golpes == 1 && r.frame_golpe[0] == 60);
    COMPROBAR(r.fin == 1 && r.frame_fin == 80);
    COMPROBAR(apoyos_dano(APOYO_REX, 0) == 16 && apoyos_dano(APOYO_REX, 1) == 8);

    /* Fenrir: una embestida a los 30; acaba a los 50. */
    apoyos_iniciar(&a);
    apoyos_activar_barra(&a);
    apoyos_desbloquear(&a, APOYO_FENRIR);
    apoyos_llenar(&a);
    COMPROBAR(apoyos_invocar(&a) == 1 && a.en_curso == APOYO_FENRIR + 1);
    correr_apoyo(&a, &r);
    COMPROBAR(r.golpes == 1 && r.frame_golpe[0] == 30 && r.levantar == 0 && r.curar == 0);
    COMPROBAR(r.fin == 1 && r.frame_fin == 50);
    COMPROBAR(apoyos_dano(APOYO_FENRIR, 0) == 20 && apoyos_dano(APOYO_FENRIR, 1) == 10);

    /* Derribo: a los normales sí, a los jefes no (ningún apoyo). */
    for (i = 0; i < APOYOS; i++)
    {
        COMPROBAR(apoyos_derriba(i, 0) == 1);
        COMPROBAR(apoyos_derriba(i, 1) == 0);
    }

    /* Selección con R: recorre solo los disponibles y da la vuelta. */
    apoyos_iniciar(&a);
    apoyos_siguiente(&a);                            /* Sin ninguno disponible: no cambia.  */
    COMPROBAR(a.seleccionado == APOYO_LUCINDA);
    apoyos_desbloquear(&a, APOYO_LUCINDA);
    apoyos_desbloquear(&a, APOYO_FENRIR);            /* Rex todavía no se ha unido.         */
    COMPROBAR(a.seleccionado == APOYO_LUCINDA);
    apoyos_siguiente(&a);
    COMPROBAR(a.seleccionado == APOYO_FENRIR);       /* Se salta a Rex.                     */
    apoyos_siguiente(&a);
    COMPROBAR(a.seleccionado == APOYO_LUCINDA);      /* Da la vuelta.                       */
    apoyos_desbloquear(&a, APOYO_REX);
    apoyos_siguiente(&a);
    COMPROBAR(a.seleccionado == APOYO_REX);
    apoyos_desbloquear(&a, 7);                       /* Un apoyo que no existe: se ignora.  */
    COMPROBAR(a.disponibles == 7);

    /* Con la barra llena pero el elegido sin desbloquear no se puede invocar. */
    apoyos_iniciar(&a);
    apoyos_activar_barra(&a);
    apoyos_llenar(&a);
    COMPROBAR(apoyos_puede_invocar(&a) == 0);        /* Ningún apoyo se ha unido todavía.   */
    apoyos_desbloquear(&a, APOYO_REX);
    COMPROBAR(apoyos_puede_invocar(&a) == 1);

    /* Caen los dos protagonistas: vuelven con la barra llena (lo hace quien llama). */
    apoyos_iniciar(&a);
    apoyos_activar_barra(&a);
    apoyos_llenar(&a);
    COMPROBAR(a.golpes == BARRA_LLENA);

    /* Rex cura 16 al activo, sin pasar de 64; los demás no curan. */
    COMPROBAR(apoyos_curar_vida(30, APOYO_REX) == 46);
    COMPROBAR(apoyos_curar_vida(60, APOYO_REX) == VIDA_PROTAGONISTA);
    COMPROBAR(apoyos_curar_vida(64, APOYO_REX) == VIDA_PROTAGONISTA);
    COMPROBAR(apoyos_curar_vida(30, APOYO_LUCINDA) == 30);
    COMPROBAR(apoyos_curar_vida(30, APOYO_FENRIR) == 30);

    /* El protagonista, mientras invoca, queda en pose fija e invulnerable, sin poder hacer un relevo. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    COMPROBAR(luchador_puede_invocar(&L) == 1);
    luchador_invocar(&L);
    COMPROBAR(L.estado == LQ_INVOCAR && L.g.invulnerable == 255);
    COMPROBAR(luchador_puede_salir(&L) == 0 && luchador_puede_invocar(&L) == 0);
    m.izquierda = 1;
    m.derecha = 0;
    m.arriba = 0;
    m.abajo = 0;
    m.correr = 0;
    m.saltar = 0;
    m.saltar_pulsado = 0;
    m.atacar_pulsado = 1;                            /* Ni andar ni atacar mientras invoca. */
    m.espada_pulsado = 0;
    m.hud_pulsado = 0;
    m.salir_pulsado = 0;
    m.actividad = 0;
    m.relevo_pulsado = 0;
    m.invocar_pulsado = 0;
    m.apoyo_pulsado = 0;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_INVOCAR && L.c.vx_fp == 0);
    luchador_terminar_invocar(&L);
    COMPROBAR(L.estado == LQ_QUIETO && L.g.invulnerable == 0);
    luchador_terminar_invocar(&L);                   /* Sin estar invocando: no hace nada.  */
    COMPROBAR(L.estado == LQ_QUIETO);

    /* En el aire no se puede invocar. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    mundo_saltar(&L.c);
    L.estado = LQ_SALTO;
    COMPROBAR(luchador_puede_invocar(&L) == 0);
}
