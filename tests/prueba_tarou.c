/*---------------------------------------------------------------------------------
    prueba_tarou.c — Pruebas de juego/tarou.c: la física y la máquina de estados
    del protagonista, frame a frame, con los valores de la spec (§4 y §5).

    PROPÓSITO
      Cada prueba crea un Tarou en reposo, le da un mando con una intención y
      avanza N frames con tarou_actualizar(); después comprueba velocidad,
      posición y estado. Son los números de docs/diseno-demo-jugable.md §5:
      ACEL_SUELO 2, VEL_CAMINAR_MAX 16, VEL_CORRER_MAX 32, FRENADO_SUELO 2...
      Si se afina una constante, se afina aquí la cuenta.

    SE CONECTA CON
      Incluye:  minitest.h, juego/tarou.h, juego/mando.h, juego/nivel.h,
                sistema/puntofijo.h, <string.h>.
      Lo usan:  tests/pruebas.c (llama a pruebas_tarou).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (cada prueba usa un Tarou y un Mando locales).
    → glosario: Estado, Punto fijo, Velocidad, aceleración y frenado
---------------------------------------------------------------------------------*/
#include <string.h>
#include "minitest.h"
#include "juego/tarou.h"
#include "juego/mando.h"
#include "juego/nivel.h"
#include "sistema/puntofijo.h"

#define SUELO 208                       /* El suelo plano de la Revisión 1.           */

/* Avanza `n` frames con el mismo mando (el suelo es plano: siempre SUELO). */
static void frames(Tarou *t, const Mando *m, u16 n)
{
    u16 i;
    for (i = 0; i < n; i++)
    {
        tarou_actualizar(t, m, SUELO);
    }
}

/* Un Tarou recién iniciado en x_pix y un mando sin nada pulsado. */
static void preparar(Tarou *t, Mando *m, u16 x_pix)
{
    memset(m, 0, sizeof(*m));
    tarou_iniciar(t, x_pix, SUELO);
}

static void prueba_inicio(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 100);
    COMPROBAR(t.estado == ESTADO_PARADO);
    COMPROBAR(t.x_fp == PF(100));
    COMPROBAR(t.y_fp == PF(SUELO - ANCHO_CELDA));       /* 144: los pies en la fila 207.*/
    COMPROBAR(t.vx_fp == 0 && t.vy_fp == 0);
    COMPROBAR(t.espada == ESPADA_ENVAINADA);
    COMPROBAR(t.direccion == DIR_DERECHA);
    COMPROBAR(t.en_suelo == 1);
}

static void prueba_caminar(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 100);
    m.derecha = 1; m.actividad = 1;
    frames(&t, &m, 1);
    COMPROBAR(t.estado == ESTADO_CAMINANDO);
    COMPROBAR(t.vx_fp == ACEL_SUELO);                   /* Primer frame: 2.              */
    frames(&t, &m, 7);                                  /* 8 frames en total...          */
    COMPROBAR(t.vx_fp == VEL_CAMINAR_MAX);              /* ...llega a 16 y se queda.     */
    frames(&t, &m, 12);                                 /* 20 frames en total.           */
    COMPROBAR(t.vx_fp == VEL_CAMINAR_MAX);
    COMPROBAR(t.x_fp == PF(100) + 264);                 /* 2+4+...+16 + 12*16 = 264.     */
    COMPROBAR(t.direccion == DIR_DERECHA);
}

static void prueba_correr_y_soltar(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 100);
    m.derecha = 1; m.correr = 1; m.actividad = 1;
    frames(&t, &m, 16);
    COMPROBAR(t.estado == ESTADO_CORRIENDO);
    COMPROBAR(t.vx_fp == VEL_CORRER_MAX);               /* 2 * 16 = 32.                  */
    m.correr = 0;                                       /* Soltar Y, seguir a la derecha.*/
    frames(&t, &m, 1);
    COMPROBAR(t.estado == ESTADO_CAMINANDO);
    COMPROBAR(t.vx_fp == VEL_CORRER_MAX - FRENADO_SUELO); /* Frena 2 por frame...        */
    frames(&t, &m, 7);
    COMPROBAR(t.vx_fp == VEL_CAMINAR_MAX);              /* ...hasta la de caminar.       */
    frames(&t, &m, 12);
    COMPROBAR(t.vx_fp == VEL_CAMINAR_MAX);              /* Y ahí se queda.               */
}

static void prueba_frenar_hasta_parar(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 100);
    m.derecha = 1; m.actividad = 1;
    frames(&t, &m, 8);                                  /* vx = 16.                      */
    m.derecha = 0; m.actividad = 0;                     /* Soltar todo.                  */
    frames(&t, &m, 7);
    COMPROBAR(t.vx_fp == 2);                            /* 16 - 7*2 = 2: todavía anda.   */
    COMPROBAR(t.estado == ESTADO_CAMINANDO);
    frames(&t, &m, 1);
    COMPROBAR(t.vx_fp == 0);                            /* Frame 8: se detiene...        */
    COMPROBAR(t.estado == ESTADO_PARADO);               /* ...y pasa a PARADO.           */
}

static void prueba_izquierda(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 100);
    m.izquierda = 1; m.actividad = 1;
    frames(&t, &m, 3);
    COMPROBAR(t.direccion == DIR_IZQUIERDA);
    COMPROBAR(t.vx_fp == -6);
    COMPROBAR(t.x_fp == PF(100) - 2 - 4 - 6);
}

/* Punto de atención 1: en el borde izquierdo no hay desbordamiento. */
static void prueba_borde_izquierdo(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 0);
    m.izquierda = 1; m.actividad = 1;
    frames(&t, &m, 30);
    COMPROBAR(t.x_fp == 0);                             /* Recortado a 0...              */
    COMPROBAR(t.vx_fp == 0);                            /* ...y sin velocidad acumulada. */
    COMPROBAR(PF_ENTERO(t.x_fp) == 0);
}

static void prueba_borde_derecho(void)
{
    Tarou t; Mando m;
    preparar(&t, &m, 440);
    m.derecha = 1; m.correr = 1; m.actividad = 1;
    frames(&t, &m, 60);
    COMPROBAR(t.x_fp == PF(ANCHO_NIVEL - ANCHO_CELDA)); /* 448: el borde.                */
    COMPROBAR(t.vx_fp == 0);
}

void pruebas_tarou(void)
{
    prueba_inicio();
    prueba_caminar();
    prueba_correr_y_soltar();
    prueba_frenar_hasta_parar();
    prueba_izquierda();
    prueba_borde_izquierdo();
    prueba_borde_derecho();
}
