/*---------------------------------------------------------------------------------
    demo_10_salto.c — Demo de la unidad 10: el salto de Tarou frame a frame, en
    punto fijo 12.4, con el mismo orden de operaciones que src/juego/tarou.c.

    PROPÓSITO
      Simula un salto sobre suelo plano (la fila 208, como tests/prueba_tarou.c)
      para varias parejas (VEL_SALTO, GRAVEDAD) e imprime:
        1. la tabla frame a frame del salto del juego (-96, 4): vy, y, altura;
        2. un resumen por pareja: frame del apogeo, altura máxima (exacta, en
           dieciseisavos), y mínima en píxeles (lo que mira la prueba
           prueba_apogeo_y_altura), frame de aterrizaje, frames en el aire y
           cuántos px avanza en ese tiempo caminando (1 px/frame) y corriendo
           (2 px/frame);
        3. qué obstáculos del tutorial permite: el muro de 48 px y el foso,
           que exige 68 px en el aire (docs/diseno-tutorial-movimiento.md §2);
        4. el salto corto (soltar B en el frame 2: VEL_SALTO_CORTE).
      Orden de cada frame en el aire (tarou_actualizar, paso 2 y mover_y):
        corte del salto -> vy += GRAVEDAD -> recorte a VEL_TERMINAL -> y += vy
        -> ¿los pies cruzan el suelo? aterrizar.
      El frame del despegue ya cuenta: saltar() pone vy = VEL_SALTO y en ese
      mismo frame actúa la gravedad (vy = -92 con los valores del juego).

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdint.h>.
      Modelo:   src/juego/tarou.c (saltar, paso 2 de tarou_actualizar, mover_y),
                src/juego/tarou.h (constantes), tests/prueba_tarou.c (las cuentas
                que esta demo debe reproducir EXACTAMENTE).
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_10.exe demo_10_salto.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES (de la struct Salto)
      y_fp, vy_fp   s16   punto fijo 12.4   posición (esquina de la celda) y velocidad
      frame         u16   1..               frames desde el despegue (el despegue es el 1)
      en_suelo      u8    0 / 1             1 cuando aterrizó

    → conceptos (README de la unidad 10): velocidad, gravedad, integrar, apogeo,
      corte del salto, velocidad terminal
---------------------------------------------------------------------------------*/
#include <stdio.h>                         /* printf.                                     */
#include <stdint.h>                        /* int16_t, uint16_t, uint8_t.                 */

typedef int16_t s16;                       /* Los tipos del juego (src/sistema/tipos.h).  */
typedef uint16_t u16;
typedef uint8_t u8;

#define PF(n) ((s16)((n) * 16))            /* Píxeles -> punto fijo 12.4.                 */
#define PF_ENTERO(x) ((s16)((x) >> 4))     /* Punto fijo -> píxel (solo valores >= 0).    */

#define SUELO 208                          /* Fila del suelo plano de las pruebas.        */
#define ALTO_CELDA 64                      /* La celda de Tarou: los pies en y + 64.      */
#define Y_INICIO (SUELO - ALTO_CELDA)      /* 144: de pie en el suelo.                    */
#define VEL_SALTO_CORTE (-32)              /* Igual que en tarou.h.                       */
#define VEL_TERMINAL PF(6)                 /* 96.                                         */
#define MURO 48                            /* Alto del muro del tutorial, en px.          */
#define FOSO_AIRE 68                       /* px que hay que recorrer en el aire en el foso.*/

typedef struct
{
    s16 y_fp, vy_fp;                       /* Posición y velocidad vertical.              */
    u16 frame;                             /* Frames desde el despegue.                   */
    u8 en_suelo;                           /* 1 = aterrizó.                               */
} Salto;

/* Un frame en el aire. `boton` = 1 si B sigue mantenido. Devuelve 1 al aterrizar. */
static u8 paso(Salto *s, s16 gravedad, u8 boton)
{
    s16 pies_antes = (s16)(PF_ENTERO(s->y_fp) + ALTO_CELDA);   /* Pies antes de moverse. */
    s->frame++;
    if (!boton && s->vy_fp < VEL_SALTO_CORTE)  /* Soltar B subiendo recorta la subida.    */
    {
        s->vy_fp = VEL_SALTO_CORTE;
    }
    s->vy_fp = (s16)(s->vy_fp + gravedad);     /* Gravedad: cada frame cae más rápido...  */
    if (s->vy_fp > VEL_TERMINAL)
    {
        s->vy_fp = VEL_TERMINAL;               /* ...hasta la velocidad terminal.         */
    }
    s->y_fp = (s16)(s->y_fp + s->vy_fp);       /* Integrar: posición += velocidad.        */
    if (s->vy_fp >= 0 && pies_antes <= SUELO && PF_ENTERO(s->y_fp) + ALTO_CELDA >= SUELO)
    {
        s->y_fp = PF(Y_INICIO);                /* Los pies, justo sobre el suelo.         */
        s->vy_fp = 0;
        s->en_suelo = 1;                       /* Aterrizó.                               */
        return 1;
    }
    return 0;
}

/* Salta con (vel, gravedad), con B mantenido hasta el frame `soltar` (0 = nunca).
   Si `tabla` vale 1, imprime cada frame. Devuelve por punteros el resumen. */
static void saltar(s16 vel, s16 gravedad, u16 soltar, u8 tabla,
                   u16 *f_apogeo, s16 *y_min_fp, u16 *f_suelo)
{
    Salto s = {PF(Y_INICIO), vel, 0, 0};   /* saltar(): vy = VEL_SALTO, en el aire.      */
    *y_min_fp = s.y_fp;
    *f_apogeo = 0;
    while (!s.en_suelo && s.frame < 200)
    {
        u8 boton = (u8)(soltar == 0 || s.frame + 1 < soltar);  /* B, hasta `soltar`.   */
        paso(&s, gravedad, boton);
        if (s.y_fp < *y_min_fp)
        {
            *y_min_fp = s.y_fp;            /* Lo más alto (y más pequeña).               */
            *f_apogeo = s.frame;
        }
        if (tabla)
        {
            printf("  frame %2u  vy=%4d  y_fp=%5d  y=%3d px  altura=%6.2f px%s\n",
                   s.frame, s.vy_fp, s.y_fp, PF_ENTERO(s.y_fp),
                   (PF(Y_INICIO) - s.y_fp) / 16.0, s.en_suelo ? "  <- aterriza" : "");
        }
    }
    *f_suelo = s.frame;                    /* Frame en que aterrizó.                     */
}

/* Una línea de resumen para una pareja. */
static void resumen(s16 vel, s16 gravedad)
{
    u16 f_apogeo, f_suelo;
    s16 y_min_fp;
    double altura;
    saltar(vel, gravedad, 0, 0, &f_apogeo, &y_min_fp, &f_suelo);
    altura = (PF(Y_INICIO) - y_min_fp) / 16.0;             /* En píxeles, con fracción.  */
    printf("VEL_SALTO %4d GRAVEDAD %d: apogeo en el frame %2u, altura %6.2f px "
           "(y_fp min %4d, y min %3d px), aterriza en el frame %2u\n",
           vel, gravedad, f_apogeo, altura, y_min_fp, PF_ENTERO(y_min_fp), f_suelo);
    printf("    en el aire %2u frames: caminando %2u px, corriendo %3u px | muro de %d px: %s | "
           "foso (%d px en el aire) corriendo: %s\n",
           f_suelo, f_suelo, 2u * f_suelo, MURO, altura >= MURO ? "SI" : "NO",
           FOSO_AIRE, 2u * f_suelo >= FOSO_AIRE ? "SI" : "NO");
}

int main(void)
{
    u16 a, b;
    s16 c;

    printf("== 1. El salto del juego (VEL_SALTO -96, GRAVEDAD 4), frame a frame ==\n");
    saltar(-96, 4, 0, 1, &a, &c, &b);

    printf("\n== 2 y 3. Resumen de varias parejas ==\n");
    resumen(-96, 4);                       /* El juego hoy.                              */
    resumen(-80, 4);                       /* Salto flojo: ¿pasa el muro?                */
    resumen(-112, 4);                      /* Salto alto y largo.                        */
    resumen(-120, 6);                      /* Misma altura, más rápido (ejercicio 10).   */

    printf("\n== 4. Salto corto: soltar B en el frame 2 (VEL_SALTO -96, GRAVEDAD 4) ==\n");
    saltar(-96, 4, 2, 1, &a, &c, &b);
    printf("  altura maxima %.2f px, aterriza en el frame %u\n", (PF(Y_INICIO) - c) / 16.0, b);
    return 0;
}
