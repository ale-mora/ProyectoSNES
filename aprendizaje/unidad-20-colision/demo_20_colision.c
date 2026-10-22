/*---------------------------------------------------------------------------------
    demo_20_colision.c — Demo de la unidad 20: el Tarou DE VERDAD contra una
    rejilla pequeña hecha a mano.

    PROPÓSITO
      Usa el código real del juego (src/juego/colision.c y src/juego/tarou.c,
      compilados en el PC con -DPRUEBAS_PC) sobre un escenario de juguete, y
      escribe una traza de x/y como la que se usó para ajustar el guion del
      tutorial. En orden, Tarou:
        1. empieza en el aire, cae y ATERRIZA en el suelo (fila 13, y 208);
        2. camina y CHOCA con la boca de un túnel de 48 px (de pie no cabe);
        3. se AGACHA y entra; suelta Abajo bajo el techo y sigue agachado
           (cabe_de_pie); al salir, se levanta solo;
        4. salta DESDE ABAJO a través de una plataforma de un solo sentido y
           se queda ENCIMA al bajar;
        5. camina hasta el borde, cae y CHOCA con un muro.
      Al final, la trampa de PF_ENTERO con un valor negativo.

    SE CONECTA CON
      Incluye:  <stdio.h>, <string.h> (memset), juego/colision.h,
                juego/tarou.h, juego/animaciones.h (NOMBRES_ESTADO),
                sistema/puntofijo.h.
      Compilar (desde aprendizaje/unidad-20-colision/):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../tests -I../../src
            -o demo_20.exe demo_20_colision.c ../../src/juego/colision.c
            ../../src/juego/tarou.c ../../src/juego/animaciones.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      rejilla   u8[128*14]  el escenario: 1 byte por celda de 16 px, fila a fila
      t, m      Tarou, Mando el protagonista y lo que "pulsa" la demo
      fase      u8          0..5: qué está intentando hacer la demo

    → glosario: Colisión por celdas, Resolución por ejes, Plataforma de un solo
      sentido, Punto fijo
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf.                                     */
#include <string.h>                      /* memset: rellenar bytes de golpe.            */
#include "juego/colision.h"              /* La rejilla y sus consultas.                 */
#include "juego/tarou.h"                 /* Tarou, tarou_iniciar, tarou_actualizar.     */
#include "juego/animaciones.h"           /* NOMBRES_ESTADO.                             */
#include "sistema/puntofijo.h"           /* PF, PF_ENTERO.                              */

/* colision.c calcula el índice con (fila << 7) | columna: la rejilla SIEMPRE tiene
   128 columnas, aunque el escenario use solo las primeras 30. */
static u8 rejilla[COLUMNAS_COLISION * FILAS_COLISION];

/* Pone el valor v en las celdas de las columnas c0..c1 y filas f0..f1 (incluidas). */
static void rect(u8 c0, u8 c1, u8 f0, u8 f1, u8 v)
{
    u8 f;                                /* Fila que se rellena.                        */
    for (f = f0; f <= f1; f++)
    {
        memset(rejilla + f * COLUMNAS_COLISION + c0, v, (size_t)(c1 - c0 + 1));
    }
}

/* Dibuja las filas 7..13 y las columnas 0..29 en ASCII: . vacío, # sólido, = plataforma. */
static void dibujar(void)
{
    u8 f, c;
    printf("      columnas 0..29 (16 px cada una), filas 7..13:\n");
    for (f = 7; f < FILAS_COLISION; f++)
    {
        printf("  y%3d ", f * CELDA_COLISION);           /* Píxel de arriba de la fila. */
        for (c = 0; c < 30; c++)
        {
            u8 v = rejilla[f * COLUMNAS_COLISION + c];
            putchar(v == COL_SOLIDO ? '#' : v == COL_PLATAFORMA ? '=' : '.');
        }
        putchar('\n');
    }
}

int main(void)
{
    Tarou t;                             /* El protagonista de verdad.                  */
    Mando m;                             /* El "mando" que maneja la demo.              */
    u16 cuadro;                          /* Frames desde el principio.                  */
    u8 fase = 0;                         /* Qué toca hacer ahora.                       */
    u8 quieto = 0;                       /* Frames seguidos sin moverse en x.           */
    s16 x_antes = -1;                    /* x del frame anterior.                       */
    u8 estado_antes = 0xFF, suelo_antes = 0xFF;   /* Para imprimir solo los cambios.    */
    static const char *const FASES[] = {
        "1 caer", "2 caminar", "3 agacharse", "3 soltar Abajo", "4 saltar", "5 seguir"};

    /*--- El escenario: un suelo, un túnel, una plataforma y un muro ---------------*/
    memset(rejilla, COL_VACIO, sizeof(rejilla));
    rect(0, 29, 13, 13, COL_SOLIDO);      /* Suelo: fila 13, y 208..223.                 */
    rect(10, 13, 7, 9, COL_SOLIDO);       /* Techo del túnel: x 160..223, hasta y 159:   */
                                          /* deja 48 px (160..207), como en el tutorial.  */
    rect(18, 23, 10, 10, COL_PLATAFORMA); /* Plataforma: x 288..383, borde en y 160.     */
    rect(28, 29, 10, 12, COL_SOLIDO);     /* Muro: x 448..479, 48 px de alto.            */
    dibujar();
    colision_usar(rejilla);               /* La física pregunta a ESTA rejilla.          */

    memset(&m, 0, sizeof(m));             /* Mando suelto.                               */
    tarou_iniciar(&t, 40, 124);           /* Celda en (40, 60): los pies en 124, en el   */
                                          /* aire (tarou_iniciar no comprueba el suelo).  */
    printf("\ncuadro fase            x    y  pies(y)  vx  vy  estado              suelo\n");

    for (cuadro = 0; cuadro < 700; cuadro++)
    {
        s16 x = (s16)PF_ENTERO(t.x_fp);   /* Píxel de la celda (aquí x e y son >= 0).   */

        /*--- Qué "pulsa" la demo en cada fase ------------------------------------*/
        memset(&m, 0, sizeof(m));
        quieto = (u8)(x == x_antes ? quieto + 1 : 0);   /* ¿Lleva frames sin avanzar?   */
        x_antes = x;
        if (fase == 0 && cuadro == 40) fase = 1;        /* Ya aterrizó: a caminar.      */
        if (fase == 1 && quieto == 3) fase = 2;         /* Chocó con el túnel.          */
        if (fase == 2 && x >= 120) fase = 3;            /* Ya dentro: suelta Abajo.     */
        if (fase == 3 && t.estado == ESTADO_CAMINANDO && x >= 236) fase = 4; /* A saltar. */
        if (fase == 4 && t.en_suelo && t.estado != ESTADO_SALTANDO && cuadro > 0
            && PF_ENTERO(t.y_fp) + 64 == 160) fase = 5; /* Encima de la plataforma.     */
        if (fase == 5 && quieto >= 3 && t.en_suelo) break;  /* Contra el muro: fin.     */

        m.derecha = (u8)(fase >= 1);                    /* Derecha desde la fase 1.     */
        m.abajo = (u8)(fase == 2);                      /* Abajo solo en la fase 2.     */
        if (fase == 4 && t.en_suelo && t.estado == ESTADO_CAMINANDO)
        {
            m.saltar_pulsado = 1;                       /* B recién pulsado...          */
        }
        m.saltar = (u8)(fase == 4);                     /* ...y mantenido: salto largo. */
        m.actividad = (u8)(m.derecha || m.abajo || m.saltar);

        tarou_actualizar(&t, &m);                       /* UN frame del juego real.     */

        /* Una línea cuando cambia el estado o el apoyo, y una cada 16 frames. */
        if (t.estado != estado_antes || t.en_suelo != suelo_antes || cuadro % 16 == 0)
        {
            s16 y = (s16)PF_ENTERO(t.y_fp);
            printf("%5u  %-14s %4d %4d  %4d  %3d %3d  %-19s %s\n", cuadro, FASES[fase],
                   (int)PF_ENTERO(t.x_fp), y, y + 64, t.vx_fp, t.vy_fp,
                   NOMBRES_ESTADO[t.estado], t.en_suelo ? "si" : "no");
        }
        estado_antes = t.estado;
        suelo_antes = t.en_suelo;
    }
    printf("%5u  fin: parado contra el muro en x=%d (borde derecho del cuerpo: %d + 24 + 52 = 448)\n",
           cuadro, (int)PF_ENTERO(t.x_fp), (int)PF_ENTERO(t.x_fp));

    /*--- La trampa de PF_ENTERO: solo vale para valores >= 0 ----------------------*/
    {
        s16 y_fp = -80;                   /* -5 px en 12.4 (-5 x 16): por ENCIMA de la    */
                                          /* pantalla, como en lo alto de un salto.       */
        printf("\nPF_ENTERO(-80) = %u  (-80 son -5 px; no da 5: el u16 da la vuelta)\n", PF_ENTERO(y_fp));
        printf("PF_ENTERO(y) >= 192 dice %s; y_fp >= PF(192) dice %s\n",
               PF_ENTERO(y_fp) >= 192 ? "CAYO (mal)" : "no cayo",
               y_fp >= PF(192) ? "CAYO" : "no cayo (bien)");
    }
    return 0;
}
