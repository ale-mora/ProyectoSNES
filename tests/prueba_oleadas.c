/*---------------------------------------------------------------------------------
    prueba_oleadas.c — Pruebas de juego/fase1.c y juego/oleadas.c.

    PROPÓSITO
      Fijar la fase 1 tal como la decidió el autor (2026-10-24) y cómo se recorre:
      las cuatro zonas (1536, 1280, 768 y 1024 px), sus oleadas (3, 3, 1 y 2), el
      reparto de comida y la única vida extra; que ninguna oleada pasa de 3 enemigos y
      el pesado es rojo; la cámara se bloquea en la x de la oleada hasta vencerla; los
      enemigos salen de uno en uno cada FRAMES_ENTRE_ENEMIGOS frames; un frame con SPAWN
      no puede dar la oleada por vencida; las escenas van antes o después y el «GO →»
      espera a que acaben; y recorridas enteras, cada zona sale como se decidió.

    SE CONECTA CON
      Incluye:  minitest.h, juego/fase1.h, juego/oleadas.h, juego/apoyos.h, juego/cifras.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_oleadas).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Fase, Zona, Oleada, Bloqueo de cámara
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/fase1.h"
#include "juego/oleadas.h"
#include "juego/apoyos.h"
#include "juego/cifras.h"

/* Lo que pasó en una zona recorrida entera. */
typedef struct
{
    u16 spawns;                      /* Enemigos que salieron.                             */
    u16 por_tipo[4];                 /* Cuántos de cada TIPO_*.                            */
    u16 gos;                         /* Veces que salió «GO →».                            */
    u8 escenas[6];                   /* Las escenas, en orden.                             */
    u8 n_escenas;
    u8 completa;                     /* 1 si salió ZONA.                                   */
    u8 bloqueo_ok;                   /* 1 si la cámara no se movió mientras había combate. */
    u8 escena_antes_de_spawn;        /* 1 si la primera escena llegó antes del primer enemigo. */
    u16 camara_final;
} Registro;

static void contar(Registro *r, u8 ev)
{
    if (ev & OL_EV_GO)
        r->gos++;
    if (ev & OL_EV_ZONA)
        r->completa = 1;
}

/* Recorre una zona: la cámara avanza 2 px por frame hasta donde la deje el bloqueo, y el jugador
   vence un enemigo cada 25 frames. Aplica las escenas a `ap` (si no es NULL). */
static void jugar(u8 zona, Registro *r, Apoyos *ap)
{
    Oleadas o;
    u16 camara;
    u16 limite;
    u16 f;
    u8 vivos;
    u8 ev;
    u8 k;
    const Oleada *w;

    r->spawns = 0;
    for (k = 0; k < 4; k++)
        r->por_tipo[k] = 0;
    r->gos = 0;
    r->n_escenas = 0;
    r->completa = 0;
    r->bloqueo_ok = 1;
    r->escena_antes_de_spawn = 0;
    camara = 0;
    vivos = 0;
    oleadas_iniciar(&o, zona);
    for (f = 0; f < 30000 && !(r->completa && camara == oleadas_limite_camara(&o)); f++)
    {
        limite = oleadas_limite_camara(&o);
        if (camara < limite)
            camara = (u16)(camara + 2);
        if (camara > limite)
            camara = limite;
        if (vivos && (f % 25) == 0)
            vivos--;                 /* El jugador vence a uno.                            */
        ev = oleadas_actualizar(&o, camara, vivos);
        if (o.estado == OL_COMBATE)
        {
            w = &fase1_zona(zona)->oleadas[o.oleada];
            if (camara != w->x)
                r->bloqueo_ok = 0;   /* Se movió durante el combate.                       */
        }
        if (ev & OL_EV_SPAWN)
        {
            vivos++;
            r->spawns++;
            r->por_tipo[o.spawn_tipo]++;
        }
        if (ev & OL_EV_ESCENA)
        {
            if (r->n_escenas == 0 && r->spawns == 0)
                r->escena_antes_de_spawn = 1;
            r->escenas[r->n_escenas] = o.escena;
            r->n_escenas++;
            if (ap)
                fase1_aplicar_escena(o.escena, ap);
            contar(r, oleadas_escena_terminada(&o));
        }
        contar(r, ev);
    }
    r->camara_final = camara;
}

void pruebas_oleadas(void)
{
    const Zona *z;
    const Oleada *w;
    const Rompible *p;
    Registro r;
    Apoyos ap;
    Oleadas o;
    u8 i, j, k;
    u8 ev;
    u16 f;
    u16 total;
    u16 camara;
    u16 comida[5];
    u16 frame_spawn[3];
    u8 spawns;

    /*--- Los datos de la fase ---------------------------------------------------------*/
    COMPROBAR(sizeof(Oleada) == 16 && sizeof(Rompible) == 4 && sizeof(EnemigoDef) == 2);
    COMPROBAR(fase1_zona(0) != 0 && fase1_zona(3) != 0 && fase1_zona(4) == 0);

    /* Longitudes y número de oleadas: 1536 + 1280 + 768 + 1024 = 4608 px. */
    COMPROBAR(fase1_zona(ZONA_ALDEA)->longitud == 1536);
    COMPROBAR(fase1_zona(ZONA_BOSQUE)->longitud == 1280);
    COMPROBAR(fase1_zona(ZONA_MONASTERIO)->longitud == 768);
    COMPROBAR(fase1_zona(ZONA_DOJO)->longitud == 1024);
    total = 0;
    for (i = 0; i < FASE1_ZONAS; i++)
        total = (u16)(total + fase1_zona(i)->longitud);
    COMPROBAR(total == 4608);
    COMPROBAR(fase1_zona(ZONA_ALDEA)->n_oleadas == 3 && fase1_zona(ZONA_BOSQUE)->n_oleadas == 3);
    COMPROBAR(fase1_zona(ZONA_MONASTERIO)->n_oleadas == 1 && fase1_zona(ZONA_DOJO)->n_oleadas == 2);

    /* Cada oleada: x creciente y dentro de la zona (la cámara llega hasta longitud - 256),
       de 1 a 3 enemigos, rangos válidos y el pesado siempre rojo; el jefe va solo y el último. */
    for (i = 0; i < FASE1_ZONAS; i++)
    {
        z = fase1_zona(i);
        for (j = 0; j < z->n_oleadas; j++)
        {
            w = &z->oleadas[j];
            COMPROBAR(w->x <= z->longitud - ANCHO_PANTALLA);
            if (j > 0)
                COMPROBAR(w->x > z->oleadas[j - 1].x);
            COMPROBAR(w->n >= 1 && w->n <= OLEADA_MAX_ENEMIGOS);
            COMPROBAR(w->lado <= LADO_ALTERNO);
            for (k = 0; k < w->n; k++)
            {
                COMPROBAR(w->e[k].tipo <= TIPO_JEFE);
                if (w->e[k].tipo != TIPO_JEFE)
                    COMPROBAR(w->e[k].rango < RANGOS);
                if (w->e[k].tipo == TIPO_PESADO)
                    COMPROBAR(w->e[k].rango == RANGO_ROJO);
                if (w->e[k].tipo == TIPO_JEFE)
                    COMPROBAR(i == ZONA_DOJO && j == z->n_oleadas - 1 && w->n == 1);
            }
        }
    }

    /* Los rompibles: dentro de su zona y en orden; el reparto de comida de cada zona; y UNA sola
       vida extra en toda la fase, en la aldea. */
    comida[CONT_ONIGIRI] = comida[CONT_PESCADO] = comida[CONT_SHURIKEN] = comida[CONT_VIDA] = 0;
    COMPROBAR(fase1_zona(ZONA_ALDEA)->n_rompibles == 6 && fase1_zona(ZONA_BOSQUE)->n_rompibles == 4);
    COMPROBAR(fase1_zona(ZONA_MONASTERIO)->n_rompibles == 3 && fase1_zona(ZONA_DOJO)->n_rompibles == 4);
    for (i = 0; i < FASE1_ZONAS; i++)
    {
        z = fase1_zona(i);
        for (j = 0; j < z->n_rompibles; j++)
        {
            p = &z->rompibles[j];
            COMPROBAR(p->x < z->longitud && p->tipo <= ROMP_SHOJI && p->contiene <= CONT_VIDA);
            if (j > 0)
                COMPROBAR(p->x > z->rompibles[j - 1].x);
            if (p->contiene != CONT_NADA)
                comida[p->contiene]++;
        }
    }
    COMPROBAR(comida[CONT_ONIGIRI] == 5 && comida[CONT_PESCADO] == 2);   /* 2 + 2 + 0 + 1; 0 + 0 + 1 + 1. */
    COMPROBAR(comida[CONT_SHURIKEN] == 1 && comida[CONT_VIDA] == 1);
    COMPROBAR(fase1_zona(ZONA_ALDEA)->rompibles[5].contiene == CONT_VIDA);
    COMPROBAR(fase1_zona(ZONA_MONASTERIO)->rompibles[1].contiene == CONT_PESCADO);

    /*--- Las escenas y sus efectos sobre los apoyos -----------------------------------*/
    apoyos_iniciar(&ap);
    fase1_aplicar_escena(ESC_FENRIR_GAG, &ap);       /* El gag no cambia nada.              */
    fase1_aplicar_escena(ESC_NADA, &ap);
    COMPROBAR(ap.disponibles == 0 && ap.activa == 0);
    fase1_aplicar_escena(ESC_LUCINDA, &ap);          /* Lucinda se une; la barra, no.       */
    COMPROBAR(ap.disponibles == 1 && ap.activa == 0 && ap.seleccionado == APOYO_LUCINDA);
    fase1_aplicar_escena(ESC_REX, &ap);              /* Rex se une y la barra se activa llena. */
    COMPROBAR((ap.disponibles & 2) != 0 && ap.activa == 1 && ap.golpes == BARRA_LLENA);

    /*--- El bloqueo de la cámara y el orden de los eventos (a mano) --------------------*/
    oleadas_iniciar(&o, ZONA_ALDEA);
    COMPROBAR(oleadas_limite_camara(&o) == 256);     /* Hasta la primera oleada.            */
    COMPROBAR(oleadas_actualizar(&o, 100, 0) == OL_EV_NADA && o.estado == OL_ESPERANDO);
    COMPROBAR(oleadas_actualizar(&o, 256, 0) == OL_EV_NADA && o.estado == OL_COMBATE);
    COMPROBAR(oleadas_limite_camara(&o) == 256);     /* Bloqueada en combate.               */

    /* Salen de uno en uno, el primero enseguida y el siguiente a los 40 frames; aunque no quede
       nadie vivo, la oleada NO se vence mientras falten por salir. */
    spawns = 0;
    frame_spawn[0] = frame_spawn[1] = frame_spawn[2] = 0;
    for (f = 1; f < 200 && !(o.estado != OL_COMBATE); f++)
    {
        ev = oleadas_actualizar(&o, 256, 0);         /* vivos = 0 todo el rato.             */
        if (ev & OL_EV_SPAWN)
        {
            if (spawns < 3)
                frame_spawn[spawns] = f;
            spawns++;
            COMPROBAR(!(ev & OL_EV_GO));             /* El frame del SPAWN no da la oleada por vencida. */
        }
    }
    COMPROBAR(spawns == 2);                          /* Los 2 de la primera oleada, ni uno menos. */
    COMPROBAR(frame_spawn[0] == 1);
    COMPROBAR(frame_spawn[1] - frame_spawn[0] == FRAMES_ENTRE_ENEMIGOS);
    COMPROBAR(o.estado == OL_ESPERANDO && o.oleada == 1);   /* Vencida tras salir los dos.  */
    COMPROBAR(oleadas_go_activo(&o) == 1);
    COMPROBAR(oleadas_limite_camara(&o) == 640);     /* Ahora, hasta la segunda.            */
    for (f = 0; f < FRAMES_GO; f++)
        oleadas_actualizar(&o, 300, 0);
    COMPROBAR(oleadas_go_activo(&o) == 0);           /* La flecha se apaga sola.            */

    /* Con enemigos vivos, la oleada no se vence aunque hayan salido todos. */
    oleadas_iniciar(&o, ZONA_ALDEA);
    oleadas_actualizar(&o, 256, 0);
    for (f = 0; f < 200; f++)
        oleadas_actualizar(&o, 256, 1);              /* Siempre hay uno vivo.               */
    COMPROBAR(o.estado == OL_COMBATE && o.aparecidos == 2);
    COMPROBAR(oleadas_actualizar(&o, 256, 0) == OL_EV_GO);   /* Al caer el último, vence.   */

    /* Una escena DESPUÉS: la oleada 1 del bosque. No hay GO hasta que la escena acaba. */
    oleadas_iniciar(&o, ZONA_BOSQUE);
    oleadas_actualizar(&o, 256, 0);                  /* Empieza el combate.                 */
    for (f = 0; f < 200 && o.aparecidos < 3; f++)
        oleadas_actualizar(&o, 256, 0);
    COMPROBAR(o.aparecidos == 3);
    ev = oleadas_actualizar(&o, 256, 0);
    COMPROBAR(ev == OL_EV_ESCENA && o.escena == ESC_LUCINDA && o.estado == OL_ESCENA_DESPUES);
    for (f = 0; f < 50; f++)
        COMPROBAR(oleadas_actualizar(&o, 256, 0) == OL_EV_NADA);   /* Esperando a la escena. */
    COMPROBAR(oleadas_go_activo(&o) == 0 && oleadas_limite_camara(&o) == 256);
    COMPROBAR(oleadas_escena_terminada(&o) == OL_EV_GO);
    COMPROBAR(o.oleada == 1 && o.estado == OL_ESPERANDO && oleadas_go_activo(&o) == 1);
    COMPROBAR(oleadas_escena_terminada(&o) == OL_EV_NADA);   /* Sin escena en marcha: nada. */

    /* Una escena ANTES (el monasterio, en x = 0): salta nada más abrir la zona y no sale nadie
       hasta que acaba; luego los enemigos entran alternando lados (el de la izquierda, al borde). */
    oleadas_iniciar(&o, ZONA_MONASTERIO);
    ev = oleadas_actualizar(&o, 0, 0);
    COMPROBAR(ev == OL_EV_ESCENA && o.escena == ESC_REX && o.estado == OL_ESCENA_ANTES);
    for (f = 0; f < 100; f++)
        COMPROBAR(oleadas_actualizar(&o, 0, 0) == OL_EV_NADA);
    COMPROBAR(o.aparecidos == 0);
    COMPROBAR(oleadas_escena_terminada(&o) == OL_EV_NADA && o.estado == OL_COMBATE);
    COMPROBAR(oleadas_actualizar(&o, 0, 0) == OL_EV_SPAWN);
    COMPROBAR(o.spawn_lado == LADO_DERECHA && o.spawn_x == 0 + ANCHO_PANTALLA + MARGEN_SPAWN);
    for (f = 0; f < FRAMES_ENTRE_ENEMIGOS - 1; f++)
        COMPROBAR(oleadas_actualizar(&o, 0, 1) == OL_EV_NADA);
    COMPROBAR(oleadas_actualizar(&o, 0, 1) == OL_EV_SPAWN);
    COMPROBAR(o.spawn_lado == LADO_IZQUIERDA && o.spawn_x == 16);   /* Al borde del mundo.    */
    for (f = 0; f < FRAMES_ENTRE_ENEMIGOS - 1; f++)
        oleadas_actualizar(&o, 0, 2);
    COMPROBAR(oleadas_actualizar(&o, 0, 2) == OL_EV_SPAWN);
    COMPROBAR(o.spawn_lado == LADO_DERECHA);
    /* Con la cámara más adelante, por la izquierda entra 32 px fuera de la pantalla. */
    oleadas_iniciar(&o, ZONA_ALDEA);
    o.estado = OL_COMBATE;
    o.oleada = 2;                                    /* La alterna: derecha, derecha... izq. */
    o.aparecidos = 1;
    o.espera = 0;
    camara = 1024;
    COMPROBAR(oleadas_actualizar(&o, camara, 0) == OL_EV_SPAWN);
    COMPROBAR(o.spawn_lado == LADO_IZQUIERDA && o.spawn_x == 1024 - MARGEN_SPAWN);
    COMPROBAR(o.spawn_tipo == TIPO_BASICO && o.spawn_rango == RANGO_BLANCO);

    /*--- Cada zona, recorrida entera, sale como se decidió ----------------------------*/
    apoyos_iniciar(&ap);
    jugar(ZONA_ALDEA, &r, &ap);
    COMPROBAR(r.completa == 1 && r.bloqueo_ok == 1);
    COMPROBAR(r.spawns == 8 && r.por_tipo[TIPO_BASICO] == 7 && r.por_tipo[TIPO_RAPIDO] == 1);
    COMPROBAR(r.gos == 3 && r.n_escenas == 0);
    COMPROBAR(r.camara_final == 1536 - 256);

    jugar(ZONA_BOSQUE, &r, &ap);
    COMPROBAR(r.completa == 1 && r.bloqueo_ok == 1);
    COMPROBAR(r.spawns == 9 && r.por_tipo[TIPO_RAPIDO] == 2 && r.por_tipo[TIPO_BASICO] == 6);
    COMPROBAR(r.por_tipo[TIPO_PESADO] == 1 && r.gos == 3);
    COMPROBAR(r.n_escenas == 2 && r.escenas[0] == ESC_LUCINDA && r.escenas[1] == ESC_FENRIR_GAG);
    COMPROBAR(r.camara_final == 1280 - 256);
    COMPROBAR(ap.disponibles == 1 && ap.activa == 0);   /* Lucinda sí; Rex y la barra, aún no. */

    jugar(ZONA_MONASTERIO, &r, &ap);
    COMPROBAR(r.completa == 1 && r.bloqueo_ok == 1 && r.spawns == 3 && r.por_tipo[TIPO_BASICO] == 3);
    COMPROBAR(r.n_escenas == 1 && r.escenas[0] == ESC_REX && r.escena_antes_de_spawn == 1);
    COMPROBAR(r.gos == 1 && r.camara_final == 768 - 256);
    COMPROBAR(ap.disponibles == 3 && ap.activa == 1 && ap.golpes == BARRA_LLENA);

    jugar(ZONA_DOJO, &r, &ap);
    COMPROBAR(r.completa == 1 && r.bloqueo_ok == 1);
    COMPROBAR(r.spawns == 4 && r.por_tipo[TIPO_BASICO] == 2 && r.por_tipo[TIPO_RAPIDO] == 1);
    COMPROBAR(r.por_tipo[TIPO_JEFE] == 1 && r.gos == 2);
    COMPROBAR(r.n_escenas == 2 && r.escenas[0] == ESC_JEFE_ENTRA && r.escenas[1] == ESC_VILLANO_HUYE);
    COMPROBAR(r.camara_final == 1024 - 256);
}
