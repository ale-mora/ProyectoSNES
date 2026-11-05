/*---------------------------------------------------------------------------------
    mundo.h — El cuerpo de un personaje en el carril del beat 'em up: x, y (profundidad)
    y z (altura), y cómo se mueve con el mando.

    PROPÓSITO
      Un beat 'em up de carril tiene tres ejes (docs/diseno-beat-em-up.md §3):
      x avanza por el escenario, y es la PROFUNDIDAD (la línea de pantalla de los pies
      sobre el suelo: más abajo = más cerca de la cámara) y z es la ALTURA del salto
      sobre el suelo. La cruceta mueve x e y; B salta; A da el combo. Mientras está en
      el aire no se cambia de dirección en x (el impulso se queda) pero sí se puede mover
      en y, como en Final Fight. La sombra se dibuja en (x, y) y el cuerpo en (x, y − z).
      Es lógica pura, sin snes.h: se prueba en el PC (tests/prueba_mundo.c y
      tests/prueba_movimientos.c).
      Movimientos (2026-11-05, §2 y §5 de la spec):
        - CORRER: doble toque a izquierda o derecha (la misma dirección otra vez antes de
          VENTANA_DOBLE_TOQUE frames); corre hasta que suelta.
        - ATAQUE AÉREO: A en el aire, una vez por salto; derriba.
        - AGARRE: andando contra un enemigo (lo decide combate.c, que ve a los ninjas): A =
          golpe de agarre (hasta 3), atrás + A = lanzamiento.
        - ESPECIAL: A + B en el suelo; invulnerable, golpea a los dos lados y cuesta vida
          solo si conecta (sin bajar de 1).
        - LANZAR ARMA: A con un arma en la mano (objetos.c se la da).
        - DERRIBADO con vida → LEVANTARSE → invulnerable INVULNERABLE_FRAMES (parpadea).
        - AGARRADO: un enemigo lo tiene (lo maneja combate.c).
      Lo que el cuerpo tiene que decir a los demás (soltar un shuriken, el impacto de un
      golpe de agarre, lanzar al agarrado) va en `evento`, que vale para UN frame.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mando.h.
      Lo usan:  juego/combate.c, juego/personaje.c, pantallas/pantalla_fase1.c y las pruebas.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Cuerpo; hay uno solo, no hace falta que mida una potencia de 2)
      x, y      u16   pies: x en el mundo; y, línea de pantalla del suelo (CARRIL_Y_MIN..MAX)
      z, vz     s16   altura y velocidad vertical en 12.4 (16 = 1 px): z = 0 en el suelo
      mira_izq  u8    1 si mira a la izquierda
      estado    u8    CUERPO_*
      golpe, reloj, cola, id   u8   el combo: golpe en curso, cuadros en él, A ya pulsada, tajo
      vida, inv, stun, empuje  u8   puntos de vida, cuadros invulnerable, aturdido y hacia dónde lo empujaron
      golpes_max u8   golpes del combo de este personaje (3 Bárbara, 4 Tarou)
      corre, dir, suelta, antes   u8   el doble toque: corriendo, última dirección (1 izq, 2 der),
                                       frames desde que se soltó, la del frame anterior
      aereo     u8    0 = no ha atacado en este salto; 1 = atacando; 2 = ya atacó
      agarra    u8    0 = nadie; k + 1 = ranura del ninja agarrado (combate.c)
      agarre_golpes, empuja, accion  u8   golpes de agarre dados, frames empujando, reloj de la acción
      cobro     u8    1 = el especial ya cobró su coste
      arma      u8    ARMA_NINGUNA / ARMA_SHURIKEN / ARMA_KUNAI
      evento    u8    EVENTO_* de este frame
    → glosario: Carril, Profundidad, Altura (z), Punto fijo
---------------------------------------------------------------------------------*/
#ifndef MUNDO_H
#define MUNDO_H

#include "sistema/tipos.h"
#include "juego/mando.h"

/*--- El carril: entre estas dos líneas de pantalla pueden estar los pies --------------
    El suelo del fondo cercano empieza en la 176 (SUELO_Y, nivel.h) y la pantalla acaba en
    la 224: 176 + 4 .. 224 − 16 deja sitio para que el cuerpo entre entero sin cruzarse
    con la sombra de la fila de abajo. */
#define CARRIL_Y_MIN 184
#define CARRIL_Y_MAX 216
#define MUNDO_X_MIN 24                   /* Límites de x (el centro del cuerpo) en TODO el */
#define MUNDO_X_MAX 2024                 /* nivel (ANCHO_NIVEL 2048 − 24). La cámara los   */
#define MARGEN_PANTALLA 24               /* estrecha con mundo_limitar_x: ni un paso más   */
                                         /* allá de este margen del borde de la pantalla.  */

/*--- Movimiento (px por frame; las cifras se ajustan jugando, §12) ------------------*/
#define PASO_X 2                         /* Andar en x.                                   */
#define PASO_CORRER 3                    /* Correr en x (doble toque).                    */
#define PASO_Y 1                         /* Andar por el carril: la profundidad es más lenta. */
#define VENTANA_DOBLE_TOQUE 12           /* Frames entre soltar y volver a pulsar.        */
#define SALTO_V0 96                      /* 6 px/frame hacia arriba (12.4).               */
#define GRAVEDAD 8                       /* 0,5 px/frame²: salto de 36 px y 24 frames.    */

/*--- Vida de Tarou y lo que pasa al recibir un golpe (cifras a ajustar jugando) ---------*/
#define VIDA_TAROU 24                    /* 12 celdas de barra de 2 puntos cada una.       */
#define RECIBIR_FRAMES 16                /* Aturdido tras un golpe normal.                 */
#define DERRIBO_FRAMES 36                /* Caído (y, sin vida, KO).                       */
#define LEVANTARSE_FRAMES 20             /* Levantándose tras un derribo con vida.         */
#define INVULNERABLE_FRAMES 60           /* Tras un golpe o al levantarse: parpadea.       */

/*--- El combo (A, A, A[, A]: el último derriba) ------------------------------------*/
#define GOLPES 3                         /* Golpes del combo de Bárbara (Tarou: 4).        */
#define GOLPES_TAROU 4
#define GOLPE_FRAMES 14                  /* Duración de los golpes que no son el último.  */
#define GOLPE_ULTIMO_FRAMES 20           /* El último tarda más en recuperarse.           */
#define GOLPE_ACTIVO_DESDE 4             /* La caja de golpe, activa de aquí...           */
#define GOLPE_ACTIVO_HASTA 8             /* ...hasta aquí (sin incluir): 4 frames.        */
#define GOLPE_ENCADENAR_DESDE 5          /* A a partir de este cuadro encadena el siguiente. */

/*--- Los otros movimientos ------------------------------------------------------------*/
#define AEREO_ACTIVO_DESDE 2             /* El ataque aéreo golpea desde aquí hasta tocar suelo. */
#define ESPECIAL_FRAMES 28               /* A + B: el giro dura esto...                   */
#define ESPECIAL_ACTIVO_DESDE 4          /* ...y golpea a los dos lados de aquí...        */
#define ESPECIAL_ACTIVO_HASTA 22         /* ...hasta aquí.                                */
#define COSTE_ESPECIAL 2                 /* Vida que cuesta si conecta (sin bajar de 1).  */
#define LANZAR_ARMA_FRAMES 16
#define LANZAR_ARMA_SUELTA 6             /* En este frame sale el arma (EVENTO_LANZA_ARMA). */
#define AGARRE_EMPUJE 10                 /* Frames andando contra un enemigo para agarrarlo. */
#define AGARRE_MAX 120                   /* Si no hace nada, el agarrado se suelta.       */
#define AGARRE_GOLPES 3                  /* Golpes de agarre como mucho.                  */
#define GOLPE_AGARRE_FRAMES 14
#define GOLPE_AGARRE_IMPACTO 6           /* En este frame pega (EVENTO_GOLPE_AGARRE).     */
#define LANZAR_FRAMES 21
#define LANZAR_SUELTA 9                  /* En este frame sale despedido (EVENTO_LANZA_AGARRADO). */

typedef enum
{
    CUERPO_PARADO = 0,
    CUERPO_ANDANDO,
    CUERPO_SUBIENDO,                     /* En el aire, vz < 0.                           */
    CUERPO_BAJANDO,                      /* En el aire, vz >= 0.                          */
    CUERPO_ATACANDO,                     /* Dando un golpe del combo.                     */
    CUERPO_RECIBIR,                      /* Aturdido: retrocede un poco y no puede actuar.*/
    CUERPO_DERRIBADO,                    /* En el suelo (sin vida: KO, se queda).         */
    CUERPO_CORRIENDO,                    /* Desde aquí, 2026-11-05.                       */
    CUERPO_AEREO,                        /* Ataque aéreo.                                 */
    CUERPO_AGARRANDO,
    CUERPO_GOLPE_AGARRE,
    CUERPO_LANZANDO,                     /* Lanzando al agarrado.                         */
    CUERPO_ESPECIAL,
    CUERPO_LANZAR_ARMA,
    CUERPO_LEVANTARSE,
    CUERPO_AGARRADO,                     /* Lo tiene un enemigo (combate.c).              */
    CUERPO_VICTORIA,
    CUERPO_ESTADOS
} EstadoCuerpo;

typedef enum
{
    ARMA_NINGUNA = 0,
    ARMA_SHURIKEN,
    ARMA_KUNAI
} Arma;

typedef enum
{
    EVENTO_NINGUNO = 0,
    EVENTO_LANZA_ARMA,                   /* Sale el arma que llevaba.                     */
    EVENTO_GOLPE_AGARRE,                 /* El golpe de agarre pega.                      */
    EVENTO_LANZA_AGARRADO,               /* El agarrado sale despedido.                   */
    EVENTO_SUELTA_AGARRADO               /* Lo suelta (tras 3 golpes, por tiempo o al recibir). */
} Evento;

typedef struct
{
    u16 x;
    u16 y;
    s16 z;
    s16 vz;
    u8 mira_izq;
    u8 estado;
    u8 golpe;                            /* 0 = sin golpe; 1..golpes_max = el del combo.  */
    u8 reloj;                            /* Cuadros que lleva en ese golpe.               */
    u8 cola;                             /* 1 = ya pulsó A a tiempo: encadena al acabar.  */
    u8 id;                               /* Identificador del tajo (1..254, cambia en cada golpe). */
    u8 vida;                             /* Puntos de vida (VIDA_TAROU al empezar; 0 = KO). */
    u8 inv;                              /* Cuadros de invulnerabilidad que quedan (parpadeo). */
    u8 stun;                             /* Cuadros que le quedan aturdido o caído.       */
    u8 empuje;                           /* 1 = el golpe lo empujó a la izquierda, 2 = a la derecha. */
    u8 golpes_max;                       /* 3 o 4: lo fija la pantalla según el personaje. */
    u8 corre;                            /* 1 = corriendo.                                */
    u8 dir;                              /* Última dirección pulsada: 1 izq, 2 der (0 ninguna). */
    u8 suelta;                           /* Frames desde que se soltó (tope 255).         */
    u8 antes;                            /* Dirección mantenida el frame anterior.        */
    u8 aereo;                            /* 0 / 1 atacando / 2 ya atacó en este salto.    */
    u8 agarra;                           /* 0 = nadie; k + 1 = ranura del agarrado.       */
    u8 agarre_golpes;
    u8 empuja;
    u8 accion;                           /* Reloj de la acción en curso.                  */
    u8 cobro;                            /* El especial ya cobró su coste.                */
    u8 arma;
    u8 evento;                           /* EVENTO_* de este frame (solo dura uno).       */
    u8 preso;                            /* k + 1 = la ranura del enemigo que lo tiene agarrado. */
} Cuerpo;

/* Pone el cuerpo en el suelo, quieto, mirando a la derecha (combo de GOLPES golpes). */
void mundo_iniciar(Cuerpo *c, u16 x, u16 y);

/* Un frame: mueve según el mando, salta, ataca, aplica la gravedad y recorta a los límites. */
void mundo_actualizar(Cuerpo *c, const Mando *m);

/* Recorta x a [x_min, x_max]: la cámara pone ahí los bordes de la pantalla (cámara +
   MARGEN_PANTALLA y cámara + 256 − MARGEN_PANTALLA), para que el protagonista no salga de
   ella ni vuelva atrás de lo ya recorrido. Después de mundo_actualizar. */
void mundo_limitar_x(Cuerpo *c, u16 x_min, u16 x_max);

/* El cuerpo recibe un golpe de `danio` puntos que viene de la izquierda si `hacia_izq` es 0
   (lo empuja a la derecha) o de la derecha si es 1. Devuelve 1 si el golpe contó; 0 si estaba
   invulnerable (también durante el especial o levantándose), caído o sin vida. Corta lo que
   estuviera haciendo (si agarraba, EVENTO_SUELTA_AGARRADO); pierde la vida y queda aturdido
   (RECIBIR) o, sin vida, caído (DERRIBADO); empieza a parpadear (INVULNERABLE_FRAMES). */
u8 mundo_recibir(Cuerpo *c, u8 danio, u8 hacia_izq);

/* Igual, pero lo DERRIBA aunque le quede vida (un golpe fuerte o un lanzamiento): cae,
   se levanta (LEVANTARSE_FRAMES) y queda invulnerable INVULNERABLE_FRAMES. */
u8 mundo_derribar(Cuerpo *c, u8 danio, u8 hacia_izq);

/* 1 si el cuerpo está atacando y su caja de golpe está activa en este cuadro (combo,
   ataque aéreo o especial). */
u8 mundo_golpe_activo(const Cuerpo *c);

/* Lo que quita ese golpe, si derriba y si golpea a los dos lados (el especial). */
u8 mundo_golpe_danio(const Cuerpo *c);
u8 mundo_golpe_derriba(const Cuerpo *c);
u8 mundo_golpe_alrededor(const Cuerpo *c);

/* El especial conectó: cobra COSTE_ESPECIAL de vida una sola vez (sin bajar de 1). */
void mundo_cobrar_especial(Cuerpo *c);

/* La altura en píxeles (z / 16, 0 en el suelo). */
u16 mundo_altura(const Cuerpo *c);

#endif /* MUNDO_H */
