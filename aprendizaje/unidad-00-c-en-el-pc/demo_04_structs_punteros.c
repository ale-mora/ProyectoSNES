/*---------------------------------------------------------------------------------
    demo_04_structs_punteros.c — Demo 4 de la unidad 00: punteros, struct,
    typedef y macros.

    PROPÓSITO
      Enseña, en seis pasos numerados en la salida, las piezas de C con las
      que está escrito src/juego/tarou.c: un puntero a un entero, una struct
      con typedef, la diferencia entre pasar una struct por valor (la función
      recibe una copia) y por puntero (la función cambia el original), el
      operador ->, un parámetro de salida (Caja *c, como tarou_caja_cuerpo) y
      por qué una macro necesita paréntesis.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (int16_t, uint8_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/juego/tarou.h (typedef struct Tarou, Caja) y src/juego/tarou.c
                (funciones que reciben Tarou *t).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_04.exe demo_04_structs_punteros.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      vidas      int          3 → 5            entero que se cambia a través de un puntero
      pv         int *        &vidas           puntero: guarda la dirección de vidas
      tarou      Personaje    x 100..106       el personaje de ejemplo
      pt         Personaje *  &tarou           puntero a la struct
      cuerpo     Caja         -                la caja que rellena caja_cuerpo

    → conceptos (README de la unidad 00): puntero, struct, typedef, #define, static
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae enteros de tamaño fijo: int16_t, uint8_t.    */

/*--- Los mismos nombres de tipo que el juego (src/sistema/tipos.h hace esto mismo) -*/
typedef int16_t s16;                 /* s16: entero con signo de 16 bits.                 */
typedef uint8_t u8;                  /* u8: entero sin signo de 8 bits (0..255).          */

/*--- Constantes y macros: el preprocesador las reemplaza antes de compilar ---------*/
#define VEL_INICIAL 2                /* Píxeles por frame.                                */
#define FRAMES 3                     /* Cuántos frames se mueve en el paso 4.             */
#define DOBLE(n) ((n) * 2)           /* Macro bien escrita: paréntesis en todo.           */
#define DOBLE_MAL(n) n * 2           /* Macro sin paréntesis: el paso 6 muestra el fallo. */

/*--- Un personaje: varias variables con nombre que viajan juntas -------------------*/
typedef struct                       /* typedef: después se escribe Personaje a secas.    */
{
    s16 x, y;                        /* Posición en píxeles.                              */
    s16 vx;                          /* Velocidad horizontal en píxeles por frame.        */
} Personaje;

/*--- Un rectángulo, como la Caja de tarou.h ----------------------------------------*/
typedef struct
{
    s16 x, y;                        /* Esquina superior izquierda.                       */
    u8 ancho, alto;                  /* Tamaño en píxeles.                                */
} Caja;

/* Por VALOR: p es una copia de lo que se pasó; cambiarla no cambia el original.
   "static": la función solo se ve dentro de este archivo (como cambiar_estado
   en tarou.c). */
static void mover_copia(Personaje p)
{
    p.x = p.x + p.vx;                /* Cambia la copia...                                */
    printf("3. dentro de mover_copia la copia vale x=%d\n", p.x);  /* ...y se ve aquí.  */
}

/* Por PUNTERO: p guarda la dirección del original; p->x ES el x del original. */
static void mover(Personaje *p)
{
    p->x = p->x + p->vx;             /* "p->x" es lo mismo que "(*p).x".                  */
}

/* Parámetro de entrada de solo lectura (const) y parámetro de SALIDA (c): la
   función no devuelve la caja, la escribe donde apunta c. Así es
   tarou_caja_cuerpo(const Tarou *t, Caja *c) en src/juego/tarou.c. */
static void caja_cuerpo(const Personaje *p, Caja *c)
{
    c->x = (s16)(p->x + 4);          /* El cuerpo empieza 4 px a la derecha de la celda.  */
    c->y = (s16)(p->y + 2);          /* ...y 2 px más abajo.                              */
    c->ancho = 8;                    /* 8 px de ancho.                                    */
    c->alto = 14;                    /* 14 px de alto.                                    */
}

int main(void)                       /* El programa empieza aquí.                         */
{
    /* 1. Un puntero a un entero: & da la dirección, * llega a lo apuntado. */
    int vidas = 3;                   /* Una variable normal.                              */
    int *pv = &vidas;                /* pv guarda DÓNDE está vidas, no su valor.          */
    *pv = 5;                         /* Escribir en *pv es escribir en vidas.             */
    printf("1. vidas = %d (cambiada a traves de pv)\n", vidas);

    /* 2. Una struct: se inicializa en el orden de sus campos (x, y, vx). */
    Personaje tarou = {100, 144, VEL_INICIAL};
    printf("2. tarou: x=%d y=%d vx=%d (ocupa %u bytes: tres s16)\n",
           tarou.x, tarou.y, tarou.vx, (unsigned)sizeof(Personaje));   /* "." lee un campo. */

    /* 3. Por valor: la función trabaja sobre una copia. */
    mover_copia(tarou);              /* Se copia toda la struct al llamar.                */
    printf("3. despues de mover_copia el original sigue en x=%d\n", tarou.x);

    /* 4. Por puntero: la función cambia el original. */
    for (int f = 1; f <= FRAMES; f++)
    {
        mover(&tarou);               /* Se pasa la dirección: la struct no se copia.      */
        printf("4. frame %d: x=%d\n", f, tarou.x);
    }

    /* 5. Un puntero a la struct y el operador ->. */
    Personaje *pt = &tarou;          /* pt apunta a tarou.                                */
    pt->vx = -1;                     /* Cambia tarou.vx a través del puntero.             */
    mover(pt);                       /* pt ya es una dirección: sin "&".                  */
    printf("5. con pt->vx = -1, tarou.x=%d\n", tarou.x);

    Caja cuerpo;                     /* Sin valor: caja_cuerpo la rellena.                */
    caja_cuerpo(&tarou, &cuerpo);    /* Entrada: &tarou. Salida: &cuerpo.                 */
    printf("5. caja del cuerpo: x=%d y=%d ancho=%d alto=%d\n",
           cuerpo.x, cuerpo.y, cuerpo.ancho, cuerpo.alto);

    /* 6. Macros: el preprocesador pega texto, no calcula. */
    printf("6. DOBLE(1 + 2) = %d\n", DOBLE(1 + 2));          /* ((1 + 2) * 2) = 6.       */
    printf("6. DOBLE_MAL(1 + 2) = %d\n", DOBLE_MAL(1 + 2));  /* 1 + 2 * 2 = 5: mal.      */

    return 0;                        /* Terminó bien.                                     */
}
