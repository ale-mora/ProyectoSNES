/*---------------------------------------------------------------------------------
    guion.h — Un mando que no toca nadie: botones leídos de una tabla de tiempo.

    PROPÓSITO
      Un GUION es una lista de tramos "del cuadro A al cuadro B, mantener el
      botón X". guion_mando() dice, para un cuadro dado, qué Mando resulta: qué
      botones están mantenidos y cuáles se acaban de pulsar (mantenido ahora y
      no en el cuadro anterior). Sirve para dos cosas:
        - En el PC, la prueba del recorrido completo del tutorial
          (tests/prueba_recorrido.c) juega el nivel entero con el guion.
        - En la SNES, una ROM de prueba (compilada con -DGUION_PRUEBA) lee el
          MISMO guion en vez del mando real (juego/mando.c): así Mesen muestra
          el recorrido sin que nadie pulse teclas, y si la ROM y el PC no hacen
          lo mismo, es que el compilador de la SNES (tcc816) hace algo distinto.

    SE CONECTA CON
      Incluye:  juego/mando.h (la struct Mando), sistema/tipos.h.
      Lo usan:  juego/mando.c (con GUION_PRUEBA), juego/guion_tutorial.c (el
                guion del nivel 1), tests/prueba_recorrido.c.

    HARDWARE QUE TOCA
      ninguno: lógica pura (se compila también en el PC).

    VARIABLES (constantes)
      BOTON_IZQUIERDA ... BOTON_ESPADA  los botones de un tramo (bits de u8)

    → glosario: Mando, Frame
---------------------------------------------------------------------------------*/
#ifndef GUION_H
#define GUION_H

#include "sistema/tipos.h"
#include "juego/mando.h"

#define BOTON_IZQUIERDA 0x01
#define BOTON_DERECHA 0x02
#define BOTON_ABAJO 0x04
#define BOTON_CORRER 0x08          /* Y */
#define BOTON_SALTAR 0x10          /* B */
#define BOTON_ATACAR 0x20          /* A */
#define BOTON_ESPADA 0x40          /* R */

typedef struct
{
    u16 desde;                     /* Primer cuadro con el botón mantenido...              */
    u16 hasta;                     /* ...y el primero SIN él (el tramo es [desde, hasta)). */
    u8 botones;                    /* BOTON_* (se pueden sumar con |).                     */
} TramoGuion;

/* Rellena *m con lo que el guion (n tramos) dice para el cuadro `cuadro`. */
void guion_mando(const TramoGuion *guion, u8 n, u16 cuadro, Mando *m);

#endif /* GUION_H */
