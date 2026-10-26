/*---------------------------------------------------------------------------------
    escena.h — El motor de escenas: valores que se mueven solos según una tabla.

    PROPÓSITO
      Una ESCENA es una animación con guion fijo (un logo que aparece, un
      destello, una cinemática). En vez de escribir a mano "en el frame 70
      sube esto, en el 112 baja aquello" con ifs y contadores en cada pantalla,
      la pantalla escribe una TABLA DE EVENTOS y este motor la recorre:

        RAMPA(0, 32, PISTA_BRILLO, 0, 15)   "desde el frame 0, en 32 frames,
                                             el brillo pasa de 0 a 15"

      Cada PISTA es un número que el motor mueve (brillo, mosaico, posición de
      un destello...). Este archivo NO toca el hardware: solo calcula los
      valores frame a frame (se prueba en el PC, tests/prueba_escena.c). Quien
      los lleva a los registros de la PPU es escena/efectos.c, en el VBlank.

      Por qué así: separar el "qué pasa y cuándo" (la tabla, que se lee como
      un guion) del "cómo se dibuja" (los registros) deja cambiar la
      coreografía sin tocar código, y probarla sin la consola.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  escena/escena.c (lo implementa), escena/efectos.c (lee los
                valores), escena/escena_nintendo.c (una tabla),
                pantallas/pantalla_nintendo.c, tests/prueba_escena.c.

    HARDWARE QUE TOCA
      ninguno: lógica pura (se compila también en el PC).

    VARIABLES (constantes y tipos)
      PISTA_BRILLO..PISTA_LUZ  u8   qué valor mueve un evento (0..3)
      ESCENA_PISTAS            4    cuántas pistas hay (potencia de 2: ver Pista)
      ESCENA_FIN               0xFF "pista" del evento que termina la escena
      EventoEscena             8 B  una línea de la tabla
      Pista                    8 B  el estado de un valor en movimiento
      Escena                        las pistas, el próximo evento y el reloj

    → glosario: Motor de escenas, Punto fijo, Frame
---------------------------------------------------------------------------------*/
#ifndef ESCENA_H
#define ESCENA_H

#include "sistema/tipos.h"

/*--- Las pistas ------------------------------------------------------------------
    Qué significa cada número lo decide escena/efectos.c (quien lo aplica). */
#define PISTA_BRILLO 0             /* 0..15: brillo de la pantalla (0 = negro).        */
#define PISTA_MOSAICO 1            /* 0..15: tamaño del mosaico (0 = imagen nítida).   */
#define PISTA_DESTELLO 2           /* 0..255: avance de la banda de luz (izq. a der.). */
#define PISTA_LUZ 3                /* 0..31: cuánta luz suma la banda (0 = apagada).   */
#define ESCENA_PISTAS 4            /* Cuántas hay.                                     */
#define ESCENA_FIN 0xFF            /* Evento especial: aquí termina la escena.         */

/*--- Un evento de la tabla (8 bytes justos) ----------------------------------------
    "En el frame `cuadro`, la pista `pista` empieza a ir hacia `hasta` y tarda
    `frames` frames, sumando `paso` en cada uno". El paso va en PUNTO FIJO 8.8:
    el byte alto es la parte entera y el bajo, 256avos (120 = 0,47 por frame).
    Así un valor puede subir menos de 1 por frame sin decimales de verdad.
    No se escribe a mano: lo calculan las macros de abajo. */
typedef struct
{
    u16 cuadro;                    /* Frame en que empieza (desde el inicio de la escena). */
    u16 frames;                    /* Cuántos frames dura el cambio (0 = de golpe).        */
    u8 pista;                      /* PISTA_* o ESCENA_FIN.                                */
    u8 hasta;                      /* Valor final.                                         */
    s16 paso;                      /* Cuánto se suma por frame, en 8.8 (negativo si baja). */
} EventoEscena;

/* RAMPA: de `desde` a `hasta` en `frames` frames. El paso lo calcula el
   COMPILADOR al leer la tabla (son todo constantes), así que en la SNES no hay
   ninguna división: tcc816 las hace con una rutina lenta (→ CLAUDE.md). Se
   divide por 128 y se multiplica por 2 en vez de multiplicar por 256 porque
   255 x 256 no cabe en el int de 16 bits de tcc816 (255 x 128 sí). Se pierde
   un 1/256 de precisión que da igual: al acabar, el valor se pone EXACTO.
   OJO: `desde` tiene que ser el valor que la pista tenga en ese momento; si
   no, la rampa va con otra velocidad y al final da un salto (lo comprueba
   tests/prueba_escena.c para cada tabla). */
#define RAMPA(cuadro, frames, pista, desde, hasta)                                     \
    {(cuadro), (frames), (pista), (hasta),                                             \
     (s16)((((s16)(hasta) - (s16)(desde)) * 128 / (frames)) * 2)}

/* FIJAR: la pista toma `valor` en ese mismo frame, sin transición. */
#define FIJAR(cuadro, pista, valor) {(cuadro), 0, (pista), (valor), 0}

/* FIN_ESCENA: la escena termina al llegar a este frame (va SIEMPRE la última). */
#define FIN_ESCENA(cuadro) {(cuadro), 0, ESCENA_FIN, 0, 0}

/*--- El estado de una pista (8 bytes: así pistas[i] no necesita multiplicar) ------ */
typedef struct
{
    u16 valor;                     /* Valor actual en 8.8 (el byte alto es el que vale). */
    s16 paso;                      /* Cuánto se suma en cada frame.                      */
    u16 quedan;                    /* Frames que le faltan (0 = quieta).                 */
    u8 hasta;                      /* Dónde termina.                                     */
    u8 libre;                      /* Relleno hasta 8 bytes.                             */
} Pista;

/*--- Una escena en marcha --------------------------------------------------------- */
typedef struct
{
    Pista pistas[ESCENA_PISTAS];   /* Los valores en movimiento.                          */
    const EventoEscena *proximo;   /* El próximo evento por empezar (la tabla, ordenada). */
    u16 cuadro;                    /* Frames que lleva la escena.                         */
    u8 terminada;                  /* 1 cuando se llegó a FIN_ESCENA.                     */
} Escena;

/* El valor entero de una pista. Es una macro y no una función porque cada
   llamada a función cuesta mucho en tcc816, y con `pista` constante el
   compilador resuelve pistas[pista] sin cuentas. */
#define ESCENA_VALOR(e, pista) ((u8)((e)->pistas[pista].valor >> 8))

/* Prepara la escena con su tabla: todas las pistas a 0, reloj a 0. */
void escena_empezar(Escena *e, const EventoEscena *tabla);

/* Un frame: arranca los eventos de este frame, mueve las pistas, avanza el reloj. */
void escena_avanzar(Escena *e);

#endif /* ESCENA_H */
