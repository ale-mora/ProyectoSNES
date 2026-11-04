/*---------------------------------------------------------------------------------
    prueba_combate.c — Pruebas del combo de Tarou (mundo.c), la caja de golpe (combate.c), el ninja
    que recibe golpes y ataca (ninja.c) y Tarou que recibe daño (mundo_recibir).

    PROPÓSITO
      Fijar las reglas del primer combate (docs/diseno-beat-em-up.md §5.3): A empieza un golpe
      y, pulsada a tiempo, encadena el siguiente (3 golpes; el último derriba); la caja de golpe
      está activa solo unos cuadros y alcanza delante de Tarou con tolerancia de profundidad; cada
      tajo golpea una sola vez a cada ninja; el combo 1 + 1 + 2 mata a un ninja de 4 de vida; un
      ninja aturdido no camina y uno sin vida desaparece al caer.

    SE CONECTA CON
      Incluye:  minitest.h, juego/mundo.h, juego/combate.h, enemigos/ninja.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_combate).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Caja de golpe, Combo, Derribo, Parada de impacto
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/mundo.h"
#include "juego/combate.h"
#include "enemigos/ninja.h"

/* Un cuadro de juego: el mando solo con atacar_pulsado = a. */
static void cuadro(Cuerpo *c, u8 a)
{
    Mando m;

    memset(&m, 0, sizeof(m));
    m.atacar_pulsado = a;
    mundo_actualizar(c, &m);
}

void pruebas_combate(void)
{
    Cuerpo c;
    Ninja n[NINJAS_MAX];
    u8 k, parada, golpes_vistos, ultimo;

    /*--- El golpe: empieza con A, dura GOLPE_FRAMES y el combo se corta si no se encadena --*/
    mundo_iniciar(&c, 400, 200);
    cuadro(&c, 0);
    COMPROBAR(c.golpe == 0 && c.estado == CUERPO_PARADO);
    cuadro(&c, 1);                                   /* A: empieza el golpe 1.              */
    COMPROBAR(c.golpe == 1 && c.estado == CUERPO_ATACANDO && c.id == 1);
    COMPROBAR(c.reloj == 0 && !mundo_golpe_activo(&c));   /* Aún sin caja: arranque.        */
    for (k = 0; k < GOLPE_ACTIVO_DESDE; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(mundo_golpe_activo(&c));               /* Caja activa a partir del cuadro 4.  */
    for (k = GOLPE_ACTIVO_DESDE; k < GOLPE_ACTIVO_HASTA; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(!mundo_golpe_activo(&c));              /* Y se apaga a los 4 cuadros.         */
    for (k = 0; k < 20; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(c.golpe == 0 && c.estado == CUERPO_PARADO);   /* Sin encadenar: acaba el combo. */

    /*--- Quieto mientras golpea ---------------------------------------------------------*/
    mundo_iniciar(&c, 400, 200);
    {
        Mando m;
        memset(&m, 0, sizeof(m));
        m.atacar_pulsado = 1;
        mundo_actualizar(&c, &m);                    /* Empieza el golpe...                 */
        m.atacar_pulsado = 0;
        m.derecha = 1; m.abajo = 1; m.saltar_pulsado = 1;
        mundo_actualizar(&c, &m);                    /* ...y ni anda ni salta ni cambia de carril. */
        COMPROBAR(c.x == 400 && c.y == 200 && c.z == 0 && c.golpe == 1);
    }

    /*--- Combo: A a tiempo encadena (3 golpes) y el tercero acaba el combo ----------------*/
    mundo_iniciar(&c, 400, 200);
    cuadro(&c, 1);                                   /* Golpe 1.                            */
    for (k = 0; k < GOLPE_ENCADENAR_DESDE; k++)
    {
        cuadro(&c, 0);
    }
    cuadro(&c, 1);                                   /* A dentro de la ventana.             */
    COMPROBAR(c.cola == 1);
    ultimo = c.id;
    for (k = 0; k < 20 && c.golpe == 1; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(c.golpe == 2 && c.id != ultimo);       /* Pasó al golpe 2, con tajo nuevo.    */
    for (k = 0; k < GOLPE_ENCADENAR_DESDE + 1; k++)
    {
        cuadro(&c, 0);
    }
    cuadro(&c, 1);
    for (k = 0; k < 20 && c.golpe == 2; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(c.golpe == GOLPES);                    /* El tercero.                         */
    cuadro(&c, 0);
    for (k = 0; k < GOLPE_ENCADENAR_DESDE + 1; k++)
    {
        cuadro(&c, 1);                               /* No hay cuarto golpe: A no encadena. */
    }
    for (k = 0; k < GOLPE_ULTIMO_FRAMES + 2; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(c.golpe == 0 && c.cola == 0);

    /* A demasiado pronto (antes de GOLPE_ENCADENAR_DESDE) no cuenta. */
    mundo_iniciar(&c, 400, 200);
    cuadro(&c, 1);
    cuadro(&c, 1);                                   /* Cuadro 1 del golpe: pronto.         */
    COMPROBAR(c.cola == 0);

    /* En el aire no se golpea todavía. */
    mundo_iniciar(&c, 400, 200);
    {
        Mando m;
        memset(&m, 0, sizeof(m));
        m.saltar_pulsado = 1;
        mundo_actualizar(&c, &m);
        m.saltar_pulsado = 0; m.atacar_pulsado = 1;
        mundo_actualizar(&c, &m);
        COMPROBAR(c.golpe == 0 && c.z > 0);
    }

    /*--- Daño y caja de golpe -----------------------------------------------------------*/
    COMPROBAR(combate_danio(1) == 1 && combate_danio(2) == 1 && combate_danio(3) == 2);
    COMPROBAR(!combate_derriba(1) && !combate_derriba(2) && combate_derriba(3));
    mundo_iniciar(&c, 400, 200);                     /* Mirando a la derecha.               */
    COMPROBAR(combate_en_caja(&c, 440, 200));        /* Delante, a 40.                      */
    COMPROBAR(combate_en_caja(&c, 400 + COMBATE_ALCANCE, 200));        /* En el límite.     */
    COMPROBAR(!combate_en_caja(&c, 400 + COMBATE_ALCANCE + 1, 200));   /* Fuera por poco.   */
    COMPROBAR(!combate_en_caja(&c, 380, 200));       /* Detrás, a 20: no.                   */
    COMPROBAR(combate_en_caja(&c, 400 - COMBATE_ATRAS, 200));          /* Justo detrás.     */
    COMPROBAR(combate_en_caja(&c, 440, 200 + COMBATE_PROFUNDIDAD));    /* Profundidad.      */
    COMPROBAR(!combate_en_caja(&c, 440, 200 + COMBATE_PROFUNDIDAD + 1));
    COMPROBAR(!combate_en_caja(&c, 440, 200 - COMBATE_PROFUNDIDAD - 1));
    c.mira_izq = 1;                                  /* Mirando a la izquierda: al revés.   */
    COMPROBAR(combate_en_caja(&c, 360, 200));
    COMPROBAR(!combate_en_caja(&c, 440, 200));
    COMPROBAR(!combate_en_caja(&c, 400 - COMBATE_ALCANCE - 1, 200));

    /*--- Un ninja que recibe golpes -----------------------------------------------------*/
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1;
    COMPROBAR(n[0].vida == NINJA_VIDA);
    COMPROBAR(ninja_recibir(n, 1, 0, 0, 7) == 0);    /* Golpe normal: lo aturde.            */
    COMPROBAR(n[0].estado == NINJA_RECIBIR && n[0].vida == NINJA_VIDA - 1);
    COMPROBAR(n[0].empuje == NINJA_EMPUJE_DER && n[0].mira_izq == 1);   /* Lo empuja a la derecha. */
    COMPROBAR(ninja_recibir(n, 1, 0, 0, 7) == 0 && n[0].vida == NINJA_VIDA - 1);   /* Mismo tajo: nada. */
    {
        u16 x0 = n[0].x;
        for (k = 0; k < NINJA_RECIBIR_FRAMES; k++)
        {
            ninja_actualizar(n, 400, 200);           /* Aturdido: sin IA, solo retrocede.   */
        }
        COMPROBAR(n[0].x > x0 && n[0].x <= x0 + 4);           /* Retrocedió un poco.  */
        COMPROBAR(n[0].estado == NINJA_PARADO && n[0].vivo);       /* Y se recupera.       */
    }
    COMPROBAR(ninja_recibir(n, 1, 0, 1, 8) == 0 && n[0].empuje == NINJA_EMPUJE_IZQ);   /* Otro tajo. */
    COMPROBAR(ninja_recibir(n, 2, 1, 1, 9) == 1);    /* Un aturdido sí puede ser derribado. */
    COMPROBAR(n[0].estado == NINJA_DERRIBADO);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1;
    COMPROBAR(ninja_recibir(n, 1, 1, 0, 3) == 1);    /* Derribo: DERRIBADO.                 */
    COMPROBAR(n[0].estado == NINJA_DERRIBADO && n[0].vida == NINJA_VIDA - 1);
    COMPROBAR(ninja_recibir(n, 1, 0, 0, 4) == 0 && n[0].vida == NINJA_VIDA - 1);   /* Caído: no. */
    for (k = 0; k < NINJA_DERRIBO_FRAMES; k++)
    {
        ninja_actualizar(n, 400, 200);
    }
    COMPROBAR(n[0].vivo == 1 && n[0].estado == NINJA_PARADO);   /* Con vida: se levanta.   */

    /*--- El combo completo mata a un ninja (1 + 1 + 2 = NINJA_VIDA) ------------------------*/
    mundo_iniciar(&c, 400, 200);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1;
    parada = 0; golpes_vistos = 0;
    cuadro(&c, 1);                                   /* Golpe 1.                            */
    for (k = 0; k < 90; k++)                         /* Pulsa A cada 10 cuadros (encadena).  */
    {
        u8 p = combate_resolver(&c, n, COMBATE_ALCANCE);
        if (p)
        {
            golpes_vistos++;
            parada = p;
        }
        cuadro(&c, (u8)(k % 10 == 7));
        if (n[0].estado != NINJA_DERRIBADO && n[0].estado != NINJA_RECIBIR)
        {
            n[0].x = 440;                            /* Se queda en el sitio.               */
        }
    }
    COMPROBAR(golpes_vistos == 3);                   /* Tres golpes conectaron, una vez cada uno. */
    COMPROBAR(parada == PARADA_DERRIBO);             /* El último, con parada larga.        */
    COMPROBAR(n[0].vida == 0);
    for (k = 0; k < NINJA_DERRIBO_FRAMES + 2; k++)
    {
        ninja_actualizar(n, 400, 200);
    }
    COMPROBAR(n[0].vivo == 0);                       /* Sin vida: desaparece al caer.       */
    COMPROBAR(ninja_vivos(n) == 0);

    /* Un ninja fuera de la caja no recibe nada. */
    mundo_iniciar(&c, 400, 200);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 600; n[0].y = 200; n[0].dentro = 1;
    cuadro(&c, 1);
    for (k = 0; k < GOLPE_ACTIVO_DESDE + 1; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(combate_resolver(&c, n, COMBATE_ALCANCE) == 0 && n[0].vida == NINJA_VIDA);

    /*--- Tarou recibe daño: vida, aturdido, invulnerable, KO ------------------------------*/
    mundo_iniciar(&c, 400, 200);
    COMPROBAR(c.vida == VIDA_TAROU && c.inv == 0 && c.stun == 0);
    COMPROBAR(mundo_recibir(&c, 2, 1) == 1);         /* Un golpe que lo empuja a la izquierda. */
    COMPROBAR(c.vida == VIDA_TAROU - 2 && c.estado == CUERPO_RECIBIR);
    COMPROBAR(c.inv == INVULNERABLE_FRAMES && c.stun == RECIBIR_FRAMES && c.empuje == 1);
    COMPROBAR(mundo_recibir(&c, 2, 1) == 0 && c.vida == VIDA_TAROU - 2);   /* Invulnerable. */
    {
        u16 x0 = c.x;
        for (k = 0; k < RECIBIR_FRAMES; k++)
        {
            cuadro(&c, 1);                           /* Aturdido: A no hace nada.           */
        }
        COMPROBAR(c.x < x0 && c.x >= x0 - 2 * 6);    /* Retrocedió hacia donde lo empujaron. */
        COMPROBAR(c.golpe == 0 && c.estado == CUERPO_PARADO && c.stun == 0);
    }
    COMPROBAR(c.inv == INVULNERABLE_FRAMES - RECIBIR_FRAMES);   /* Sigue parpadeando.      */
    for (k = 0; k < INVULNERABLE_FRAMES; k++)
    {
        cuadro(&c, 0);
    }
    COMPROBAR(c.inv == 0 && mundo_recibir(&c, 2, 0) == 1);       /* Ya puede recibir otro.  */
    COMPROBAR(c.empuje == 2);
    /* Un golpe corta el que Tarou estuviera dando. */
    mundo_iniciar(&c, 400, 200);
    cuadro(&c, 1);
    COMPROBAR(c.golpe == 1);
    mundo_recibir(&c, 2, 1);
    COMPROBAR(c.golpe == 0 && c.estado == CUERPO_RECIBIR);
    /* KO: sin vida cae y se queda en el suelo, sin hacer caso del mando. */
    mundo_iniciar(&c, 400, 200);
    c.vida = 2;
    COMPROBAR(mundo_recibir(&c, 2, 0) == 1 && c.vida == 0);
    COMPROBAR(c.estado == CUERPO_DERRIBADO && c.stun == DERRIBO_FRAMES);
    for (k = 0; k < 200; k++)
    {
        cuadro(&c, 1);
    }
    COMPROBAR(c.estado == CUERPO_DERRIBADO && c.vida == 0 && c.golpe == 0);
    COMPROBAR(mundo_recibir(&c, 2, 0) == 0);         /* KO: ya no recibe más.               */
    /* La vida no baja de 0 con un golpe de más. */
    mundo_iniciar(&c, 400, 200);
    mundo_recibir(&c, 200, 0);
    COMPROBAR(c.vida == 0);

    /*--- El ninja ataca: turno, cadencia, tajo y caja -------------------------------------*/
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1; n[0].estado = NINJA_PARADO;
    COMPROBAR(n[0].espera == NINJA_ESPERA_INICIAL);              /* Al nacer, espera.       */
    COMPROBAR(ninja_intentar_atacar(n, 400, 200, 1) == 0);       /* Descansando: no.        */
    n[0].espera = 0;
    COMPROBAR(ninja_intentar_atacar(n, 400, 200, 0) == 0);       /* Sin turno: no.          */
    COMPROBAR(ninja_intentar_atacar(n, 100, 200, 1) == 0);       /* Lejos: no.              */
    COMPROBAR(ninja_intentar_atacar(n, 400, 230, 1) == 0);       /* Otro carril: no.        */
    COMPROBAR(ninja_intentar_atacar(n, 400, 200, 1) == 1);       /* Cerca, con turno.       */
    COMPROBAR(n[0].estado == NINJA_ATACANDO && ninja_dibujo(n) == NINJA_DIB_ATAQUE);
    COMPROBAR(ninja_atacando(n) == 1 && !ninja_golpe_activo(n)); /* Aún sin caja: arranque. */
    /* El puñetazo: los 3 dibujos de ataque, caja activa de GOLPE_DESDE a GOLPE_HASTA. */
    {
        u8 activos = 0, cuadro_max = 0;
        for (k = 0; k < NINJA_ATAQUE_FRAMES - 1; k++)
        {
            ninja_actualizar(n, 400, 200);
            if (ninja_golpe_activo(n))
            {
                activos++;
            }
            if (ninja_dibujo(n) > cuadro_max)
            {
                cuadro_max = ninja_dibujo(n);
            }
        }
        COMPROBAR(activos == NINJA_GOLPE_HASTA - NINJA_GOLPE_DESDE);
        COMPROBAR(cuadro_max == NINJA_DIB_ATAQUE + 2);   /* Hasta el puño fuera.           */
    }
    ninja_actualizar(n, 400, 200);                   /* El último cuadro del tajo.          */
    COMPROBAR(n[0].estado == NINJA_PARADO && n[0].espera == NINJA_ESPERA && n[0].cuadro == 0);
    COMPROBAR(ninja_intentar_atacar(n, 400, 200, 1) == 0);       /* Descansa tras el tajo.  */
    /* Los turnos: con dos ya atacando, el tercero espera. */
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 2);
    for (k = 0; k < NINJAS_MAX; k++)
    {
        n[k].x = 440; n[k].y = 200; n[k].dentro = 1; n[k].espera = 0; n[k].estado = NINJA_PARADO;
    }
    COMPROBAR(ninja_atacando(n) == 0);
    for (k = 0; k < NINJAS_MAX; k++)
    {
        ninja_intentar_atacar(n + k, 400, 200, (u8)(ninja_atacando(n) < NINJA_TURNOS));
    }
    COMPROBAR(ninja_atacando(n) == NINJA_TURNOS);    /* Solo dos a la vez.                  */
    /* Un golpe recibido cancela el tajo. */
    ninja_recibir(n, 1, 0, 0, 5);
    COMPROBAR(n[0].estado == NINJA_RECIBIR && ninja_atacando(n) == NINJA_TURNOS - 1);

    /*--- El tajo del ninja alcanza a Tarou (una sola vez) ----------------------------------*/
    mundo_iniciar(&c, 400, 200);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1; n[0].espera = 0; n[0].estado = NINJA_PARADO;
    n[0].mira_izq = 1;                               /* Mira a Tarou (a su izquierda).      */
    ninja_intentar_atacar(n, 400, 200, 1);
    golpes_vistos = 0;
    parada = 0;
    for (k = 0; k < NINJA_ATAQUE_FRAMES + 2; k++)
    {
        u8 p = combate_ninjas_resolver(n, &c);
        if (p)
        {
            golpes_vistos++;
            parada = p;
        }
        ninja_actualizar(n, 400, 200);
        cuadro(&c, 0);
    }
    COMPROBAR(golpes_vistos == 1 && parada == PARADA_GOLPE);     /* Un tajo, un golpe.      */
    COMPROBAR(c.vida == VIDA_TAROU - NINJA_DANIO && c.empuje == 1);   /* Empujado a la izquierda. */
    /* Fuera de alcance no acierta. */
    mundo_iniciar(&c, 100, 200);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1; n[0].espera = 0; n[0].estado = NINJA_PARADO;
    n[0].mira_izq = 1;
    ninja_intentar_atacar(n, 440, 200, 1);           /* (Ataca al aire: Tarou está lejos.)  */
    for (k = 0; k < NINJA_ATAQUE_FRAMES; k++)
    {
        combate_ninjas_resolver(n, &c);
        ninja_actualizar(n, 440, 200);
    }
    COMPROBAR(c.vida == VIDA_TAROU);
    /* Un ninja que ataca de espaldas (mirando al otro lado) tampoco acierta. */
    mundo_iniciar(&c, 400, 200);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1; n[0].espera = 0; n[0].estado = NINJA_PARADO;
    ninja_intentar_atacar(n, 400, 200, 1);
    n[0].mira_izq = 0;                               /* Mira a la derecha, Tarou queda detrás. */
    for (k = 0; k < NINJA_ATAQUE_FRAMES; k++)
    {
        combate_ninjas_resolver(n, &c);
        n[0].timer++;
    }
    COMPROBAR(c.vida == VIDA_TAROU);
    /* Un golpe que le da a Tarou invulnerable se pierde sin repetirse. */
    mundo_iniciar(&c, 400, 200);
    c.inv = 30;
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 440; n[0].y = 200; n[0].dentro = 1; n[0].espera = 0; n[0].estado = NINJA_PARADO;
    n[0].mira_izq = 1;
    ninja_intentar_atacar(n, 400, 200, 1);
    for (k = 0; k < NINJA_GOLPE_DESDE; k++)
    {
        ninja_actualizar(n, 400, 200);
    }
    combate_ninjas_resolver(n, &c);                  /* Pega, pero está invulnerable.       */
    COMPROBAR(c.vida == VIDA_TAROU && n[0].conecto == 1);
    c.inv = 0;
    combate_ninjas_resolver(n, &c);                  /* El mismo tajo no pega otra vez.     */
    COMPROBAR(c.vida == VIDA_TAROU);

    /* El combo completo conecta contra un ninja que camina: el retroceso no lo saca del alcance. */
    mundo_iniciar(&c, 400, 200);
    ninja_iniciar_todos(n);
    ninja_oleada(n, 300, 0);
    n[0].x = 400 + NINJA_DISTANCIA; n[0].y = 200; n[0].dentro = 1; n[0].espera = 250;
    n[0].estado = NINJA_PARADO; n[0].mira_izq = 1;
    {
        u8 golpes = 0, antes = 0;
        u16 t;
        for (t = 0; t < 120; t++)
        {
            cuadro(&c, (u8)(t == 0 || (c.estado == CUERPO_ATACANDO && c.reloj == GOLPE_ENCADENAR_DESDE)));
            if (combate_resolver(&c, n, COMBATE_ALCANCE) && c.id != antes)
            {
                golpes++;
            }
            antes = c.id;
            ninja_actualizar(n, c.x, c.y);
        }
        COMPROBAR(n[0].vida == 0);                   /* 1 + 1 + 2 = 4: los tres golpes conectaron. */
        (void)golpes;
    }
}
