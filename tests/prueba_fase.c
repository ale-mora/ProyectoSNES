/*---------------------------------------------------------------------------------
    prueba_fase.c — Pruebas de juego/fase.c (el director de la fase 1).

    PROPÓSITO
      Fijar cómo se juega la fase: la cámara sigue al protagonista y se bloquea con la
      oleada; el shinai le quita vida al protagonista (también mirando a la izquierda);
      el protagonista le pega al enemigo y suma puntos y barra; como mucho 2 enemigos
      atacan a la vez; el golpe de entrada del relevo daña una sola vez; el apoyo congela
      la acción, aplica su daño (la mitad al jefe) y Rex cura; las escenas paran la acción
      y al acabar aplican sus efectos; la zona completa da la salida; perder una vida,
      el fin de partida y los continues; y, de punta a punta, un bot que juega las cuatro
      zonas hasta el STAGE CLEAR con las cinco escenas en su orden.

    SE CONECTA CON
      Incluye:  minitest.h, juego/fase.h, juego/cifras.h, juego/luchador.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_fase).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (estáticas de este archivo)
      f   Fase   la partida (grande: no va en la pila)
    → glosario: Director de fase, Oleada, Relevo, Apoyo
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/fase.h"
#include "juego/cifras.h"
#include "juego/luchador.h"

static Fase f;

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
    m->invocar_pulsado = 0;
    m->apoyo_pulsado = 0;
}

/* Una partida nueva sin oleadas: para probar una cosa a la vez. */
static void partida_aislada(void)
{
    fase_iniciar(&f);
    f.oleadas.estado = OL_COMPLETA;
    f.oleadas.oleada = 3;
    f.camara = 200;                  /* Que el carril (cámara + 16 .. + 240) deje a Tarou en x = 300. */
}

/* Un enemigo en el hueco `i`, en (x, y), de ese rango. */
static void poner_ninja(u8 i, u16 x, u16 y, u8 rango)
{
    ninja_iniciar(&f.ninjas[i], x, y, rango);
    f.vivo[i] = 1;
    f.jefe[i] = 0;
    f.contado[i] = 0;
    f.golpeados_p[i] = 0;
    f.golpeados_n[i] = 0;
}

static void correr(u8 frames, const Mando *m)
{
    while (frames--)
        fase_actualizar(&f, m);
}

/* Cuántos enemigos están atacando ahora. */
static u8 atacando(void)
{
    u8 i, n;

    n = 0;
    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (f.vivo[i] && f.ninjas[i].estado == NQ_ATACAR)
            n++;
    }
    return n;
}

/*--- El bot: juega la fase entera -----------------------------------------------------*/
typedef struct
{
    u8 zonas_vistas;                 /* Cuántas veces se pasó a la zona siguiente.         */
    u8 escenas[8];
    u8 n_escenas;
    u8 clear;                        /* 1 si salió el STAGE CLEAR.                         */
    u8 max_atacando;                 /* El máximo de enemigos atacando a la vez.           */
    u8 max_vivos;                    /* El máximo de enemigos a la vez.                    */
    u8 apoyos_usados;
    u8 relevos;
    u8 camara_retrocedio;            /* 1 si la cámara alguna vez fue hacia atrás.         */
    unsigned long frames;            /* Solo en el PC: 150000 frames no caben en u16. */
} Partida;

static s16 abs_s16(s16 v)
{
    if (v < 0)
        return -v;
    return v;
}

static void bot_mando(Mando *m, unsigned long frame)
{
    Luchador *p;
    s16 mejor;
    s16 dx, dy;
    u8 i, objetivo, hay;

    limpiar(m);
    p = relevo_activo(&f.relevo);
    hay = 0;
    objetivo = 0;
    mejor = 32000;
    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (!f.vivo[i] || f.ninjas[i].g.vida == 0)
            continue;
        dx = (s16)(PF_ENTERO(f.ninjas[i].c.x_fp) - PF_ENTERO(p->c.x_fp));
        if (abs_s16(dx) < mejor)
        {
            mejor = abs_s16(dx);
            objetivo = i;
            hay = 1;
        }
    }
    if (!hay)
    {
        m->derecha = 1;              /* Sin enemigos: a seguir hacia la derecha.            */
    }
    else
    {
        dx = (s16)(PF_ENTERO(f.ninjas[objetivo].c.x_fp) - PF_ENTERO(p->c.x_fp));
        dy = (s16)(PF_ENTERO(f.ninjas[objetivo].c.y_fp) - PF_ENTERO(p->c.y_fp));
        if (dy > 6)
            m->abajo = 1;
        else if (dy < -6)
            m->arriba = 1;
        if (abs_s16(dx) > 26)
        {
            if (dx < 0)
                m->izquierda = 1;
            else
                m->derecha = 1;
        }
        else if ((p->mira_izquierda != 0) != (dx < 0))
        {
            if (dx < 0)
                m->izquierda = 1;    /* Se gira hacia él.                                  */
            else
                m->derecha = 1;
        }
        else if ((frame % 5) == 0)
        {
            m->atacar_pulsado = 1;   /* Y le da.                                           */
        }
    }
    if (apoyos_puede_invocar(&f.apoyos))
        m->invocar_pulsado = 1;      /* Usa los apoyos en cuanto puede.                    */
    if ((frame % 300) == 299)
        m->relevo_pulsado = 1;       /* Y cambia de protagonista de vez en cuando.         */
}

static void jugar_todo(Partida *r)
{
    Mando m;
    u8 ev;
    u16 camara_antes;
    u8 activo_antes;

    r->zonas_vistas = 0;
    r->n_escenas = 0;
    r->clear = 0;
    r->max_atacando = 0;
    r->max_vivos = 0;
    r->apoyos_usados = 0;
    r->relevos = 0;
    r->camara_retrocedio = 0;
    fase_iniciar(&f);
    for (r->frames = 0; r->frames < 150000 && !r->clear; r->frames++)
    {
        bot_mando(&m, r->frames);
        /* Invulnerable: el bot solo comprueba el flujo, no sabe sobrevivir. */
        if (relevo_activo(&f.relevo)->estado != LQ_KO)
            relevo_activo(&f.relevo)->g.vida = VIDA_PROTAGONISTA;
        camara_antes = f.camara;
        activo_antes = f.relevo.activo;
        ev = fase_actualizar(&f, &m);
        if (f.relevo.activo != activo_antes)
            r->relevos++;
        if (f.camara < camara_antes)
            r->camara_retrocedio = 1;
        if (atacando() > r->max_atacando)
            r->max_atacando = atacando();
        if (fase_vivos(&f) > r->max_vivos)
            r->max_vivos = fase_vivos(&f);
        if (ev & FASE_EV_APOYO)
            r->apoyos_usados++;
        if (ev & FASE_EV_ESCENA)
        {
            r->escenas[r->n_escenas] = f.escena;
            r->n_escenas++;
            ev = (u8)(ev | fase_escena_terminada(&f));
        }
        if (ev & FASE_EV_SALIDA)
        {
            fase_siguiente_zona(&f);
            r->zonas_vistas++;
        }
        if (ev & FASE_EV_CLEAR)
            r->clear = 1;
    }
}

void pruebas_fase(void)
{
    Mando m;
    Partida r;
    Luchador *p;
    u16 antes;
    u8 i, k, ev;
    u8 vida_antes;
    u16 x_antes[FASE_ENEMIGOS];
    u8 maximo;

    /*--- Empezar -----------------------------------------------------------------------*/
    fase_iniciar(&f);
    COMPROBAR(f.zona == ZONA_ALDEA && f.estado == FASE_JUGANDO && f.camara == 0);
    COMPROBAR(f.relevo.activo == QUIEN_TAROU && f.marcador.vidas == VIDAS_INICIALES);
    COMPROBAR(fase_vivos(&f) == 0 && f.zona_completa == 0);
    COMPROBAR(PF_ENTERO(f.relevo.tarou.c.x_fp) == X_INICIAL_ZONA);

    /*--- La cámara sigue al protagonista y se bloquea con la oleada ---------------------*/
    limpiar(&m);
    m.derecha = 1;
    for (i = 0; i < 250; i++)
    {
        relevo_activo(&f.relevo)->g.vida = VIDA_PROTAGONISTA;
        fase_actualizar(&f, &m);
    }
    COMPROBAR(f.camara == 256);                      /* Hasta la primera oleada, y ni un px más. */
    COMPROBAR(f.oleadas.estado == OL_COMBATE);
    COMPROBAR(PF_ENTERO(relevo_activo(&f.relevo)->c.x_fp) <= f.camara + ANCHO_PANTALLA - 16);
    COMPROBAR(fase_vivos(&f) >= 1 && fase_vivos(&f) <= FASE_ENEMIGOS);

    /*--- El shinai le quita vida al protagonista (con el ninja a la derecha, mirando a la izquierda) */
    partida_aislada();
    p = relevo_activo(&f.relevo);
    p->c.x_fp = PF(300);
    p->c.y_fp = PF(180);
    poner_ninja(0, 324, 180, RANGO_BLANCO);
    limpiar(&m);
    for (i = 0; i < 200 && p->g.vida == VIDA_PROTAGONISTA; i++)
        fase_actualizar(&f, &m);
    COMPROBAR(p->g.vida < VIDA_PROTAGONISTA);        /* El golpe del ninja conecta.         */
    COMPROBAR(f.ninjas[0].mira_izquierda == 1);

    /*--- El protagonista le pega al enemigo: quita vida, suma puntos y llena la barra ----------*/
    partida_aislada();
    apoyos_activar_barra(&f.apoyos);
    f.apoyos.golpes = 0;
    p = relevo_activo(&f.relevo);
    p->c.x_fp = PF(300);
    p->c.y_fp = PF(180);
    poner_ninja(0, 322, 180, RANGO_ROJO);            /* Vida de sobra: no muere durante la prueba. */
    f.ninjas[0].espera = 255;                        /* Y no ataca.                         */
    limpiar(&m);
    m.atacar_pulsado = 1;
    fase_actualizar(&f, &m);
    limpiar(&m);
    for (i = 0; i < 12; i++)
        fase_actualizar(&f, &m);
    COMPROBAR(f.ninjas[0].g.vida < cifras_vida_ninja(RANGO_ROJO));
    COMPROBAR(f.marcador.resto >= PUNTOS_GOLPE && f.apoyos.golpes >= 1);

    /*--- Como mucho 2 enemigos atacan a la vez ------------------------------------------------*/
    partida_aislada();
    p = relevo_activo(&f.relevo);
    p->c.x_fp = PF(300);
    p->c.y_fp = PF(180);
    poner_ninja(0, 322, 165, RANGO_ROJO);
    poner_ninja(1, 326, 180, RANGO_ROJO);
    poner_ninja(2, 330, 195, RANGO_ROJO);
    limpiar(&m);
    maximo = 0;
    for (i = 0; i < 250; i++)
    {
        p->g.vida = VIDA_PROTAGONISTA;
        fase_actualizar(&f, &m);
        if (atacando() > maximo)
            maximo = atacando();
    }
    COMPROBAR(maximo == 2);                          /* Llegan a 2, nunca a 3 (decisión de la spec §6). */

    /*--- El golpe de entrada del relevo: daña una sola vez, a menos de 48 px ------------------*/
    partida_aislada();
    p = relevo_activo(&f.relevo);
    p->c.x_fp = PF(300);
    p->c.y_fp = PF(180);
    poner_ninja(0, 330, 180, RANGO_ROJO);
    poner_ninja(1, 380, 180, RANGO_ROJO);            /* A 80 px: no le llega.               */
    f.ninjas[0].espera = 255;
    f.ninjas[1].espera = 255;
    vida_antes = f.ninjas[0].g.vida;
    limpiar(&m);
    m.relevo_pulsado = 1;
    fase_actualizar(&f, &m);
    COMPROBAR(f.relevo.activo == QUIEN_BARBARA);
    limpiar(&m);
    for (i = 0; i < 80 && relevo_activo(&f.relevo)->estado != LQ_ENTRADA_GOLPE; i++)
        fase_actualizar(&f, &m);
    COMPROBAR(f.ninjas[0].g.vida == vida_antes - DANO_ENTRADA_RELEVO);
    COMPROBAR(f.ninjas[1].g.vida == cifras_vida_ninja(RANGO_ROJO));
    for (i = 0; i < 15; i++)
        fase_actualizar(&f, &m);
    COMPROBAR(f.ninjas[0].g.vida == vida_antes - DANO_ENTRADA_RELEVO);   /* Una sola vez.    */

    /*--- Un apoyo congela la acción y aplica su daño ------------------------------------------*/
    partida_aislada();
    apoyos_desbloquear(&f.apoyos, APOYO_LUCINDA);
    apoyos_activar_barra(&f.apoyos);                 /* Llena.                              */
    p = relevo_activo(&f.relevo);
    p->c.x_fp = PF(300);
    p->c.y_fp = PF(180);
    poner_ninja(0, 400, 165, RANGO_BLANCO);
    poner_ninja(1, 420, 195, RANGO_AMARILLO);
    for (i = 0; i < 2; i++)
        x_antes[i] = f.ninjas[i].c.x_fp;
    limpiar(&m);
    m.invocar_pulsado = 1;
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_APOYO);
    COMPROBAR(f.estado == FASE_APOYO && f.apoyo == APOYO_LUCINDA && f.apoyos.golpes == 0);
    COMPROBAR(p->estado == LQ_INVOCAR);
    limpiar(&m);
    m.izquierda = 1;                                 /* Mientras actúa, el mando no hace nada. */
    antes = f.marcador.resto;
    for (i = 0; i < 47; i++)
        fase_actualizar(&f, &m);
    COMPROBAR(f.estado == FASE_APOYO);               /* Aún no acabó (dura 48).             */
    COMPROBAR(f.ninjas[0].c.x_fp == x_antes[0] && f.ninjas[1].c.x_fp == x_antes[1]);   /* Quietos. */
    fase_actualizar(&f, &m);
    COMPROBAR(f.estado == FASE_JUGANDO && p->estado == LQ_QUIETO && p->g.invulnerable == 0);
    COMPROBAR(f.ninjas[0].g.vida == 0);              /* 3 tajos de 8 = 24: el blanco (16) cae. */
    COMPROBAR(f.ninjas[1].g.vida == cifras_vida_ninja(RANGO_AMARILLO) - 24 + 0 || f.ninjas[1].g.vida == 0);
    COMPROBAR(f.marcador.resto >= antes + cifras_puntos_ninja(RANGO_BLANCO));   /* Sus puntos. */

    /* Contra un jefe, la mitad del daño y sin derribarlo. */
    partida_aislada();
    apoyos_desbloquear(&f.apoyos, APOYO_FENRIR);
    apoyos_activar_barra(&f.apoyos);
    poner_ninja(0, 400, 180, RANGO_ROJO);
    f.jefe[0] = 1;
    f.ninjas[0].g.vida = VIDA_GOMASURI;
    limpiar(&m);
    m.invocar_pulsado = 1;
    fase_actualizar(&f, &m);
    limpiar(&m);
    correr(60, &m);
    COMPROBAR(f.ninjas[0].g.vida == VIDA_GOMASURI - DANO_APOYO_FENRIR / 2);
    COMPROBAR(f.ninjas[0].estado != NQ_DERRIBADO && f.ninjas[0].estado != NQ_EN_SUELO);

    /* Rex cura 16 al protagonista activo. */
    partida_aislada();
    apoyos_desbloquear(&f.apoyos, APOYO_REX);
    apoyos_activar_barra(&f.apoyos);
    p = relevo_activo(&f.relevo);
    p->g.vida = 30;
    limpiar(&m);
    m.invocar_pulsado = 1;
    fase_actualizar(&f, &m);
    limpiar(&m);
    correr(80, &m);
    COMPROBAR(f.estado == FASE_JUGANDO && p->g.vida == 30 + CURA_REX);

    /* Sin barra (bloqueada) o sin apoyo disponible, X no hace nada. */
    partida_aislada();
    limpiar(&m);
    m.invocar_pulsado = 1;
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_NADA && f.estado == FASE_JUGANDO);

    /* R cambia al siguiente apoyo. */
    partida_aislada();
    apoyos_desbloquear(&f.apoyos, APOYO_LUCINDA);
    apoyos_desbloquear(&f.apoyos, APOYO_FENRIR);
    limpiar(&m);
    m.apoyo_pulsado = 1;
    fase_actualizar(&f, &m);
    COMPROBAR(f.apoyos.seleccionado == APOYO_FENRIR);

    /*--- Las escenas paran la acción y, al acabar, aplican sus efectos ------------------------*/
    fase_iniciar(&f);
    fase_siguiente_zona(&f);
    fase_siguiente_zona(&f);
    COMPROBAR(f.zona == ZONA_MONASTERIO && f.camara == 0);
    limpiar(&m);
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_ESCENA);   /* Abre la zona con la escena de Rex. */
    COMPROBAR(f.escena == ESC_REX && f.estado == FASE_ESCENA);
    antes = (u16)PF_ENTERO(f.relevo.tarou.c.x_fp);
    m.derecha = 1;
    correr(30, &m);                                  /* Parada: no se mueve ni sale nadie.   */
    COMPROBAR(PF_ENTERO(f.relevo.tarou.c.x_fp) == antes && fase_vivos(&f) == 0);
    COMPROBAR(f.apoyos.activa == 0);                 /* Aún no se unió.                      */
    COMPROBAR(fase_escena_terminada(&f) == FASE_EV_NADA);
    COMPROBAR(f.estado == FASE_JUGANDO && f.escena == ESC_NADA);
    COMPROBAR((f.apoyos.disponibles & 2) != 0 && f.apoyos.activa == 1 && f.apoyos.golpes == BARRA_LLENA);
    limpiar(&m);
    correr(5, &m);
    COMPROBAR(fase_vivos(&f) >= 1);                  /* Ahora sí salen los enemigos.         */
    COMPROBAR(fase_escena_terminada(&f) == FASE_EV_NADA && f.estado == FASE_JUGANDO);   /* Sin escena: nada. */

    /*--- La zona completa y el final de la zona dan la salida ---------------------------------*/
    partida_aislada();
    f.zona_completa = 1;
    f.camara = 1280;
    p = relevo_activo(&f.relevo);
    p->c.x_fp = PF(1510);
    limpiar(&m);
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_SALIDA && f.estado == FASE_SALIDA);
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_NADA);        /* Parada hasta que se cargue la siguiente. */
    fase_siguiente_zona(&f);
    COMPROBAR(f.zona == ZONA_BOSQUE && f.camara == 0 && f.estado == FASE_JUGANDO);
    COMPROBAR(PF_ENTERO(f.relevo.tarou.c.x_fp) == X_INICIAL_ZONA && f.zona_completa == 0);
    COMPROBAR(f.oleadas.zona == ZONA_BOSQUE && f.oleadas.oleada == 0);
    /* Sin completar la zona, llegar al final no da salida. */
    partida_aislada();
    f.camara = 1280;
    relevo_activo(&f.relevo)->c.x_fp = PF(1510);
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_NADA && f.estado == FASE_JUGANDO);

    /*--- Perder una vida, el fin de la partida y los continues --------------------------------*/
    partida_aislada();
    apoyos_activar_barra(&f.apoyos);
    f.apoyos.golpes = 0;
    f.relevo.tarou.g.vida = 0;
    f.relevo.tarou.vencido = 1;
    f.relevo.barbara.g.vida = 0;
    limpiar(&m);
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_VIDA_PERDIDA);
    COMPROBAR(f.marcador.vidas == VIDAS_INICIALES - 1 && f.estado == FASE_JUGANDO);
    COMPROBAR(f.relevo.tarou.g.vida == VIDA_PROTAGONISTA && f.relevo.barbara.g.vida == VIDA_PROTAGONISTA);
    COMPROBAR(f.apoyos.golpes == BARRA_LLENA);       /* Vuelven con la barra llena.          */
    for (k = 0; k < 2; k++)
    {
        f.relevo.tarou.g.vida = 0;
        f.relevo.tarou.vencido = 1;
        f.relevo.barbara.g.vida = 0;
        ev = fase_actualizar(&f, &m);
    }
    COMPROBAR(ev == FASE_EV_FIN_JUEGO && f.estado == FASE_FIN_JUEGO && f.marcador.vidas == 0);
    COMPROBAR(fase_actualizar(&f, &m) == FASE_EV_NADA);        /* Parada.                  */
    COMPROBAR(fase_continuar(&f) == 1 && f.estado == FASE_JUGANDO);
    COMPROBAR(f.marcador.vidas == VIDAS_INICIALES && f.marcador.continues == CONTINUES_INICIALES - 1);
    COMPROBAR(fase_continuar(&f) == 0);              /* Sin fin de partida, no hay continue. */
    for (k = 0; k < CONTINUES_INICIALES; k++)
    {
        f.marcador.vidas = 1;
        f.relevo.tarou.g.vida = 0;
        f.relevo.tarou.vencido = 1;
        f.relevo.barbara.g.vida = 0;
        fase_actualizar(&f, &m);
        fase_continuar(&f);
    }
    f.marcador.vidas = 1;
    f.relevo.tarou.g.vida = 0;
    f.relevo.tarou.vencido = 1;
    f.relevo.barbara.g.vida = 0;
    fase_actualizar(&f, &m);
    COMPROBAR(f.estado == FASE_FIN_JUEGO && fase_continuar(&f) == 0 && f.marcador.continues == 0);

    /*--- De punta a punta: un bot juega las cuatro zonas hasta el STAGE CLEAR ------------------*/
    jugar_todo(&r);
    COMPROBAR(r.clear == 1);
    COMPROBAR(r.zonas_vistas == 3);                  /* Aldea → bosque → monasterio → dojo.   */
    COMPROBAR(r.n_escenas == 5);
    COMPROBAR(r.escenas[0] == ESC_LUCINDA && r.escenas[1] == ESC_FENRIR_GAG && r.escenas[2] == ESC_REX);
    COMPROBAR(r.escenas[3] == ESC_JEFE_ENTRA && r.escenas[4] == ESC_VILLANO_HUYE);
    COMPROBAR(f.estado == FASE_CLEAR && f.zona == ZONA_DOJO);
    COMPROBAR(r.max_atacando <= 2 && r.max_vivos <= 3);
    COMPROBAR(r.camara_retrocedio == 0);             /* La cámara nunca va hacia atrás.       */
    COMPROBAR((f.apoyos.disponibles & 3) == 3 && f.apoyos.activa == 1);   /* Lucinda y Rex.  */
    COMPROBAR(r.apoyos_usados >= 1 && r.relevos >= 1);
    COMPROBAR(f.marcador.miles >= 1 || f.marcador.resto >= 1000);   /* Más de 1000 puntos.   */
    COMPROBAR(f.marcador.vidas >= 1);
}
