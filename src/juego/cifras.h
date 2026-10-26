/*---------------------------------------------------------------------------------
    cifras.h — Las cifras del combate del beat 'em up, todas en un sitio.

    PROPÓSITO
      Son los valores de docs/diseno-beat-em-up.md §5.5, decididos con el autor
      el 2026-10-23: vida, daño, velocidades, tiempos, rangos y jefes. Son el
      punto de partida y se ajustan jugando; por eso viven juntas, para que
      afinar el juego sea cambiar números aquí y nada más. Unidades: puntos de
      vida, frames (60 por segundo) y px por frame en punto fijo 12.4.
      Las que dependen del rango del ninja son TABLAS de 4 entradas (una por
      color de cinturón), nunca una multiplicación en el frame (tcc816 llama a
      una rutina lenta para multiplicar).

    SE CONECTA CON
      Incluye:  sistema/tipos.h, sistema/puntofijo.h.
      Lo usan:  juego/mundo.c, juego/combate.c, juego/doble_toque.c y sus
                pruebas en tests/.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: solo #define (constantes del preprocesador).
    → glosario: Punto fijo, #define, Rango
---------------------------------------------------------------------------------*/
#ifndef CIFRAS_H
#define CIFRAS_H

#include "sistema/tipos.h"
#include "sistema/puntofijo.h"

/*--- Protagonistas: vida y velocidades (§5.5) --------------------------------------*/
#define VIDA_PROTAGONISTA 64         /* Cabe en un u8 y en una barra de 64 px.             */
#define VEL_ANDAR_TAROU 24           /* 1,5 px por frame en x.                             */
#define VEL_CORRER_TAROU 48          /* 3,0 px por frame en x.                             */
#define VEL_ANDAR_BARBARA 20         /* 1,25 px por frame en x.                            */
#define VEL_CORRER_BARBARA 40        /* 2,5 px por frame en x.                             */
/* En profundidad (y) se va a la mitad de x: la velocidad en y es la de x >> 1.         */

/*--- Daño de los protagonistas -------------------------------------------------------*/
#define DANO_COMBO_TAROU 3           /* Golpes 1 a 3 del combo de Tarou.                   */
#define DANO_MORDISCO_TAROU 5        /* Golpe 4: el mordisco, derriba.                     */
#define DANO_COMBO_BARBARA 5         /* Golpes 1 y 2 del combo de Bárbara.                 */
#define DANO_TAJO_BARBARA 8          /* Golpe 3: el tajo descendente, derriba.             */
#define DANO_AEREO_TAROU 5           /* Ataque aéreo y ataque corriendo de Tarou.          */
#define DANO_AEREO_BARBARA 7         /* Ataque aéreo y ataque corriendo de Bárbara.        */
#define DANO_PATADA_TAROU 7          /* Patada voladora de Tarou (decisión 40).            */
#define DANO_PATADA_BARBARA 10       /* Patada voladora de Bárbara.                        */
#define DANO_AGARRE_TAROU 2          /* Cada zarpazo del agarre.                           */
#define DANO_AGARRE_BARBARA 4        /* Cada golpe de empuñadura del agarre.               */
#define DANO_LANZAR_TAROU 6          /* Lanzamiento (también a quien choque).              */
#define DANO_LANZAR_BARBARA 10       /* Lanzamiento de judo.                               */
#define DANO_ARMA 6                  /* Shuriken o kunai lanzado, para los dos.            */
#define DANO_ESPECIAL 6              /* A+B: a todos los cercanos.                         */
#define COSTE_ESPECIAL 4             /* Vida que cuesta A+B, solo si conecta.              */

/*--- Reglas con cifra ----------------------------------------------------------------*/
#define VENTANA_COMBO 12             /* Frames para volver a pulsar A y encadenar.         */
#define VENTANA_DOBLE_TOQUE 12       /* Frames entre soltar y repetir la dirección.        */
#define FRAMES_INVULNERABLE 60       /* Al levantarse, con parpadeo.                       */
#define FRAMES_PARADA 4              /* Parada de impacto de un golpe fuerte.              */
#define FRAMES_DESTELLO 2            /* Parpadeo en blanco del golpeado.                   */
#define FRAMES_PARA_AGARRAR 10       /* Andando contra el enemigo.                         */
#define TOLERANCIA_Y PF(8)           /* 8 px de profundidad para que un golpe conecte.     */
#define ENFRIAMIENTO_RELEVO 120      /* Frames entre dos relevos.                          */
#define DANO_APOYO_LUCINDA 8         /* Cada uno de los 3 tajos de Lucinda (24 en total).    */
#define DANO_APOYO_REX 16            /* Al soltar Rex a los enemigos levantados.             */
#define DANO_APOYO_FENRIR 20         /* La embestida de Fenrir.                              */
#define CURA_REX 16                  /* Lo que cura Rex al protagonista activo.              */
#define DANO_ENTRADA_RELEVO 4        /* Golpe de entrada del que llega.                    */
#define ALCANCE_ENTRADA_RELEVO 48    /* px: empuja a los que estén a menos de esto.        */
#define FRAMES_CURA_FUERA 120        /* El que espera fuera gana 1 de vida cada 2 s...     */
#define TOPE_CURA_FUERA 48           /* ...hasta 48 de 64 (75 %).                          */
#define BARRA_LLENA 48               /* Golpes que conectan para llenar la barra.          */
#define CURA_ONIGIRI 16              /* Un cuarto de barra.                                */
#define CURA_PESCADO 64              /* La barra entera.                                   */
#define ALTURA_ENTRADA_RELEVO 80     /* px desde los que cae el que entra (≈25 frames).    */
#define FRAMES_ENTRADA_GOLPE 8       /* Lo que dura el golpe de entrada tras aterrizar.    */
#define VIDAS_INICIALES 3            /* Decisión 12.                                       */
#define CONTINUES_INICIALES 3        /* Decisión 12.                                       */

/*--- El salto en z (heredado de la física de plataformas, más bajo) ----------------*/
#define GRAVEDAD_Z 4                 /* 0,25 px por frame cada frame.                      */
#define VEL_SALTO_Z 80               /* 5 px por frame al despegar: ~50 px de altura.      */

/*--- Ninjas por rango (cinturón): índices de las tablas ------------------------------*/
enum { RANGO_BLANCO = 0, RANGO_AMARILLO = 1, RANGO_VERDE = 2, RANGO_ROJO = 3, RANGOS = 4 };
#define VEL_NINJA 16                 /* 1 px por frame en x (inicial).                     */
#define DANO_SHINAI 4                /* Shinai y patada del básico.                        */
#define DANO_SHURIKEN 3              /* Shuriken y tajo rápido del rápido.                 */
#define DANO_PESADO 8                /* Golpe del kendoka pesado.                          */
#define DANO_LANZAR_PESADO 10        /* Lanzamiento del kendoka pesado.                    */

/*--- Jefes ---------------------------------------------------------------------------*/
#define VIDA_GOMASURI 160            /* Cambia de patrón al 50 %.                          */
#define DANO_CARGA_GOMASURI 10       /* Su carga.                                          */
#define DANO_TAJO_GOMASURI 8         /* Su tajo de alcance.                                */

/*--- Puntuación ----------------------------------------------------------------------*/
#define PUNTOS_GOLPE 10              /* Cada golpe que conecta.                            */
#define PUNTOS_JEFE 5000             /* Al vencer a un jefe.                               */
#define PUNTOS_ROMPIBLE 50           /* Barril, caja, farol...                             */
#define PUNTOS_COMIDA 200            /* Al recoger comida.                                 */

/* Tablas por rango (definidas en juego/combate.c; static const para que vivan en ROM). */
u8 cifras_vida_ninja(u8 rango);      /* 16 / 24 / 32 / 48.                                 */
u8 cifras_bonus_dano(u8 rango);      /* +0 / +1 / +2 / +3.                                 */
u16 cifras_puntos_ninja(u8 rango);   /* 100 / 150 / 200 / 300.                             */

#endif /* CIFRAS_H */
