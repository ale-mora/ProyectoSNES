/*---------------------------------------------------------------------------------
    fase.c — El director de la fase 1 (ver fase.h).

    PROPÓSITO
      Cada frame, en este orden: apoyos (R y X) → relevo y protagonista → enemigos →
      golpes en los dos sentidos → golpe de entrada del relevo → bajas y puntos →
      oleadas → cámara → salida de la zona. Si hay una escena o un apoyo en marcha, la
      acción está parada y el frame solo atiende a eso. Las reglas están en los
      módulos puros que usa (luchador, ninja, combate, apoyos, oleadas, marcador); aquí
      solo se les da la mano.

    SE CONECTA CON
      Incluye:  juego/fase.h, juego/cifras.h, juego/luchador.h.
      Lo usan:  tests/prueba_fase.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tablas en ROM)
      Y_HUECO[3]   u8      profundidad de los pies de cada hueco de enemigo
      celda_ninja  Celda   la celda de 64x96 de los ninjas
    → glosario: Director de fase, Oleada, Relevo, Apoyo
---------------------------------------------------------------------------------*/
#include "juego/fase.h"
#include "juego/cifras.h"
#include "juego/luchador.h"

/* Profundidad de los pies de cada hueco: repartidos, nunca tres a la misma altura (decisión 36). */
static const u8 Y_HUECO[FASE_ENEMIGOS] = {165, 180, 195};

static const Celda celda_ninja = {NINJA_ANCHO, NINJA_MITAD, NINJA_ALTO};

/*--- Ayudas ------------------------------------------------------------------------*/

u8 fase_vivos(const Fase *f)
{
    u8 i;
    u8 n;

    n = 0;
    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (f->vivo[i])
            n++;
    }
    return n;
}

static void limpiar_enemigos(Fase *f)
{
    u8 i;

    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        f->vivo[i] = 0;
        f->jefe[i] = 0;
        f->contado[i] = 0;
        f->golpeados_p[i] = 0;
        f->golpeados_n[i] = 0;
    }
}

/* Los límites de x del protagonista: no sale de la pantalla (16 px de margen a cada lado). */
static void carril_protagonista(const Fase *f, Carril *c)
{
    c->x_min_fp = PF(f->camara + 16);
    c->x_max_fp = PF(f->camara + ANCHO_PANTALLA - 16);
    c->y_arriba_fp = PF(150);
    c->y_abajo_fp = PF(208);
}

/* Los de los enemigos: también pueden estar fuera de la pantalla (por donde entran). */
static void carril_enemigos(const Fase *f, Carril *c)
{
    c->x_min_fp = PF(16);
    c->x_max_fp = PF(fase1_zona(f->zona)->longitud + 64);
    c->y_arriba_fp = PF(150);
    c->y_abajo_fp = PF(208);
}

static void empezar_zona(Fase *f)
{
    oleadas_iniciar(&f->oleadas, f->zona);
    f->camara = 0;
    f->zona_completa = 0;
    f->estado = FASE_JUGANDO;
    f->escena = ESC_NADA;
    limpiar_enemigos(f);
}

/* Sale el enemigo que pide la oleada: en un hueco libre, repartido en profundidad. */
static void sacar(Fase *f)
{
    u8 i;
    u8 rango;

    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (!f->vivo[i])
            break;
    }
    if (i >= FASE_ENEMIGOS)
        return;                      /* Sin hueco libre: no puede salir (no debería pasar).  */
    rango = f->oleadas.spawn_rango;
    if (f->oleadas.spawn_tipo == TIPO_JEFE)
        rango = RANGO_ROJO;          /* El jefe pega como un rojo; su vida, aparte.          */
    ninja_iniciar(&f->ninjas[i], f->oleadas.spawn_x, Y_HUECO[i], rango);
    f->vivo[i] = 1;
    f->jefe[i] = 0;
    f->contado[i] = 0;
    f->golpeados_p[i] = 0;
    f->golpeados_n[i] = 0;
    if (f->oleadas.spawn_tipo == TIPO_JEFE)
    {
        f->jefe[i] = 1;
        f->ninjas[i].g.vida = VIDA_GOMASURI;
    }
}

/*--- Enemigos ----------------------------------------------------------------------*/

/* Mueve a los enemigos. Como mucho MAX_ATACANDO atacan a la vez: el que ya ataca sigue; uno nuevo
   solo empieza si queda turno (se cuenta cada ataque que empieza en este frame). */
static void actualizar_enemigos(Fase *f)
{
    const Cuerpo *objetivo;
    Carril cn;
    u8 i;
    u8 atacando;
    u8 antes;
    u8 permiso;

    objetivo = &relevo_activo(&f->relevo)->c;
    carril_enemigos(f, &cn);
    atacando = 0;
    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (f->vivo[i] && f->ninjas[i].estado == NQ_ATACAR)
            atacando++;
    }
    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (!f->vivo[i])
            continue;
        antes = f->ninjas[i].estado == NQ_ATACAR;
        permiso = antes || atacando < MAX_ATACANDO;
        ninja_actualizar(&f->ninjas[i], objetivo, permiso, &cn);
        if (!antes && f->ninjas[i].estado == NQ_ATACAR)
            atacando++;              /* Empezó un ataque: gasta un turno.                    */
    }
}

/* Los golpes entre el protagonista activo y cada enemigo, en los dos sentidos. */
static void golpes(Fase *f)
{
    Luchador *p;
    Ninja *n;
    Celda celda_p;
    CajaCuadro cuerpo_p;
    CajaCuadro cuerpo_n;
    CajaCuadro ataque;
    const CajaCuadro *caja;
    CajaMundo golpe;
    CajaMundo cuerpo;
    u8 i;
    u8 r;

    p = relevo_activo(&f->relevo);
    luchador_celda(p, &celda_p);
    luchador_caja_dano(p, &cuerpo_p);
    caja = luchador_caja_golpe(p);
    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (!f->vivo[i])
            continue;
        n = &f->ninjas[i];

        /* El protagonista le pega al enemigo. */
        if (caja == 0)
        {
            f->golpeados_p[i] = 0;   /* Sin caja activa: el próximo ataque vuelve a contar.  */
        }
        else
        {
            ninja_caja_dano(n, &cuerpo_n);
            combate_caja_mundo(caja, &celda_p, &p->c, p->mira_izquierda, &golpe);
            combate_caja_mundo(&cuerpo_n, &celda_ninja, &n->c, n->mira_izquierda, &cuerpo);
            r = combate_golpear(&golpe, caja, &p->c, &p->g, &cuerpo, &n->c, &n->g,
                                &f->golpeados_p[i], 1);
            if (r != GOLPE_FALLA)
            {
                luchador_marcar_conecto(p);
                marcador_sumar(&f->marcador, PUNTOS_GOLPE);
                apoyos_golpe_conecto(&f->apoyos);        /* Cada golpe que conecta suma a la barra. */
                ninja_reaccionar(n, r);
            }
        }

        /* El enemigo le pega al protagonista con su shinai. */
        if (ninja_caja_golpe(n, &ataque))
        {
            combate_caja_mundo(&ataque, &celda_ninja, &n->c, n->mira_izquierda, &golpe);
            combate_caja_mundo(&cuerpo_p, &celda_p, &p->c, p->mira_izquierda, &cuerpo);
            r = combate_golpear(&golpe, &ataque, &n->c, &n->g, &cuerpo, &p->c, &p->g,
                                &f->golpeados_n[i], 1);
            if (r != GOLPE_FALLA)
                luchador_reaccionar(p, r);
        }
        else
        {
            f->golpeados_n[i] = 0;
        }
    }
}

/* El golpe de entrada del relevo: 4 de daño y retroceso a los que estén a menos de 48 px. */
static void golpe_de_entrada(Fase *f)
{
    Luchador *p;
    u8 i;
    u8 r;

    p = relevo_activo(&f->relevo);
    if (!luchador_entrada_activa(p))
        return;
    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (!f->vivo[i] || f->ninjas[i].g.vida == 0)
            continue;
        if (!combate_empuje_cercano(&p->c, &f->ninjas[i].c, ALCANCE_ENTRADA_RELEVO))
            continue;
        r = combate_dano_directo(&f->ninjas[i].g, DANO_ENTRADA_RELEVO, EFECTO_RETROCESO);
        if (r != GOLPE_FALLA)
            ninja_reaccionar(&f->ninjas[i], r);
    }
}

/* Los que se quedaron sin vida dan sus puntos (una sola vez); los que acabaron su KO liberan el hueco. */
static void contar_bajas(Fase *f)
{
    u8 i;
    u16 puntos;

    for (i = 0; i < FASE_ENEMIGOS; i++)
    {
        if (!f->vivo[i])
            continue;
        if (f->ninjas[i].g.vida == 0 && !f->contado[i])
        {
            f->contado[i] = 1;
            if (f->jefe[i])
                puntos = PUNTOS_JEFE;
            else
                puntos = cifras_puntos_ninja(f->ninjas[i].rango);
            marcador_sumar(&f->marcador, puntos);
        }
        if (f->ninjas[i].vencido)
            f->vivo[i] = 0;
    }
}

/*--- Apoyos ------------------------------------------------------------------------*/

/* Un frame del apoyo en curso: aplica sus eventos a todos los enemigos y al protagonista. */
static void actualizar_apoyo(Fase *f)
{
    Luchador *p;
    u8 ev;
    u8 i;
    u8 dano;
    u8 efecto;
    u8 r;

    p = relevo_activo(&f->relevo);
    ev = apoyos_actualizar(&f->apoyos);
    f->eventos_apoyo = ev;
    if (ev & APOYO_EV_GOLPE)
    {
        for (i = 0; i < FASE_ENEMIGOS; i++)
        {
            if (!f->vivo[i] || f->ninjas[i].g.vida == 0)
                continue;
            dano = apoyos_dano(f->apoyo, f->jefe[i]);
            efecto = apoyos_derriba(f->apoyo, f->jefe[i]) ? EFECTO_DERRIBO : EFECTO_RETROCESO;
            r = combate_dano_directo(&f->ninjas[i].g, dano, efecto);
            if (r != GOLPE_FALLA)
                ninja_reaccionar(&f->ninjas[i], r);
        }
    }
    if (ev & APOYO_EV_CURAR)
        p->g.vida = apoyos_curar_vida(p->g.vida, f->apoyo);
    contar_bajas(f);
    if (ev & APOYO_EV_FIN)
    {
        luchador_terminar_invocar(p);
        f->estado = FASE_JUGANDO;
    }
}

/*--- Oleadas, cámara y salida ------------------------------------------------------*/

static u8 avanzar_oleadas(Fase *f)
{
    u8 oe;

    oe = oleadas_actualizar(&f->oleadas, f->camara, fase_vivos(f));
    if (oe & OL_EV_SPAWN)
        sacar(f);
    if (oe & OL_EV_ZONA)
        f->zona_completa = 1;
    if (oe & OL_EV_ESCENA)
    {
        f->escena = f->oleadas.escena;
        f->estado = FASE_ESCENA;     /* La acción se para hasta fase_escena_terminada.       */
        return FASE_EV_ESCENA;
    }
    return FASE_EV_NADA;
}

/* La cámara sigue al protagonista hacia la derecha, sin retroceder y sin pasar del bloqueo. */
static void actualizar_camara(Fase *f)
{
    u16 x;
    u16 limite;
    u16 nueva;

    x = PF_ENTERO(relevo_activo(&f->relevo)->c.x_fp);
    if (x <= f->camara + CAMARA_ADELANTE)
        return;                      /* Aún no pasó de la mitad de la pantalla.              */
    limite = oleadas_limite_camara(&f->oleadas);
    nueva = x - CAMARA_ADELANTE;
    if (nueva > limite)
        nueva = limite;              /* Bloqueada por la oleada (o por el fin de la zona).   */
    if (nueva > f->camara)
        f->camara = nueva;
}

/* Zona completa y el protagonista llegó al final: se pasa a la siguiente. En el dojo no hay salida:
   al vencer al jefe y acabar su escena viene el STAGE CLEAR. */
static u8 comprobar_salida(Fase *f)
{
    u16 x;

    if (!f->zona_completa || f->zona == ZONA_DOJO)
        return FASE_EV_NADA;
    x = PF_ENTERO(relevo_activo(&f->relevo)->c.x_fp);
    if (x + SALIDA_MARGEN >= fase1_zona(f->zona)->longitud)
    {
        f->estado = FASE_SALIDA;
        return FASE_EV_SALIDA;
    }
    return FASE_EV_NADA;
}

/*--- Vidas -------------------------------------------------------------------------*/

/* Cayeron los dos: pierde una vida. Sin vidas, fin de la partida; si no, los dos vuelven con la
   vida entera y la barra llena (spec §4), donde está la cámara. */
static u8 perder_vida(Fase *f)
{
    if (marcador_perder_vida(&f->marcador))
    {
        f->estado = FASE_FIN_JUEGO;
        return FASE_EV_FIN_JUEGO;
    }
    relevo_iniciar(&f->relevo, (u16)(f->camara + X_INICIAL_ZONA), 180);
    if (f->apoyos.activa)
        apoyos_llenar(&f->apoyos);
    return FASE_EV_VIDA_PERDIDA;
}

/*--- Funciones públicas ------------------------------------------------------------*/

void fase_iniciar(Fase *f)
{
    marcador_iniciar(&f->marcador);
    apoyos_iniciar(&f->apoyos);
    relevo_iniciar(&f->relevo, X_INICIAL_ZONA, 180);
    f->zona = ZONA_ALDEA;
    f->apoyo = 0;
    f->eventos_apoyo = 0;
    empezar_zona(f);
}

u8 fase_actualizar(Fase *f, const Mando *m)
{
    Luchador *p;
    Carril cp;
    u8 ev;
    u8 r;

    f->eventos_apoyo = 0;
    if (f->estado == FASE_APOYO)
    {
        actualizar_apoyo(f);         /* La acción está parada: solo actúa el apoyo.          */
        return FASE_EV_NADA;
    }
    if (f->estado != FASE_JUGANDO)
        return FASE_EV_NADA;         /* Escena, salida, clear o fin de partida: parada.      */

    if (m->apoyo_pulsado)
        apoyos_siguiente(&f->apoyos);                    /* R: siguiente apoyo.             */
    p = relevo_activo(&f->relevo);
    if (m->invocar_pulsado && apoyos_puede_invocar(&f->apoyos) && luchador_puede_invocar(p))
    {
        if (apoyos_invocar(&f->apoyos))                  /* X: invoca el elegido.           */
        {
            f->apoyo = (u8)(f->apoyos.en_curso - 1);
            luchador_invocar(p);
            f->estado = FASE_APOYO;
            return FASE_EV_APOYO;
        }
    }

    carril_protagonista(f, &cp);
    r = relevo_actualizar(&f->relevo, m, &cp);           /* Relevo (L) y protagonista.      */
    if (r == RELEVO_PIERDE_VIDA)
        return perder_vida(f);

    actualizar_enemigos(f);
    golpes(f);
    golpe_de_entrada(f);
    contar_bajas(f);
    ev = avanzar_oleadas(f);
    actualizar_camara(f);
    ev = (u8)(ev | comprobar_salida(f));
    return ev;
}

u8 fase_escena_terminada(Fase *f)
{
    u8 oe;

    if (f->estado != FASE_ESCENA)
        return FASE_EV_NADA;
    fase1_aplicar_escena(f->escena, &f->apoyos);         /* Lucinda y Rex se unen; la barra se activa. */
    oe = oleadas_escena_terminada(&f->oleadas);
    f->escena = ESC_NADA;
    f->estado = FASE_JUGANDO;
    if (oe & OL_EV_ZONA)
    {
        f->zona_completa = 1;
        if (f->zona == ZONA_DOJO)
        {
            f->estado = FASE_CLEAR;                      /* Venció al jefe y acabó su escena.  */
            return FASE_EV_CLEAR;
        }
    }
    return FASE_EV_NADA;
}

void fase_siguiente_zona(Fase *f)
{
    if (f->zona + 1 < FASE1_ZONAS)
        f->zona++;
    empezar_zona(f);
    /* Los dos protagonistas empiezan la zona al principio, con la vida que traen. */
    f->relevo.tarou.c.x_fp = PF(X_INICIAL_ZONA);
    f->relevo.barbara.c.x_fp = PF(X_INICIAL_ZONA);
}

u8 fase_continuar(Fase *f)
{
    if (f->estado != FASE_FIN_JUEGO)
        return 0;
    if (!marcador_continuar(&f->marcador))
        return 0;                    /* No quedan continues: al título.                      */
    relevo_iniciar(&f->relevo, (u16)(f->camara + X_INICIAL_ZONA), 180);
    if (f->apoyos.activa)
        apoyos_llenar(&f->apoyos);
    f->estado = FASE_JUGANDO;
    return 1;
}
