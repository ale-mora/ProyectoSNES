/*---------------------------------------------------------------------------------
    prueba_combate.c — Pruebas de juego/combate.c.

    PROPÓSITO
      Fijar las reglas del golpe (docs/diseno-beat-em-up.md §3, §5.2, §5.3 y
      §5.5): la caja se espeja al mirar al oeste, el golpe solo conecta si las
      cajas se pisan Y la profundidad está a TOLERANCIA_Y o menos, un ataque
      daña una sola vez a cada objetivo, el derribo congela a los dos
      FRAMES_PARADA frames, el último golpe deja la vida en 0 sin pasarse y el
      especial nunca mata. También las tablas por rango.

    SE CONECTA CON
      Incluye:  minitest.h, juego/combate.h, juego/cifras.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_combate).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Caja de golpe / caja de daño, Parada de impacto
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/combate.h"
#include "juego/cifras.h"

/* Rellena una caja de celda. */
static void caja(CajaCuadro *c, u8 x, u8 y, u8 ancho, u8 alto, u8 dano, u8 efecto)
{
    c->x = x;
    c->y = y;
    c->ancho = ancho;
    c->alto = alto;
    c->dano = dano;
    c->efecto = efecto;
    c->relleno[0] = 0;
    c->relleno[1] = 0;
}

void pruebas_combate(void)
{
    Celda tarou = {96, 48, 64};                      /* Celda de 96x64.                     */
    Celda ninja = {64, 32, 96};                      /* Celda de 64x96.                     */
    Cuerpo atacante, objetivo;
    Golpeable atk, obj;
    CajaCuadro golpe_c, dano_c;
    CajaMundo golpe, dano;
    u8 golpeados;

    COMPROBAR(sizeof(CajaCuadro) == 8);              /* §5.2: 8 bytes por entrada.          */
    COMPROBAR(sizeof(Golpeable) == 4);

    /* De la celda al mundo: pies en (100, 180) => celda de Tarou en (52, 116). */
    mundo_colocar(&atacante, 100, 180);
    caja(&golpe_c, 60, 20, 30, 16, DANO_COMBO_TAROU, EFECTO_RETROCESO);
    combate_caja_mundo(&golpe_c, &tarou, &atacante, 0, &golpe);
    COMPROBAR(golpe.x == 112 && golpe.y == 136 && golpe.ancho == 30 && golpe.alto == 16);

    /* Mirando al oeste se espeja dentro de la celda: x' = 96 − 60 − 30 = 6. */
    combate_caja_mundo(&golpe_c, &tarou, &atacante, 1, &golpe);
    COMPROBAR(golpe.x == 58 && golpe.y == 136);

    /* En el aire la caja sube con z. */
    atacante.z_fp = PF(20);
    combate_caja_mundo(&golpe_c, &tarou, &atacante, 0, &golpe);
    COMPROBAR(golpe.y == 116);
    atacante.z_fp = 0;

    /* Cruce de rectángulos: tocarse por el borde no cuenta, pisarse 1 px sí. */
    golpe.x = 0; golpe.y = 0; golpe.ancho = 10; golpe.alto = 10;
    dano.x = 10; dano.y = 0; dano.ancho = 10; dano.alto = 10;
    COMPROBAR(combate_se_cruzan(&golpe, &dano) == 0);
    dano.x = 9;
    COMPROBAR(combate_se_cruzan(&golpe, &dano) == 1);
    dano.ancho = 0;                                  /* Caja vacía: nunca toca.             */
    COMPROBAR(combate_se_cruzan(&golpe, &dano) == 0);

    /* Un ninja delante de Tarou, a la misma profundidad. */
    mundo_colocar(&atacante, 100, 180);
    mundo_colocar(&objetivo, 130, 180);
    atk.vida = VIDA_PROTAGONISTA; atk.invulnerable = 0; atk.destello = 0; atk.parada = 0;
    obj.vida = 16; obj.invulnerable = 0; obj.destello = 0; obj.parada = 0;
    caja(&dano_c, 16, 8, 32, 88, 0, 0);
    combate_caja_mundo(&golpe_c, &tarou, &atacante, 0, &golpe);
    combate_caja_mundo(&dano_c, &ninja, &objetivo, 1, &dano);
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_RETROCESO);
    COMPROBAR(obj.vida == 13 && obj.destello == FRAMES_DESTELLO && obj.parada == 0);
    COMPROBAR(atk.parada == 0);

    /* El mismo ataque no le vuelve a dar. */
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_FALLA);
    COMPROBAR(obj.vida == 13);

    /* La regla de profundidad: a 8 px conecta, a 9 px pasa de largo. */
    objetivo.y_fp = PF(188);
    combate_caja_mundo(&dano_c, &ninja, &objetivo, 1, &dano);
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_RETROCESO);
    objetivo.y_fp = PF(171);
    combate_caja_mundo(&dano_c, &ninja, &objetivo, 1, &dano);
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_FALLA);
    COMPROBAR(obj.vida == 10);

    /* Derribo: el golpe fuerte congela a los dos. */
    objetivo.y_fp = PF(180);
    combate_caja_mundo(&dano_c, &ninja, &objetivo, 1, &dano);
    caja(&golpe_c, 60, 20, 30, 16, DANO_MORDISCO_TAROU, EFECTO_DERRIBO);
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_DERRIBO);
    COMPROBAR(obj.vida == 5 && obj.parada == FRAMES_PARADA && atk.parada == FRAMES_PARADA);

    /* Los contadores bajan un frame cada vez y se paran en 0. */
    combate_tic(&obj);
    COMPROBAR(obj.parada == FRAMES_PARADA - 1 && obj.destello == FRAMES_DESTELLO - 1);
    combate_tic(&obj); combate_tic(&obj); combate_tic(&obj); combate_tic(&obj);
    COMPROBAR(obj.parada == 0 && obj.destello == 0);

    /* Invulnerable: no recibe. */
    obj.invulnerable = FRAMES_INVULNERABLE;
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_FALLA);
    obj.invulnerable = 0;

    /* El último golpe deja la vida en 0, sin dar la vuelta del u8. */
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_VENCE);
    COMPROBAR(obj.vida == 0);
    golpeados = 0;                                   /* Vencido: ya no recibe.              */
    COMPROBAR(combate_golpear(&golpe, &golpe_c, &atacante, &atk, &dano, &objetivo, &obj,
                              &golpeados, 1) == GOLPE_FALLA);

    /* El combo de Tarou (3+3+3+5) no basta para un ninja amarillo; uno blanco cae con él + 1. */
    COMPROBAR(3 * DANO_COMBO_TAROU + DANO_MORDISCO_TAROU == 14);
    COMPROBAR(2 * DANO_COMBO_BARBARA + DANO_TAJO_BARBARA >= cifras_vida_ninja(RANGO_BLANCO));

    /* El especial nunca mata. */
    COMPROBAR(combate_pagar_especial(64) == 60);
    COMPROBAR(combate_pagar_especial(5) == 1);
    COMPROBAR(combate_pagar_especial(4) == 1);
    COMPROBAR(combate_pagar_especial(1) == 1);

    /* Tablas por rango. */
    COMPROBAR(cifras_vida_ninja(RANGO_BLANCO) == 16 && cifras_vida_ninja(RANGO_ROJO) == 48);
    COMPROBAR(cifras_bonus_dano(RANGO_AMARILLO) == 1 && cifras_bonus_dano(RANGO_ROJO) == 3);
    COMPROBAR(cifras_puntos_ninja(RANGO_VERDE) == 200);
}
