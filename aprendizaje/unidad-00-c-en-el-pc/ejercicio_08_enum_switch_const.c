/*---------------------------------------------------------------------------------
    ejercicio_08_enum_switch_const.c — Ejercicio 8 de la unidad 00: enum, switch y const.

    PROPÓSITO
      Rellenar dos tablas const indexadas por un enum de estados (como
      ANIMACIONES[] y NOMBRES_ESTADO[] en src/juego/animaciones.c) y escribir
      dos funciones: una que lee una fila de la tabla a través de un puntero
      a const y otra que agrupa los estados con un switch (como
      tarou_actualizar en src/juego/tarou.c:509). main ya está escrito: no lo
      toques; solo rellena los TODO 1 a 4.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t, uint16_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/juego/animaciones.h (enum Estado, AnimDef),
                src/juego/animaciones.c (las tablas), src/juego/tarou.c:507-535.
      Compilar: gcc -std=c99 -Wall -Wextra -o ejercicio_08.exe ejercicio_08_enum_switch_const.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      ANIMACIONES[7]     const AnimDef       cuadros, frames por cuadro y bucle de cada estado
      NOMBRES_ESTADO[7]  const char *const   nombre de cada estado
      e                  u8                  estado que se imprime (0..6)

    → conceptos (README de la unidad 00): enum, switch, case, break, default, const
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae uint8_t y uint16_t.                          */

/*
 * SALIDA ESPERADA (exacta):
 *   0 PARADO: 4 cuadros x 12 frames = 48 frames, bucle 1, grupo suelo
 *   1 CAMINANDO: 8 cuadros x 6 frames = 48 frames, bucle 1, grupo suelo
 *   2 CORRIENDO: 8 cuadros x 4 frames = 32 frames, bucle 1, grupo suelo
 *   3 SALTANDO: 3 cuadros x 6 frames = 18 frames, bucle 0, grupo aire
 *   4 CAYENDO: 2 cuadros x 6 frames = 12 frames, bucle 0, grupo aire
 *   5 ATACANDO: 1 cuadros x 20 frames = 20 frames, bucle 0, grupo espada
 *   6 DORMIDO: 2 cuadros x 20 frames = 40 frames, bucle 1, grupo sueno
 *   Estado 7: grupo desconocido
 *
 * Los cuadros de este ejercicio son de ejemplo; los del juego los escribe el
 * pipeline del arte en tarou_anim.h.
 */

typedef uint8_t u8;                  /* Los nombres del juego (src/sistema/tipos.h).      */
typedef uint16_t u16;

/* El orden importa: es el índice de las dos tablas. */
typedef enum
{
    ESTADO_PARADO = 0,               /* 0                                                 */
    ESTADO_CAMINANDO,                /* 1                                                 */
    ESTADO_CORRIENDO,                /* 2                                                 */
    ESTADO_SALTANDO,                 /* 3                                                 */
    ESTADO_CAYENDO,                  /* 4                                                 */
    ESTADO_ATACANDO,                 /* 5                                                 */
    ESTADO_DORMIDO,                  /* 6                                                 */
    ESTADO_CANTIDAD                  /* 7: no es un estado, es cuántos hay.               */
} Estado;

typedef struct
{
    u8 cuadros;                      /* Cuántos cuadros tiene la animación.               */
    u8 frames_por_cuadro;            /* Cada cuántos frames cambia de cuadro.             */
    u8 bucle;                        /* 1 = vuelve a empezar; 0 = se queda en el último.  */
} AnimDef;

/* TODO 1: rellena la tabla, una fila {cuadros, frames_por_cuadro, bucle} por
           estado y en el orden del enum, con los valores de la salida
           esperada. Ahora todas las filas valen {0, 0, 0}: {{0}} pone a cero
           la primera y C rellena con ceros las que faltan. */
static const AnimDef ANIMACIONES[ESTADO_CANTIDAD] = {{0}};

/* TODO 2: los nombres, en el orden del enum ("PARADO", "CAMINANDO"...). Ahora
           todos son "?". Hay dos const: el primero dice que las letras no se
           cambian y el segundo, que los punteros de la tabla tampoco. */
static const char *const NOMBRES_ESTADO[ESTADO_CANTIDAD] = {
    "?", "?", "?", "?", "?", "?", "?",
};

/* TODO 3: devuelve la duración de la animación del estado en frames: cuadros
           por frames_por_cuadro. Lee la fila con un puntero a const:
               const AnimDef *a = &ANIMACIONES[estado];
           y después a->cuadros y a->frames_por_cuadro. */
u16 duracion(u8 estado)
{
    (void)estado;                    /* Calla «parámetro sin usar»: bórrala en el TODO 3. */
    return 0;
}

/* TODO 4: con un switch sobre estado, devuelve "suelo" para PARADO, CAMINANDO
           y CORRIENDO (tres case seguidos con un solo return), "aire" para
           SALTANDO y CAYENDO, "espada" para ATACANDO, "sueno" para DORMIDO y
           "desconocido" en el default. Con return dentro de cada case no
           hace falta break. */
const char *grupo(u8 estado)
{
    (void)estado;                    /* Calla «parámetro sin usar»: bórrala en el TODO 4. */
    return "?";
}

/*--- main: ya está escrito, no lo cambies -------------------------------------------*/
int main(void)
{
    for (u8 e = 0; e < ESTADO_CANTIDAD; e++)        /* Un estado por línea.              */
    {
        printf("%d %s: %d cuadros x %d frames = %d frames, bucle %d, grupo %s\n",
               e, NOMBRES_ESTADO[e], ANIMACIONES[e].cuadros,
               ANIMACIONES[e].frames_por_cuadro, duracion(e),
               ANIMACIONES[e].bucle, grupo(e));
    }
    printf("Estado %d: grupo %s\n", ESTADO_CANTIDAD, grupo(ESTADO_CANTIDAD));   /* Fuera de la lista. */
    return 0;                                        /* Terminó bien.                     */
}
