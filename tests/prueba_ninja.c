/*---------------------------------------------------------------------------------
    prueba_ninja.c — Pruebas de enemigos/ninja.c.

    PROPÓSITO
      Fijar cómo se comporta el ninja de prueba (docs/diseno-beat-em-up.md §5.5
      y §6): vida y daño por rango; se acerca al objetivo y se para a tiro; ataca
      solo con permiso, en rango y fuera de enfriamiento; su shinai pega en sus
      frames activos y le quita vida a Tarou con las reglas de combate; el
      enfriamiento separa dos ataques COOLDOWN_NINJA frames; y las reacciones
      (recibir, derribo, KO) siguen el mismo patrón que Tarou.

    SE CONECTA CON
      Incluye:  minitest.h, enemigos/ninja.h, juego/combate.h, juego/cifras.h,
                juego/mundo.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_ninja).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Rango, Turnos de ataque, Caja de golpe, Derribo
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "enemigos/ninja.h"
#include "juego/combate.h"
#include "juego/cifras.h"
#include "juego/mundo.h"

/* `n` frames del ninja contra el objetivo; `permiso` fijo. */
static void correr(Ninja *n, const Cuerpo *obj, u8 permiso, const Carril *c, u8 frames)
{
    while (frames--)
        ninja_actualizar(n, obj, permiso, c);
}

void pruebas_ninja(void)
{
    Ninja n;
    Cuerpo tarou;
    Carril c;
    CajaCuadro caja;
    CajaCuadro dano_tarou;
    CajaMundo golpe;
    CajaMundo cuerpo_tarou;
    Celda celda_ninja;
    Celda celda_tarou;
    Golpeable golpeable_tarou;
    u8 golpeados;
    u16 x_ini;
    u8 n_frames;                     /* Cuadros contados en un bucle.                       */

    c.x_min_fp = PF(16);
    c.x_max_fp = PF(240);
    c.y_arriba_fp = PF(150);
    c.y_abajo_fp = PF(208);

    celda_ninja.ancho = NINJA_ANCHO;
    celda_ninja.mitad_ancho = NINJA_MITAD;
    celda_ninja.alto = NINJA_ALTO;
    celda_tarou.ancho = 96;
    celda_tarou.mitad_ancho = 48;
    celda_tarou.alto = 64;

    COMPROBAR(sizeof(Ninja) == 32);

    /* Vida y daño por rango: blanco 16, rojo 48; el rojo pega 3 más. */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    COMPROBAR(n.g.vida == 16 && n.estado == NQ_QUIETO);
    ninja_iniciar(&n, 100, 180, RANGO_ROJO);
    COMPROBAR(n.g.vida == 48);

    /* Se acerca: lejos de Tarou se mueve hacia él, mirándole. */
    mundo_colocar(&tarou, 200, 180);
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    x_ini = n.c.x_fp;
    ninja_actualizar(&n, &tarou, 0, &c);
    COMPROBAR(n.estado == NQ_ACERCARSE && n.c.vx_fp == VEL_NINJA && n.mira_izquierda == 0);
    correr(&n, &tarou, 0, &c, 19);
    COMPROBAR(n.c.x_fp == x_ini + 20 * VEL_NINJA);

    /* A tiro sin permiso: se para de pie y no ataca nunca. */
    mundo_colocar(&tarou, 120, 180);
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    correr(&n, &tarou, 0, &c, 200);
    COMPROBAR(n.estado == NQ_QUIETO && n.tiempo > 0);
    COMPROBAR(n.c.vx_fp == 0);

    /* A tiro CON permiso: ataca, y la caja de golpe sale en sus frames activos (8 a 11). */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    ninja_actualizar(&n, &tarou, 1, &c);
    COMPROBAR(n.estado == NQ_ATACAR && n.espera == COOLDOWN_NINJA);
    COMPROBAR(ninja_caja_golpe(&n, &caja) == 0);     /* Tiempo 0: aún no pega.              */
    correr(&n, &tarou, 1, &c, 8);
    COMPROBAR(n.tiempo == 8 && ninja_caja_golpe(&n, &caja) == 1);
    COMPROBAR(caja.dano == DANO_SHINAI && caja.efecto == EFECTO_RETROCESO);
    correr(&n, &tarou, 1, &c, 4);
    COMPROBAR(ninja_caja_golpe(&n, &caja) == 0);     /* Tiempo 12: ya no.                   */

    /* El rojo pega DANO_SHINAI + 3. */
    ninja_iniciar(&n, 100, 180, RANGO_ROJO);
    ninja_actualizar(&n, &tarou, 1, &c);
    correr(&n, &tarou, 1, &c, 8);
    COMPROBAR(ninja_caja_golpe(&n, &caja) == 1 && caja.dano == DANO_SHINAI + 3);

    /* El shinai conecta con Tarou (cajas de celda, regla de golpe): le quita 4. */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    mundo_colocar(&tarou, 124, 180);
    ninja_actualizar(&n, &tarou, 1, &c);
    correr(&n, &tarou, 1, &c, 8);
    ninja_caja_golpe(&n, &caja);
    combate_caja_mundo(&caja, &celda_ninja, &n.c, n.mira_izquierda, &golpe);
    dano_tarou.x = 10;
    dano_tarou.y = 8;
    dano_tarou.ancho = 40;
    dano_tarou.alto = 52;
    dano_tarou.dano = 0;
    dano_tarou.efecto = EFECTO_RETROCESO;
    dano_tarou.relleno[0] = 0;
    dano_tarou.relleno[1] = 0;
    combate_caja_mundo(&dano_tarou, &celda_tarou, &tarou, 0, &cuerpo_tarou);
    golpeable_tarou.vida = VIDA_PROTAGONISTA;
    golpeable_tarou.invulnerable = 0;
    golpeable_tarou.destello = 0;
    golpeable_tarou.parada = 0;
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &caja, &n.c, &n.g, &cuerpo_tarou, &tarou,
                              &golpeable_tarou, &golpeados, 1) == GOLPE_RETROCESO);
    COMPROBAR(golpeable_tarou.vida == VIDA_PROTAGONISTA - DANO_SHINAI);

    /* El shinai mirando a la IZQUIERDA (como ataca en el juego: Tarou a su izquierda)
       también conecta: la caja se espeja dentro de la celda sin salirse de ella. */
    ninja_iniciar(&n, 124, 180, RANGO_BLANCO);
    mundo_colocar(&tarou, 100, 180);
    ninja_actualizar(&n, &tarou, 1, &c);
    COMPROBAR(n.mira_izquierda == 1);
    correr(&n, &tarou, 1, &c, 8);
    COMPROBAR(ninja_caja_golpe(&n, &caja) == 1);
    combate_caja_mundo(&caja, &celda_ninja, &n.c, n.mira_izquierda, &golpe);
    combate_caja_mundo(&dano_tarou, &celda_tarou, &tarou, 0, &cuerpo_tarou);
    COMPROBAR(golpe.x >= 124 - NINJA_MITAD - 1 && golpe.x + golpe.ancho <= 124 + NINJA_MITAD + 1);
    golpeable_tarou.vida = VIDA_PROTAGONISTA;
    golpeable_tarou.invulnerable = 0;
    golpeados = 0;
    COMPROBAR(combate_golpear(&golpe, &caja, &n.c, &n.g, &cuerpo_tarou, &tarou,
                              &golpeable_tarou, &golpeados, 1) == GOLPE_RETROCESO);

    /* Enfriamiento: dos ataques empiezan COOLDOWN_NINJA frames separados. */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    mundo_colocar(&tarou, 120, 180);
    ninja_actualizar(&n, &tarou, 1, &c);             /* Primer ataque: empieza aquí.        */
    n_frames = 0;
    do
    {
        ninja_actualizar(&n, &tarou, 1, &c);
        n_frames++;
    } while (!(n.estado == NQ_ATACAR && n.tiempo == 0) && n_frames < 200);
    COMPROBAR(n_frames == COOLDOWN_NINJA);           /* El siguiente ataque empieza en 60.  */

    /* Su caja de daño es el cuerpo entero y no pega. */
    ninja_caja_dano(&n, &dano_tarou);
    COMPROBAR(dano_tarou.ancho == 32 && dano_tarou.alto == 88 && dano_tarou.dano == 0);

    /* Parada de impacto: no se mueve mientras dura. */
    ninja_iniciar(&n, 100, 200, RANGO_BLANCO);
    mundo_colocar(&tarou, 200, 180);
    n.g.parada = FRAMES_PARADA;
    x_ini = n.c.x_fp;
    correr(&n, &tarou, 0, &c, FRAMES_PARADA);
    COMPROBAR(n.c.x_fp == x_ini);

    /* Recibir: retrocede 12 frames y vuelve a quieto. */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    ninja_reaccionar(&n, GOLPE_RETROCESO);
    COMPROBAR(n.estado == NQ_RECIBIR);
    correr(&n, &tarou, 0, &c, 12);
    COMPROBAR(n.estado == NQ_ACERCARSE);             /* Tarou está lejos: vuelve a acercarse. */

    /* Derribo: cae, aterriza, está en el suelo, se levanta invulnerable y vuelve a acercarse. */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    ninja_reaccionar(&n, GOLPE_DERRIBO);
    COMPROBAR(n.estado == NQ_DERRIBADO && n.c.en_suelo == 0);
    n_frames = 0;
    while (n.estado == NQ_DERRIBADO && n_frames < 120)
    {
        ninja_actualizar(&n, &tarou, 0, &c);
        n_frames++;
    }
    COMPROBAR(n.estado == NQ_EN_SUELO);
    correr(&n, &tarou, 0, &c, 30);
    COMPROBAR(n.estado == NQ_LEVANTARSE && n.g.invulnerable == FRAMES_INVULNERABLE);
    correr(&n, &tarou, 0, &c, 36);
    COMPROBAR(n.estado == NQ_ACERCARSE);             /* Tarou está lejos: vuelve a acercarse. */

    /* KO: vencido al acabar su tiempo, no antes. */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    ninja_reaccionar(&n, GOLPE_VENCE);
    COMPROBAR(n.estado == NQ_KO && n.vencido == 0);
    correr(&n, &tarou, 0, &c, 39);
    COMPROBAR(n.vencido == 0);
    correr(&n, &tarou, 0, &c, 1);
    COMPROBAR(n.vencido == 1);

    /* Un golpe que no pega (FALLA) no cambia nada. */
    ninja_iniciar(&n, 100, 180, RANGO_BLANCO);
    ninja_reaccionar(&n, GOLPE_FALLA);
    COMPROBAR(n.estado == NQ_QUIETO);
}
