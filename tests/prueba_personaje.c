/*---------------------------------------------------------------------------------
    prueba_personaje.c — Pruebas de juego/personaje.c (qué animación enseña la
    protagonista y cómo avanza).

    PROPÓSITO
      Fijar las reglas del 2026-11-04: cada estado del Cuerpo tiene su animación, cada
      golpe del combo la suya; los golpes de Bárbara caben en la duración del golpe
      (los números del combate no cambian); su alcance es 72 y el de Tarou 52; las
      animaciones sin bucle se quedan en el último cuadro y las de bucle vuelven al 0.

    SE CONECTA CON
      Incluye:  minitest.h, juego/personaje.h, juego/combate.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_personaje).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (cada prueba usa un Cuerpo y un AnimEstado locales).
    → glosario: Tabla de animaciones
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/personaje.h"
#include "juego/combate.h"

void pruebas_personaje(void)
{
    Cuerpo c;
    AnimEstado e;
    const AnimProta *b = personaje_anims(PERSONAJE_BARBARA);
    const AnimProta *t = personaje_anims(PERSONAJE_TAROU);
    u8 k, p;

    /* Cada estado, su animación; al atacar, la del golpe en curso. */
    mundo_iniciar(&c, 100, 200);
    COMPROBAR(personaje_anim_de(&c) == ANIM_GUARDIA);
    c.estado = CUERPO_ANDANDO;    COMPROBAR(personaje_anim_de(&c) == ANIM_CAMINAR);
    c.estado = CUERPO_SUBIENDO;   COMPROBAR(personaje_anim_de(&c) == ANIM_SALTAR);
    c.estado = CUERPO_BAJANDO;    COMPROBAR(personaje_anim_de(&c) == ANIM_CAER);
    c.estado = CUERPO_RECIBIR;    COMPROBAR(personaje_anim_de(&c) == ANIM_RECIBIR);
    c.estado = CUERPO_DERRIBADO;  COMPROBAR(personaje_anim_de(&c) == ANIM_KO);
    c.estado = CUERPO_ATACANDO;
    c.golpe = 1;                  COMPROBAR(personaje_anim_de(&c) == ANIM_GOLPE_1);
    c.golpe = 2;                  COMPROBAR(personaje_anim_de(&c) == ANIM_GOLPE_2);
    c.golpe = 3;                  COMPROBAR(personaje_anim_de(&c) == ANIM_GOLPE_3);

    /* Los golpes de Bárbara caben en el golpe: los dibujos se acaban como mucho un
       dibujo después de que acabe (el último se ve el tiempo que quede). */
    COMPROBAR(b[ANIM_GOLPE_1].cuadros * b[ANIM_GOLPE_1].frames <= GOLPE_FRAMES + b[ANIM_GOLPE_1].frames);
    COMPROBAR(b[ANIM_GOLPE_2].cuadros * b[ANIM_GOLPE_2].frames <= GOLPE_FRAMES + b[ANIM_GOLPE_2].frames);
    COMPROBAR(b[ANIM_GOLPE_3].cuadros * b[ANIM_GOLPE_3].frames <= GOLPE_ULTIMO_FRAMES + b[ANIM_GOLPE_3].frames);
    COMPROBAR(b[ANIM_RECIBIR].cuadros * b[ANIM_RECIBIR].frames <= RECIBIR_FRAMES + b[ANIM_RECIBIR].frames);
    /* Y cada tira de Bárbara es la suya, en el orden de barbara_p16_bancos. */
    for (k = 0; k < ANIM_PROTA_CANTIDAD; k++)
    {
        COMPROBAR(b[k].tira == k && b[k].cuadros >= 1 && t[k].cuadros >= 1);
    }

    /* Celdas y alcances. */
    COMPROBAR(personaje_ancho(PERSONAJE_TAROU) == 96 && personaje_alto(PERSONAJE_TAROU) == 64);
    COMPROBAR(personaje_ancho(PERSONAJE_BARBARA) == 112 && personaje_alto(PERSONAJE_BARBARA) == 96);
    COMPROBAR(personaje_alcance(PERSONAJE_TAROU) == COMBATE_ALCANCE);
    COMPROBAR(personaje_alcance(PERSONAJE_BARBARA) == ALCANCE_BARBARA && ALCANCE_BARBARA > COMBATE_ALCANCE);

    /* Avance: cambiar de animación empieza en el cuadro 0 sin contar ese frame. */
    memset(&e, 0, sizeof(e));
    e.anim = ANIM_GUARDIA;
    personaje_animar(&e, b, ANIM_GOLPE_1);
    COMPROBAR(e.anim == ANIM_GOLPE_1 && e.cuadro == 0 && e.reloj == 0);
    /* Sin bucle: tras recorrer todos los dibujos se queda en el último. */
    for (k = 0; k < 40; k++)
    {
        personaje_animar(&e, b, ANIM_GOLPE_1);
    }
    COMPROBAR(e.cuadro == b[ANIM_GOLPE_1].cuadros - 1);
    /* Con bucle: vuelve al 0 justo después del último. */
    personaje_animar(&e, b, ANIM_CAMINAR);
    for (p = 0; p < b[ANIM_CAMINAR].cuadros; p++)
    {
        for (k = 0; k < b[ANIM_CAMINAR].frames; k++)
        {
            personaje_animar(&e, b, ANIM_CAMINAR);
        }
    }
    COMPROBAR(e.cuadro == 0);
}
