/*---------------------------------------------------------------------------------
    demo_23_combate.c — Demo de la unidad 23: el combate del beat 'em up cuadro a cuadro,
    con el código REAL del juego (mundo.c, combate.c, ninja.c), no con una copia.

    PROPÓSITO
      Se compila contra los fuentes de src/ con -DPRUEBAS_PC y enseña cinco cosas:
        1. La CAJA DE GOLPE de Tarou y la del ninja dibujadas en ASCII (combate_caja):
           un rectángulo en el suelo delante del luchador, con tolerancia en y.
        2. El COMBO cuadro a cuadro contra un ninja: quién golpea a quién, la ventana de
           encadenado (cola), los 4 cuadros activos de cada tajo que solo pegan UNA vez
           (id) y la PARADA DE IMPACTO. Repite exactamente el orden del frame de
           src/pantallas/pantalla_fase1.c (mundo_actualizar → combate_resolver → IA del
           ninja → combate_ninjas_resolver).
        3. El BUFFER DE ENTRADA: la misma partida con y sin «a_guardada» (una A pulsada
           durante la parada se guarda): sin el buffer el combo se rompe.
        4. El otro sentido: el tajo del ninja contra Tarou (mundo_recibir), la
           INVULNERABILIDAD con parpadeo y un segundo tajo que se pierde.
        5. El KO: Tarou sin vida cae y no recibe más.
      Todo lo que imprime sale de funciones del juego; lo único propio de la demo es el
      «jugador» (un guion de A) y el bucle del frame.

    SE CONECTA CON
      Incluye:  <stdio.h>, <string.h>, juego/mundo.h, juego/combate.h, enemigos/ninja.h.
      Modelo:   src/pantallas/pantalla_fase1.c (pantalla_fase1_actualizar), tests/prueba_combate.c.
      Compilar (desde esta carpeta, PowerShell o MSYS2; ver README, «Cómo se prueba»):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_23.exe demo_23_combate.c
            ../../src/juego/mundo.c ../../src/juego/combate.c ../../src/enemigos/ninja.c
        (una sola línea)

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES (de la struct Mundo de esta demo: el mini-juego que reúne lo que en el
    juego son las estáticas de pantalla_fase1.c)
      tarou       Cuerpo    el protagonista
      ninjas      Ninja[3]  las ranuras de enemigos
      parada      u8        cuadros de parada de impacto que quedan
      a_guardada  u8        1 si hubo una A durante la parada (el buffer)
    → conceptos (README de la unidad 23): caja de golpe, combo, cola, id, parada, buffer
---------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "juego/mundo.h"
#include "juego/combate.h"
#include "enemigos/ninja.h"

typedef struct
{
    Cuerpo tarou;                                    /* El protagonista.                    */
    Ninja ninjas[NINJAS_MAX];                        /* Las tres ranuras de enemigos.       */
    u8 parada;                                       /* Cuadros de parada que quedan.       */
    u8 a_guardada;                                   /* El buffer: A pulsada en la parada.  */
    u8 con_buffer;                                   /* 0 = la demo desactiva el buffer.    */
} Mundo;

/* Un frame del juego: la misma secuencia que pantalla_fase1_actualizar, sin cámara ni
   oleadas. `a` es lo que pulsó el jugador. Devuelve 1 si el mundo estaba congelado. */
static u8 frame(Mundo *w, u8 a)
{
    Mando m;
    u8 k, turnos, golpe_ninja;

    memset(&m, 0, sizeof(m));                        /* Ninguna tecla...                    */
    m.atacar_pulsado = a;                            /* ...salvo A, recién pulsada.         */
    if (w->parada)                                   /* Parada de impacto: todo congelado.  */
    {
        w->parada--;
        if (a && w->con_buffer)
        {
            w->a_guardada = 1;                       /* Se guarda para cuando acabe.        */
        }
        return 1;
    }
    if (w->a_guardada)
    {
        m.atacar_pulsado = 1;                        /* La A guardada vale ahora.           */
        w->a_guardada = 0;
    }
    mundo_actualizar(&w->tarou, &m);
    w->parada = combate_resolver(&w->tarou, w->ninjas);       /* Tarou → ninjas.            */
    turnos = ninja_atacando(w->ninjas);
    for (k = 0; k < NINJAS_MAX; k++)
    {
        if (w->ninjas[k].vivo)
        {
            ninja_actualizar(w->ninjas + k, w->tarou.x, w->tarou.y);
            w->ninjas[k].dentro = 1;                 /* (Sin pantalla: ya están dentro.)    */
            if (ninja_intentar_atacar(w->ninjas + k, w->tarou.x, w->tarou.y,
                                      (u8)(turnos < NINJA_TURNOS)))
            {
                turnos++;
            }
        }
    }
    golpe_ninja = combate_ninjas_resolver(w->ninjas, &w->tarou);   /* Ninjas → Tarou.       */
    if (golpe_ninja > w->parada)
    {
        w->parada = golpe_ninja;                     /* La parada mayor.                    */
    }
    return 0;
}

static const char *nombre_ninja(u8 e)
{
    static const char *N[] = {"PARADO  ", "ANDANDO ", "ATACANDO", "RECIBIR ", "DERRIBAD"};
    return N[e];
}

/* Prepara un mundo: Tarou en (400, 200) y un solo ninja delante, a `dx` px. */
static void preparar(Mundo *w, u16 ninja_dx, u8 con_buffer)
{
    memset(w, 0, sizeof(*w));
    mundo_iniciar(&w->tarou, 400, 200);
    ninja_iniciar_todos(w->ninjas);
    ninja_oleada(w->ninjas, 300, 0);                 /* Una ranura ocupada: la 0.           */
    w->ninjas[0].x = (u16)(400 + ninja_dx);
    w->ninjas[0].y = 204;                            /* 4 px más cerca de la cámara.        */
    w->ninjas[0].dentro = 1;
    w->ninjas[0].espera = 200;                       /* No ataca en esta parte.             */
    w->con_buffer = con_buffer;
}

/*--- 1. Las cajas, dibujadas ----------------------------------------------------------*/
static void dibujar_caja(const char *titulo, u8 mira_izq, u16 alcance, u16 profundidad)
{
    int dx, dy;

    printf("%s\n", titulo);
    printf("      dx: -16 -12 -8 -4  0  4  8 12 16 20 24 28 32 36 40 44 48 52 56 60\n");
    for (dy = -16; dy <= 16; dy += 4)
    {
        printf("  dy %+3d  ", dy);
        for (dx = -16; dx <= 60; dx += 4)
        {
            /* El luchador está en (100, 200). El objetivo, a (dx, dy) de él; `dx` positivo
               es hacia donde MIRA (se invierte si mira a la izquierda). */
            u16 tx = (u16)(100 + (mira_izq ? -dx : dx));
            u16 ty = (u16)(200 + dy);
            char c = combate_caja(100, 200, mira_izq, alcance, profundidad, tx, ty) ? '#' : '.';
            if (dx == 0 && dy == 0)
            {
                c = (c == '#') ? '@' : 'o';
            }
            printf("  %c ", c);
        }
        printf("\n");
    }
    printf("  (# = un objetivo con los pies ahí recibiría el golpe; @ = los pies del luchador)\n\n");
}

/*--- 2 y 3. El combo cuadro a cuadro -----------------------------------------------------*/
/* El «jugador»: pulsa A en el cuadro 0 y, en los golpes 1 y 2, en el primer cuadro de
   parada que vea (un jugador que reacciona al choque, antes de GOLPE_ENCADENAR_DESDE). */
static u8 jugador(const Mundo *w, u16 n, u8 *pulso_en_golpe)
{
    if (n == 0)
    {
        return 1;                                    /* Empieza el combo.                   */
    }
    if (w->parada > 0 && w->tarou.golpe < GOLPES && *pulso_en_golpe != w->tarou.id)
    {
        *pulso_en_golpe = w->tarou.id;               /* Una sola A por tajo.                */
        return 1;
    }
    return 0;
}

static void combo(u8 con_buffer, u8 imprimir, u8 fijo)
{
    Mundo w;
    u16 n, activos_dentro = 0, golpes_contados = 0;
    u8 pulso = 0, a, congelado, vida_antes;
    u8 golpe_max = 0;

    preparar(&w, 44, con_buffer);
    if (fijo)
    {
        w.ninjas[0].x = 440;                         /* Distancia de combate: 40 px.        */
    }
    if (imprimir)
    {
        printf("  cuadro  A  parada | Tarou: golpe reloj cola  id activo | ninja: estado   vida id_golpe x\n");
    }
    for (n = 0; n < 60; n++)
    {
        a = jugador(&w, n, &pulso);
        vida_antes = w.ninjas[0].vida;
        congelado = frame(&w, a);
        if (fijo)
        {
            w.ninjas[0].x = 440;                     /* Clavado en su sitio (como la prueba). */
        }
        if (!congelado && mundo_golpe_activo(&w.tarou) &&
            combate_en_caja(&w.tarou, w.ninjas[0].x, w.ninjas[0].y))
        {
            activos_dentro++;                        /* Cuadros de juego con la caja encima. */
        }
        if (w.ninjas[0].vida < vida_antes)
        {
            golpes_contados++;
        }
        if (w.tarou.golpe > golpe_max)
        {
            golpe_max = w.tarou.golpe;
        }
        if (imprimir)
        {
            printf("  %5u  %c  %6u | %5u %5u %4u %3u %6s | %s  %4u %8u %u%s%s\n", (unsigned)n,
                   a ? 'A' : '-', (unsigned)w.parada, (unsigned)w.tarou.golpe,
                   (unsigned)w.tarou.reloj, (unsigned)w.tarou.cola, (unsigned)w.tarou.id,
                   mundo_golpe_activo(&w.tarou) ? "SI" : "no", nombre_ninja(w.ninjas[0].estado),
                   (unsigned)w.ninjas[0].vida, (unsigned)w.ninjas[0].id_golpe,
                   (unsigned)w.ninjas[0].x, congelado ? "  <- congelado" : "",
                   (w.ninjas[0].vida < vida_antes) ? "  <- GOLPE CUENTA" : "");
        }
    }
    printf("  Resultado %s buffer, ninja %s: golpe más alto del combo = %u, golpes que contaron = %u,\n"
           "  cuadros de juego con la caja activa encima del ninja = %u, vida final del ninja = %u de %u.\n\n",
           con_buffer ? "CON" : "SIN", fijo ? "clavado" : "libre", (unsigned)golpe_max,
           (unsigned)golpes_contados, (unsigned)activos_dentro, (unsigned)w.ninjas[0].vida,
           (unsigned)NINJA_VIDA);
}

/*--- 4. El otro sentido: ninjas contra Tarou --------------------------------------------*/
static void tajos(void)
{
    Mundo w;
    u16 n;
    u8 vida_antes;

    preparar(&w, 40, 1);
    w.ninjas[0].espera = 0;                          /* Listo para atacar ya.               */
    w.ninjas[0].mira_izq = 1;                        /* Mira a Tarou.                       */
    ninja_oleada(w.ninjas, 300, 1);                  /* Ocupa también la ranura 1...        */
    w.ninjas[0].x = 440; w.ninjas[0].y = 204; w.ninjas[0].dentro = 1;   /* (la 0 se recoloca) */
    w.ninjas[1].x = 360; w.ninjas[1].y = 196; w.ninjas[1].dentro = 1;   /* ...a la izquierda. */
    w.ninjas[1].mira_izq = 0;
    w.ninjas[1].espera = 18;                         /* Su tajo cae con Tarou invulnerable. */
    printf("  cuadro | ninja0: estado   timer conecto | ninja1: estado   timer conecto | "
           "Tarou: vida inv stun estado    dibujo\n");
    for (n = 0; n < 100; n++)
    {
        vida_antes = w.tarou.vida;
        frame(&w, 0);
        if (n <= 1 || (n >= 10 && n <= 13) || (n >= 30 && n <= 41) ||
            (n >= 48 && n <= 76 && (n % 4 == 0)) || (n >= 90 && n <= 99) ||
            w.tarou.vida != vida_antes)
        {
            static const char *E[] = {"PARADO  ", "ANDANDO ", "SUBIENDO", "BAJANDO ",
                                      "ATACANDO", "RECIBIR ", "DERRIBAD"};
            printf("  %5u  | %s %5u %7s | %s %5u %7s | %5u %3u %4u %s  %s%s\n", (unsigned)n,
                   nombre_ninja(w.ninjas[0].estado), (unsigned)w.ninjas[0].timer,
                   w.ninjas[0].conecto ? "SI" : "no",
                   nombre_ninja(w.ninjas[1].estado), (unsigned)w.ninjas[1].timer,
                   w.ninjas[1].conecto ? "SI" : "no", (unsigned)w.tarou.vida,
                   (unsigned)w.tarou.inv, (unsigned)w.tarou.stun, E[w.tarou.estado],
                   (w.tarou.inv != 0 && (w.tarou.inv & 2)) ? "(oculto)" : "visible",
                   (w.tarou.vida != vida_antes) ? "   <- TAJO CUENTA" : "");
        }
    }
    printf("  Tarou: vida %u de %u. conecto del ninja 1 = %u (su tajo se gastó contra la invulnerabilidad).\n\n",
           (unsigned)w.tarou.vida, (unsigned)VIDA_TAROU, (unsigned)w.ninjas[1].conecto);
}

/*--- 5. KO -------------------------------------------------------------------------------*/
static void ko(void)
{
    Cuerpo c;
    u16 n;

    mundo_iniciar(&c, 400, 200);
    c.vida = 2;                                      /* Le queda un tajo.                   */
    printf("  mundo_recibir con vida 2 y daño %u -> devuelve %u\n", (unsigned)NINJA_DANIO,
           (unsigned)mundo_recibir(&c, NINJA_DANIO, 0));
    printf("  vida %u, estado %u (DERRIBADO = %u), stun %u, inv %u\n", (unsigned)c.vida,
           (unsigned)c.estado, (unsigned)CUERPO_DERRIBADO, (unsigned)c.stun, (unsigned)c.inv);
    for (n = 0; n < 200; n++)
    {
        Mando m;
        memset(&m, 0, sizeof(m));
        m.atacar_pulsado = 1;                        /* Machaca A: no hace nada.            */
        mundo_actualizar(&c, &m);
    }
    printf("  tras 200 cuadros machacando A: estado %u, golpe %u, stun %u; otro golpe -> %u\n",
           (unsigned)c.estado, (unsigned)c.golpe, (unsigned)c.stun,
           (unsigned)mundo_recibir(&c, NINJA_DANIO, 0));
}

int main(void)
{
    printf("=== Unidad 23: combate (código real de src/) ===\n\n");
    printf("Constantes: GOLPES %d, GOLPE_FRAMES %d (el último %d), activo %d..%d (sin incluir), "
           "encadenar desde %d,\n  COMBATE_ALCANCE %d, COMBATE_ATRAS %d, COMBATE_PROFUNDIDAD %d, "
           "PARADA_GOLPE %d, PARADA_DERRIBO %d,\n  NINJA_VIDA %d, NINJA_DANIO %d, VIDA_TAROU %d, "
           "INVULNERABLE_FRAMES %d\n\n",
           GOLPES, GOLPE_FRAMES, GOLPE_ULTIMO_FRAMES, GOLPE_ACTIVO_DESDE, GOLPE_ACTIVO_HASTA,
           GOLPE_ENCADENAR_DESDE, COMBATE_ALCANCE, COMBATE_ATRAS, COMBATE_PROFUNDIDAD,
           PARADA_GOLPE, PARADA_DERRIBO, NINJA_VIDA, NINJA_DANIO, VIDA_TAROU, INVULNERABLE_FRAMES);

    printf("--- 1. La caja de golpe ---\n\n");
    dibujar_caja("Tarou mirando a la derecha (alcance 52, profundidad 14):", 0, COMBATE_ALCANCE,
                 COMBATE_PROFUNDIDAD);
    dibujar_caja("Tarou mirando a la izquierda (la misma caja, reflejada):", 1, COMBATE_ALCANCE,
                 COMBATE_PROFUNDIDAD);
    dibujar_caja("Un ninja mirando a la derecha (alcance 48, profundidad 12):", 0, NINJA_ALCANCE,
                 NINJA_PROFUNDIDAD);

    printf("--- 2. El combo CON buffer, cuadro a cuadro (ninja a 44 px, 4 px más abajo) ---\n\n");
    combo(1, 1, 0);
    printf("--- 3. Variantes (solo el resumen) ---\n\n");
    printf("  El mismo jugador SIN buffer (a_guardada desactivada):\n");
    combo(0, 0, 0);
    printf("  CON buffer pero con el ninja clavado a 40 px, como en tests/prueba_combate.c\n"
           "  (esa prueba ni llama a ninja_actualizar, así que el empujón no lo mueve):\n");
    combo(1, 0, 1);

    printf("--- 4. Los tajos de dos ninjas contra Tarou ---\n\n");
    tajos();

    printf("--- 5. KO ---\n\n");
    ko();
    return 0;
}
