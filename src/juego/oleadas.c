/*---------------------------------------------------------------------------------
    oleadas.c — La máquina de estados de las zonas (ver oleadas.h).

    PROPÓSITO
      Dos estados por zona: LIBRE (la cámara avanza hasta el punto de la zona) y
      BLOQUEADA (oleada viva: la cámara no se mueve). Llegar al punto bloquea;
      oleadas_vencer libera y pasa a la zona siguiente. Los puntos son posiciones de la
      cámara (píxeles del escenario). Cada punto se recorta a CAMARA_MAX con
      oleadas_alcanzable: la cámara nunca pasa de ahí, y un punto mayor (p. ej. 1800) no
      se alcanzaría nunca y la zona no saldría. Desde el 2026-11-05 hay una tabla por
      escenario (aldea, bosque, monasterio, dojo) con sus puntos y sus oleadas.

    SE CONECTA CON
      Incluye:  juego/oleadas.h, juego/camara.h (CAMARA_MAX), enemigos/ninja.h (NINJA_*).
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_oleadas.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      PUNTOS   u16[4][4]    posición de la cámara en que se bloquea cada zona
      ZONAS    u8[4]        cuántas zonas tiene cada escenario
      ANCHO    u16[4]       el ancho de cada escenario
      OLEADA   u8[4][4][4]  los tipos de cada oleada (3) y cuántos son (el cuarto byte)
    → glosario: Zona, Oleada
---------------------------------------------------------------------------------*/
#include "juego/oleadas.h"
#include "juego/camara.h"
#include "enemigos/ninja.h"

#define N NINJA_NORMAL               /* Abreviaturas para que la tabla quepa a la vista.   */
#define M NINJA_MEDIANO
#define G NINJA_GRANDE
#define R NINJA_RAPIDO
#define K NINJA_KENDOKA
#define J NINJA_JEFE

static const u16 PUNTOS[ESCENARIOS][OLEADAS_ZONAS] = {
    {256, 640, 1024, 1536},          /* Aldea (2048 px): las cuatro de siempre.            */
    {192, 480, 768, 0},              /* Bosque: ninjas rápidos entre el bambú.             */
    {384, 0, 0, 0},                  /* Monasterio: una oleada tras la escena de Rex.      */
    {256, 768, 0, 0},                /* Dojo: kendoka y, al fondo, el jefe.                */
};
static const u8 ZONAS[ESCENARIOS] = {4, 3, 1, 2};
static const u16 ANCHO[ESCENARIOS] = {2048, 1024, 1024, 1024};

/* Por escenario y oleada: hasta 3 tipos (en el orden en que buscan ranura: el hueco 0 solo
   admite normales y rápidos) y, en el cuarto byte, cuántos salen. */
static const u8 OLEADA[ESCENARIOS][OLEADAS_ZONAS][4] = {
    {{N, 0, 0, 1}, {N, M, 0, 2}, {N, M, G, 3}, {N, M, G, 3}},
    {{R, 0, 0, 1}, {R, N, 0, 2}, {R, M, G, 3}, {0, 0, 0, 0}},
    {{N, R, G, 3}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{N, K, 0, 2}, {J, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
};

u16 oleadas_alcanzable(u16 punto)
{
    return (u16)(punto > CAMARA_MAX ? CAMARA_MAX : punto);    /* La cámara no pasa de ahí. */
}

void oleadas_iniciar_escenario(Oleadas *o, u8 escenario)
{
    o->actual = 0;                                   /* Primera zona, todavía lejos.        */
    o->bloqueada = 0;
    o->escenario = escenario;
    o->zonas = ZONAS[escenario];
}

void oleadas_iniciar(Oleadas *o)
{
    oleadas_iniciar_escenario(o, ESCENARIO_ALDEA);
}

u16 oleadas_ancho(u8 escenario)
{
    return ANCHO[escenario];
}

u8 oleadas_terminadas(const Oleadas *o)
{
    return (u8)(o->actual >= o->zonas);
}

const u8 *oleadas_tipos(const Oleadas *o)
{
    return OLEADA[o->escenario][o->actual];
}

u8 oleadas_cuantos(const Oleadas *o)
{
    return OLEADA[o->escenario][o->actual][3];
}

u16 oleadas_limite(const Oleadas *o)
{
    if (o->actual >= o->zonas)
    {
        return oleadas_alcanzable((u16)(ANCHO[o->escenario] - ANCHO_PANTALLA));   /* El final. */
    }
    return oleadas_alcanzable(PUNTOS[o->escenario][o->actual]);
}

u8 oleadas_actualizar(Oleadas *o, u16 camara)
{
    if (!o->bloqueada && o->actual < o->zonas && camara >= oleadas_limite(o))
    {
        o->bloqueada = 1;                            /* Llegó: la cámara se para y salen... */
        return 1;                                    /* ...los enemigos de esta oleada.     */
    }
    return 0;
}

void oleadas_vencer(Oleadas *o)
{
    if (o->bloqueada)
    {
        o->bloqueada = 0;                            /* Libre otra vez («GO →»)...          */
        o->actual++;                                 /* ...hasta el punto de la siguiente.  */
    }
}
