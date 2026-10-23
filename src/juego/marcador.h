/*---------------------------------------------------------------------------------
    marcador.h — Puntuación, vidas, continues y vidas extra del arcade (spec §5.5 y
    decisión 12).

    PROPÓSITO
      Lleva la cuenta de lo que el jugador ve en el marcador: los puntos (que pasan
      de 10 000 y, en el arcade, pueden llegar a cientos de miles), las vidas
      que le quedan, los continues y las vidas extra (a 50 000 y a 150 000 puntos,
      cada una una sola vez). No hay `long` en tcc816: los puntos se guardan en DOS
      u16: `resto` (0..9999) y `miles` (cada 10 000 puntos). Así una suma es un
      acarreo a mano, y las vidas extra salen de comparar `miles`.
      Cada golpe, ninja o comida suma por su cuenta; aquí solo se suma y se
      comprueba. Lógica pura, probada en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/cifras.h.
      Lo usan:  pantallas/prototipo.c (cuando exista el marcador en pantalla) y
                tests/prueba_marcador.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Marcador)
      resto           u16  0..9999                   puntos sin contar los de 10 000
      miles           u16  cada 10 000 puntos
      vidas           u8   vidas que quedan (0 = se acabó la partida)
      continues       u8   continues que quedan
      extra_50k       u8   1 si ya dio la vida extra de 50 000
      extra_150k      u8   1 si ya dio la vida extra de 150 000
      relleno         u8[2]

    → glosario: Vida extra, Continue, Puntuación
---------------------------------------------------------------------------------*/
#ifndef MARCADOR_H
#define MARCADOR_H

#include "sistema/tipos.h"

#define MARCADOR_EXTRA_50K_MILES 5      /* 50 000 puntos = 5 veces 10 000.                  */
#define MARCADOR_EXTRA_150K_MILES 15    /* 150 000 puntos = 15 veces 10 000.                */

typedef struct
{
    u16 resto;                       /* Puntos por debajo de 10 000.                       */
    u16 miles;                       /* Cuántas veces se han llenado los 10 000.           */
    u8 vidas;                        /* Vidas que quedan.                                  */
    u8 continues;                    /* Continues que quedan.                              */
    u8 extra_50k;                    /* 1 si la vida extra de 50 000 ya se dio.            */
    u8 extra_150k;                   /* 1 si la de 150 000 ya se dio.                      */
    u8 relleno[2];                   /* Hasta 12 bytes.                                    */
} Marcador;

/* Al empezar la partida: 0 puntos, las vidas y los continues iniciales (decisión 12). */
void marcador_iniciar(Marcador *m);

/* Suma `puntos` (menos de 10 000 cada vez) y da las vidas extra que toquen. Devuelve
   cuántas vidas extra ha dado esta suma (0, 1 o 2). */
u8 marcador_sumar(Marcador *m, u16 puntos);

/* Escribe los puntos en decimal, sin ceros delante, terminados en 0 (`salida` de 12
   caracteres como mínimo). Si hay miles, los 4 dígitos del resto van con ceros
   (151234, no 15 1234). Sin divisiones: resta por potencias de diez. */
void marcador_texto(const Marcador *m, char *salida);

/* Pierde una vida. Devuelve 1 si se acabó la partida (sin vidas), 0 si sigue. */
u8 marcador_perder_vida(Marcador *m);

/* Usa un continue: vuelve con las vidas iniciales. Devuelve 1 si lo había, 0 si no. */
u8 marcador_continuar(Marcador *m);

#endif /* MARCADOR_H */
