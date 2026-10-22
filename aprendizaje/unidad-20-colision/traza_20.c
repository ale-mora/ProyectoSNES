/*---------------------------------------------------------------------------------
    traza_20.c — Herramienta de la unidad 20: juega el tutorial con el guion, en el
    PC, e imprime la traza de x/y cuadro a cuadro.

    PROPÓSITO
      Hace exactamente lo mismo que tests/prueba_recorrido.c (rejilla REAL del
      nivel, guion de src/juego/guion_tutorial.c, mando del guion →
      tarou_actualizar → partida_actualizar), pero en vez de comprobar, CUENTA
      lo que pasa. Es la herramienta con la que se ajusta el guion cuando se
      retoca el nivel en Tiled:
        traza_20.exe              solo los cuadros "interesantes": cambia el
                                  estado, los botones o el apoyo, o hay evento.
        traza_20.exe 1990 2060    todos los cuadros de ese tramo.
      Al final dice si llegó a la salida (y en qué cuadro, para
      GUION_TUTORIAL_FIN) o en qué cuadro tuvo que reaparecer.

    SE CONECTA CON
      Incluye:  <stdio.h>, <stdlib.h> (atoi), juego/guion_tutorial.h,
                juego/partida.h, juego/colision.h, juego/tarou.h, juego/nivel.h,
                juego/animaciones.h (NOMBRES_ESTADO), sistema/puntofijo.h,
                nivel1_datos.h (la rejilla y los objetos, generados).
      Compilar (desde aprendizaje/unidad-20-colision/; -I../../tests va PRIMERO
      para usar la misma copia fija de tarou_anim.h que las pruebas):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../tests -I../../src -I../../res/nivel1
            -o traza_20.exe traza_20.c ../../src/juego/colision.c ../../src/juego/tarou.c
            ../../src/juego/animaciones.c ../../src/juego/partida.c
            ../../src/juego/guion.c ../../src/juego/guion_tutorial.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      t, m, p         Tarou, Mando, Partida   lo mismo que en pantalla_nivel1.c
      desde, hasta    u16                     tramo que se imprime entero (0, 0 = ninguno)
      antes_*         u8                      lo del cuadro anterior, para ver cambios

    → glosario: Guion de mando, Colisión por celdas, Punto de control
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf.                                      */
#include <stdlib.h>                      /* atoi: el texto de los argumentos, a número.  */
#include "juego/guion_tutorial.h"        /* GUION_TUTORIAL, sus tramos y su FIN.         */
#include "juego/partida.h"               /* Partida, partida_actualizar, PARTIDA_*.      */
#include "juego/colision.h"              /* colision_usar.                               */
#include "juego/tarou.h"                 /* Tarou, tarou_actualizar.                     */
#include "juego/nivel.h"                 /* SUELO_Y, ANCHO_CELDA.                        */
#include "juego/animaciones.h"           /* NOMBRES_ESTADO: "PARADO", "CAMINANDO"...     */
#include "sistema/puntofijo.h"           /* PF_ENTERO.                                   */
#include "nivel1_datos.h"                /* nivel1_colision y NIVEL1_* (generados).      */

static const u16 controles[] = NIVEL1_CONTROLES_X;   /* x de los carteles.                */

/* Los botones del mando como letras: < > v (abajo) Y (correr) B (saltar), y en
   minúscula los recién pulsados b (saltar) a (atacar) r (espada). '.' = suelto. */
static void botones(const Mando *m, char *s)
{
    s[0] = m->izquierda ? '<' : '.';     /* Cada posición, un botón.                     */
    s[1] = m->derecha ? '>' : '.';
    s[2] = m->abajo ? 'v' : '.';
    s[3] = m->correr ? 'Y' : '.';
    s[4] = m->saltar ? 'B' : '.';
    s[5] = m->saltar_pulsado ? 'b' : '.';
    s[6] = m->atacar_pulsado ? 'a' : '.';
    s[7] = m->espada_pulsado ? 'r' : '.';
    s[8] = '\0';                         /* Fin de la cadena.                            */
}

/* Una línea de la traza: cuadro, botones, celda (x, y), pies, velocidades, estado. */
static void linea(u16 cuadro, const Mando *m, const Tarou *t, const char *evento)
{
    char b[9];                           /* 8 letras + el '\0'.                          */
    s16 x = (s16)(t->x_fp >> 4);         /* Píxel con signo (y puede ser negativa).      */
    s16 y = (s16)(t->y_fp >> 4);
    botones(m, b);
    printf("%5u %s x=%4d y=%4d pies=%4d..%4d vx=%4d vy=%4d %-19s %s %s\n",
           cuadro, b, x, y, x + 36, x + 64, t->vx_fp, t->vy_fp,
           NOMBRES_ESTADO[t->estado], t->en_suelo ? "suelo" : "aire ", evento);
}

int main(int argc, char **argv)
{
    Tarou t;                             /* El protagonista.                             */
    Mando m;                             /* Lo que dice el guion en cada cuadro.         */
    Partida p;                           /* Puntos de control, tronco, salida.           */
    u16 cuadro;                          /* Reloj: cuadros desde que empieza el nivel.   */
    u16 desde = 0, hasta = 0;            /* Tramo que se imprime entero (opcional).      */
    u8 e = PARTIDA_SIGUE;                /* Evento del cuadro.                           */
    u8 antes_estado = 0xFF, antes_suelo = 0xFF, antes_botones = 0xFF;
    u8 ahora_botones;                    /* Los botones del cuadro, en un byte.          */
    const char *nombre;                  /* Nombre del evento, para imprimirlo.          */

    if (argc == 3)                       /* "traza_20.exe 1990 2060": un tramo entero.   */
    {
        desde = (u16)atoi(argv[1]);
        hasta = (u16)atoi(argv[2]);
    }

    colision_usar(nivel1_colision);      /* La rejilla real, como pantalla_nivel1.c.     */
    partida_iniciar(&p, NIVEL1_INICIO_X, controles, NIVEL1_CONTROLES, NIVEL1_TRONCO_X, NIVEL1_SALIDA_X);
    tarou_iniciar(&t, NIVEL1_INICIO_X - ANCHO_CELDA / 2, SUELO_Y);
    printf("cuadro botones  celda          pies (x)      vx/vy en 1/16 px   estado\n");

    for (cuadro = 0; cuadro < GUION_TUTORIAL_FIN + 300; cuadro++)
    {
        guion_mando(GUION_TUTORIAL, GUION_TUTORIAL_TRAMOS, cuadro, &m);  /* El "mando". */
        tarou_actualizar(&t, &m);        /* Física y estados contra la rejilla.          */
        e = partida_actualizar(&p, &t);  /* Las reglas del tutorial.                     */

        nombre = "";                     /* ¿Qué evento hubo?                            */
        if (e == PARTIDA_REAPARECER) nombre = "<- REAPARECER";
        if (e == PARTIDA_TRONCO_ROTO) nombre = "<- TRONCO ROTO";
        if (e == PARTIDA_SALIDA) nombre = "<- SALIDA";

        /* Los botones en un byte, para ver si cambiaron respecto al cuadro anterior. */
        ahora_botones = (u8)(m.izquierda | m.derecha << 1 | m.abajo << 2 | m.correr << 3
                             | m.saltar << 4 | m.atacar_pulsado << 5 | m.espada_pulsado << 6);
        if ((cuadro >= desde && cuadro <= hasta && hasta != 0)    /* Dentro del tramo,   */
            || t.estado != antes_estado || t.en_suelo != antes_suelo /* o algo cambió.     */
            || ahora_botones != antes_botones || e != PARTIDA_SIGUE)
        {
            linea(cuadro, &m, &t, nombre);
        }
        antes_estado = t.estado;         /* Para comparar en el cuadro siguiente.        */
        antes_suelo = t.en_suelo;
        antes_botones = ahora_botones;

        if (e == PARTIDA_REAPARECER || e == PARTIDA_SALIDA)
        {
            break;                       /* Como prueba_recorrido: se para aquí.         */
        }
    }

    if (e == PARTIDA_SALIDA)             /* El resumen: lo que hay que copiar al .h.     */
    {
        printf("SALIDA en el cuadro %u (GUION_TUTORIAL_FIN vale %u). Ultimo control: x %u\n",
               cuadro, GUION_TUTORIAL_FIN, p.reaparicion);
    }
    else if (e == PARTIDA_REAPARECER)
    {
        printf("REAPARECER en el cuadro %u: el guion no supera el obstaculo de x %d.\n",
               cuadro, (int)(t.x_fp >> 4) + 48);
    }
    else
    {
        printf("Sin salida ni reaparicion en %u cuadros: el guion se queda corto.\n", cuadro);
    }
    return 0;                            /* Siempre 0: es una herramienta, no una prueba. */
}
