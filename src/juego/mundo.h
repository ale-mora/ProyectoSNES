/*---------------------------------------------------------------------------------
    mundo.h — El cuerpo de un personaje en el carril del beat 'em up: x, y (profundidad)
    y z (altura), y cómo se mueve con el mando.

    PROPÓSITO
      Un beat 'em up de carril tiene tres ejes (docs/diseno-beat-em-up.md §3):
      x avanza por el escenario, y es la PROFUNDIDAD (la línea de pantalla de los pies
      sobre el suelo: más abajo = más cerca de la cámara) y z es la ALTURA del salto
      sobre el suelo. La cruceta mueve x e y; B salta; A da el combo de tres golpes. Mientras está en el aire no se
      cambia de dirección en x (el impulso se queda) pero sí se puede mover en y, como
      en Final Fight. La sombra se dibuja en (x, y) y el cuerpo en (x, y − z).
      Es lógica pura, sin snes.h: se prueba en el PC (tests/prueba_mundo.c).
      Esqueleto de la Revisión 3: solo andar, ir por el carril y saltar; los golpes
      (combate.c), los enemigos y las oleadas se añaden sobre este cuerpo.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mando.h.
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_mundo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Cuerpo; 16 bytes: potencia de 2)
      x, y      u16   pies: x en el mundo; y, línea de pantalla del suelo (CARRIL_Y_MIN..MAX)
      z, vz     s16   altura y velocidad vertical en 12.4 (16 = 1 px): z = 0 en el suelo
      mira_izq  u8    1 si mira a la izquierda
      estado    u8    CUERPO_PARADO / ANDANDO / SUBIENDO / BAJANDO / ATACANDO
      golpe, reloj, cola, id   u8   el combo: golpe en curso, cuadros en él, A ya pulsada, tajo
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
#define PASO_Y 1                         /* Andar por el carril: la profundidad es más lenta. */
#define SALTO_V0 96                      /* 6 px/frame hacia arriba (12.4).               */
#define GRAVEDAD 8                       /* 0,5 px/frame²: salto de 36 px y 24 frames.    */

/*--- El combo de Tarou (A, A, A: el último derriba; cifras a ajustar jugando) --------*/
#define GOLPES 3                         /* Golpes del combo.                             */
#define GOLPE_FRAMES 14                  /* Duración de los golpes 1 y 2.                 */
#define GOLPE_ULTIMO_FRAMES 20           /* El último tarda más en recuperarse.           */
#define GOLPE_ACTIVO_DESDE 4             /* La caja de golpe, activa de aquí...           */
#define GOLPE_ACTIVO_HASTA 8             /* ...hasta aquí (sin incluir): 4 frames.        */
#define GOLPE_ENCADENAR_DESDE 5          /* A a partir de este cuadro encadena el siguiente. */

typedef enum
{
    CUERPO_PARADO = 0,
    CUERPO_ANDANDO,
    CUERPO_SUBIENDO,                     /* En el aire, vz < 0.                           */
    CUERPO_BAJANDO,                      /* En el aire, vz >= 0.                          */
    CUERPO_ATACANDO,                     /* Dando un golpe del combo (golpe 1..3).        */
    CUERPO_ESTADOS
} EstadoCuerpo;

typedef struct
{
    u16 x;
    u16 y;
    s16 z;
    s16 vz;
    u8 mira_izq;
    u8 estado;
    u8 golpe;                            /* 0 = sin golpe; 1..GOLPES = el del combo en curso. */
    u8 reloj;                            /* Cuadros que lleva en ese golpe.               */
    u8 cola;                             /* 1 = ya pulsó A a tiempo: encadena al acabar.  */
    u8 id;                               /* Identificador del tajo (1..254, cambia en cada golpe). */
    u16 libre;                           /* Relleno hasta 16 bytes.                       */
} Cuerpo;

/* Pone el cuerpo en el suelo, quieto, mirando a la derecha. */
void mundo_iniciar(Cuerpo *c, u16 x, u16 y);

/* Un frame: mueve según el mando, salta, aplica la gravedad y recorta a los límites. */
void mundo_actualizar(Cuerpo *c, const Mando *m);

/* Recorta x a [x_min, x_max]: la cámara pone ahí los bordes de la pantalla (cámara +
   MARGEN_PANTALLA y cámara + 256 − MARGEN_PANTALLA), para que el protagonista no salga de
   ella ni vuelva atrás de lo ya recorrido. Después de mundo_actualizar. */
void mundo_limitar_x(Cuerpo *c, u16 x_min, u16 x_max);

/* 1 si el cuerpo está dando un golpe y su caja de golpe está activa en este cuadro. */
u8 mundo_golpe_activo(const Cuerpo *c);

/* La altura en píxeles (z / 16, 0 en el suelo). */
u16 mundo_altura(const Cuerpo *c);

#endif /* MUNDO_H */
