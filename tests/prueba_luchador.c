/*---------------------------------------------------------------------------------
    prueba_luchador.c — Pruebas de juego/luchador.c.

    PROPÓSITO
      Fijar cómo se comporta Tarou en combate (docs/diseno-beat-em-up.md §4, §5.1,
      §5.3 y §5.5): el combo solo sigue si conectó y se pulsó A en su ventana; el
      especial cuesta vida solo si conecta y es invulnerable mientras dura; el
      derribo cae, aterriza, se levanta invulnerable y sale; el KO termina por
      tiempo; la parada de impacto congela; el agarre da hasta 3 golpes y atrás+A
      lanza; el doble toque corre.

    SE CONECTA CON
      Incluye:  minitest.h, juego/luchador.h, juego/cifras.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_luchador).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Estado, Combo, Derribo, Parada de impacto
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/luchador.h"
#include "juego/cifras.h"

/* Mando con todo sin pulsar. */
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

/* `n` frames con el mando tal como está. */
static void correr(Luchador *L, const Mando *m, const Carril *c, u8 n)
{
    while (n--)
        luchador_actualizar(L, m, c);
}

void pruebas_luchador(void)
{
    Luchador L;
    Mando m;
    Carril c;
    u8 n;
    u16 x_ini;
    u8 aterrizaje;

    c.x_min_fp = PF(16);
    c.x_max_fp = PF(240);
    c.y_arriba_fp = PF(150);
    c.y_abajo_fp = PF(208);

    /* Luchador (33 bytes) no se indexa en arrays: Tarou y Bárbara son instancias separadas. */

    /* Al empezar: quieto, con vida, sin combo ni agarre. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    COMPROBAR(L.estado == LQ_QUIETO && L.g.vida == 64 && L.vencido == 0);

    /* Andar: la cruceta mueve en x a 1,5 px por frame y mira a la derecha. */
    limpiar(&m);
    m.derecha = 1;
    x_ini = L.c.x_fp;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_ANDAR && L.c.vx_fp == VEL_ANDAR_TAROU);
    COMPROBAR(L.c.x_fp == x_ini + VEL_ANDAR_TAROU && L.mira_izquierda == 0);

    /* Arriba y abajo: la profundidad va a la mitad de velocidad y arriba es hacia el fondo. */
    limpiar(&m);
    m.arriba = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.c.vy_fp == -(VEL_ANDAR_TAROU >> 1));
    limpiar(&m);
    m.abajo = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.c.vy_fp == VEL_ANDAR_TAROU >> 1);

    /* Izquierda: mira a la izquierda. */
    limpiar(&m);
    m.izquierda = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.mira_izquierda == 1 && L.c.vx_fp == -VEL_ANDAR_TAROU);

    /* Doble toque: un toque, soltar y volver a pulsar la misma corre. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.derecha = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_ANDAR);
    limpiar(&m);
    luchador_actualizar(&L, &m, &c);
    m.derecha = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_CORRER && L.c.vx_fp == VEL_CORRER_TAROU);

    /* Salto: B despega, el aire dura y aterriza en quieto, sobre el suelo. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.saltar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_SALTO && L.c.en_suelo == 0 && L.c.z_fp > 0);
    limpiar(&m);
    n = 0;
    aterrizaje = 0;
    while (n < 120 && !aterrizaje)
    {
        luchador_actualizar(&L, &m, &c);
        if (L.estado == LQ_QUIETO)
            aterrizaje = 1;
        n++;
    }
    COMPROBAR(aterrizaje == 1 && L.c.z_fp == 0 && L.c.en_suelo == 1);
    COMPROBAR(n >= 30 && n <= 60);                   /* Unos 40 frames en el aire.          */

    /* Combo: A, conecta en su parte activa, A en la ventana y se encadena. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_GOLPE_1 && L.tiempo == 0);
    limpiar(&m);
    correr(&L, &m, &c, 4);                           /* Tiempo 4: empieza la caja.          */
    luchador_marcar_conecto(&L);
    correr(&L, &m, &c, 4);                           /* Tiempo 8: acaba la parte activa.    */
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);                 /* Tiempo 9: dentro de la ventana.     */
    limpiar(&m);
    correr(&L, &m, &c, 11);                          /* Tiempo 20: acaba el golpe.          */
    COMPROBAR(L.estado == LQ_GOLPE_2 && L.tiempo == 0);

    /* Sin conectar no encadena: el combo se corta aunque se pulse A. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 8);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 11);
    COMPROBAR(L.estado == LQ_QUIETO);

    /* Conecta, pero A llega antes de la ventana (tiempo 3, no 8 o más): no encadena. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    luchador_marcar_conecto(&L);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);                 /* Tiempo 1: aún no es la ventana.     */
    limpiar(&m);
    correr(&L, &m, &c, 19);                          /* Tiempo 20: acaba.                   */
    COMPROBAR(L.estado == LQ_QUIETO);

    /* La ventana empieza justo en su último cuadro activo: A en el tiempo 8 ya encadena. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 4);
    luchador_marcar_conecto(&L);
    correr(&L, &m, &c, 3);                           /* Tiempo 7: aún no es la ventana.     */
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);                 /* Tiempo 8: primer cuadro de ventana. */
    limpiar(&m);
    correr(&L, &m, &c, 12);
    COMPROBAR(L.estado == LQ_GOLPE_2);

    /* Caja de golpe: solo en la parte activa (tiempos 4 a 7) y con su daño. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 3);
    COMPROBAR(luchador_caja_golpe(&L) == 0);         /* Tiempo 3: aún no pega.              */
    correr(&L, &m, &c, 1);
    COMPROBAR(luchador_caja_golpe(&L) != 0);         /* Tiempo 4: pega.                     */
    COMPROBAR(luchador_caja_golpe(&L)->dano == DANO_COMBO_TAROU);
    COMPROBAR(luchador_caja_golpe(&L)->efecto == EFECTO_RETROCESO);
    correr(&L, &m, &c, 4);
    COMPROBAR(luchador_caja_golpe(&L) == 0);         /* Tiempo 8: ya no.                    */

    /* Golpe 4 (mordisco): derriba. Se alcanza encadenando los tres primeros. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    L.estado = LQ_GOLPE_3;
    L.tiempo = 0;
    L.conecto = 1;
    L.encadenar = 1;
    correr(&L, &m, &c, 22);
    COMPROBAR(L.estado == LQ_GOLPE_4);
    correr(&L, &m, &c, 6);                           /* Tiempo 6: empieza la caja.          */
    COMPROBAR(luchador_caja_golpe(&L) != 0 && luchador_caja_golpe(&L)->efecto == EFECTO_DERRIBO);
    COMPROBAR(luchador_caja_golpe(&L)->dano == DANO_MORDISCO_TAROU);

    /* Especial A+B: cuesta vida solo si conecta, y no mata; es invulnerable mientras dura. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_ESPECIAL && L.g.invulnerable == 24);
    limpiar(&m);
    correr(&L, &m, &c, 5);
    COMPROBAR(L.g.vida == 64);                       /* Sin conectar, no cuesta.            */
    luchador_marcar_conecto(&L);
    COMPROBAR(L.g.vida == 64 - COSTE_ESPECIAL);      /* Conecta: paga 4.                    */
    luchador_marcar_conecto(&L);
    COMPROBAR(L.g.vida == 64 - COSTE_ESPECIAL);      /* Una sola vez por especial.          */

    /* La parada de impacto congela el movimiento durante FRAMES_PARADA frames. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    L.g.parada = FRAMES_PARADA;
    limpiar(&m);
    m.derecha = 1;
    x_ini = L.c.x_fp;
    correr(&L, &m, &c, FRAMES_PARADA);
    COMPROBAR(L.c.x_fp == x_ini);
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.c.x_fp == x_ini + VEL_ANDAR_TAROU);

    /* Recibir un golpe normal: retrocede y vuelve a quieto. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    luchador_reaccionar(&L, GOLPE_RETROCESO);
    COMPROBAR(L.estado == LQ_RECIBIR);
    limpiar(&m);
    correr(&L, &m, &c, FRAMES_RECIBIR);
    COMPROBAR(L.estado == LQ_QUIETO);

    /* Derribo: vuela, cae, está en el suelo, se levanta invulnerable y sale. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    luchador_reaccionar(&L, GOLPE_DERRIBO);
    COMPROBAR(L.estado == LQ_DERRIBADO && L.c.en_suelo == 0 && L.c.vz_fp > 0);
    n = 0;
    while (n < 120 && L.estado == LQ_DERRIBADO)
    {
        luchador_actualizar(&L, &m, &c);
        n++;
    }
    COMPROBAR(L.estado == LQ_EN_SUELO && L.c.en_suelo == 1);
    correr(&L, &m, &c, 30);
    COMPROBAR(L.estado == LQ_LEVANTARSE && L.g.invulnerable == FRAMES_INVULNERABLE);
    correr(&L, &m, &c, 36);
    COMPROBAR(L.estado == LQ_QUIETO);

    /* KO: vencido solo al acabar su tiempo. */
    luchador_iniciar(&L, 100, 180, 0);
    luchador_reaccionar(&L, GOLPE_VENCE);
    COMPROBAR(L.estado == LQ_KO && L.vencido == 0);
    correr(&L, &m, &c, 39);
    COMPROBAR(L.vencido == 0);
    correr(&L, &m, &c, 1);
    COMPROBAR(L.vencido == 1);

    /* Un golpe que no pega (FALLA) no cambia nada. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    luchador_reaccionar(&L, GOLPE_FALLA);
    COMPROBAR(L.estado == LQ_QUIETO);

    /* Agarre: hasta 3 zarpazos; el cuarto A no hace nada. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    luchador_agarrar(&L);
    COMPROBAR(L.estado == LQ_AGARRAR && L.golpes_agarre == 0);
    limpiar(&m);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_GOLPE_AGARRE && L.golpes_agarre == 1);
    limpiar(&m);
    correr(&L, &m, &c, 14);                          /* Vuelve a agarrar.                   */
    COMPROBAR(L.estado == LQ_AGARRAR);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 14);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 14);
    COMPROBAR(L.golpes_agarre == 3 && L.estado == LQ_AGARRAR);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_AGARRAR && L.golpes_agarre == 3);

    /* Atrás + A lanza (mira a la derecha, así que atrás es izquierda). */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    luchador_agarrar(&L);
    limpiar(&m);
    m.izquierda = 1;
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_LANZAR);
    limpiar(&m);
    correr(&L, &m, &c, 20);
    COMPROBAR(L.estado == LQ_QUIETO);

    /* Soltar al agarrado: el agarre acaba. */
    luchador_agarrar(&L);
    luchador_soltar(&L);
    COMPROBAR(L.estado == LQ_QUIETO);

    /* Lo agarra un enemigo y no puede hacer nada; luego lo lanzan y aterriza. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    luchador_ser_agarrado(&L);
    COMPROBAR(L.estado == LQ_AGARRADO);
    limpiar(&m);
    m.atacar_pulsado = 1;
    correr(&L, &m, &c, 5);
    COMPROBAR(L.estado == LQ_AGARRADO);
    luchador_lanzar(&L, VEL_ANDAR_TAROU, 48);
    COMPROBAR(L.estado == LQ_LANZADO && L.c.en_suelo == 0);
    n = 0;
    while (n < 120 && L.estado == LQ_LANZADO)
    {
        luchador_actualizar(&L, &m, &c);
        n++;
    }
    COMPROBAR(L.estado == LQ_EN_SUELO);

    /* Tajo corriendo: el doble toque y A dan CARRERA_ATAQUE, que derriba. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.derecha = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    luchador_actualizar(&L, &m, &c);
    m.derecha = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_CORRER);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_CARRERA_ATAQUE && L.c.vx_fp == VEL_CORRER_TAROU);
    limpiar(&m);
    correr(&L, &m, &c, 6);                           /* Tiempo 6: empieza la caja.          */
    COMPROBAR(luchador_caja_golpe(&L) != 0 && luchador_caja_golpe(&L)->efecto == EFECTO_DERRIBO);

    /* Ataque aéreo: en el salto, A pega derribando hasta aterrizar. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.saltar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 3);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_AEREO && luchador_caja_golpe(&L) != 0);
    COMPROBAR(luchador_caja_golpe(&L)->efecto == EFECTO_DERRIBO);
    limpiar(&m);
    n = 0;
    while (n < 120 && L.estado == LQ_AEREO)
    {
        luchador_actualizar(&L, &m, &c);
        n++;
    }
    COMPROBAR(L.estado == LQ_QUIETO && L.c.z_fp == 0);

    /* Patada voladora (decisión 40): A en el aire con la cruceta hacia donde mira. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.saltar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    m.derecha = 1;
    correr(&L, &m, &c, 3);                           /* En el aire, mirando a la derecha.   */
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_PATADA);
    COMPROBAR(luchador_caja_golpe(&L) != 0);
    COMPROBAR(luchador_caja_golpe(&L)->ancho == 56);
    COMPROBAR(luchador_caja_golpe(&L)->dano == DANO_PATADA_TAROU);
    COMPROBAR(luchador_caja_golpe(&L)->efecto == EFECTO_DERRIBO);
    COMPROBAR(L.c.vx_fp != 0);                       /* Sigue avanzando mientras patea.     */
    limpiar(&m);
    n = 0;
    while (n < 120 && L.estado == LQ_PATADA)
    {
        luchador_actualizar(&L, &m, &c);
        n++;
    }
    COMPROBAR(L.estado == LQ_QUIETO && L.c.z_fp == 0);   /* Aterriza igual que el aéreo.    */

    /* Sin cruceta es el ataque aéreo de siempre, más corto que la patada. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.saltar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    correr(&L, &m, &c, 3);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_AEREO);
    COMPROBAR(luchador_caja_golpe(&L)->ancho == 36);
    COMPROBAR(DANO_PATADA_TAROU > DANO_AEREO_TAROU && DANO_PATADA_BARBARA > DANO_AEREO_BARBARA);

    /* Cruceta CONTRARIA a donde mira: no es patada, es aéreo. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.saltar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    m.derecha = 1;
    correr(&L, &m, &c, 3);                           /* Mira a la derecha.                  */
    limpiar(&m);
    m.izquierda = 1;
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_AEREO);

    /* Mirando a la izquierda: izquierda + A es patada. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.saltar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    limpiar(&m);
    m.izquierda = 1;
    correr(&L, &m, &c, 3);
    COMPROBAR(L.mira_izquierda == 1);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_PATADA && L.c.vx_fp < 0);

    /* En el suelo, A + cruceta no es patada (esa orden es para andar y atacar). */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.derecha = 1;
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado != LQ_PATADA && L.c.en_suelo == 1);

    /*--- Bárbara (spec §4; decisiones del 2026-10-24): combo lento de 3, alcance largo -----*/
    luchador_iniciar_como(&L, QUIEN_BARBARA, 100, 180, VIDA_PROTAGONISTA);
    COMPROBAR(L.quien == QUIEN_BARBARA && L.estado == LQ_QUIETO);

    /* Más lenta que Tarou: 1,25 px por frame andando y 2,5 corriendo. */
    limpiar(&m);
    m.derecha = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.c.vx_fp == VEL_ANDAR_BARBARA && VEL_ANDAR_BARBARA < VEL_ANDAR_TAROU);
    COMPROBAR(L.c.vy_fp == 0);
    limpiar(&m);
    luchador_actualizar(&L, &m, &c);
    m.derecha = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_CORRER && L.c.vx_fp == VEL_CORRER_BARBARA);

    /* Combo de 3: golpe 1 a 24 frames, con caja de 44 px y 5 de daño; el tajo (golpe 3) dura
       30, hace 8 y derriba; no existe un golpe 4. */
    luchador_iniciar_como(&L, QUIEN_BARBARA, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_GOLPE_1);
    limpiar(&m);
    correr(&L, &m, &c, 5);
    COMPROBAR(luchador_caja_golpe(&L) == 0);         /* Tiempo 5: aún no pega.              */
    correr(&L, &m, &c, 1);
    COMPROBAR(luchador_caja_golpe(&L) != 0);         /* Tiempo 6: pega.                     */
    COMPROBAR(luchador_caja_golpe(&L)->ancho == 44);
    COMPROBAR(luchador_caja_golpe(&L)->dano == DANO_COMBO_BARBARA);
    COMPROBAR(luchador_caja_golpe(&L)->efecto == EFECTO_RETROCESO);
    luchador_marcar_conecto(&L);
    correr(&L, &m, &c, 5);                           /* Tiempo 11: aún pega (hasta 12).     */
    COMPROBAR(luchador_caja_golpe(&L) != 0);
    correr(&L, &m, &c, 1);
    COMPROBAR(luchador_caja_golpe(&L) == 0);         /* Tiempo 12: ya no; empieza la ventana. */
    m.atacar_pulsado = 1;
    luchador_actualizar(&L, &m, &c);                 /* Tiempo 13: dentro de la ventana.    */
    limpiar(&m);
    correr(&L, &m, &c, 11);                          /* Tiempo 24: acaba el golpe.          */
    COMPROBAR(L.estado == LQ_GOLPE_2);

    /* Golpe 2 y tajo: el tajo derriba con 8 y cierra el combo (queda quieta, no golpe 4). */
    L.conecto = 1;
    L.encadenar = 1;
    correr(&L, &m, &c, 24);
    COMPROBAR(L.estado == LQ_GOLPE_3);
    correr(&L, &m, &c, 8);
    COMPROBAR(luchador_caja_golpe(&L) != 0);
    COMPROBAR(luchador_caja_golpe(&L)->ancho == 52);
    COMPROBAR(luchador_caja_golpe(&L)->dano == DANO_TAJO_BARBARA);
    COMPROBAR(luchador_caja_golpe(&L)->efecto == EFECTO_DERRIBO);
    L.conecto = 1;
    L.encadenar = 1;
    correr(&L, &m, &c, 22);                          /* Hasta el final del tajo (30).       */
    COMPROBAR(L.estado == LQ_QUIETO);

    /* Su caja de golpe cabe en la celda de 96x96 (x + ancho <= 96), salvo el especial. */
    Celda celda;
    luchador_celda(&L, &celda);
    COMPROBAR(celda.ancho == 96 && celda.mitad_ancho == 48 && celda.alto == 96);
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    luchador_celda(&L, &celda);
    COMPROBAR(celda.ancho == 96 && celda.mitad_ancho == 48 && celda.alto == 64);

    /* Especial: corte circular de 128 px, centrado en los pies (x = -16: igual a los dos lados). */
    luchador_iniciar_como(&L, QUIEN_BARBARA, 100, 180, VIDA_PROTAGONISTA);
    limpiar(&m);
    m.atacar_pulsado = 1;
    m.saltar = 1;
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(L.estado == LQ_ESPECIAL);
    limpiar(&m);
    correr(&L, &m, &c, 6);
    COMPROBAR(luchador_caja_golpe(&L) != 0 && luchador_caja_golpe(&L)->ancho == 128);
    {
        CajaMundo este, oeste;
        luchador_celda(&L, &celda);
        combate_caja_mundo(luchador_caja_golpe(&L), &celda, &L.c, 0, &este);
        combate_caja_mundo(luchador_caja_golpe(&L), &celda, &L.c, 1, &oeste);
        COMPROBAR(este.x == 100 - 64 && este.x + este.ancho == 100 + 64);   /* ±64 de los pies. */
        COMPROBAR(oeste.x == este.x && oeste.ancho == este.ancho);          /* Igual espejado.  */
    }

    /* Patada y tajo de Bárbara hacen más que los de Tarou. */
    COMPROBAR(DANO_PATADA_BARBARA == 10 && DANO_TAJO_BARBARA > DANO_MORDISCO_TAROU);

    /*--- El relevo: entrar cayendo, el golpe de entrada y quién puede salir ---------------*/
    luchador_iniciar_como(&L, QUIEN_BARBARA, 100, 180, VIDA_PROTAGONISTA);
    luchador_entrar(&L, PF(140), PF(170), 36);
    COMPROBAR(L.estado == LQ_ENTRADA && L.c.en_suelo == 0);
    COMPROBAR(L.c.x_fp == PF(140) && L.c.y_fp == PF(170) && L.c.z_fp == PF(ALTURA_ENTRADA_RELEVO));
    COMPROBAR(L.g.invulnerable == 36);
    COMPROBAR(luchador_entrada_activa(&L) == 0);     /* Todavía cayendo.                    */
    limpiar(&m);
    n = 0;
    aterrizaje = 0;
    while (n < 120 && L.estado == LQ_ENTRADA)
    {
        luchador_actualizar(&L, &m, &c);
        n++;
    }
    COMPROBAR(L.estado == LQ_ENTRADA_GOLPE && L.c.z_fp == 0);
    COMPROBAR(n >= 20 && n <= 32);                   /* ~25 frames de caída desde 80 px.    */
    COMPROBAR(luchador_entrada_activa(&L) == 1);     /* El frame del aterrizaje.            */
    luchador_actualizar(&L, &m, &c);
    COMPROBAR(luchador_entrada_activa(&L) == 0);     /* Una sola vez.                       */
    correr(&L, &m, &c, FRAMES_ENTRADA_GOLPE);
    COMPROBAR(L.estado == LQ_QUIETO);

    /* Mientras cae es invulnerable: el contador baja, no se pierde al cambiar de estado. */
    luchador_entrar(&L, PF(100), PF(180), 60);
    correr(&L, &m, &c, 10);
    COMPROBAR(L.g.invulnerable == 50);

    /* Quién puede salir: parados, andando, saltando o atacando sí; recibiendo, derribados, en el
       suelo, agarrados, KO, entrando o agarrando, no. */
    luchador_iniciar(&L, 100, 180, VIDA_PROTAGONISTA);
    COMPROBAR(luchador_puede_salir(&L) == 1);
    L.estado = LQ_ANDAR;     COMPROBAR(luchador_puede_salir(&L) == 1);
    L.estado = LQ_SALTO;     COMPROBAR(luchador_puede_salir(&L) == 1);
    L.estado = LQ_GOLPE_2;   COMPROBAR(luchador_puede_salir(&L) == 1);
    L.estado = LQ_RECIBIR;   COMPROBAR(luchador_puede_salir(&L) == 0);
    L.estado = LQ_DERRIBADO; COMPROBAR(luchador_puede_salir(&L) == 0);
    L.estado = LQ_EN_SUELO;  COMPROBAR(luchador_puede_salir(&L) == 0);
    L.estado = LQ_AGARRADO;  COMPROBAR(luchador_puede_salir(&L) == 0);
    L.estado = LQ_KO;        COMPROBAR(luchador_puede_salir(&L) == 0);
    L.estado = LQ_ENTRADA;   COMPROBAR(luchador_puede_salir(&L) == 0);
    L.estado = LQ_AGARRAR;   COMPROBAR(luchador_puede_salir(&L) == 0);
}
