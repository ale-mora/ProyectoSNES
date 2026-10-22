/*---------------------------------------------------------------------------------
    demo_11_animacion.c — Demo de la unidad 11: el reloj de las animaciones de
    Tarou, con la misma regla que avanzar_animacion() de src/juego/tarou.c.

    PROPÓSITO
      1. Imprime la tabla de las 13 animaciones con los cuadros del ARTE REAL
         (res/personajes/tarou/tarou_anim.h) y los frames por cuadro de
         src/juego/animaciones.c: cuánto dura cada una en frames y segundos.
         Así se ve el problema del ejercicio: desenvainar, envainar y
         despertar duran demasiado.
      2. Traza frame a frame una animación sin bucle (desenvainar): cuadro,
         frames_cuadro y anim_terminada, y en qué frame cambia el estado.
      3. Traza una con bucle (caminar) hasta que da la vuelta.
      4. Compara con los cuadros de la COPIA FIJA de las pruebas
         (tests/tarou_anim.h): las mismas reglas dan otras duraciones.
      La regla, una vez por frame y al FINAL del frame:
        frames_cuadro++; si llega a frames_por_cuadro: frames_cuadro = 0, cuadro++;
        si cuadro llega a cuadros: con bucle vuelve a 0; sin bucle se queda en el
        último y anim_terminada = 1. El estado cambia al frame SIGUIENTE (lo mira
        manejar_espada al principio del frame).

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdint.h>.
      Modelo:   src/juego/tarou.c (avanzar_animacion, cambiar_estado, manejar_espada),
                src/juego/animaciones.c (ANIMACIONES), res/personajes/tarou/tarou_anim.h
                (cuadros reales), tests/tarou_anim.h (copia fija).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_11.exe demo_11_animacion.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      ANIMS            Anim[13]  nombre, cuadros reales, cuadros de la copia fija,
                                 frames por cuadro (arte y copia fija) y bucle
                                 (cópialos si cambias el juego)
      cuadro, frames_cuadro, anim_terminada  u8  el reloj de una animación (como en Tarou)

    → conceptos (README de la unidad 11): cuadro, frames por cuadro, bucle,
      anim_terminada, tabla indexada por estado
---------------------------------------------------------------------------------*/
#include <stdio.h>                         /* printf.                                     */
#include <stdint.h>                        /* uint8_t.                                    */

typedef uint8_t u8;                        /* El tipo del juego.                          */

#define FPS 60                             /* Frames por segundo (NTSC).                  */

typedef struct
{
    const char *nombre;                    /* Nombre de la animación (y del estado).      */
    u8 cuadros;                            /* TAROU_<ANIM>_CUADROS del arte real.         */
    u8 cuadros_fija;                       /* Los de tests/tarou_anim.h.                  */
    u8 frames_por_cuadro;                  /* Los de src/juego/animaciones.c.             */
    u8 fpc_fija;                           /* Los mismos, salvo atacar: 20 / 5 = 4.       */
    u8 bucle;                              /* 1 = vuelve a empezar.                       */
} Anim;

/* En el orden del enum Estado. Columnas: nombre, cuadros (arte), cuadros (copia fija),
   frames por cuadro (arte), frames por cuadro (copia fija), bucle. Atacar usa
   DURACION_ATAQUE / cuadros: 20 / 1 = 20 con el arte, 20 / 5 = 4 con la copia fija. */
static const Anim ANIMS[13] = {
    {"reposo (PARADO)", 9, 4, 12, 12, 1},
    {"caminar", 8, 8, 6, 6, 1},
    {"correr", 8, 8, 4, 4, 1},
    {"saltar", 3, 2, 6, 6, 0},
    {"caer", 3, 2, 6, 6, 0},
    {"agachado", 1, 2, 12, 12, 1},
    {"agachado_caminar", 8, 6, 8, 8, 1},
    {"desenvainar", 9, 4, 5, 5, 0},
    {"envainar", 9, 4, 5, 5, 0},
    {"atacar", 1, 5, 20, 4, 0},
    {"echarse", 6, 4, 8, 8, 0},
    {"dormido", 9, 4, 20, 20, 1},
    {"despertar", 10, 4, 8, 8, 0},
};

/* Frames hasta que una animación sin bucle marca anim_terminada; en las de bucle,
   lo que dura una vuelta. Las dos son cuadros x frames por cuadro. */
static unsigned duracion(u8 cuadros, u8 fpc)
{
    return (unsigned)cuadros * fpc;
}

/* Traza `total` frames de una animación desde su primer cuadro. */
static void trazar(const Anim *a, unsigned total, unsigned cada)
{
    u8 cuadro = 0, frames_cuadro = 0, anim_terminada = 0;   /* cambiar_estado: a cero.  */
    for (unsigned f = 1; f <= total; f++)
    {
        /* avanzar_animacion, al final del frame f: */
        frames_cuadro++;
        if (frames_cuadro >= a->frames_por_cuadro)
        {
            frames_cuadro = 0;
            cuadro++;
            if (cuadro >= a->cuadros)
            {
                if (a->bucle)
                {
                    cuadro = 0;            /* Bucle: vuelve al primero.                    */
                }
                else
                {
                    cuadro = (u8)(a->cuadros - 1);   /* Sin bucle: se queda en el último...*/
                    anim_terminada = 1;              /* ...y lo avisa.                     */
                }
            }
        }
        if (f % cada == 0 || anim_terminada)
        {
            printf("  fin del frame %3u: cuadro %u  frames_cuadro %u  anim_terminada %u\n",
                   f, cuadro, frames_cuadro, anim_terminada);
        }
        if (anim_terminada)
        {
            printf("  -> el frame %u (el siguiente) manejar_espada ve anim_terminada y pasa a PARADO\n",
                   f + 1);
            return;
        }
    }
}

int main(void)
{
    printf("== 1. Las 13 animaciones con el ARTE REAL ==\n");
    printf("  %-18s %7s %8s %6s %7s %8s\n", "animacion", "cuadros", "fr/cuad", "bucle", "frames", "segundos");
    for (unsigned i = 0; i < 13; i++)
    {
        const Anim *a = &ANIMS[i];
        unsigned d = duracion(a->cuadros, a->frames_por_cuadro);
        printf("  %-18s %7u %8u %6s %7u %8.2f%s\n", a->nombre, a->cuadros, a->frames_por_cuadro,
               a->bucle ? "si" : "no", d, (double)d / FPS,
               (!a->bucle && d > 40) ? "   <- larga (gesto sin bucle de mas de 2/3 s)" : "");
    }

    printf("\n== 2. desenvainar (arte real: 9 cuadros x 5 frames), frame a frame ==\n");
    trazar(&ANIMS[7], 200, 5);

    printf("\n== 3. caminar (8 cuadros x 6 frames, con bucle), una vuelta ==\n");
    trazar(&ANIMS[1], 49, 6);

    printf("\n== 4. Las mismas reglas con la COPIA FIJA de las pruebas (tests/tarou_anim.h) ==\n");
    for (unsigned i = 7; i < 13; i++)
    {
        const Anim *a = &ANIMS[i];
        printf("  %-18s arte %2u x %2u = %3u frames | copia fija %2u x %2u = %3u frames\n",
               a->nombre, a->cuadros, a->frames_por_cuadro, duracion(a->cuadros, a->frames_por_cuadro),
               a->cuadros_fija, a->fpc_fija, duracion(a->cuadros_fija, a->fpc_fija));
    }
    return 0;
}
