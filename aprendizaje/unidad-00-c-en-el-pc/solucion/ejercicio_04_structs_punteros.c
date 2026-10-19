/*---------------------------------------------------------------------------------
    solucion/ejercicio_04_structs_punteros.c — Solución del ejercicio 4 de la unidad 00.

    PROPÓSITO
      Las cuatro funciones del ejercicio, escritas como las de src/juego/tarou.c:
      reciben el personaje por puntero (Personaje *p) y lo cambian; la de la
      caja lo recibe de solo lectura (const) y escribe el resultado en un
      parámetro de salida (Caja *c).

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (int16_t, uint8_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   tarou_iniciar, acelerar y tarou_caja_cuerpo en src/juego/tarou.c.
      Compilar: gcc -std=c99 -Wall -Wextra -o solucion_04.exe solucion/ejercicio_04_structs_punteros.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      tarou      Personaje   x 0..X_MAX, vx -VEL_MAX..VEL_MAX   el personaje que se mueve
      cuerpo     Caja        -                                  su caja al final
      f          int         1..9                               número de frame

    → conceptos (README de la unidad 00): puntero, struct, typedef, const, #define
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae int16_t y uint8_t.                           */

typedef int16_t s16;                 /* Entero con signo de 16 bits, como en el juego.    */
typedef uint8_t u8;                  /* Entero sin signo de 8 bits.                       */

#define ACEL 1                       /* Cuánto cambia vx en cada frame (px/frame).        */
#define VEL_MAX 3                    /* Velocidad máxima en px/frame.                     */
#define X_MAX 160                    /* Borde derecho: x no puede pasar de aquí.          */
#define CUERPO_X 24                  /* La caja del cuerpo empieza 24 px a la derecha...  */
#define CUERPO_Y 8                   /* ...y 8 px más abajo que la esquina del personaje. */
#define CUERPO_ANCHO 48              /* Ancho de la caja.                                 */
#define CUERPO_ALTO 56               /* Alto de la caja.                                  */

enum { DIR_DERECHA = 0, DIR_IZQUIERDA = 1 };  /* Los mismos valores que en tarou.h.       */

typedef struct
{
    s16 x, y;                        /* Posición en píxeles (esquina superior izquierda). */
    s16 vx;                          /* Velocidad horizontal en px por frame.             */
    u8 direccion;                    /* DIR_DERECHA o DIR_IZQUIERDA.                      */
} Personaje;

typedef struct
{
    s16 x, y;                        /* Esquina superior izquierda.                       */
    u8 ancho, alto;                  /* Tamaño en píxeles.                                */
} Caja;

/*--- Lo que en el juego iría en un .h: las firmas de las funciones -----------------*/
void personaje_iniciar(Personaje *p, s16 x, s16 y);
void personaje_acelerar(Personaje *p, s16 acel, s16 max);
void personaje_mover(Personaje *p, s16 x_max);
void personaje_caja(const Personaje *p, Caja *c);

/* TODO 1: deja al personaje quieto en (x, y), mirando a la derecha. */
void personaje_iniciar(Personaje *p, s16 x, s16 y)
{
    p->x = x;                        /* Escribe en la struct del que llama, no en una copia. */
    p->y = y;
    p->vx = 0;                       /* Quieto.                                           */
    p->direccion = DIR_DERECHA;      /* Mira a la derecha.                                */
}

/* TODO 2: suma acel a vx sin pasar de +max ni de -max; la dirección sigue al
   signo de acel. */
void personaje_acelerar(Personaje *p, s16 acel, s16 max)
{
    if (acel > 0)                    /* Empuje hacia la derecha.                          */
    {
        p->direccion = DIR_DERECHA;
    }
    else if (acel < 0)               /* Empuje hacia la izquierda.                        */
    {
        p->direccion = DIR_IZQUIERDA;
    }
    p->vx = (s16)(p->vx + acel);     /* Acelera.                                          */
    if (p->vx > max)                 /* Recorte por arriba...                             */
    {
        p->vx = max;
    }
    if (p->vx < -max)                /* ...y por abajo.                                   */
    {
        p->vx = (s16)-max;
    }
}

/* TODO 3: integra (x += vx) y recorta x a 0..x_max; al chocar con un borde,
   vx pasa a 0. */
void personaje_mover(Personaje *p, s16 x_max)
{
    p->x = (s16)(p->x + p->vx);      /* Integrar: la posición avanza lo que dice la velocidad. */
    if (p->x < 0)                    /* Se salió por la izquierda.                        */
    {
        p->x = 0;
        p->vx = 0;
    }
    if (p->x > x_max)                /* Se salió por la derecha.                          */
    {
        p->x = x_max;
        p->vx = 0;
    }
}

/* TODO 4: rellena c con la caja del cuerpo. p es const: aquí solo se lee. */
void personaje_caja(const Personaje *p, Caja *c)
{
    c->x = (s16)(p->x + CUERPO_X);   /* Desplazada dentro de la celda.                    */
    c->y = (s16)(p->y + CUERPO_Y);
    c->ancho = CUERPO_ANCHO;
    c->alto = CUERPO_ALTO;
}

/*--- main: igual en el ejercicio y en la solución -----------------------------------*/
int main(void)
{
    Personaje tarou = {0, 0, 0, 0};  /* La struct vive aquí; las funciones reciben &tarou. */
    Caja cuerpo = {0, 0, 0, 0};      /* La rellena personaje_caja.                        */

    personaje_iniciar(&tarou, 150, 144);   /* Empieza cerca del borde derecho.            */
    printf("Inicio: x=%d vx=%d dir=%d\n", tarou.x, tarou.vx, tarou.direccion);

    for (int f = 1; f <= 9; f++)     /* 9 frames: 6 hacia la derecha y 3 a la izquierda.  */
    {
        if (f <= 6)
        {
            personaje_acelerar(&tarou, ACEL, VEL_MAX);            /* Empuja a la derecha.  */
        }
        else
        {
            personaje_acelerar(&tarou, (s16)-ACEL, VEL_MAX);      /* Empuja a la izquierda.*/
        }
        personaje_mover(&tarou, X_MAX);                           /* Avanza y recorta.     */
        printf("Frame %d: x=%d vx=%d dir=%d\n", f, tarou.x, tarou.vx, tarou.direccion);
    }

    personaje_caja(&tarou, &cuerpo);                              /* Salida en cuerpo.     */
    printf("Caja: x=%d y=%d ancho=%d alto=%d\n", cuerpo.x, cuerpo.y, cuerpo.ancho, cuerpo.alto);

    return 0;                        /* Terminó bien.                                     */
}
