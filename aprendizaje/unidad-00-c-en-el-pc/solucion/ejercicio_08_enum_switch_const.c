/*---------------------------------------------------------------------------------
    solucion/ejercicio_08_enum_switch_const.c — Solución del ejercicio 8 de la unidad 00.

    PROPÓSITO
      Las dos tablas const, en el orden del enum, y las dos funciones: una
      lee la fila con un puntero a const y la otra agrupa estados con un
      switch de case seguidos. main es idéntico al del ejercicio.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t, uint16_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/juego/animaciones.h (enum Estado, AnimDef),
                src/juego/animaciones.c (las tablas), src/juego/tarou.c:507-535.
      Compilar: gcc -std=c99 -Wall -Wextra -o solucion_08.exe solucion/ejercicio_08_enum_switch_const.c

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

/* TODO 1: una fila por estado, en el orden del enum. */
static const AnimDef ANIMACIONES[ESTADO_CANTIDAD] = {
    {4, 12, 1},                      /* ESTADO_PARADO                                     */
    {8, 6, 1},                       /* ESTADO_CAMINANDO                                  */
    {8, 4, 1},                       /* ESTADO_CORRIENDO                                  */
    {3, 6, 0},                       /* ESTADO_SALTANDO                                   */
    {2, 6, 0},                       /* ESTADO_CAYENDO                                    */
    {1, 20, 0},                      /* ESTADO_ATACANDO                                   */
    {2, 20, 1},                      /* ESTADO_DORMIDO                                    */
};

/* TODO 2: los nombres, en el mismo orden. const char: las letras no cambian;
   *const: los punteros de la tabla tampoco. */
static const char *const NOMBRES_ESTADO[ESTADO_CANTIDAD] = {
    "PARADO", "CAMINANDO", "CORRIENDO", "SALTANDO", "CAYENDO", "ATACANDO", "DORMIDO",
};

/* TODO 3: la fila se lee a través de un puntero a const (solo lectura). */
u16 duracion(u8 estado)
{
    const AnimDef *a = &ANIMACIONES[estado];         /* Dirección de la fila.             */
    return (u16)(a->cuadros * a->frames_por_cuadro); /* Cuadros por frames de cada uno.   */
}

/* TODO 4: case seguidos comparten el return; el default recoge el resto. */
const char *grupo(u8 estado)
{
    switch (estado)
    {
        case ESTADO_PARADO:
        case ESTADO_CAMINANDO:
        case ESTADO_CORRIENDO:
            return "suelo";                          /* Los tres de pie.                  */
        case ESTADO_SALTANDO:
        case ESTADO_CAYENDO:
            return "aire";
        case ESTADO_ATACANDO:
            return "espada";
        case ESTADO_DORMIDO:
            return "sueno";
        default:
            return "desconocido";                    /* ESTADO_CANTIDAD o cualquier otro. */
    }
}

/*--- main: igual en el ejercicio y en la solución -----------------------------------*/
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
