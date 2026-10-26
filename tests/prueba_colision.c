/*---------------------------------------------------------------------------------
    prueba_colision.c — Pruebas de juego/colision.c y de la física de Tarou contra
    el escenario, muchas sobre la rejilla REAL del tutorial.

    PROPÓSITO
      1. colision_en: la celda correcta para cada píxel y los bordes del nivel
         (paredes a los lados, vacío arriba y abajo).
      2. Cada obstáculo del tutorial (docs/diseno-tutorial-movimiento.md §2)
         exige su movimiento: el escalón para al caminar y se sube saltando, el
         muro no se pasa con un salto corto y sí con uno largo, el túnel no se
         pasa de pie y sí agachado (y bajo el techo no se levanta), el foso no
         se cruza caminando y sí corriendo, y la plataforma se atraviesa desde
         abajo y sostiene desde arriba.
      La rejilla del nivel viene de nivel1_datos.h, que genera
      scripts/preparar_nivel.py: si alguien retoca el nivel en Tiled y rompe un
      obstáculo, estas pruebas lo dicen.

    SE CONECTA CON
      Incluye:  minitest.h, juego/colision.h, juego/tarou.h, juego/mando.h,
                sistema/puntofijo.h, nivel1_datos.h (res/nivel1, generado).
      Lo usa:   tests/pruebas.c (pruebas_colision).

    HARDWARE QUE TOCA
      ninguno (solo PC).

    VARIABLES (estáticas)
      pequena  u8[]  una rejilla hecha a mano para la plataforma
    → glosario: Colisión, Plataforma de un solo sentido
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/colision.h"
#include "juego/tarou.h"
#include "juego/mando.h"
#include "juego/nivel.h"
#include "sistema/puntofijo.h"
#include "nivel1_datos.h"

static u8 pequena[COLUMNAS_COLISION * FILAS_COLISION];

/* Tarou de pie sobre el suelo base del tutorial en x_pix, con el mando suelto. */
static void en_tutorial(Tarou *t, Mando *m, u16 x_pix)
{
    memset(m, 0, sizeof(*m));
    colision_usar(nivel1_colision);
    tarou_iniciar(t, x_pix, SUELO_Y);
}

/* Avanza hasta que x llegue a `x_pix` (o `limite` frames). Devuelve los frames. */
static u16 hasta_x(Tarou *t, const Mando *m, s16 x_pix, u16 limite)
{
    u16 i;
    for (i = 0; i < limite && PF_ENTERO(t->x_fp) < x_pix; i++)
    {
        tarou_actualizar(t, m);
    }
    return i;
}

static void avanzar(Tarou *t, const Mando *m, u16 n)
{
    u16 i;
    for (i = 0; i < n; i++)
    {
        tarou_actualizar(t, m);
    }
}

/* Salta: pulsa B un frame (y lo mantiene si `largo`). */
static void saltar_ya(Tarou *t, Mando *m, u8 largo)
{
    m->saltar_pulsado = 1; m->saltar = 1; m->actividad = 1;
    tarou_actualizar(t, m);
    m->saltar_pulsado = 0;
    m->saltar = largo;
}

static void prueba_celdas(void)
{
    memset(pequena, COL_VACIO, sizeof(pequena));
    pequena[(2 << 7) | 3] = COL_SOLIDO;              /* Fila 2, columna 3.                  */
    colision_usar(pequena);
    COMPROBAR(colision_en(48, 32) == COL_SOLIDO);    /* Esquina de arriba a la izquierda.   */
    COMPROBAR(colision_en(63, 47) == COL_SOLIDO);    /* Esquina de abajo a la derecha.      */
    COMPROBAR(colision_en(64, 32) == COL_VACIO);     /* Un píxel más: otra celda.           */
    COMPROBAR(colision_en(47, 32) == COL_VACIO);
    COMPROBAR(colision_en(2048, 100) == COL_SOLIDO); /* Más allá del borde derecho: pared.  */
    COMPROBAR(colision_en((u16)-1, 100) == COL_SOLIDO); /* "x = -1": pared también.         */
    COMPROBAR(colision_en(100, 224) == COL_VACIO);   /* Debajo de la pantalla: vacío.       */
    COMPROBAR(colision_en(100, (u16)-5) == COL_VACIO);  /* Encima: vacío.                   */
    COMPROBAR(colision_apoya(48, 32) == 1);
    /* Consultas de una columna y de una fila enteras. */
    COMPROBAR(colision_columna_solida(50, 0, 31) == 0);    /* Encima de la celda: nada.    */
    COMPROBAR(colision_columna_solida(50, 0, 32) == 1);    /* Toca su primera fila.        */
    COMPROBAR(colision_columna_solida(50, 47, 200) == 1);
    COMPROBAR(colision_columna_solida(50, 48, 200) == 0);  /* Ya por debajo.               */
    COMPROBAR(colision_columna_solida(50, -40, 40) == 1);  /* Desde encima de la pantalla. */
    COMPROBAR(colision_columna_solida(2100, 0, 10) == 1);  /* Fuera del nivel: pared.      */
    COMPROBAR(colision_fila_es(0, 47, 40, COL_SOLIDO) == 0);
    COMPROBAR(colision_fila_es(0, 48, 40, COL_SOLIDO) == 1);
    COMPROBAR(colision_fila_es(60, 300, 40, COL_SOLIDO) == 1);
    COMPROBAR(colision_fila_es(64, 300, 40, COL_SOLIDO) == 0);
    COMPROBAR(colision_fila_es(0, 300, -3, COL_SOLIDO) == 0); /* Encima de la pantalla.    */
    COMPROBAR(colision_fila_es(0, 300, 40, COL_ZARZA) == 0);
    colision_bloque(100, 20, 32, 40);                /* Un bloque: x 100..131, y 20..59.    */
    COMPROBAR(colision_columna_solida(110, 0, 19) == 0);
    COMPROBAR(colision_columna_solida(110, 0, 20) == 1);
    COMPROBAR(colision_columna_solida(132, 0, 100) == 0);
    COMPROBAR(colision_en(131, 59) == COL_SOLIDO);
    colision_bloque(0, 0, 0, 0);
    COMPROBAR(colision_en(131, 59) == COL_VACIO);
    pequena[(2 << 7) | 3] = COL_PLATAFORMA;
    COMPROBAR(colision_solido(48, 32) == 0);         /* La plataforma no es pared...        */
    COMPROBAR(colision_apoya(48, 32) == 1);          /* ...pero sostiene.                   */
    COMPROBAR(colision_columna_solida(50, 0, 100) == 0);
}

static void prueba_nivel_real(void)
{
    COMPROBAR(NIVEL1_COLS == COLUMNAS_COLISION);
    COMPROBAR(NIVEL1_FILAS == FILAS_COLISION);
    colision_usar(nivel1_colision);
    COMPROBAR(colision_en(10, SUELO_Y) == COL_SOLIDO);     /* Suelo base.                  */
    COMPROBAR(colision_en(10, SUELO_Y - 1) == COL_VACIO);  /* Aire justo encima.           */
}

/* Desde el inicio real del .tmj, caminando a la derecha llega al primer escalón. */
static void prueba_inicio(void)
{
    Tarou t; Mando m;
    COMPROBAR(NIVEL1_INICIO_X >= ANCHO_CELDA / 2);  /* La celda no empieza en x negativa.  */
    en_tutorial(&t, &m, NIVEL1_INICIO_X - ANCHO_CELDA / 2);
    m.derecha = 1; m.actividad = 1;
    avanzar(&t, &m, 300);
    COMPROBAR(PF_ENTERO(t.x_fp) == 336 - 24 - 52);   /* Contra el escalón (x 260).          */
}

static void prueba_escalon(void)
{
    Tarou t; Mando m;
    en_tutorial(&t, &m, 200);
    m.derecha = 1; m.actividad = 1;
    avanzar(&t, &m, 150);
    COMPROBAR(PF_ENTERO(t.x_fp) == 336 - 24 - 52);   /* Parado contra el escalón (x 260).   */
    COMPROBAR(t.en_suelo == 1);
    saltar_ya(&t, &m, 1);                            /* Salto largo, sin soltar Derecha.    */
    avanzar(&t, &m, 60);
    COMPROBAR(t.en_suelo == 1);
    COMPROBAR(PF_ENTERO(t.y_fp) + ALTO_CELDA == 160 || PF_ENTERO(t.y_fp) + ALTO_CELDA == 144);
}

static void prueba_bajar_de_un_borde(void)
{
    Tarou t; Mando m;
    en_tutorial(&t, &m, 400);                        /* Sobre el escalón alto (384..431)?   */
    colision_usar(nivel1_colision);
    t.y_fp = PF(144 - ALTO_CELDA);                   /* Lo ponemos encima, de pie.          */
    m.derecha = 1; m.actividad = 1;
    avanzar(&t, &m, 30);                             /* Camina hacia el borde y cae...      */
    avanzar(&t, &m, 60);
    COMPROBAR(t.en_suelo == 1);
    COMPROBAR(PF_ENTERO(t.y_fp) + ALTO_CELDA == SUELO_Y); /* ...hasta el suelo base.        */
}

static void prueba_muro(void)
{
    Tarou t; Mando m;
    /* Pegado al muro (592..639, 48 px), un salto corto no llega arriba. */
    en_tutorial(&t, &m, 470);
    m.derecha = 1; m.actividad = 1;
    avanzar(&t, &m, 80);
    COMPROBAR(PF_ENTERO(t.x_fp) == 592 - 24 - 52);   /* Parado contra el muro (x 516).      */
    saltar_ya(&t, &m, 0);
    avanzar(&t, &m, 80);
    COMPROBAR(PF_ENTERO(t.x_fp) == 592 - 24 - 52);   /* Sigue a la izquierda del muro.      */
    /* Manteniendo B, sí: sube, cae encima del muro y sigue andando. (Saltar caminando
       DESDE LEJOS no basta: a 1 px por cuadro, cuando el pie llega encima del muro ya
       viene bajando por debajo de su borde. Corriendo, sí.) */
    saltar_ya(&t, &m, 1);
    avanzar(&t, &m, 200);                            /* Cae encima del muro y lo cruza.     */
    COMPROBAR(PF_ENTERO(t.x_fp) > 640);              /* Al otro lado.                       */
    COMPROBAR(t.en_suelo == 1);
}

static void prueba_tunel(void)
{
    Tarou t; Mando m;
    /* De pie: para en la boca del túnel (el techo empieza en x 848). */
    en_tutorial(&t, &m, 740);
    m.derecha = 1; m.actividad = 1;
    avanzar(&t, &m, 120);
    COMPROBAR(PF_ENTERO(t.x_fp) == 848 - 24 - 52);
    /* Agachado: pasa. Y soltando Abajo en medio, sigue agachado. */
    m.abajo = 1;
    hasta_x(&t, &m, 880, 400);
    COMPROBAR(PF_ENTERO(t.x_fp) >= 880);
    m.abajo = 0;                                     /* Suelta Abajo bajo el techo...       */
    avanzar(&t, &m, 5);
    COMPROBAR(t.estado == ESTADO_AGACHADO || t.estado == ESTADO_AGACHADO_CAMINANDO);
    m.saltar_pulsado = 1;                            /* ...y tampoco puede saltar.          */
    avanzar(&t, &m, 1);
    m.saltar_pulsado = 0;
    COMPROBAR(t.en_suelo == 1);
    hasta_x(&t, &m, 960, 400);                       /* Sale por el otro lado (944).        */
    avanzar(&t, &m, 3);
    COMPROBAR(t.estado == ESTADO_CAMINANDO);         /* Fuera del túnel ya se levanta.      */
}

/* Foso de 1120..1215: se salta en x 1080 (el pie de atrás aún en tierra). */
static void prueba_foso(void)
{
    Tarou t; Mando m;
    en_tutorial(&t, &m, 1000);                       /* Caminando: cae dentro.              */
    m.derecha = 1; m.actividad = 1;
    hasta_x(&t, &m, 1080, 200);
    saltar_ya(&t, &m, 1);
    avanzar(&t, &m, 120);
    COMPROBAR(t.en_suelo == 0);
    COMPROBAR(PF_ENTERO(t.y_fp) > 224);              /* Por debajo de la pantalla.          */

    en_tutorial(&t, &m, 960);                        /* Corriendo: lo cruza.                */
    m.derecha = 1; m.correr = 1; m.actividad = 1;
    hasta_x(&t, &m, 1080, 200);
    saltar_ya(&t, &m, 1);
    avanzar(&t, &m, 70);
    COMPROBAR(t.en_suelo == 1);
    COMPROBAR(PF_ENTERO(t.y_fp) + ALTO_CELDA == SUELO_Y);
    COMPROBAR(PF_ENTERO(t.x_fp) > 1152);
}

static void prueba_plataforma(void)
{
    Tarou t; Mando m;
    memset(pequena, COL_VACIO, sizeof(pequena));
    memset(pequena + 13 * COLUMNAS_COLISION, COL_SOLIDO, COLUMNAS_COLISION); /* Suelo, 208. */
    memset(pequena + 10 * COLUMNAS_COLISION + 4, COL_PLATAFORMA, 6);  /* x 64..159, y 160.  */
    memset(&m, 0, sizeof(m));
    colision_usar(pequena);
    tarou_iniciar(&t, 60, 208);                      /* Debajo de la plataforma.            */
    saltar_ya(&t, &m, 1);                            /* La atraviesa subiendo...            */
    avanzar(&t, &m, 60);
    COMPROBAR(t.en_suelo == 1);
    COMPROBAR(PF_ENTERO(t.y_fp) + ALTO_CELDA == 160); /* ...y se queda encima al bajar.     */
    m.derecha = 1; m.actividad = 1;                  /* Andando sobre ella no la atraviesa. */
    avanzar(&t, &m, 20);
    COMPROBAR(t.en_suelo == 1);
}

void pruebas_colision(void)
{
    prueba_celdas();
    prueba_nivel_real();
    prueba_inicio();
    prueba_escalon();
    prueba_bajar_de_un_borde();
    prueba_muro();
    prueba_tunel();
    prueba_foso();
    prueba_plataforma();
}
