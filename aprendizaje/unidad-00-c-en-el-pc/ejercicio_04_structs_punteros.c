/*---------------------------------------------------------------------------------
    ejercicio_04_structs_punteros.c — Ejercicio 4 de la unidad 00: struct y punteros.

    PROPÓSITO
      Escribir cuatro funciones que reciben un personaje POR PUNTERO, igual que
      las de src/juego/tarou.c reciben "Tarou *t": iniciarlo, acelerarlo con
      un tope de velocidad, moverlo sin salirse de la pantalla y calcular su
      caja del cuerpo en un parámetro de salida. main ya está escrito: no lo
      toques; solo rellena los TODO 1 a 4.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (int16_t, uint8_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   tarou_iniciar, acelerar y tarou_caja_cuerpo en src/juego/tarou.c
                (léelas después de resolverlo: hacen lo mismo con más casos).
      Compilar: gcc -std=c99 -Wall -Wextra -o ejercicio_04.exe ejercicio_04_structs_punteros.c

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

/*
 * SALIDA ESPERADA (exacta):
 *   Inicio: x=150 vx=0 dir=0
 *   Frame 1: x=151 vx=1 dir=0
 *   Frame 2: x=153 vx=2 dir=0
 *   Frame 3: x=156 vx=3 dir=0
 *   Frame 4: x=159 vx=3 dir=0
 *   Frame 5: x=160 vx=0 dir=0
 *   Frame 6: x=160 vx=0 dir=0
 *   Frame 7: x=159 vx=-1 dir=1
 *   Frame 8: x=157 vx=-2 dir=1
 *   Frame 9: x=154 vx=-3 dir=1
 *   Caja: x=178 y=152 ancho=48 alto=56
 *
 * Antes de escribir código, comprueba a mano dos líneas: por qué en el frame
 * 4 vx sigue en 3 (el tope) y por qué en el frame 5 x vale 160 y vx 0 (el
 * borde). Si lo entiendes en papel, el código sale solo.
 */

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

/* Las líneas "(void)param;" solo callan el aviso de gcc «parámetro sin usar»
   mientras la función está vacía. Bórralas al escribir cada TODO. */

/* TODO 1: deja al personaje quieto en (x, y), mirando a la derecha: x, y, vx = 0
           y direccion = DIR_DERECHA. Usa p->campo para escribir en la struct. */
void personaje_iniciar(Personaje *p, s16 x, s16 y)
{
    (void)p; (void)x; (void)y;
}

/* TODO 2: si acel > 0, direccion = DIR_DERECHA; si acel < 0, DIR_IZQUIERDA.
           Después suma acel a vx y recórtalo: si pasa de max, vale max; si
           baja de -max, vale -max. */
void personaje_acelerar(Personaje *p, s16 acel, s16 max)
{
    (void)p; (void)acel; (void)max;
}

/* TODO 3: suma vx a x (integrar). Si x queda por debajo de 0, x = 0 y vx = 0;
           si queda por encima de x_max, x = x_max y vx = 0. */
void personaje_mover(Personaje *p, s16 x_max)
{
    (void)p; (void)x_max;
}

/* TODO 4: rellena la caja c: x = p->x + CUERPO_X, y = p->y + CUERPO_Y,
           ancho = CUERPO_ANCHO, alto = CUERPO_ALTO. p es const: solo se lee. */
void personaje_caja(const Personaje *p, Caja *c)
{
    (void)p; (void)c;
}

/*--- main: ya está escrito, no lo cambies -------------------------------------------*/
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
